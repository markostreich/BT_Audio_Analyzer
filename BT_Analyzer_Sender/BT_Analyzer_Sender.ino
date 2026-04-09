#include <Arduino.h>
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

HardwareSerial VisSerial(SERIAL_PORT);

// ===================== SHARED VIS DATA ==========
volatile uint8_t g_bars[NUM_BARS];
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
    40, 56, 78, 110, 156, 220, 312, 255
  };

  for (uint8_t b = 0; b < NUM_BARS; b++) {
    uint16_t startBin = bandEdges[b];
    uint16_t endBin = bandEdges[b + 1];

    if (endBin > FFT_SAMPLES / 2 - 1) endBin = FFT_SAMPLES / 2 - 1;
    if (startBin >= endBin) {
      outBars[b] = 0;
      continue;
    }

    double sum = 0.0;
    for (uint16_t k = startBin; k < endBin; k++) {
      sum += mag[k];
    }

    double avg = sum / (endBin - startBin);
    double scaled = avg * 180.0;
    int bar = (int)scaled;
    bar = clampi(bar, 0, 60);
    outBars[b] = (uint8_t)bar;
  }
}

void sendVisualizationFrame() {
  VisPacket pkt;

  portENTER_CRITICAL(&g_barMux);
  for (int i = 0; i < NUM_BARS; i++) {
    pkt.bars[i] = g_bars[i];
  }
  pkt.peak = g_peakHz;
  portEXIT_CRITICAL(&g_barMux);

  visPacketFinalize(pkt);
  VisSerial.write((const uint8_t*)&pkt, sizeof(pkt));
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
  float smoothBars[NUM_BARS] = {0};

  while (true) {
    if (xQueueReceive(audioQueue, &block, portMAX_DELAY) == pdTRUE) {
      for (size_t i = 0; i + 1 < block.sample_count; i += 2) {
        int32_t left = block.samples[i];
        int32_t right = block.samples[i + 1];
        int16_t mono = (left + right) / 2;

        vReal[fftIndex] = (double)mono / 32768.0;
        vImag[fftIndex] = 0.0;
        fftIndex++;

        if (fftIndex >= FFT_SAMPLES) {
          double mean = 0.0;
          for (uint16_t n = 0; n < FFT_SAMPLES; n++) mean += vReal[n];
          mean /= FFT_SAMPLES;
          for (uint16_t n = 0; n < FFT_SAMPLES; n++) vReal[n] -= mean;

          FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
          FFT.compute(FFTDirection::Forward);
          FFT.complexToMagnitude();

          double maxMag = 0.0;
          uint16_t maxBin = 1;
          for (uint16_t k = 1; k < FFT_SAMPLES / 2; k++) {
            if (vReal[k] > maxMag) {
              maxMag = vReal[k];
              maxBin = k;
            }
          }

          uint16_t peakHz = (uint16_t)((maxBin * SAMPLE_RATE) / FFT_SAMPLES);

          uint8_t newBars[NUM_BARS];
          computeBandsFromFFT(vReal, newBars);

          for (uint8_t b = 0; b < NUM_BARS; b++) {
            float target = newBars[b];
            smoothBars[b] = smoothBars[b] * 0.70f + target * 0.30f;
          }

          portENTER_CRITICAL(&g_barMux);
          g_peakHz = peakHz;
          for (uint8_t b = 0; b < NUM_BARS; b++) {
            g_bars[b] = (uint8_t)clampi((int)smoothBars[b], 0, 60);
          }
          portEXIT_CRITICAL(&g_barMux);

          fftIndex = 0;
        }
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
    sendVisualizationFrame();
    vTaskDelay(pdMS_TO_TICKS(33));
  }
}

void setup() {
  Serial.begin(115200);

  memset((void*)g_bars, 0, sizeof(g_bars));

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
  
  VisSerial.begin(115200, SERIAL_8N1, RECEIVER_PIN, TRANSMITTER_PIN);

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