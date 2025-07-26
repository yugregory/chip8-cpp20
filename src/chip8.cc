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
  std::byte b1 = memory_[program_counter_];
  std::byte b2 = memory_[program_counter_ + 1];
  std::cout << "The instruction for this cycle is: " << std::hex << std::setw(2)
            << std::setfill('0') << static_cast<unsigned int>(b1) << " "
            << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<unsigned int>(b2)
            << " at address: " << program_counter_ << std::endl;
  uint8_t opcode = static_cast<uint8_t>(b1 >> 4);
  // CLS - Clear Screen
  if (opcode == 0x00 && static_cast<uint8_t>(b2) == 0xE0) {
    display_ = std::array<uint8_t, 64 * 32>{};
    redraw_ = true;
  }
  program_counter_ += 2;
  return true;
}

bool Chip8::should_draw() { return redraw_; }

const std::array<uint8_t, 64 * 32> &Chip8::display() const { return display_; }

} // namespace chip8
