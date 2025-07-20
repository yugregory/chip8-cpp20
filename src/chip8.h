#ifndef SRC_CHIP8_H
#define SRC_CHIP8_H

#include <array>
#include <cstddef>

namespace chip8 {

class Chip8 {
private:
  std::array<std::byte, 4096> memory_;
  std::array<std::uint8_t, 16> registers_;
  uint16_t index_register_;
  uint16_t program_counter_;
  std::array<std::uint16_t, 16> stack_;
  std::uint8_t stack_pointer_;
};

} // namespace chip8

#endif
