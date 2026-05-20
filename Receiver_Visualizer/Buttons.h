#pragma once

#include <Arduino.h>
#include "esp32-hal-gpio.h"
#include <stdint.h>

constexpr uint8_t BUTTON_MATRIX_UART_PORT = 1;
constexpr uint32_t BUTTON_MATRIX_UART_BAUD = 115200;
constexpr int8_t BUTTON_MATRIX_UART_RX = 4;
constexpr int8_t BUTTON_MATRIX_UART_TX = -1;

constexpr uint8_t DRAW_BARS_RAINBOW_VERTICAL = 0;
constexpr uint8_t DRAW_BARS_RAINBOW_MIDDLE = 1;
constexpr uint8_t DRAW_BARS_RAINBOW_MIDDLE_MIRRORED = 2;
constexpr uint8_t DRAW_CASSETTE = 3;
constexpr uint8_t DRAW_BLOBS = 4;
constexpr uint8_t DRAW_BARS_RAINBOW = 5;
constexpr uint8_t DRAW_AUDIO_BLOB = 6;
constexpr uint8_t DRAW_OSCILLOSCOPE = 7;
constexpr uint8_t DRAW_PEAK_TRAIL = 8;
constexpr uint8_t DRAW_SCROLLING_WAVEFORM = 9;
constexpr uint8_t DRAW_SPECTRUM_HISTORY = 10;
constexpr uint8_t DRAW_ENVELOPE = 11;

constexpr uint8_t MATRIX_SWITCH_BUTTON = 1;
constexpr uint8_t FIRST_MATRIX_PANEL_BUTTON = 2;
constexpr uint8_t BRIGHTNESS_DOWN_BUTTON = 9;
constexpr uint8_t BRIGHTNESS_UP_BUTTON = 10;
constexpr uint8_t BRIGHTNESS_RESET_BUTTON = 11;
constexpr uint8_t OSCILLOSCOPE_BUTTON = 12;
constexpr uint8_t PEAK_TRAIL_BUTTON = 13;
constexpr uint8_t SCROLLING_WAVEFORM_BUTTON = 14;
constexpr uint8_t SPECTRUM_HISTORY_BUTTON = 15;
constexpr uint8_t ENVELOPE_BUTTON = 16;
constexpr uint8_t DEFAULT_BRIGHTNESS = 32;
constexpr uint8_t BRIGHTNESS_STEP = 8;

portMUX_TYPE g_readButtonsMux = portMUX_INITIALIZER_UNLOCKED;
inline HardwareSerial ButtonMatrixSerial(BUTTON_MATRIX_UART_PORT);

const uint8_t panelModes[] = {
  DRAW_BARS_RAINBOW_VERTICAL,
  DRAW_BARS_RAINBOW_MIDDLE,
  DRAW_BARS_RAINBOW_MIDDLE_MIRRORED,
  DRAW_CASSETTE,
  DRAW_BLOBS,
  DRAW_BARS_RAINBOW,
  DRAW_AUDIO_BLOB,
  DRAW_OSCILLOSCOPE,
  DRAW_PEAK_TRAIL,
  DRAW_SCROLLING_WAVEFORM,
  DRAW_SPECTRUM_HISTORY,
  DRAW_ENVELOPE
};

const uint8_t panelModeCount = sizeof(panelModes) / sizeof(panelModes[0]);

uint16_t brightness = DEFAULT_BRIGHTNESS;
volatile uint8_t currentPanelMode = DRAW_BARS_RAINBOW_VERTICAL;

void buttonsTask(void *param);

inline void setPanelMode(uint8_t panelMode) {
  portENTER_CRITICAL(&g_readButtonsMux);
  currentPanelMode = panelMode;
  portEXIT_CRITICAL(&g_readButtonsMux);
}

inline void selectNextPanelMode() {
  uint8_t nextPanelMode = panelModes[0];

  portENTER_CRITICAL(&g_readButtonsMux);

  for (uint8_t i = 0; i < panelModeCount; i++) {
    if (panelModes[i] == currentPanelMode) {
      nextPanelMode = panelModes[(i + 1) % panelModeCount];
      break;
    }
  }

  currentPanelMode = nextPanelMode;

  portEXIT_CRITICAL(&g_readButtonsMux);
}

inline void changeBrightness(int16_t amount) {
  const int16_t newBrightness = constrain((int16_t)brightness + amount, 1, 255);
  brightness = (uint16_t)newBrightness;
}

inline void resetBrightness() {
  brightness = DEFAULT_BRIGHTNESS;
}

inline bool handleMatrixButton(uint8_t button) {
  if (button == MATRIX_SWITCH_BUTTON) {
    selectNextPanelMode();
    return true;
  }

  if (button == BRIGHTNESS_DOWN_BUTTON) {
    changeBrightness(-BRIGHTNESS_STEP);
    return true;
  }

  if (button == BRIGHTNESS_UP_BUTTON) {
    changeBrightness(BRIGHTNESS_STEP);
    return true;
  }

  if (button == BRIGHTNESS_RESET_BUTTON) {
    resetBrightness();
    return true;
  }

  if (button == OSCILLOSCOPE_BUTTON) {
    setPanelMode(DRAW_OSCILLOSCOPE);
    return true;
  }

  if (button == PEAK_TRAIL_BUTTON) {
    setPanelMode(DRAW_PEAK_TRAIL);
    return true;
  }

  if (button == SCROLLING_WAVEFORM_BUTTON) {
    setPanelMode(DRAW_SCROLLING_WAVEFORM);
    return true;
  }

  if (button == SPECTRUM_HISTORY_BUTTON) {
    setPanelMode(DRAW_SPECTRUM_HISTORY);
    return true;
  }

  if (button == ENVELOPE_BUTTON) {
    setPanelMode(DRAW_ENVELOPE);
    return true;
  }

  if (button < FIRST_MATRIX_PANEL_BUTTON) {
    return false;
  }

  const uint8_t panelModeIndex = button - FIRST_MATRIX_PANEL_BUTTON;
  if (panelModeIndex >= panelModeCount) {
    return false;
  }

  setPanelMode(panelModes[panelModeIndex]);
  return true;
}

inline void readButtonMatrix() {
  static uint16_t parsedButton = 0;
  static bool hasDigits = false;

  while (ButtonMatrixSerial.available()) {
    const uint8_t value = (uint8_t)ButtonMatrixSerial.read();

    if (value >= '0' && value <= '9') {
      hasDigits = true;
      parsedButton = (parsedButton * 10) + (value - '0');

      if (parsedButton > 255) {
        parsedButton = 0;
        hasDigits = false;
      }
      continue;
    }

    if (hasDigits) {
      handleMatrixButton((uint8_t)parsedButton);
      parsedButton = 0;
      hasDigits = false;
      continue;
    }

    handleMatrixButton(value);
  }
}

inline void initButtonMatrix() {
  ButtonMatrixSerial.begin(
    BUTTON_MATRIX_UART_BAUD,
    SERIAL_8N1,
    BUTTON_MATRIX_UART_RX,
    BUTTON_MATRIX_UART_TX
  );
}

void initButtons() {
  initButtonMatrix();

  xTaskCreatePinnedToCore(
    buttonsTask,
    "Buttons Task",
    4096,
    nullptr,
    2,
    nullptr,
    0
  );
}

void buttonsTask(void *param) {
  while (true) {
    readButtonMatrix();

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
