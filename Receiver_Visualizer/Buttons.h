#pragma once

#include <Arduino.h>
#include "esp32-hal-gpio.h"
#include <stdint.h>

/* LED Potentiometer */
#define POT 13

#define SWITCH_BUTTON 2

#define DRAW_BARS_RAINBOW_VERTICAL 0
#define DRAW_BARS_RAINBOW_MIDDLE 1
#define DRAW_BARS_RAINBOW_MIDDLE_MIRRORED 2
#define DRAW_CASSETTE 3
#define DRAW_BLOBS 4
#define DRAW_BARS_RAINBOW 5
#define DRAW_AUDIO_BLOB 6

portMUX_TYPE g_readButtonsMux = portMUX_INITIALIZER_UNLOCKED;

const uint8_t switchButton = SWITCH_BUTTON;
volatile bool switchPressed = false;

const uint8_t panelModes[] = {
  DRAW_BARS_RAINBOW_VERTICAL,
  DRAW_BARS_RAINBOW_MIDDLE,
  DRAW_BARS_RAINBOW_MIDDLE_MIRRORED,
  DRAW_CASSETTE,
  DRAW_BLOBS,
  DRAW_BARS_RAINBOW,
  DRAW_AUDIO_BLOB
};

const uint8_t panelModeCount = sizeof(panelModes) / sizeof(panelModes[0]);
const uint8_t firstMatrixPanelButton = 1;

/** @brief Global variable for LED panel brightness. */
uint16_t brightness = 0;

volatile uint8_t currentPanelMode = 0;

void buttonsTask(void *param);

inline void setPanelMode(uint8_t panelMode) {
  portENTER_CRITICAL(&g_readButtonsMux);
  currentPanelMode = panelMode;
  portEXIT_CRITICAL(&g_readButtonsMux);
}

inline bool setPanelModeFromMatrixButton(uint8_t button) {
  if (button < firstMatrixPanelButton) {
    return false;
  }

  const uint8_t panelModeIndex = button - firstMatrixPanelButton;
  if (panelModeIndex >= panelModeCount) {
    return false;
  }

  setPanelMode(panelModes[panelModeIndex]);
  return true;
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

inline void readButtonMatrix() {
  static uint16_t matrixButton = 0;
  static bool hasMatrixButtonDigits = false;

  while (Serial.available()) {
    const uint8_t value = (uint8_t)Serial.read();

    if (value >= '0' && value <= '9') {
      hasMatrixButtonDigits = true;
      matrixButton = (matrixButton * 10) + (value - '0');
      if (matrixButton > 255) {
        matrixButton = 0;
        hasMatrixButtonDigits = false;
      }
      continue;
    }

    if (hasMatrixButtonDigits) {
      setPanelModeFromMatrixButton((uint8_t)matrixButton);
      matrixButton = 0;
      hasMatrixButtonDigits = false;
      continue;
    }

    setPanelModeFromMatrixButton(value);
  }
}

/**
 * @brief Adjusts the brightness of the LED panel based on the potentiometer value.
 *
 * This function reads the potentiometer value and maps it to a range suitable for setting the LED panel's brightness.
 * It then updates the brightness if the new value differs significantly from the current brightness, within a tolerance range.
 * This approach minimizes unnecessary updates to the LED panel, improving efficiency.
 *
 * @note This function is designed to be used in both normal operation and test mode. In test mode, it prints the calculated brightness value to the serial monitor instead of adjusting the LED panel's brightness.
 *
 * @warning The brightness adjustment is sensitive to small changes in the potentiometer value. Ensure the potentiometer is properly connected and calibrated for accurate brightness control.
 *
 * @bug None known.
 */
void adjustBrightness() {
  /* Set Brightness with potentiometer */
  uint16_t potValue = analogRead(POT);
  uint16_t newBrightness = map(potValue, 0, 1023, 1, 255);
  // Update brightness if there's a significant change
  // This threshold can be adjusted based on the desired sensitivity
  const uint8_t brightnessThreshold = 1;
  if (abs(brightness - newBrightness) > brightnessThreshold) {
    brightness = newBrightness;
  }
}

void initButtons() {
  // Button wired between GPIO 2 and GND
  // Uses ESP32 internal pull-up resistor
  pinMode(switchButton, INPUT_PULLUP);

  adjustBrightness();

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
    // INPUT_PULLUP means:
    // not pressed = HIGH
    // pressed     = LOW
    bool isPressed = digitalRead(switchButton) == LOW;

    if (isPressed && !switchPressed) {
      switchPressed = true;

      selectNextPanelMode();

    } else if (!isPressed) {
      switchPressed = false;
    }

    readButtonMatrix();
    adjustBrightness();

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
