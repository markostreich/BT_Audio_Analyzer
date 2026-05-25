#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include "AudioTools.h"
#include <BluetoothA2DPSink.h>
#include <arduinoFFT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "VisPacket.h"

#define ESP_BT_NAME "ESP32 Party Music"

// ===================== OLED =====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 19
#define OLED_SCL 21
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ===================== A2DP =====================
I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

// ===================== FFT ======================
static constexpr uint16_t FFT_SAMPLES = 512;
static constexpr float SAMPLE_RATE = 44100.0f;

double vReal[FFT_SAMPLES];
double vImag[FFT_SAMPLES];
ArduinoFFT<double> FFT(vReal, vImag, FFT_SAMPLES, SAMPLE_RATE);

// ===================== QUEUE ====================
struct AudioBlock {
  size_t sample_count;
  int16_t samples[256];
};

QueueHandle_t audioQueue = nullptr;

// ===================== UART =====================
#define SERIAL_PORT 2
#define TRANSMITTER_PIN 17
#define RECEIVER_PIN 16
#define VIS_UART_BAUD 460800

HardwareSerial VisSerial(SERIAL_PORT);

// ===================== SHARED VIS DATA ==========
volatile uint8_t g_bars[NUM_BARS];
volatile uint8_t g_scope[NUM_SCOPE_SAMPLES];
volatile uint16_t g_peakHz = 0;
volatile bool g_audioSeen = false;

portMUX_TYPE g_barMux = portMUX_INITIALIZER_UNLOCKED;

// ===================== HELPERS ==================
static inline int clampi(int v, int lo, int hi) {
  return (v < lo) ? lo : (v > hi) ? hi : v;
}

void computeBandsFromFFT(double *mag, uint8_t *outBars) {
  const uint16_t bandEdges[NUM_BARS + 1] = {
    1, 2, 3, 5, 7, 10, 14, 20, 28,
    40, 56, 78, 110, 156, 220, 230, 255
  };

  static double autoLevel = 0.01;  // running loudness reference

  double rawBars[NUM_BARS];
  double frameMax = 0.0;

  for (uint8_t b = 0; b < NUM_BARS; b++) {
    uint16_t startBin = bandEdges[b];
    uint16_t endBin = bandEdges[b + 1];

    if (endBin > FFT_SAMPLES / 2 - 1) {
      endBin = FFT_SAMPLES / 2 - 1;
    }

    if (startBin >= endBin) {
      rawBars[b] = 0.0;
      continue;
    }

    double sum = 0.0;

    for (uint16_t k = startBin; k < endBin; k++) {
      sum += mag[k];
    }

    double avg = sum / (endBin - startBin);

    rawBars[b] = avg;

    if (avg > frameMax) {
      frameMax = avg;
    }
  }

  // Ignore near-silence
  if (frameMax < 0.00001) {
    for (uint8_t b = 0; b < NUM_BARS; b++) {
      outBars[b] = 0;
    }
    return;
  }

  // Automatic gain control:
  // rise quickly when music gets louder, fall slowly when music gets quieter
  if (frameMax > autoLevel) {
    autoLevel = autoLevel * 0.70 + frameMax * 0.30;
  } else {
    autoLevel = autoLevel * 0.98 + frameMax * 0.02;
  }

  if (autoLevel < 0.00001) {
    autoLevel = 0.00001;
  }

  for (uint8_t b = 0; b < NUM_BARS; b++) {
    double normalized = rawBars[b] / autoLevel;

    // Optional curve: makes weaker frequencies more visible
    normalized = sqrt(normalized);

    int bar = (int)(normalized * 60.0);

    bar = clampi(bar, 0, 60);
    outBars[b] = (uint8_t)bar;
  }
}

void computeScopeFromWaveform(double *samples, uint8_t *outScope) {
  static double autoLevel = 0.05;
  static float smoothedScope[NUM_SCOPE_SAMPLES] = {0};
  double framePeak = 0.0;

  for (uint16_t i = 0; i < FFT_SAMPLES; i++) {
    const double value = fabs(samples[i]);
    if (value > framePeak) {
      framePeak = value;
    }
  }

  if (framePeak > autoLevel) {
    autoLevel = autoLevel * 0.85 + framePeak * 0.15;
  } else {
    autoLevel = autoLevel * 0.995 + framePeak * 0.005;
  }

  if (autoLevel < 0.01) {
    autoLevel = 0.01;
  }

  for (uint8_t x = 0; x < NUM_SCOPE_SAMPLES; x++) {
    const uint16_t startIndex = ((uint32_t)x * FFT_SAMPLES) / NUM_SCOPE_SAMPLES;
    uint16_t endIndex = ((uint32_t)(x + 1) * FFT_SAMPLES) / NUM_SCOPE_SAMPLES;
    if (endIndex <= startIndex) {
      endIndex = startIndex + 1;
    }
    if (endIndex > FFT_SAMPLES) {
      endIndex = FFT_SAMPLES;
    }

    double selectedSample = 0.0;
    double selectedMagnitude = 0.0;

    for (uint16_t i = startIndex; i < endIndex; i++) {
      const double magnitude = fabs(samples[i]);
      if (magnitude > selectedMagnitude) {
        selectedMagnitude = magnitude;
        selectedSample = samples[i];
      }
    }

    const double normalized = selectedSample / autoLevel;
    const int value = clampi((int)(128.0 + normalized * 128.0), 0, 255);

    if (smoothedScope[x] == 0.0f) {
      smoothedScope[x] = value;
    } else {
      smoothedScope[x] = smoothedScope[x] * 0.65f + value * 0.35f;
    }

    outScope[x] = (uint8_t)clampi((int)smoothedScope[x], 0, 255);
  }
}

void sendVisualizationFrame() {
  VisPacket pkt;

  portENTER_CRITICAL(&g_barMux);
  for (int i = 0; i < NUM_BARS; i++) {
    pkt.bars[i] = g_bars[i];
  }
  for (int i = 0; i < NUM_SCOPE_SAMPLES; i++) {
    pkt.scope[i] = g_scope[i];
  }
  pkt.peak = g_peakHz;
  portEXIT_CRITICAL(&g_barMux);

  visPacketFinalize(pkt);
  VisSerial.write((const uint8_t*)&pkt, sizeof(pkt));
}

void removeDcOffset(double *samples) {
  double mean = 0.0;

  for (uint16_t n = 0; n < FFT_SAMPLES; n++) {
    mean += samples[n];
  }
  mean /= FFT_SAMPLES;

  for (uint16_t n = 0; n < FFT_SAMPLES; n++) {
    samples[n] -= mean;
  }
}

uint16_t computePeakHz(double *mag) {
  double maxMag = 0.0;
  uint16_t maxBin = 1;

  for (uint16_t k = 1; k < FFT_SAMPLES / 2; k++) {
    if (mag[k] > maxMag) {
      maxMag = mag[k];
      maxBin = k;
    }
  }

  return (uint16_t)((maxBin * SAMPLE_RATE) / FFT_SAMPLES);
}

void smoothBars(const uint8_t *newBars, float *smoothedBars) {
  for (uint8_t b = 0; b < NUM_BARS; b++) {
    const float target = newBars[b];
    smoothedBars[b] = smoothedBars[b] * 0.85f + target * 0.15f;
  }
}

void publishVisualizationData(uint16_t peakHz, const float *smoothedBars, const uint8_t *scope) {
  portENTER_CRITICAL(&g_barMux);

  g_peakHz = peakHz;
  for (uint8_t b = 0; b < NUM_BARS; b++) {
    g_bars[b] = (uint8_t)clampi((int)smoothedBars[b], 0, 60);
  }
  for (uint8_t x = 0; x < NUM_SCOPE_SAMPLES; x++) {
    g_scope[x] = scope[x];
  }

  portEXIT_CRITICAL(&g_barMux);
}

void processFftFrame(float *smoothedBars) {
  uint8_t newScope[NUM_SCOPE_SAMPLES];
  uint8_t newBars[NUM_BARS];

  removeDcOffset(vReal);
  computeScopeFromWaveform(vReal, newScope);

  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();

  const uint16_t peakHz = computePeakHz(vReal);
  computeBandsFromFFT(vReal, newBars);
  smoothBars(newBars, smoothedBars);
  publishVisualizationData(peakHz, smoothedBars, newScope);
  sendVisualizationFrame();
}

// ===================== CALLBACK =================
void read_data_stream(const uint8_t *data, uint32_t length) {
  if (!data || length == 0 || !audioQueue) return;

  g_audioSeen = true;

  const int16_t *pcm = reinterpret_cast<const int16_t *>(data);
  size_t total_samples = length / sizeof(int16_t);

  while (total_samples > 0) {
    AudioBlock block{};
    block.sample_count = min(total_samples, (size_t)256);
    memcpy(block.samples, pcm, block.sample_count * sizeof(int16_t));

    xQueueSend(audioQueue, &block, 0);

    pcm += block.sample_count;
    total_samples -= block.sample_count;
  }
}

// ===================== FFT TASK =================
void fftTask(void *param) {
  AudioBlock block;
  size_t fftIndex = 0;
  float smoothedBars[NUM_BARS] = {0};

  while (true) {
    if (xQueueReceive(audioQueue, &block, portMAX_DELAY) != pdTRUE) {
      continue;
    }

    for (size_t i = 0; i + 1 < block.sample_count; i += 2) {
      const int32_t left = block.samples[i];
      const int32_t right = block.samples[i + 1];
      const int16_t mono = (left + right) / 2;

      vReal[fftIndex] = (double)mono / 32768.0;
      vImag[fftIndex] = 0.0;
      fftIndex++;

      if (fftIndex >= FFT_SAMPLES) {
        processFftFrame(smoothedBars);
        fftIndex = 0;
      }
    }
  }
}

// ===================== DISPLAY TASK =============
void displayTask(void *param) {
  uint8_t localBars[NUM_BARS];
  uint16_t localPeakHz = 0;

  const int topMargin = 10;
  const int graphHeight = 54;
  const int barWidth = SCREEN_WIDTH / NUM_BARS;

  while (true) {
    portENTER_CRITICAL(&g_barMux);
    for (uint8_t i = 0; i < NUM_BARS; i++) localBars[i] = g_bars[i];
    localPeakHz = g_peakHz;
    portEXIT_CRITICAL(&g_barMux);

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    if (g_audioSeen) {
      display.print("BT Music ");
      display.print(localPeakHz);
      display.print("Hz");
    } else {
      display.print("Waiting for audio...");
    }

    for (uint8_t i = 0; i < NUM_BARS; i++) {
      int x = i * barWidth;
      int h = map(localBars[i], 0, 60, 0, graphHeight);
      int y = SCREEN_HEIGHT - h;
      int w = max(1, barWidth - 1);

      display.fillRect(x, y, w, h, SSD1306_WHITE);

      if (h > 2) {
        int peakY = max(topMargin, y - 2);
        display.drawFastHLine(x, peakY, w, SSD1306_WHITE);
      }
    }

    display.display();
    vTaskDelay(pdMS_TO_TICKS(33));
  }
}

void setup() {
  Serial.begin(115200);

  memset((void*)g_bars, 0, sizeof(g_bars));
  memset((void*)g_scope, 128, sizeof(g_scope));

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 init failed");
    while (true) delay(1000);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();

  audioQueue = xQueueCreate(12, sizeof(AudioBlock));
  if (!audioQueue) {
    Serial.println("Failed to create audio queue");
    while (true) delay(1000);
  }
  
  VisSerial.begin(VIS_UART_BAUD, SERIAL_8N1, RECEIVER_PIN, TRANSMITTER_PIN);

  xTaskCreatePinnedToCore(
    fftTask,
    "FFT Task",
    8192,
    nullptr,
    1,
    nullptr,
    1
  );

  xTaskCreatePinnedToCore(
    displayTask,
    "Display Task",
    4096,
    nullptr,
    1,
    nullptr,
    1
  );

  auto cfg = i2s.defaultConfig();
  cfg.pin_bck = 14;
  cfg.pin_ws = 15;
  cfg.pin_data = 22;
  i2s.begin(cfg);

  a2dp_sink.set_stream_reader(read_data_stream, true);
  a2dp_sink.start(ESP_BT_NAME);
}

void loop() {
  delay(1000);
}
