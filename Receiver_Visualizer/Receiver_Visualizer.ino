#include <Arduino.h>
#include "VisPacket.h"
#include "Oled.h"
#include "LedPanel.h"
#include "LedStrips.h"
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
  uint8_t lastPanelMode = DRAW_BARS_RAINBOW_VERTICAL;
  uint16_t regularModeBrightness = brightness;
  uint16_t lastSeenBrightness = brightness;

  while (true) {
    portENTER_CRITICAL(&g_frameMux);
    localFrame = g_sharedFrame;
    portEXIT_CRITICAL(&g_frameMux);

    portENTER_CRITICAL(&g_readButtonsMux);
    localCurrentPanelMode = currentPanelMode;
    portEXIT_CRITICAL(&g_readButtonsMux);

    const bool frameChanged = localFrame.frameId != lastSeenFrame;
    const bool panelModeChanged = localCurrentPanelMode != lastPanelMode;
    const bool brightnessChanged = brightness != lastSeenBrightness;
    const bool needsAnimationFrame = localCurrentPanelMode == DRAW_HEART_RAIN;

    if (frameChanged || panelModeChanged || brightnessChanged || needsAnimationFrame) {
      if (panelModeChanged) {
        const bool lastModeHadCustomBrightness = lastPanelMode == DRAW_OSCILLOSCOPE || lastPanelMode == DRAW_BLOBS;

        if (!lastModeHadCustomBrightness) {
          regularModeBrightness = brightness;
        }

        if (localCurrentPanelMode == DRAW_OSCILLOSCOPE) {
          brightness = 9;
        } else if (localCurrentPanelMode == DRAW_BLOBS) {
          brightness = 255;
        } else if (lastModeHadCustomBrightness) {
          brightness = regularModeBrightness;
        }
      }

      lastSeenFrame = localFrame.frameId;
      lastPanelMode = localCurrentPanelMode;
      lastSeenBrightness = brightness;
      if (DRAW_CASSETTE != localCurrentPanelMode)
        notInitializedCassette = true;
      if (DRAW_HEART_RAIN != localCurrentPanelMode)
        notInitializedHeartRain = true;
      switch (localCurrentPanelMode) {
        case DRAW_BARS_RAINBOW_VERTICAL:
          ledPanelDrawBarsRainbowVertical(localFrame.pkt);
          break;
        case DRAW_BARS_RAINBOW_VERTICAL_PEAKS:
          ledPanelDrawBarsRainbowVerticalPeaks(localFrame.pkt);
          break;
        case DRAW_BARS_RAINBOW_MIDDLE:
          ledPanelDrawBarsRainbowVerticalMiddle(localFrame.pkt);
          break;
        case DRAW_BARS_RAINBOW_MIDDLE_MIRRORED:
          ledPanelDrawBarsRainbowVerticalMiddleMirrored(localFrame.pkt);
          break;
        case DRAW_CASSETTE:
          ledPanelDrawCassette(localFrame.pkt);
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
        case DRAW_OSCILLOSCOPE:
          ledPanelDrawOscilloscope(localFrame.pkt);
          break;
        case DRAW_RAINBOW_OSCILLOSCOPE:
          ledPanelDrawRainbowOscilloscope(localFrame.pkt);
          break;
        case DRAW_PEAK_TRAIL:
          ledPanelDrawPeakTrail(localFrame.pkt);
          break;
        case DRAW_SCROLLING_WAVEFORM:
          ledPanelDrawScrollingWaveform(localFrame.pkt);
          break;
        case DRAW_RAINBOW_SCROLLING_WAVEFORM:
          ledPanelDrawRainbowScrollingWaveform(localFrame.pkt);
          break;
        case DRAW_HEART_RAIN:
          ledPanelDrawHeartRain(localFrame.pkt);
          break;
        default:
          ledPanelDrawBarsRainbowVertical(localFrame.pkt);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void stripTask(void *param) {
  SharedFrame localFrame;
  uint8_t localStripMode = STRIP_RAINBOW_CHASE;

  while (true) {
    portENTER_CRITICAL(&g_frameMux);
    localFrame = g_sharedFrame;
    portEXIT_CRITICAL(&g_frameMux);

    portENTER_CRITICAL(&g_readButtonsMux);
    localStripMode = currentStripMode;
    portEXIT_CRITICAL(&g_readButtonsMux);

    ledStripsDrawMode(localStripMode, localFrame.pkt);

    vTaskDelay(pdMS_TO_TICKS(25));
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
  initLedOutputs();
  uartBegin();

  initButtons();

  xTaskCreatePinnedToCore(uartTask, "UART Task", 4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(oledTask, "OLED Task", 4096, nullptr, 1, nullptr, 1);
  xTaskCreatePinnedToCore(matrixTask, "Matrix Task", 4096, nullptr, 1, nullptr, 1);
  xTaskCreatePinnedToCore(stripTask, "Strip Task", 4096, nullptr, 1, nullptr, 1);
}

void loop() {
  delay(1000);
}
