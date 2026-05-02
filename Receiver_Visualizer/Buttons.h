#pragma once

#include "esp32-hal-gpio.h"
#include <stdint.h>

#define SWITCH_BUTTON 2

#define DRAW_BARS_RAINBOW_VERTICAL 0
#define DRAW_BARS_RAINBOW_MIDDLE 1
#define DRAW_BARS_RAINBOW_MIDDLE_MIRRORED 2
#define DRAW_BARS 3
#define DRAW_BARS_MIDDLE 4
#define DRAW_BARS_RAINBOW 5
#define DRAW_AUDIO_BLOB 6

portMUX_TYPE g_readButtonsMux = portMUX_INITIALIZER_UNLOCKED;

const uint8_t switchButton = SWITCH_BUTTON;
volatile bool switchPressed = false;

const uint8_t panelModes[] = {
  DRAW_BARS_RAINBOW_VERTICAL,
  DRAW_BARS_RAINBOW_MIDDLE,
  DRAW_BARS_RAINBOW_MIDDLE_MIRRORED,
  DRAW_BARS,
  DRAW_BARS_MIDDLE,
  DRAW_BARS_RAINBOW,
  DRAW_AUDIO_BLOB
};

const uint8_t panelModeCount = sizeof(panelModes) / sizeof(panelModes[0]);

volatile uint8_t currentPanelMode = 0;

void buttonsTask(void *param);

void initButtons() {
  // Button wired between GPIO 2 and GND
  // Uses ESP32 internal pull-up resistor
  pinMode(switchButton, INPUT_PULLUP);

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

      portENTER_CRITICAL(&g_readButtonsMux);

      currentPanelMode++;
      if (currentPanelMode >= panelModeCount) {
        currentPanelMode = 0;
      }

      portEXIT_CRITICAL(&g_readButtonsMux);

    } else if (!isPressed) {
      switchPressed = false;
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}