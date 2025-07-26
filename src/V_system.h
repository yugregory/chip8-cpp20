#ifndef SRC_V_SYSTEM_H
#define SRC_V_SYSTEM_H

#include "chip8.h"
#include <concepts>
#include <ostream>

namespace chip8 {

template <typename T>
concept VSystem = requires(T t, const Chip8 &chip8, int width, int height) {
  { T(width, height) };

  { t.poll_events(std::declval<bool &>()) } -> std::same_as<void>;
  { t.draw(chip8) } -> std::same_as<void>;
};

} // namespace chip8

#endif
