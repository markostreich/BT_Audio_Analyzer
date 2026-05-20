#pragma once

#include <Arduino.h>

static constexpr uint8_t VIS_PACKET_START = 0xAA;
static constexpr uint8_t NUM_BARS = 16;
static constexpr uint8_t NUM_SCOPE_SAMPLES = 15;

struct __attribute__((packed)) VisPacket {
  uint8_t start = VIS_PACKET_START;
  uint8_t bars[NUM_BARS] = {0};
  uint8_t scope[NUM_SCOPE_SAMPLES] = {0};
  uint16_t peak = 0;
  uint8_t checksum = 0;
};

struct SharedFrame {
  VisPacket pkt;
  uint32_t frameId = 0;
};

inline uint8_t visPacketChecksum(const VisPacket &pkt) {
  uint8_t sum = pkt.start;
  for (uint8_t i = 0; i < NUM_BARS; i++) {
    sum += pkt.bars[i];
  }
  for (uint8_t i = 0; i < NUM_SCOPE_SAMPLES; i++) {
    sum += pkt.scope[i];
  }
  sum += (uint8_t)(pkt.peak & 0xFF);
  sum += (uint8_t)((pkt.peak >> 8) & 0xFF);
  return sum;
}

inline void visPacketFinalize(VisPacket &pkt) {
  pkt.start = VIS_PACKET_START;
  pkt.checksum = visPacketChecksum(pkt);
}

inline bool visPacketIsValid(const VisPacket &pkt) {
  if (pkt.start != VIS_PACKET_START) return false;
  return pkt.checksum == visPacketChecksum(pkt);
}
