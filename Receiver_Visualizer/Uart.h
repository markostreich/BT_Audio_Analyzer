#pragma once

#include <Arduino.h>
#include "VisPacket.h"

#define VIS_UART_PORT 2
#define VIS_UART_BAUD 115200
#define VIS_UART_RX   16
#define VIS_UART_TX   17

inline HardwareSerial VisSerial(VIS_UART_PORT);

inline void uartBegin() {
  VisSerial.begin(VIS_UART_BAUD, SERIAL_8N1, VIS_UART_RX, VIS_UART_TX);
}

inline bool uartReadPacket(VisPacket &pkt) {
  static uint8_t buffer[sizeof(VisPacket)];
  static size_t index = 0;

  while (VisSerial.available()) {
    uint8_t b = (uint8_t)VisSerial.read();

    if (index == 0 && b != VIS_PACKET_START) {
      continue;
    }

    buffer[index++] = b;

    if (index == sizeof(VisPacket)) {
      memcpy(&pkt, buffer, sizeof(VisPacket));
      index = 0;
      return visPacketIsValid(pkt);
    }
  }

  return false;
}

inline void uartWritePacket(const VisPacket &pkt) {
  VisSerial.write((const uint8_t *)&pkt, sizeof(VisPacket));
}