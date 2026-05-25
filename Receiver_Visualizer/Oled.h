#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Buttons.h"
#include "VisPacket.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 19
#define OLED_SCL 21
#define OLED_RESET -1
#define OLED_ADDR 0x3C

inline Adafruit_SSD1306 oledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

inline bool oledBegin() {
  Wire.begin(OLED_SDA, OLED_SCL);

  if (!oledDisplay.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    return false;
  }

  oledDisplay.clearDisplay();
  oledDisplay.display();
  return true;
}

inline void oledShowStartup() {
  oledDisplay.clearDisplay();
  oledDisplay.setTextSize(1);
  oledDisplay.setTextColor(SSD1306_WHITE);
  oledDisplay.setCursor(0, 0);
  oledDisplay.println("Waiting for FFT...");
  oledDisplay.display();
}

inline void oledShowError(const char *msg) {
  oledDisplay.clearDisplay();
  oledDisplay.setTextSize(1);
  oledDisplay.setTextColor(SSD1306_WHITE);
  oledDisplay.setCursor(0, 0);
  oledDisplay.println(msg);
  oledDisplay.display();
}

inline void oledDrawBars(const VisPacket &pkt) {
  oledDisplay.clearDisplay();
  oledDisplay.setTextSize(1);
  oledDisplay.setTextColor(SSD1306_WHITE);
  oledDisplay.setCursor(0, 0);
  oledDisplay.print("FFT ");
  oledDisplay.print(pkt.peak);
  oledDisplay.print("Hz B:");
  oledDisplay.print(brightness);

  const int topMargin = 10;
  const int graphHeight = 54;
  const int barWidth = SCREEN_WIDTH / NUM_BARS;

  for (int i = 0; i < NUM_BARS; i++) {
    int x = i * barWidth;
    int h = map(pkt.bars[i], 0, 60, 0, graphHeight);
    int y = SCREEN_HEIGHT - h;
    int w = max(1, barWidth - 1);

    oledDisplay.fillRect(x, y, w, h, SSD1306_WHITE);

      if (h > 2) {
        int peakY = max(topMargin, y - 2);
        oledDisplay.drawFastHLine(x, peakY, w, SSD1306_WHITE);
      }
  }

  oledDisplay.display();
}
