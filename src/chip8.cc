#include "chip8.h"
#include "app_error.h"

#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace chip8 {
namespace {

constexpr size_t program_start = 0x200;

void print_instructions(uint8_t b1, uint8_t b2, uint16_t program_counter) {
  std::cout << "The instruction for this cycle is: " << std::hex
            << std::uppercase << std::setfill('0') << std::setw(2) << (int)b1
            << " " << std::hex << std::uppercase << std::setfill('0')
            << std::setw(2) << (int)b2 << " at address: " << program_counter
            << std::endl;
}

} // namespace

common::Status Chip8::loadRom(const std::filesystem::path &path) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file) {
    return std::unexpected(
        common::AppError{common::ErrorCode::IOError, "Unable to open file"});
  }
  std::streampos file_size = file.tellg();
  file.seekg(0, std::ios::beg);
  file.read(reinterpret_cast<char *>(&memory_[program_start]), file_size);
  if (!file) {
    return std::unexpected(common::AppError{
        common::ErrorCode::IOError, "Failed to read ROM bytes from file"});
  }
  file.close();
  program_counter_ = program_start;
  return {};
}

common::Status Chip8::execute_cycle() {
  redraw_ = false;
  uint8_t b1 = static_cast<uint8_t>(memory_[program_counter_]);
  uint8_t b2 = static_cast<uint8_t>(memory_[program_counter_ + 1]);
  print_instructions(b1, b2, program_counter_);
  uint8_t opcode = b1 >> 4u;
  if (opcode == 0x00u && b2 == 0xE0u) { // CLS
    display_ = std::array<uint8_t, 64 * 32>{};
    redraw_ = true;
  } else if (opcode == 0x01u) { // JP addr
    program_counter_ = ((b1 & 0x0Fu) << 8u) | b2;
    return {};
  } else if (opcode == 0x0Au) { // Load Index Register
    index_register_ = ((b1 & 0x0Fu) << 8u) | b2;
  } else if (opcode == 0x06) { // Load Vx with value in b2
    uint8_t Vx = b1 & 0x0Fu;
    registers_[Vx] = b2;
  } else if (opcode == 0x07) { // Add to Vx with value in b2
    uint8_t Vx = b1 & 0x0Fu;
    registers_[Vx] += b2;
  } else if (opcode == 0x0Du) { // Draw contents to display
    uint8_t Vx = b1 & 0x0Fu;
    uint8_t Vy = (b2 & 0xF0u) >> 4u;
    uint8_t height = b2 & 0x0Fu;
    uint8_t x_coord = registers_[Vx];
    uint8_t y_coord = registers_[Vy];
    registers_[0xF] = 0;
    for (uint8_t i = 0; i < height; i++) {
      uint8_t sprite_byte = static_cast<uint8_t>(memory_[index_register_ + i]);
      for (uint8_t j = 0; j < 8; j++) {
        if ((sprite_byte & (0x80 >> j)) != 0) {
          int x = (x_coord + j) % 64;
          int y = (y_coord + i) % 32;
          int pix = (y * 64) + x;
          if (display_[pix] == 1) {
            registers_[0xF] = 1;
          }
          display_[pix] ^= 1;
        }
      }
    }
    redraw_ = true;
  }
  program_counter_ += 2;
  return {};
}

bool Chip8::should_draw() { return redraw_; }

const std::array<uint8_t, 64 * 32> &Chip8::display() const { return display_; }

} // namespace chip8
