#pragma once

#include "Buttons.h"
#include "Graphics.h"
#include "VisPacket.h"

inline uint8_t stripClampByte(int value) {
  if (value < 0) return 0;
  if (value > 255) return 255;
  return (uint8_t)value;
}

inline uint8_t stripPacketEnergy(const VisPacket &pkt) {
  int total = 0;
  for (uint8_t i = 0; i < NUM_BARS; i++) {
    total += pkt.bars[i];
  }
  return constrain(total / NUM_BARS, 0, 60);
}

inline uint8_t stripBassLevel(const VisPacket &pkt) {
  int bass = 0;
  for (uint8_t i = 0; i < 4; i++) {
    bass += pkt.bars[i];
  }
  return constrain(bass / 4, 0, 60);
}

inline void setStripBrightness() {
  rightStrip.setBrightness(stripBrightness);
  leftStrip.setBrightness(stripBrightness);
}

inline void showStrips() {
  rightStrip.show();
  leftStrip.show();
}

inline void clearStrips() {
  rightStrip.clear();
  leftStrip.clear();
}

inline uint8_t stripColorPart(uint32_t color, uint8_t channel) {
  if (channel == 0) return (uint8_t)(color >> 16);
  if (channel == 1) return (uint8_t)(color >> 8);
  return (uint8_t)color;
}

inline void fadeStripPixels(uint8_t damper) {
  for (uint16_t i = 0; i < STRIP_PIXELS; i++) {
    const uint32_t rightColor = rightStrip.getPixelColor(i);
    const uint32_t leftColor = leftStrip.getPixelColor(i);

    if (rightColor != 0) {
      rightStrip.setPixelColor(i, rightStrip.Color(
        ((uint16_t)stripColorPart(rightColor, 0) * damper) / 255,
        ((uint16_t)stripColorPart(rightColor, 1) * damper) / 255,
        ((uint16_t)stripColorPart(rightColor, 2) * damper) / 255
      ));
    }

    if (leftColor != 0) {
      leftStrip.setPixelColor(i, leftStrip.Color(
        ((uint16_t)stripColorPart(leftColor, 0) * damper) / 255,
        ((uint16_t)stripColorPart(leftColor, 1) * damper) / 255,
        ((uint16_t)stripColorPart(leftColor, 2) * damper) / 255
      ));
    }
  }
}

inline uint32_t stripRainbow1530(uint16_t i) {
  i %= 1530;
  if (i > 1274) return rightStrip.Color(255, 0, 255 - (i % 255));
  if (i > 1019) return rightStrip.Color(i % 255, 0, 255);
  if (i > 764) return rightStrip.Color(0, 255 - (i % 255), 255);
  if (i > 509) return rightStrip.Color(0, 255, i % 255);
  if (i > 255) return rightStrip.Color(255 - (i % 255), 255, 0);
  return rightStrip.Color(255, i, 0);
}

inline uint32_t stripWheel(uint8_t wheelPos, uint8_t value) {
  if (wheelPos < 85) {
    return rightStrip.Color(
      ((uint16_t)wheelPos * 3 * value) / 255,
      ((uint16_t)(255 - wheelPos * 3) * value) / 255,
      0
    );
  }

  if (wheelPos < 170) {
    wheelPos -= 85;
    return rightStrip.Color(
      ((uint16_t)(255 - wheelPos * 3) * value) / 255,
      0,
      ((uint16_t)wheelPos * 3 * value) / 255
    );
  }

  wheelPos -= 170;
  return rightStrip.Color(
    0,
    ((uint16_t)wheelPos * 3 * value) / 255,
    ((uint16_t)(255 - wheelPos * 3) * value) / 255
  );
}

inline void ledStripsDrawRainbowChase(const VisPacket &pkt) {
  static uint16_t hueOffset = 0;

  const uint8_t energy = stripPacketEnergy(pkt);
  const uint8_t value = constrain(115 + energy * 2, 115, 235);

  for (uint16_t i = 0; i < STRIP_PIXELS; i++) {
    const uint16_t rightHue = hueOffset + (uint16_t)((uint32_t)i * 65535UL / STRIP_PIXELS);
    const uint16_t leftHue = hueOffset + (uint16_t)((uint32_t)(STRIP_PIXELS - 1 - i) * 65535UL / STRIP_PIXELS);

    rightStrip.setPixelColor(i, rightStrip.gamma32(rightStrip.ColorHSV(rightHue, 255, value)));
    leftStrip.setPixelColor(i, leftStrip.gamma32(leftStrip.ColorHSV(leftHue, 255, value)));
  }

  hueOffset += 384;
  setStripBrightness();
  showStrips();
}

inline void ledStripsDrawAudioPulse(const VisPacket &pkt) {
  static uint16_t hueOffset = 0;

  const uint8_t energy = stripPacketEnergy(pkt);
  const uint8_t bass = stripBassLevel(pkt);
  const uint16_t lit = constrain(map(bass, 0, 60, 2, STRIP_PIXELS / 2), 2, STRIP_PIXELS / 2);
  const int16_t centerLeft = (STRIP_PIXELS / 2) - 1;
  const int16_t centerRight = STRIP_PIXELS / 2;
  const uint8_t value = constrain(120 + energy * 2, 120, 245);

  clearStrips();

  const uint16_t distanceDenominator = lit > 1 ? lit - 1 : 1;
  const uint16_t spanDenominator = (lit * 2) > 1 ? (lit * 2) - 1 : 1;

  for (uint16_t i = 0; i < lit; i++) {
    const uint8_t fade = map(i, 0, distanceDenominator, value, 35);
    const uint16_t hueA = hueOffset + (uint16_t)((uint32_t)(lit - 1 - i) * 65535UL / spanDenominator);
    const uint16_t hueB = hueOffset + (uint16_t)((uint32_t)(lit + i) * 65535UL / spanDenominator);
    const uint32_t rightColorA = rightStrip.gamma32(rightStrip.ColorHSV(hueA, 255, fade));
    const uint32_t rightColorB = rightStrip.gamma32(rightStrip.ColorHSV(hueB, 255, fade));
    const uint32_t leftColorA = leftStrip.gamma32(leftStrip.ColorHSV(hueA, 255, fade));
    const uint32_t leftColorB = leftStrip.gamma32(leftStrip.ColorHSV(hueB, 255, fade));
    const int16_t pixelA = centerLeft - i;
    const int16_t pixelB = centerRight + i;

    if (pixelA >= 0) {
      rightStrip.setPixelColor(pixelA, rightColorA);
      leftStrip.setPixelColor(pixelA, leftColorA);
    }
    if (pixelB < STRIP_PIXELS) {
      rightStrip.setPixelColor(pixelB, rightColorB);
      leftStrip.setPixelColor(pixelB, leftColorB);
    }
  }

  hueOffset += 384;
  setStripBrightness();
  showStrips();
}

inline void ledStripsDrawDualComet(const VisPacket &pkt) {
  static int16_t pos = 0;
  static int8_t direction = 1;

  const uint8_t energy = stripPacketEnergy(pkt);
  const uint8_t tail = 26;
  const uint8_t headR = stripClampByte(60 + energy * 2);
  const uint8_t headG = stripClampByte(120 + energy);
  const uint8_t headB = 255;

  clearStrips();

  for (uint8_t t = 0; t < tail; t++) {
    const int rightIndex = direction > 0 ? pos - t : pos + t;
    const int leftIndex = STRIP_PIXELS - 1 - rightIndex;

    if (rightIndex < 0 || rightIndex >= STRIP_PIXELS) {
      continue;
    }

    const uint8_t fade = map(t, 0, tail - 1, 255, 10);
    const uint32_t rightColor = rightStrip.Color((headR * fade) / 255, (headG * fade) / 255, (headB * fade) / 255);
    const uint32_t leftColor = leftStrip.Color((headR * fade) / 255, (headG * fade) / 255, (headB * fade) / 255);

    rightStrip.setPixelColor(rightIndex, rightColor);
    if (leftIndex >= 0 && leftIndex < STRIP_PIXELS) {
      leftStrip.setPixelColor(leftIndex, leftColor);
    }
  }

  pos += direction;
  if (pos >= (int16_t)STRIP_PIXELS - 1) {
    pos = STRIP_PIXELS - 1;
    direction = -1;
  } else if (pos == 0) {
    direction = 1;
  }

  setStripBrightness();
  showStrips();
}

inline void ledStripsDrawRainbowBars(const VisPacket &pkt) {
  static uint16_t hueOffset = 0;

  clearStrips();

  for (uint8_t bar = 0; bar < NUM_BARS; bar++) {
    const uint16_t lit = constrain(map(pkt.bars[bar], 0, 60, 0, STRIP_PIXELS), 0, STRIP_PIXELS);
    const uint16_t hue = hueOffset + (uint16_t)((uint32_t)bar * 65535UL / NUM_BARS);
    const uint32_t color = rightStrip.gamma32(rightStrip.ColorHSV(hue, 255, 180));

    for (uint16_t i = 0; i < lit; i++) {
      rightStrip.setPixelColor(i, color);
      leftStrip.setPixelColor(i, color);
    }
  }

  hueOffset += 192;
  setStripBrightness();
  showStrips();
}

inline void ledStripsDrawCenterPulse(const VisPacket &pkt) {
  static uint16_t gradient = 0;
  static uint8_t lastVolume = 0;
  static float maxVol = 15.0f;
  static float avgVol = 0.0f;
  static float avgBump = 0.0f;

  const uint8_t energy = stripPacketEnergy(pkt);
  const uint8_t bass = stripBassLevel(pkt);
  uint8_t volume = bass > energy ? bass : energy;

  avgVol = (avgVol + volume) / 2.0f;
  if (volume < 2 || volume < avgVol / 2.0f) {
    volume = 0;
  }

  if (volume > maxVol) {
    maxVol = volume;
  }

  if (gradient > 1529) {
    gradient %= 1530;
    maxVol = (maxVol + volume) / 2.0f;
    if (maxVol < 8.0f) {
      maxVol = 8.0f;
    }
  }

  const int16_t delta = (int16_t)volume - (int16_t)lastVolume;
  const int16_t avgDelta = (int16_t)avgVol - (int16_t)lastVolume;
  if (delta > avgDelta && avgDelta > 0) {
    avgBump = (avgBump + delta) / 2.0f;
  }
  const bool bump = volume > 0 && delta > avgBump;

  fadeStripPixels(190);

  if (bump) {
    gradient += 64;
  }

  if (volume > 0) {
    const float level = constrain(volume / maxVol, 0.0f, 1.0f);
    uint16_t lit = (uint16_t)((STRIP_PIXELS / 2) * level);
    if (lit < 1) {
      lit = 1;
    }

    const uint32_t color = stripRainbow1530(gradient);
    const uint8_t red = stripColorPart(color, 0);
    const uint8_t green = stripColorPart(color, 1);
    const uint8_t blue = stripColorPart(color, 2);
    const int16_t centerLeft = (STRIP_PIXELS / 2) - 1;
    const int16_t centerRight = STRIP_PIXELS / 2;
    const uint16_t denominator = lit > 1 ? lit - 1 : 1;

    for (uint16_t i = 0; i < lit; i++) {
      const uint16_t distance = (uint16_t)denominator - i;
      const uint16_t damp = (distance * distance * 255U) / ((uint16_t)denominator * denominator);
      const uint32_t pulseColor = rightStrip.Color(
        ((uint16_t)red * damp) / 255,
        ((uint16_t)green * damp) / 255,
        ((uint16_t)blue * damp) / 255
      );
      const int16_t pixelA = centerLeft - i;
      const int16_t pixelB = centerRight + i;

      if (pixelA >= 0) {
        rightStrip.setPixelColor(pixelA, pulseColor);
        leftStrip.setPixelColor(pixelA, pulseColor);
      }
      if (pixelB < STRIP_PIXELS) {
        rightStrip.setPixelColor(pixelB, pulseColor);
        leftStrip.setPixelColor(pixelB, pulseColor);
      }
    }
  }

  gradient++;
  lastVolume = volume;
  setStripBrightness();
  showStrips();
}

inline uint32_t stripReactiveSplitColor(uint8_t level, uint8_t bass) {
  if (level >= 54) return rightStrip.Color(0, 0, 255);
  if (level >= 48) return rightStrip.Color(153, 153, 0);
  if (level >= 42) return rightStrip.Color(255, 50, 255);
  if (level >= 36) return rightStrip.Color(10, 25, 217);
  if (level >= 30) return rightStrip.Color(50, 50, 150);
  if (level >= 24) return rightStrip.Color(230, 0, 10);
  if (level >= 18) return rightStrip.Color(0, 160, 0);
  if (level >= 10) return rightStrip.Color(8, 0, 8);

  const uint8_t green = stripClampByte((int)bass * 4);
  return rightStrip.Color(152, green, 10);
}

inline void ledStripsDrawReactiveSplit(const VisPacket &pkt) {
  const uint8_t energy = stripPacketEnergy(pkt);
  const uint8_t bass = stripBassLevel(pkt);
  const uint8_t level = energy > bass ? energy : bass;
  const uint16_t centerLeft = (STRIP_PIXELS / 2) - 1;
  const uint16_t centerRight = STRIP_PIXELS / 2;

  for (uint16_t i = 0; i < centerLeft; i++) {
    rightStrip.setPixelColor(i, rightStrip.getPixelColor(i + 1));
    leftStrip.setPixelColor(i, leftStrip.getPixelColor(i + 1));
  }

  for (uint16_t i = STRIP_PIXELS - 1; i > centerRight; i--) {
    rightStrip.setPixelColor(i, rightStrip.getPixelColor(i - 1));
    leftStrip.setPixelColor(i, leftStrip.getPixelColor(i - 1));
  }

  uint32_t centerColor = 0;
  if (level > 2) {
    centerColor = stripReactiveSplitColor(level, bass);
  }

  rightStrip.setPixelColor(centerLeft, centerColor);
  rightStrip.setPixelColor(centerRight, centerColor);
  leftStrip.setPixelColor(centerLeft, centerColor);
  leftStrip.setPixelColor(centerRight, centerColor);

  setStripBrightness();
  showStrips();
}

inline void ledStripsDrawVuMeter(const VisPacket &pkt) {
  static uint16_t peak = 0;
  static uint16_t hueOffset = 0;
  static uint8_t dotCount = 0;

  const uint8_t energy = stripPacketEnergy(pkt);
  const uint8_t bass = stripBassLevel(pkt);
  const uint8_t level = bass > energy ? bass : energy;
  const int heightValue = constrain((int)map(level, 0, 60, 0, (int)STRIP_PIXELS), 0, (int)STRIP_PIXELS);
  const uint16_t height = (uint16_t)heightValue;

  clearStrips();

  for (uint16_t i = 0; i < STRIP_PIXELS; i++) {
    if (i >= height) {
      continue;
    }

    const uint16_t hue = hueOffset + (uint16_t)((uint32_t)i * 65535UL / STRIP_PIXELS);
    const uint32_t color = rightStrip.gamma32(rightStrip.ColorHSV(hue, 255, 220));
    rightStrip.setPixelColor(i, color);
    leftStrip.setPixelColor(i, color);
  }

  if (height > peak) {
    peak = height;
  }

  if (peak > 0 && peak <= STRIP_PIXELS) {
    const uint16_t peakPixel = peak - 1;
    const uint32_t peakColor = rightStrip.Color(255, 255, 255);
    rightStrip.setPixelColor(peakPixel, peakColor);
    leftStrip.setPixelColor(peakPixel, peakColor);
  }

  dotCount++;
  if (dotCount >= 4) {
    dotCount = 0;
    if (peak > 0) {
      peak--;
    }
  }

  hueOffset += 256;

  setStripBrightness();
  showStrips();
}

inline void ledStripsDrawAudioRipple(const VisPacket &pkt) {
  static uint8_t avgLevel = 0;
  static uint8_t lastLevel = 0;
  static int16_t step = -1;
  static uint16_t center = 0;
  static uint8_t hue = 0;
  static uint8_t peakHold = 0;

  const uint8_t energy = stripPacketEnergy(pkt);
  const uint8_t bass = stripBassLevel(pkt);
  const uint8_t level = bass > energy ? bass : energy;

  avgLevel = (uint8_t)(((uint16_t)avgLevel * 7 + level) / 8);
  const bool peakHit = level > 3 && level >= avgLevel + 4 && level >= lastLevel + 1;

  fadeStripPixels(196);

  if (peakHit && peakHold == 0) {
    center = random(STRIP_PIXELS);
    hue += 37;
    step = 0;
    peakHold = 1;
  }

  if (step >= 0) {
    const uint8_t value = stripClampByte(255 - step * 12);
    const uint32_t color = rightStrip.gamma32(rightStrip.ColorHSV((uint16_t)hue * 257U, 255, value));

    const int16_t leftPixel = (int16_t)center - step;
    const int16_t rightPixel = (int16_t)center + step;

    if (leftPixel >= 0) {
      rightStrip.setPixelColor(leftPixel, color);
      leftStrip.setPixelColor(leftPixel, color);
    }
    if (rightPixel < STRIP_PIXELS) {
      rightStrip.setPixelColor(rightPixel, color);
      leftStrip.setPixelColor(rightPixel, color);
    }

    step++;
    if (step > 20) {
      step = -1;
    }
  }

  if (peakHold > 0) {
    peakHold--;
  }
  lastLevel = level;

  setStripBrightness();
  showStrips();
}

inline uint32_t theatreChaseColor(uint8_t colorMode, uint8_t value, uint16_t hueOffset) {
  switch (colorMode) {
    case THEATRE_CHASE_RED:
      return rightStrip.Color(value, 0, 0);
    case THEATRE_CHASE_YELLOW:
      return rightStrip.Color(value, value, 0);
    case THEATRE_CHASE_GREEN:
      return rightStrip.Color(0, value, 0);
    case THEATRE_CHASE_CYAN:
      return rightStrip.Color(0, value, value);
    case THEATRE_CHASE_BLUE:
      return rightStrip.Color(0, 0, value);
    case THEATRE_CHASE_MAGENTA:
      return rightStrip.Color(value, 0, value);
    case THEATRE_CHASE_WHITE:
      return rightStrip.Color(value, value, value);
    case THEATRE_CHASE_RAINBOW:
    default:
      return rightStrip.gamma32(rightStrip.ColorHSV(hueOffset, 255, value));
  }
}

inline void ledStripsDrawTheatreChase(const VisPacket &pkt) {
  static uint8_t chaseOffset = 0;
  static uint16_t hueOffset = 0;
  static uint8_t frameHold = 0;

  const uint8_t energy = stripPacketEnergy(pkt);
  const uint8_t value = constrain(120 + energy * 2, 120, 245);
  uint8_t colorMode = THEATRE_CHASE_RAINBOW;

  portENTER_CRITICAL(&g_readButtonsMux);
  colorMode = currentTheatreChaseColorMode;
  portEXIT_CRITICAL(&g_readButtonsMux);

  const uint32_t rightColor = theatreChaseColor(colorMode, value, hueOffset);
  const uint32_t leftColor = rightColor;

  clearStrips();

  for (uint16_t i = chaseOffset; i < STRIP_PIXELS; i += 3) {
    rightStrip.setPixelColor(i, rightColor);
    leftStrip.setPixelColor(i, leftColor);
  }

  frameHold++;
  if (frameHold >= 3) {
    frameHold = 0;
    chaseOffset = (chaseOffset + 1) % 3;
  }

  hueOffset += 128;

  setStripBrightness();
  showStrips();
}

inline void ledStripsDrawOff() {
  clearStrips();
  showStrips();
}

inline void ledStripsDrawMode(uint8_t stripMode, const VisPacket &pkt) {
  switch (stripMode) {
    case STRIP_RAINBOW_CHASE:
      ledStripsDrawRainbowChase(pkt);
      break;
    case STRIP_AUDIO_PULSE:
      ledStripsDrawAudioPulse(pkt);
      break;
    case STRIP_DUAL_COMET:
      ledStripsDrawDualComet(pkt);
      break;
    case STRIP_RAINBOW_BARS:
      ledStripsDrawRainbowBars(pkt);
      break;
    case STRIP_THEATRE_CHASE:
      ledStripsDrawTheatreChase(pkt);
      break;
    case STRIP_CENTER_PULSE:
      ledStripsDrawCenterPulse(pkt);
      break;
    case STRIP_REACTIVE_SPLIT:
      ledStripsDrawReactiveSplit(pkt);
      break;
    case STRIP_VU_METER:
      ledStripsDrawVuMeter(pkt);
      break;
    case STRIP_AUDIO_RIPPLE:
      ledStripsDrawAudioRipple(pkt);
      break;
    case STRIP_OFF:
    default:
      ledStripsDrawOff();
      break;
  }
}
