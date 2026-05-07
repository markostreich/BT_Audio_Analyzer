#include <Arduino.h>
#include "VisPacket.h"
#include "Oled.h"
#include "LedPanel.h"
#include "Uart.h"
#include "Buttons.h"

SharedFrame g_sharedFrame = {};
portMUX_TYPE g_frameMux = portMUX_INITIALIZER_UNLOCKED;

void uartTask(void *param) {
  VisPacket pkt;

  while (true) {
    if (uartReadPacket(pkt)) {
      portENTER_CRITICAL(&g_frameMux);
      g_sharedFrame.pkt = pkt;
      g_sharedFrame.frameId++;
      portEXIT_CRITICAL(&g_frameMux);
    }

    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void oledTask(void *param) {
  uint32_t lastSeenFrame = 0;
  SharedFrame localFrame;

  while (true) {
    portENTER_CRITICAL(&g_frameMux);
    localFrame = g_sharedFrame;
    portEXIT_CRITICAL(&g_frameMux);

    if (localFrame.frameId != lastSeenFrame) {
      lastSeenFrame = localFrame.frameId;
      oledDrawBars(localFrame.pkt);
    }

    vTaskDelay(pdMS_TO_TICKS(33));
  }
}

void matrixTask(void *param) {
  uint32_t lastSeenFrame = 0;
  SharedFrame localFrame;
  uint8_t localCurrentPanelMode = DRAW_BARS_RAINBOW_VERTICAL;

  while (true) {
    portENTER_CRITICAL(&g_frameMux);
    localFrame = g_sharedFrame;
    portEXIT_CRITICAL(&g_frameMux);

    portENTER_CRITICAL(&g_readButtonsMux);
    localCurrentPanelMode = currentPanelMode;
    portEXIT_CRITICAL(&g_readButtonsMux);

    if (localFrame.frameId != lastSeenFrame) {
      lastSeenFrame = localFrame.frameId;
      if (DRAW_CASSETTE != localCurrentPanelMode)
        notInitializedCassette = true;
      switch (localCurrentPanelMode) {
        case DRAW_BARS_RAINBOW_VERTICAL:
          ledPanelDrawBarsRainbowVertical(localFrame.pkt);
          break;
        case DRAW_BARS_RAINBOW_MIDDLE:
          ledPanelDrawBarsRainbowVerticalMiddle(localFrame.pkt);
          break;
        case DRAW_BARS_RAINBOW_MIDDLE_MIRRORED:
          ledPanelDrawBarsRainbowVerticalMiddleMirrored(localFrame.pkt);
          break;
        case DRAW_CASSETTE:
          ledPanelDrawCassette();
          break;
        case DRAW_BLOBS:
          ledPanelDrawTwoAudioBlobs(localFrame.pkt);
          break;
        case DRAW_BARS_RAINBOW:
          ledPanelDrawBarsRainbow(localFrame.pkt);
          break;
        case DRAW_AUDIO_BLOB:
          ledPanelDrawAudioBlob(localFrame.pkt);
          break;
        default:
          ledPanelDrawBarsRainbowVertical(localFrame.pkt);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void setup() {
  Serial.begin(115200);

  if (!oledBegin()) {
    while (true) {
      delay(1000);
    }
  }

  oledShowStartup();
  uartBegin();

  initButtons();

  xTaskCreatePinnedToCore(uartTask, "UART Task", 4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(oledTask, "OLED Task", 4096, nullptr, 1, nullptr, 1);
  xTaskCreatePinnedToCore(matrixTask, "Matrix Task", 4096, nullptr, 1, nullptr, 1);
}

void loop() {
  delay(1000);
}