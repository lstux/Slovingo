#pragma once
#include <stdint.h>

enum Button : uint16_t {
  BTN_UP = 1 << 0,
  BTN_DOWN = 1 << 1,
  BTN_LEFT = 1 << 2,
  BTN_RIGHT = 1 << 3,
  BTN_A = 1 << 4,       // sauter
  BTN_B = 1 << 5,       // courir
  BTN_START = 1 << 6,   // pause
  BTN_SELECT = 1 << 7,
  BTN_MENU = 1 << 8,
  BTN_VOLUME = 1 << 9,
};

struct Input {
  uint16_t held = 0, pressed = 0, released = 0;
  void update(uint16_t raw) {
    pressed = raw & ~held;
    released = held & ~raw;
    held = raw;
  }
  bool down(uint16_t b) const { return (held & b) != 0; }
  bool hit(uint16_t b) const { return (pressed & b) != 0; }
};
