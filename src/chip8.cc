#include "chip8.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace chip8 {
namespace {

constexpr size_t program_start = 0x200;

} // namespace

bool Chip8::loadRom(const std::filesystem::path &path) {
  std::cout << "Loading rom file at path: " << path << std::endl;
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Could not open ROM file!" << std::endl;
    std::cerr << "Error: " << strerror(errno) << std::endl;
    return false;
  }
  file.seekg(0, std::ios::end);
  std::streampos file_size = file.tellg();
  file.seekg(0, std::ios::beg);
  std::vector<char> buffer(file_size);
  file.read(buffer.data(), file_size);
  if (!file) {
    std::cerr << "Failed to read the bytes from ROM file!" << std::endl;
    return false;
  }
  file.close();
  std::cout << "Successfully read " << file_size << " bytes from: " << path
            << std::endl;
  std::transform(buffer.begin(), buffer.end(), memory_.begin() + program_start,
                 [](char c) { return static_cast<std::byte>(c); });
  std::cout
      << "Successfully copied buffer into program memory starting at address: "
      << program_start << std::endl;
  program_end_address_ = program_start + file_size;
  program_counter_ = program_start;
  return true;
}

bool Chip8::execute_cycle() {
  redraw_ = false;
  uint8_t b1 = static_cast<uint8_t>(memory_[program_counter_]);
  uint8_t b2 = static_cast<uint8_t>(memory_[program_counter_ + 1]);
  uint8_t opcode = b1 >> 4u;
  std::cout << "The instruction for this cycle is: " << std::hex
            << std::uppercase << std::setfill('0') << std::setw(2) << (int)b1
            << " " << std::hex << std::uppercase << std::setfill('0')
            << std::setw(2) << (int)b2 << " at address: " << program_counter_
            << std::endl;
  if (opcode == 0x00u && b2 == 0xE0u) { // CLS
    display_ = std::array<uint8_t, 64 * 32>{};
    redraw_ = true;
  } else if (opcode == 0x01u) { // JP addr
    program_counter_ = ((b1 & 0x0Fu) << 8u) | b2;
    return true;
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
  return true;
}

bool Chip8::should_draw() { return redraw_; }

const std::array<uint8_t, 64 * 32> &Chip8::display() const { return display_; }

} // namespace chip8
