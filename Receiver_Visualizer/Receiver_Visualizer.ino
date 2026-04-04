#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 19
#define OLED_SCL 21
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
HardwareSerial VisSerial(2);

static constexpr uint8_t NUM_BARS = 16;
uint8_t visBars[NUM_BARS] = {0};
uint16_t peakHz = 0;

struct VisPacket {
  uint8_t start;
  uint8_t bars[NUM_BARS];
  uint16_t peak;
  uint8_t checksum;
};

bool readPacket(VisPacket &pkt) {
  static uint8_t buffer[sizeof(VisPacket)];
  static size_t index = 0;

  while (VisSerial.available()) {
    uint8_t b = VisSerial.read();

    if (index == 0 && b != 0xAA) continue;

    buffer[index++] = b;

    if (index == sizeof(VisPacket)) {
      memcpy(&pkt, buffer, sizeof(VisPacket));
      index = 0;

      uint8_t sum = pkt.start;
      for (int i = 0; i < NUM_BARS; i++) sum += pkt.bars[i];
      sum += (uint8_t)(pkt.peak & 0xFF);
      sum += (uint8_t)((pkt.peak >> 8) & 0xFF);

      if (sum == pkt.checksum) {
        return true;
      }
    }
  }

  return false;
}

void drawBars() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Remote FFT ");
  display.print(peakHz);
  display.print("Hz");

  const int topMargin = 10;
  const int graphHeight = 54;
  const int barWidth = SCREEN_WIDTH / NUM_BARS;

  for (int i = 0; i < NUM_BARS; i++) {
    int x = i * barWidth;
    int h = map(visBars[i], 0, 60, 0, graphHeight);
    int y = SCREEN_HEIGHT - h;
    int w = max(1, barWidth - 1);

    display.fillRect(x, y, w, h, SSD1306_WHITE);

    if (h > 2) {
      int peakY = max(topMargin, y - 2);
      display.drawFastHLine(x, peakY, w, SSD1306_WHITE);
    }
  }

  display.display();
}

void setup() {
  Serial.begin(115200);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (true) delay(1000);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Waiting for FFT...");
  display.display();

  // UART2: RX=16, TX=17
  VisSerial.begin(115200, SERIAL_8N1, 16, 17);
}

void loop() {
  VisPacket pkt;
  if (readPacket(pkt)) {
    for (int i = 0; i < NUM_BARS; i++) {
      visBars[i] = pkt.bars[i];
    }
    peakHz = pkt.peak;
    drawBars();
  }
}