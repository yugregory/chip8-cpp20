#ifndef SRC_CHIP8_H
#define SRC_CHIP8_H

#include "app_error.h"
#include <array>
#include <cstddef>
#include <filesystem>

namespace chip8 {

class Chip8 {
public:
  common::Status loadRom(const std::filesystem::path &path);

  common::Status execute_cycle();
  bool should_draw();
  const std::array<uint8_t, 64 * 32> &display() const;

private:
  bool emulate_cycle(uint16_t address);

  std::array<std::byte, 4096> memory_;
  std::array<std::uint8_t, 16> registers_;
  uint16_t index_register_;
  uint16_t program_counter_;
  std::array<std::uint16_t, 16> stack_;
  std::uint8_t stack_pointer_;

  std::array<uint8_t, 64 * 32> display_;
  bool redraw_;
};

} // namespace chip8

#endif
