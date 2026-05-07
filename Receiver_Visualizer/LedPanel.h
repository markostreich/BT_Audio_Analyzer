#pragma once

#include "JsonAdapter.h"
#include "LedPanelObjects.h"
#include "Buttons.h"
#include "Graphics.h"
#include "VisPacket.h"

long firstPixelHue = 0;
constexpr long maxPixelHue = 5 * 65536;

boolean notInitializedCassette = true;
LedPanelObject cassette;
LedPanelObject cassetteWheelA;
LedPanelObject cassetteWheelB;
float angleCassetteWheel = 0.0;
float angleCassetteWheelB = 0.0;

inline void ledPanelDrawCassette() {
  if (notInitializedCassette) {
    cassette = parseLedPanelJson(cassetteJson);
    cassetteWheelA = parseLedPanelJson(cassetteWheelAJson);
    cassetteWheelB = parseLedPanelJson(cassetteWheelBJson);
    notInitializedCassette = false;
  }
  pixels.clear();
  drawRotatedObject(&cassetteWheelA, angleCassetteWheel);
  drawRotatedObject(&cassetteWheelB, angleCassetteWheelB);
  drawObject(&cassette);
  angleCassetteWheel -= 18.0;
  angleCassetteWheelB -= 17.0;
  pixels.setBrightness(30);
  pixels.show();
  delay(1);
}

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
  pixels.setBrightness(brightness);
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
    int x_pos = i * barWidth + 1;
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
  pixels.setBrightness(brightness);
  pixels.show();
  firstPixelHue += 256;
}

inline void ledPanelDrawBarsRainbowVertical(const VisPacket &pkt) {
  pixels.clear();

  const int graphHeight = size_y + 1;
  const int barWidth = 2;
  const int numBars = NUM_BARS < size_x ? NUM_BARS : size_x;

  for (int i = 0; i < numBars; i++) {
    const int x_pos = i * barWidth + 1;
    const int h = map(pkt.bars[i], 0, 60, 0, graphHeight);

    for (int y_pos = 0; y_pos < h; ++y_pos) {
      if (y_pos < h - 1) {
        const int pixelHue = firstPixelHue + (x_pos * 65536L / size_x);
        drawPixel(x_pos, y_pos, pixels.gamma32(pixels.ColorHSV(pixelHue)));
      } else {
        drawPixel(x_pos, y_pos, 50, 50, 50);
      }
    }
  }
  pixels.setBrightness(brightness);
  pixels.show();
  firstPixelHue += 256;
}

inline void ledPanelDrawBarsRainbowVerticalMiddle(const VisPacket &pkt) {
  pixels.clear();

  const int graphHeight = size_y + 1;
  const int barWidth = 2;

  const int maxBarsOnPanel = size_x / barWidth;
  const int numBars = NUM_BARS < maxBarsOnPanel ? NUM_BARS : maxBarsOnPanel;

  const int centerX = size_x / 2;

  for (int i = 0; i < numBars; i++) {
    const int step = (i + 1) / 2;
    const int direction = (i % 2 == 0) ? 1 : -1;

    const int x_pos = centerX + direction * step * barWidth;

    if (x_pos < 0 || x_pos >= size_x) {
      continue;
    }

    const int h = map(pkt.bars[i], 0, 60, 0, graphHeight);

    for (int y_pos = 0; y_pos < h; ++y_pos) {
      if (y_pos < h - 1) {
        const int pixelHue = firstPixelHue + (x_pos * 65536L / size_x);
        drawPixel(x_pos, y_pos, pixels.gamma32(pixels.ColorHSV(pixelHue)));
      } else {
        drawPixel(x_pos, y_pos, 50, 50, 50);
      }
    }
  }

  pixels.setBrightness(brightness);
  pixels.show();

  firstPixelHue += 256;
}

inline void ledPanelDrawBarsRainbowVerticalMiddleMirrored(const VisPacket &pkt) {
  pixels.clear();

  const int graphHeight = size_y + 1;
  const int barWidth = 2;

  const int centerX = size_x / 2;
  const int barsPerSide = min((int)NUM_BARS, (centerX / barWidth) + 1);

  for (int i = 0; i < barsPerSide; i++) {
    const int offset = i * barWidth;

    const int x_left = centerX - offset;
    const int x_right = centerX + offset;

    const int h = map(pkt.bars[i], 0, 60, 0, graphHeight);

    const int pixelHue = firstPixelHue + (i * 65536L / barsPerSide);
    const uint32_t color = pixels.gamma32(pixels.ColorHSV(pixelHue));

    for (int y_pos = 0; y_pos < h; ++y_pos) {
      if (y_pos < h - 1) {
        if (x_left >= 0 && x_left < size_x) {
          drawPixel(x_left, y_pos, color);
        }

        if (x_right >= 0 && x_right < size_x && x_right != x_left) {
          drawPixel(x_right, y_pos, color);
        }
      } else {
        if (x_left >= 0 && x_left < size_x) {
          drawPixel(x_left, y_pos, 50, 50, 50);
        }

        if (x_right >= 0 && x_right < size_x && x_right != x_left) {
          drawPixel(x_right, y_pos, 50, 50, 50);
        }
      }
    }
  }

  pixels.setBrightness(brightness);
  pixels.show();

  firstPixelHue += 256;
}

inline void ledPanelDrawBarsRainbowMiddle(const VisPacket &pkt) {
  pixels.clear();

  const int graphHeight = size_y;
  const int barWidth = 1;
  const int numBars = NUM_BARS < size_x ? NUM_BARS : size_x;

  for (int i = 0; i < numBars; i++) {
    const int x_pos = i * barWidth;
    const int h = map(pkt.bars[i], 0, 60, 0, graphHeight);

    for (int y_pos = 0; y_pos < h; ++y_pos) {
      const int pixelHue = firstPixelHue + (x_pos * 65536L / size_x);
      drawPixel(x_pos + 15, y_pos, pixels.gamma32(pixels.ColorHSV(pixelHue)));
      drawPixel(14 - x_pos, y_pos, pixels.gamma32(pixels.ColorHSV(pixelHue)));
    }
  }
  pixels.setBrightness(brightness);
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
  pixels.setBrightness(brightness);
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
  pixels.setBrightness(brightness);
  pixels.show();
}

inline uint8_t clampByte(int v) {
  if (v < 0) return 0;
  if (v > 255) return 255;
  return (uint8_t)v;
}

inline void ledPanelDrawAudioBlob(const VisPacket &pkt) {
  pixels.clear();

  /*
  for (int8_t y = 0; y < size_y; ++y)
    for (int8_t x = 0; x < size_x; ++x)
      drawPixel(x, y, 2, 0, 2);
  */

  // ---------- audio feature extraction ----------
  int bass = 0;
  int mid = 0;
  int high = 0;
  int total = 0;

  // Low bands
  for (int i = 0; i < 4; i++) {
    bass += pkt.bars[i];
  }

  // Mid bands
  for (int i = 4; i < 10; i++) {
    mid += pkt.bars[i];
  }

  // High bands
  for (int i = 10; i < NUM_BARS; i++) {
    high += pkt.bars[i];
  }

  for (int i = 0; i < NUM_BARS; i++) {
    total += pkt.bars[i];
  }

  const int bassLevel = constrain(bass / 4, 0, 60);
  const int midLevel = constrain(mid / 6, 0, 60);
  const int highLevel = constrain(high / 6, 0, 60);
  const int energy = constrain(total / NUM_BARS, 0, 60);

  // ---------- smooth blob parameters ----------
  static float smoothCx = size_x / 2.0f;
  static float smoothCy = size_y / 2.0f;
  static float smoothRadius = 4.0f;
  static float smoothIntensity = 0.0f;

  float centerX = size_x / 2.0f;
  float centerY = size_y / 2.0f;

  float cx = centerX;
  float cy = centerY;

  // Smaller movement zone around the center
  float moveX = map(highLevel - bassLevel, -60, 60, -size_x / 8, size_x / 8);
  float moveY = map(midLevel - bassLevel, -60, 60, size_y / 10, -size_y / 10);

  cx += moveX;
  cy += moveY;

  // Pull position back toward center
  cx = centerX * 0.50f + cx * 0.50f;
  cy = centerY * 0.50f + cy * 0.50f;

  // Bass makes the blob bigger
  float radius = 2.5f + (bassLevel / 60.0f) * 9.0f;

  // Overall energy makes the blob brighter
  float intensity = energy / 60.0f;

  // Smooth movement and pulsing
  smoothCx = smoothCx * 0.85f + cx * 0.15f;
  smoothCy = smoothCy * 0.85f + cy * 0.15f;
  smoothRadius = smoothRadius * 0.80f + radius * 0.20f;
  smoothIntensity = smoothIntensity * 0.75f + intensity * 0.25f;

  // ---------- render blob ----------
  for (int y = 0; y < size_y; y++) {
    for (int x = 0; x < size_x; x++) {
      const float dx = x - smoothCx;
      const float dy = y - smoothCy;
      const float dist = sqrtf(dx * dx + dy * dy);

      float falloff = 1.0f - (dist / smoothRadius);
      if (falloff < 0.0f) falloff = 0.0f;

      // softer center-to-edge fade
      falloff = falloff * falloff;

      const int brightness = (int)(255.0f * smoothIntensity * falloff);

      if (brightness <= 1) {
        continue;
      }

      // Color reacts to spectrum:
      // bass -> blue/cyan
      // mid  -> green
      // high -> red/pink
      uint8_t r = clampByte((highLevel * brightness) / 60);
      uint8_t g = clampByte((midLevel * brightness) / 60);
      uint8_t b = clampByte(((bassLevel + 20) * brightness) / 80);

      drawPixel(x, y, r, g, b);
    }
  }

  pixels.setBrightness(brightness);
  pixels.show();
}

inline void ledPanelDrawDiscoBall(const VisPacket &pkt) {
  pixels.clear();

  int total = 0;
  int bass = 0;

  for (int i = 0; i < NUM_BARS; i++) {
    total += pkt.bars[i];
  }

  for (int i = 0; i < 4; i++) {
    bass += pkt.bars[i];
  }

  const int energy = constrain(total / NUM_BARS, 0, 60);
  const int bassLevel = constrain(bass / 4, 0, 60);

  static float angle = 0.0f;
  angle += 0.06f + (bassLevel / 60.0f) * 0.06f;

  if (angle > TWO_PI) {
    angle -= TWO_PI;
  }

  const float cx = (size_x - 1) / 2.0f;
  const float cy = (size_y - 1) / 2.0f;

  // Important: use width for radius so it appears round on 20x30.
  const float radius = size_y * 0.46f;

  const float pulse = 0.55f + (energy / 60.0f) * 0.45f;

  for (int y = 0; y < size_y; y++) {
    for (int x = 0; x < size_x; x++) {
      const float dx = x - cx;
      const float dy = y - cy;
      const float dist = sqrtf(dx * dx + dy * dy);

      if (dist > radius) {
        continue;
      }

      const float nx = dx / radius;
      const float ny = dy / radius;

      // Strong round ball mask
      float sphere = 1.0f - (dist / radius);
      sphere = constrain(sphere, 0.0f, 1.0f);
      sphere = sphere * sphere;
      sphere = 0.08f + sphere * 0.92f;

      const float rotatedX = nx * cosf(angle) - ny * sinf(angle);
      const float rotatedY = nx * sinf(angle) + ny * cosf(angle);

      // Smaller tile grid helps it read as a ball at low resolution
      const int tileX = floor((rotatedX + 1.0f) * 4.0f);
      const int tileY = floor((rotatedY + 1.0f) * 4.0f);

      bool tileSparkle = ((tileX + tileY) % 2 == 0);

      // Curved highlight, strongest near center
      float highlight = 1.0f - fabsf(rotatedX - 0.25f);
      highlight = constrain(highlight, 0.0f, 1.0f);
      highlight = highlight * highlight * sphere;

      int base = tileSparkle ? 55 : 28;
      int sparkle = (int)(highlight * 220.0f);

      int brightness = (int)((base + sparkle) * sphere * pulse);
      brightness = constrain(brightness, 0, 255);

      // Silver-blue disco color
      uint8_t r = brightness;
      uint8_t g = constrain(brightness + 15, 0, 255);
      uint8_t b = constrain(brightness + 35, 0, 255);

      drawPixel(x, y, r, g, b);
    }
  }

  pixels.setBrightness(brightness);
  pixels.show();
}

inline void ledPanelDrawTwoAudioBlobs(const VisPacket &pkt) {
  pixels.clear();

  // faint blue background
  for (int8_t y = 0; y < size_y; ++y) {
    for (int8_t x = 0; x < size_x; ++x) {
      drawPixel(x, y, 0, 0, 2);
    }
  }

  // ---------- audio feature extraction ----------
  int bass = 0;
  int mid = 0;
  int high = 0;
  int total = 0;

  for (int i = 0; i < 4; i++) {
    bass += pkt.bars[i];
  }

  for (int i = 4; i < 10; i++) {
    mid += pkt.bars[i];
  }

  for (int i = 10; i < NUM_BARS; i++) {
    high += pkt.bars[i];
  }

  for (int i = 0; i < NUM_BARS; i++) {
    total += pkt.bars[i];
  }

  const int bassLevel = constrain(bass / 4, 0, 60);
  const int midLevel = constrain(mid / 6, 0, 60);
  const int highLevel = constrain(high / 6, 0, 60);
  const int energy = constrain(total / NUM_BARS, 0, 60);

  // ---------- animation state ----------
  static float t = 0.0f;

  static float smoothCx1 = size_x * 0.38f;
  static float smoothCy1 = size_y * 0.50f;
  static float smoothCx2 = size_x * 0.62f;
  static float smoothCy2 = size_y * 0.50f;

  static float smoothRadius1 = 4.5f;
  static float smoothRadius2 = 5.0f;
  static float smoothIntensity1 = 0.0f;
  static float smoothIntensity2 = 0.0f;

  const float centerX = (size_x - 1) / 2.0f;
  const float centerY = (size_y - 1) / 2.0f;

  // ---------- audio-driven sizes ----------
  float radius1 = 3.2f + (bassLevel / 60.0f) * 5.0f;
  float intensity1 = 0.28f + (energy / 60.0f) * 0.72f;

  float magentaDrive = (midLevel * 0.45f + highLevel * 0.55f);
  float magentaNorm = constrain(magentaDrive / 60.0f, 0.0f, 1.0f);
  float magentaBoost = powf(magentaNorm, 0.65f);

  float radius2 = 4.5f + magentaBoost * 5.5f;
  float intensity2 = 0.38f + magentaBoost * 0.82f;

  const float maxRadius = min(size_x, size_y) * 0.42f;
  radius1 = constrain(radius1, 2.8f, maxRadius);
  radius2 = constrain(radius2, 3.8f, maxRadius);

  // ---------- slower lava-lamp motion ----------
  t += 0.035f + (energy / 60.0f) * 0.010f;

  // multi-wave organic motion
  float wanderX1 =
    sinf(t * 0.80f) * (size_x * 0.18f) + sinf(t * 0.31f + 1.4f) * (size_x * 0.10f) + cosf(t * 0.17f + 0.7f) * (size_x * 0.06f);

  float wanderY1 =
    cosf(t * 0.62f + 0.8f) * (size_y * 0.16f) + sinf(t * 0.27f + 2.1f) * (size_y * 0.08f) + cosf(t * 0.19f + 0.4f) * (size_y * 0.05f);

  float wanderX2 =
    cosf(t * 0.74f + 2.2f) * (size_x * 0.18f) + sinf(t * 0.29f + 0.2f) * (size_x * 0.11f) + cosf(t * 0.15f + 2.5f) * (size_x * 0.05f);

  float wanderY2 =
    sinf(t * 0.58f + 1.7f) * (size_y * 0.16f) + cosf(t * 0.24f + 0.6f) * (size_y * 0.08f) + sinf(t * 0.18f + 1.1f) * (size_y * 0.05f);

  // slight audio offsets
  wanderX1 += ((highLevel - bassLevel) / 60.0f) * 1.0f;
  wanderY1 += ((midLevel - bassLevel) / 60.0f) * 0.8f;

  wanderX2 -= ((highLevel - bassLevel) / 60.0f) * 1.0f;
  wanderY2 -= ((midLevel - bassLevel) / 60.0f) * 0.8f;

  float targetCx1 = centerX + wanderX1;
  float targetCy1 = centerY + wanderY1;
  float targetCx2 = centerX + wanderX2;
  float targetCy2 = centerY + wanderY2;

  // ---------- soft attraction / repulsion ----------
  // makes them feel like drifting blobs rather than separate dots
  float dxC = targetCx2 - targetCx1;
  float dyC = targetCy2 - targetCy1;
  float distC = sqrtf(dxC * dxC + dyC * dyC);

  if (distC > 0.001f) {
    float nx = dxC / distC;
    float ny = dyC / distC;

    // preferred distance between blobs
    float preferred = (radius1 + radius2) * 0.85f;

    if (distC < preferred) {
      // repel a bit if too close
      float force = (preferred - distC) * 0.08f;
      targetCx1 -= nx * force;
      targetCy1 -= ny * force;
      targetCx2 += nx * force;
      targetCy2 += ny * force;
    } else if (distC > preferred + 3.0f) {
      // attract a bit if too far away
      float force = (distC - preferred - 3.0f) * 0.015f;
      targetCx1 += nx * force;
      targetCy1 += ny * force;
      targetCx2 -= nx * force;
      targetCy2 -= ny * force;
    }
  }

  // ---------- keep centers inside borders ----------
  const float margin1 = radius1 + 0.5f;
  const float margin2 = radius2 + 0.5f;

  targetCx1 = constrain(targetCx1, margin1, (size_x - 1) - margin1);
  targetCy1 = constrain(targetCy1, margin1, (size_y - 1) - margin1);

  targetCx2 = constrain(targetCx2, margin2, (size_x - 1) - margin2);
  targetCy2 = constrain(targetCy2, margin2, (size_y - 1) - margin2);

  // ---------- stronger smoothing ----------
  smoothCx1 = smoothCx1 * 0.85f + targetCx1 * 0.15f;
  smoothCy1 = smoothCy1 * 0.85f + targetCy1 * 0.15f;
  smoothCx2 = smoothCx2 * 0.85f + targetCx2 * 0.15f;
  smoothCy2 = smoothCy2 * 0.85f + targetCy2 * 0.15f;

  smoothRadius1 = smoothRadius1 * 0.90f + radius1 * 0.10f;
  smoothRadius2 = smoothRadius2 * 0.90f + radius2 * 0.10f;

  smoothIntensity1 = smoothIntensity1 * 0.84f + intensity1 * 0.16f;
  smoothIntensity2 = smoothIntensity2 * 0.84f + intensity2 * 0.16f;

  // clamp smoothed positions too
  smoothCx1 = constrain(smoothCx1, margin1, (size_x - 1) - margin1);
  smoothCy1 = constrain(smoothCy1, margin1, (size_y - 1) - margin1);

  smoothCx2 = constrain(smoothCx2, margin2, (size_x - 1) - margin2);
  smoothCy2 = constrain(smoothCy2, margin2, (size_y - 1) - margin2);

  // ---------- render ----------
  for (int y = 0; y < size_y; y++) {
    for (int x = 0; x < size_x; x++) {
      float dx1 = x - smoothCx1;
      float dy1 = y - smoothCy1;
      float dist1 = sqrtf(dx1 * dx1 + dy1 * dy1);

      float dx2 = x - smoothCx2;
      float dy2 = y - smoothCy2;
      float dist2 = sqrtf(dx2 * dx2 + dy2 * dy2);

      float falloff1 = 1.0f - (dist1 / smoothRadius1);
      float falloff2 = 1.0f - (dist2 / smoothRadius2);

      if (falloff1 < 0.0f) falloff1 = 0.0f;
      if (falloff2 < 0.0f) falloff2 = 0.0f;

      // softer, lava-lamp-like fade
      falloff1 = falloff1 * falloff1;
      falloff2 = falloff2 * falloff2;

      int brightness1 = (int)(255.0f * smoothIntensity1 * falloff1);
      int brightness2 = (int)(255.0f * smoothIntensity2 * falloff2);

      if (brightness1 <= 1 && brightness2 <= 1) {
        continue;
      }

      // blob 1: cyan / blue
      int r = 0;
      int g = (midLevel * brightness1) / 75;
      int b = ((bassLevel + 35) * brightness1) / 65;

      // blob 2: magenta / purple
      r += ((highLevel + 30) * brightness2) / 60;
      g += (midLevel * brightness2) / 180;
      b += ((highLevel + 40) * brightness2) / 70;

      // overlap -> brighter / whiter
      int overlap = min(brightness1, brightness2);
      if (overlap > 0) {
        r += overlap / 3;
        g += overlap / 5;
        b += overlap / 3;
      }

      drawPixel(x, y, clampByte(r), clampByte(g), clampByte(b));
    }
  }

  pixels.setBrightness(brightness);
  pixels.show();
}
