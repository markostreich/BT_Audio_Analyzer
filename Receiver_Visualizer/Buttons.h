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
constexpr uint8_t DRAW_RAINBOW_OSCILLOSCOPE = 10;
constexpr uint8_t DRAW_RAINBOW_SCROLLING_WAVEFORM = 11;
constexpr uint8_t DRAW_HEART_RAIN = 12;
constexpr uint8_t DRAW_BARS_RAINBOW_VERTICAL_PEAKS = 13;

constexpr uint8_t STRIP_RAINBOW_CHASE = 0;
constexpr uint8_t STRIP_AUDIO_PULSE = 1;
constexpr uint8_t STRIP_DUAL_COMET = 2;
constexpr uint8_t STRIP_RAINBOW_BARS = 3;
constexpr uint8_t STRIP_THEATRE_CHASE = 4;
constexpr uint8_t STRIP_CENTER_PULSE = 5;
constexpr uint8_t STRIP_REACTIVE_SPLIT = 6;
constexpr uint8_t STRIP_VU_METER = 7;
constexpr uint8_t STRIP_AUDIO_RIPPLE = 8;
constexpr uint8_t STRIP_OFF = 9;

constexpr uint8_t THEATRE_CHASE_RAINBOW = 0;
constexpr uint8_t THEATRE_CHASE_RED = 1;
constexpr uint8_t THEATRE_CHASE_YELLOW = 2;
constexpr uint8_t THEATRE_CHASE_GREEN = 3;
constexpr uint8_t THEATRE_CHASE_CYAN = 4;
constexpr uint8_t THEATRE_CHASE_BLUE = 5;
constexpr uint8_t THEATRE_CHASE_MAGENTA = 6;
constexpr uint8_t THEATRE_CHASE_WHITE = 7;
constexpr uint8_t THEATRE_CHASE_COLOR_COUNT = 8;

constexpr uint8_t BRIGHTNESS_DOWN_BUTTON = 1;
constexpr uint8_t BRIGHTNESS_UP_BUTTON = 2;
constexpr uint8_t STRIP_BRIGHTNESS_DOWN_BUTTON = 3;
constexpr uint8_t STRIP_BRIGHTNESS_UP_BUTTON = 4;
constexpr uint8_t BRIGHTNESS_RESET_BUTTON = 5;
constexpr uint8_t STRIP_BRIGHTNESS_RESET_BUTTON = 6;
constexpr uint8_t MATRIX_SWITCH_BUTTON = 7;
constexpr uint8_t STRIP_SWITCH_BUTTON = 8;

constexpr uint8_t BARS_BUTTON = 9;
constexpr uint8_t BARS_MIDDLE_BUTTON = 10;
constexpr uint8_t BARS_MIRRORED_BUTTON = 11;
constexpr uint8_t CASSETTE_BUTTON = 12;
constexpr uint8_t BLOBS_BUTTON = 13;
constexpr uint8_t RAINBOW_OSCILLOSCOPE_BUTTON = 14;
constexpr uint8_t RAINBOW_SCROLLING_WAVEFORM_BUTTON = 15;
constexpr uint8_t HEART_RAIN_BUTTON = 16;

constexpr uint8_t STRIP_RAINBOW_CHASE_BUTTON = 17;
constexpr uint8_t STRIP_AUDIO_PULSE_BUTTON = 18;
constexpr uint8_t STRIP_RAINBOW_BARS_BUTTON = 19;
constexpr uint8_t STRIP_THEATRE_CHASE_BUTTON = 20;
constexpr uint8_t STRIP_REACTIVE_SPLIT_BUTTON = 21;
constexpr uint8_t STRIP_VU_METER_BUTTON = 22;
constexpr uint8_t STRIP_AUDIO_RIPPLE_BUTTON = 23;
constexpr uint8_t STRIP_OFF_BUTTON = 24;

constexpr uint8_t DEFAULT_BRIGHTNESS = 32;
constexpr uint8_t DEFAULT_STRIP_BRIGHTNESS = 200;
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
  DRAW_RAINBOW_OSCILLOSCOPE,
  DRAW_PEAK_TRAIL,
  DRAW_SCROLLING_WAVEFORM,
  DRAW_RAINBOW_SCROLLING_WAVEFORM,
  DRAW_HEART_RAIN,
  DRAW_BARS_RAINBOW_VERTICAL_PEAKS
};

const uint8_t panelModeCount = sizeof(panelModes) / sizeof(panelModes[0]);
const uint8_t stripModes[] = {
  STRIP_RAINBOW_CHASE,
  STRIP_AUDIO_PULSE,
  STRIP_DUAL_COMET,
  STRIP_RAINBOW_BARS,
  STRIP_THEATRE_CHASE,
  STRIP_CENTER_PULSE,
  STRIP_REACTIVE_SPLIT,
  STRIP_VU_METER,
  STRIP_AUDIO_RIPPLE,
  STRIP_OFF
};
const uint8_t stripModeCount = sizeof(stripModes) / sizeof(stripModes[0]);

uint16_t brightness = DEFAULT_BRIGHTNESS;
uint16_t stripBrightness = DEFAULT_STRIP_BRIGHTNESS;
volatile uint8_t currentPanelMode = DRAW_BARS_RAINBOW_VERTICAL;
volatile uint8_t currentStripMode = STRIP_RAINBOW_CHASE;
volatile uint8_t currentTheatreChaseColorMode = THEATRE_CHASE_RAINBOW;

void buttonsTask(void *param);

inline void setPanelMode(uint8_t panelMode) {
  portENTER_CRITICAL(&g_readButtonsMux);
  currentPanelMode = panelMode;
  portEXIT_CRITICAL(&g_readButtonsMux);
}

inline void setStripMode(uint8_t stripMode) {
  portENTER_CRITICAL(&g_readButtonsMux);
  currentStripMode = stripMode;
  portEXIT_CRITICAL(&g_readButtonsMux);
}

inline void setTheatreChaseColorMode(uint8_t colorMode) {
  portENTER_CRITICAL(&g_readButtonsMux);
  currentTheatreChaseColorMode = colorMode % THEATRE_CHASE_COLOR_COUNT;
  portEXIT_CRITICAL(&g_readButtonsMux);
}

inline void selectNextTheatreChaseColorMode() {
  portENTER_CRITICAL(&g_readButtonsMux);
  currentTheatreChaseColorMode = (currentTheatreChaseColorMode + 1) % THEATRE_CHASE_COLOR_COUNT;
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

inline void selectNextStripMode() {
  uint8_t nextStripMode = stripModes[0];

  portENTER_CRITICAL(&g_readButtonsMux);

  for (uint8_t i = 0; i < stripModeCount; i++) {
    if (stripModes[i] == currentStripMode) {
      nextStripMode = stripModes[(i + 1) % stripModeCount];
      break;
    }
  }

  currentStripMode = nextStripMode;

  portEXIT_CRITICAL(&g_readButtonsMux);
}

inline void changeBrightness(int16_t amount) {
  const int16_t newBrightness = constrain((int16_t)brightness + amount, 1, 255);
  brightness = (uint16_t)newBrightness;
}

inline void resetBrightness() {
  brightness = DEFAULT_BRIGHTNESS;
}

inline void changeStripBrightness(int16_t amount) {
  const int16_t newBrightness = constrain((int16_t)stripBrightness + amount, 1, 255);
  stripBrightness = (uint16_t)newBrightness;
}

inline void resetStripBrightness() {
  stripBrightness = DEFAULT_STRIP_BRIGHTNESS;
}

inline bool handleMatrixButton(uint8_t button) {
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

  if (button == STRIP_BRIGHTNESS_DOWN_BUTTON) {
    changeStripBrightness(-BRIGHTNESS_STEP);
    return true;
  }

  if (button == STRIP_BRIGHTNESS_UP_BUTTON) {
    changeStripBrightness(BRIGHTNESS_STEP);
    return true;
  }

  if (button == STRIP_BRIGHTNESS_RESET_BUTTON) {
    resetStripBrightness();
    return true;
  }

  if (button == MATRIX_SWITCH_BUTTON) {
    selectNextPanelMode();
    return true;
  }

  if (button == STRIP_SWITCH_BUTTON) {
    selectNextStripMode();
    return true;
  }

  if (button == BARS_BUTTON) {
    setPanelMode(DRAW_BARS_RAINBOW_VERTICAL);
    return true;
  }

  if (button == BARS_MIDDLE_BUTTON) {
    setPanelMode(DRAW_BARS_RAINBOW_MIDDLE);
    return true;
  }

  if (button == BARS_MIRRORED_BUTTON) {
    setPanelMode(DRAW_BARS_RAINBOW_MIDDLE_MIRRORED);
    return true;
  }

  if (button == CASSETTE_BUTTON) {
    setPanelMode(DRAW_CASSETTE);
    return true;
  }

  if (button == BLOBS_BUTTON) {
    setPanelMode(DRAW_BLOBS);
    return true;
  }

  if (button == RAINBOW_OSCILLOSCOPE_BUTTON) {
    uint8_t panelMode = DRAW_BARS_RAINBOW_VERTICAL;

    portENTER_CRITICAL(&g_readButtonsMux);
    panelMode = currentPanelMode;
    portEXIT_CRITICAL(&g_readButtonsMux);

    setPanelMode(panelMode == DRAW_OSCILLOSCOPE ? DRAW_RAINBOW_OSCILLOSCOPE : DRAW_OSCILLOSCOPE);
    return true;
  }

  if (button == RAINBOW_SCROLLING_WAVEFORM_BUTTON) {
    uint8_t panelMode = DRAW_BARS_RAINBOW_VERTICAL;

    portENTER_CRITICAL(&g_readButtonsMux);
    panelMode = currentPanelMode;
    portEXIT_CRITICAL(&g_readButtonsMux);

    setPanelMode(panelMode == DRAW_SCROLLING_WAVEFORM ? DRAW_RAINBOW_SCROLLING_WAVEFORM : DRAW_SCROLLING_WAVEFORM);
    return true;
  }

  if (button == HEART_RAIN_BUTTON) {
    setPanelMode(DRAW_HEART_RAIN);
    return true;
  }

  if (button == STRIP_RAINBOW_CHASE_BUTTON) {
    setStripMode(STRIP_RAINBOW_CHASE);
    return true;
  }

  if (button == STRIP_AUDIO_PULSE_BUTTON) {
    setStripMode(STRIP_AUDIO_PULSE);
    return true;
  }

  if (button == STRIP_RAINBOW_BARS_BUTTON) {
    setStripMode(STRIP_RAINBOW_BARS);
    return true;
  }

  if (button == STRIP_THEATRE_CHASE_BUTTON) {
    bool alreadyInTheatreChase = false;

    portENTER_CRITICAL(&g_readButtonsMux);
    alreadyInTheatreChase = currentStripMode == STRIP_THEATRE_CHASE;
    portEXIT_CRITICAL(&g_readButtonsMux);

    if (alreadyInTheatreChase) {
      selectNextTheatreChaseColorMode();
    } else {
      setTheatreChaseColorMode(THEATRE_CHASE_RAINBOW);
      setStripMode(STRIP_THEATRE_CHASE);
    }
    return true;
  }

  if (button == STRIP_REACTIVE_SPLIT_BUTTON) {
    setStripMode(STRIP_REACTIVE_SPLIT);
    return true;
  }

  if (button == STRIP_VU_METER_BUTTON) {
    setStripMode(STRIP_VU_METER);
    return true;
  }

  if (button == STRIP_AUDIO_RIPPLE_BUTTON) {
    setStripMode(STRIP_AUDIO_RIPPLE);
    return true;
  }

  if (button == STRIP_OFF_BUTTON) {
    setStripMode(STRIP_OFF);
    return true;
  }

  return false;
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
