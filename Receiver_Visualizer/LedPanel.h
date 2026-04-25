#pragma once

#include "Graphics.h"
#include "VisPacket.h"

uint16_t localBrightness = 20;

long firstPixelHue = 0;
constexpr long maxPixelHue = 5 * 65536;

inline void ledPanelDrawBars(const VisPacket &pkt) {
  pixels.clear();
  const int8_t r = 0;
  const int8_t g = 100;
  const int8_t b = 100;

  const int graphHeight = size_y + 1;
  const int barWidth = 2;
  const int numBars = NUM_BARS < size_x ? NUM_BARS : size_x;

  for (int i = 0; i < numBars; i++) {
    int x_pos = i * barWidth;
    int h = map(pkt.bars[i], 0, 60, 0, graphHeight);
    if (x_pos == 0) h *= 0.95;

    for (int y_pos = 0; y_pos < h; ++y_pos) {
      if (y_pos < h - 1)
        drawPixel(x_pos, y_pos, r, g, b);
      else
        drawPixel(x_pos, y_pos, 100, g, 30);
    }
  }
  pixels.setBrightness(localBrightness);
  pixels.show();
}

inline void ledPanelDrawBarsRainbow(const VisPacket &pkt) {
  pixels.clear();
  const int8_t r = 0;
  const int8_t g = 100;
  const int8_t b = 100;

  const int graphHeight = size_y + 1;
  const int barWidth = 2;
  const int numBars = NUM_BARS < size_x ? NUM_BARS : size_x;

  for (int i = 0; i < numBars; i++) {
    int x_pos = i * barWidth;
    int h = map(pkt.bars[i], 0, 60, 0, graphHeight);

    for (int y_pos = 0; y_pos < h; ++y_pos) {
      if (y_pos < h - 1) {
        int pixelHue = firstPixelHue + (y_pos * 65536L / size_y);
        drawPixel(x_pos, y_pos, pixels.gamma32(pixels.ColorHSV(pixelHue)));
      } else {
        drawPixel(x_pos, y_pos, 255, 255, 255);
      }
    }
  }
  pixels.setBrightness(localBrightness);
  pixels.show();
  firstPixelHue += 256;
}

inline void ledPanelDrawBarsRainbowVertical(const VisPacket &pkt) {
  pixels.clear();
  const int8_t r = 0;
  const int8_t g = 100;
  const int8_t b = 100;

  const int graphHeight = size_y + 1;
  const int barWidth = 2;
  const int numBars = NUM_BARS < size_x ? NUM_BARS : size_x;

  for (int i = 0; i < numBars; i++) {
    int x_pos = i * barWidth;
    int h = map(pkt.bars[i], 0, 60, 0, graphHeight);

    for (int y_pos = 0; y_pos < h; ++y_pos) {
      if (y_pos < h - 1) {
        int pixelHue = firstPixelHue + (x_pos * 65536L / size_x);
        drawPixel(x_pos, y_pos, pixels.gamma32(pixels.ColorHSV(pixelHue)));
      } else {
        drawPixel(x_pos, y_pos, 50, 50, 50);
      }
    }
  }
  pixels.setBrightness(localBrightness);
  pixels.show();
  firstPixelHue += 256;
}

inline void ledPanelDrawBarsMiddle(const VisPacket &pkt) {
  pixels.clear();
  const int8_t r = 0;
  const int8_t g = 100;
  const int8_t b = 100;

  const int graphHeight = size_y;
  const int barWidth = 1;
  const int numBars = NUM_BARS < size_x ? NUM_BARS : size_x;

  for (int i = 0; i < numBars; i++) {
    int x_pos = i * barWidth;
    int h = map(pkt.bars[i], 0, 60, 0, graphHeight);

    for (int y_pos = 0; y_pos < h; ++y_pos) {
      drawPixel(x_pos + 15, y_pos, r, g, b);
      drawPixel(14 - x_pos, y_pos, r, g, b);
      //drawPixel(x_pos + 1, y_pos, r, g, b);
    }
  }
  pixels.setBrightness(localBrightness);
  pixels.show();
}

inline void ledPanelDrawBarsVertical(const VisPacket &pkt) {
  pixels.clear();
  const int8_t r = 0;
  const int8_t g = 100;
  const int8_t b = 100;

  const int graphHeight = size_x;
  const int barWidth = 2;
  const int numBars = NUM_BARS < size_y ? NUM_BARS : size_y;

  for (int i = numBars - 1; i > 0; --i) {
    int y_pos = i;
    int w = map(pkt.bars[i], 0, 60, 0, graphHeight);

    for (int x_pos = 0; x_pos < w; ++x_pos) {
      drawPixel(x_pos, y_pos, r, g, b);
      //drawPixel(x_pos + 1, y_pos, r, g, b);
    }
  }
  pixels.setBrightness(localBrightness);
  pixels.show();
}
