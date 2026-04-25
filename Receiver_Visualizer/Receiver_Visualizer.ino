#include <Arduino.h>
#include "VisPacket.h"
#include "Oled.h"
#include "LedPanel.h"
#include "Uart.h"

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

  while (true) {
    portENTER_CRITICAL(&g_frameMux);
    localFrame = g_sharedFrame;
    portEXIT_CRITICAL(&g_frameMux);

    if (localFrame.frameId != lastSeenFrame) {
      lastSeenFrame = localFrame.frameId;
      //ledPanelDrawBars(localFrame.pkt);
      ledPanelDrawBarsRainbowVertical(localFrame.pkt);
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

  xTaskCreatePinnedToCore(uartTask, "UART Task", 4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(oledTask, "OLED Task", 4096, nullptr, 1, nullptr, 1);
  xTaskCreatePinnedToCore(matrixTask, "Matrix Task", 4096, nullptr, 1, nullptr, 1);
}

void loop() {
  delay(1000);
}