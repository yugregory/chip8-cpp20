#ifndef SRC_CHIP8_H
#define SRC_CHIP8_H

#include "app_error.h"
#include <array>
#include <cstddef>
#include <filesystem>
#include <random>

namespace chip8 {

class Chip8 {
public:
  Chip8();
  common::Status loadRom(const std::filesystem::path &path);
  common::Status execute_cycle();

public:
  std::default_random_engine rand_gen_;
  std::uniform_int_distribution<uint8_t> rand_byte_;

  std::array<std::byte, 4096> memory_;
  uint16_t program_counter_;
  uint16_t index_register_ = 0;
  std::array<std::uint8_t, 16> registers_;
  std::array<std::uint16_t, 16> stack_;
  std::uint8_t stack_pointer_ = 0;

  std::array<uint8_t, 64 * 32> display_;
  bool redraw_;

  std::array<uint8_t, 16> keypad_;

  std::array<common::Status (*)(Chip8 &, uint8_t, uint8_t), 16> execute_;
};

} // namespace chip8

#endif
