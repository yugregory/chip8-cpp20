#include <iostream>

#include "chip8.h"
#include "core.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "No ROM passed into emulator." << std::endl;
    return -1;
  }
  chip8::Chip8 emulator;
  bool success = emulator.loadRom(std::filesystem::path(argv[1]));
  if (!success) {
    return -1;
  }
  success = emulator.run();
  if (!success) {
    return -1;
  }
  return 0;
}
