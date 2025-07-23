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

bool Chip8::emulate_cycle(uint16_t address) {
  std::cout << "The instruction for this cycle is: " << std::hex << std::setw(2)
            << std::setfill('0') << static_cast<unsigned int>(memory_[address])
            << " " << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<unsigned int>(memory_[address + 1])
            << " at address: " << address << std::endl;
  return true;
}

bool Chip8::run() {
  while (program_counter_ < program_end_address_) {
    if (!emulate_cycle(program_counter_)) {
      std::cerr << "Failed to emulate_cycle at program_counter: "
                << program_counter_ << std::endl;
      return false;
    }
    program_counter_ += 2;
  }
  return true;
}

} // namespace chip8
