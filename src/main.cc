#include <SDL3/SDL.h>
#include <iostream>

#include "SDL_system.h"
#include "app_error.h"
#include "chip8.h"
#include "signal.h"

namespace {

constexpr int kWidth = 64;
constexpr int kHeight = 32;
constexpr int kScale = 16;

} // namespace

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "No ROM passed into emulator." << std::endl;
    return -1;
  }

  chip8::Chip8 chip8;
  common::Status status = chip8.loadRom(std::filesystem::path(argv[1]));
  if (!status) {
    std::cerr << "Error when loading rom: " << status.error() << std::endl;
    return -1;
  }

  chip8::SDLSystem system(kWidth, kHeight, kScale);
  while (true) {
    bool quit = false;
    system.poll_events(quit);
    if (quit) {
      return 0;
    }
    common::Status cycle_status = chip8.execute_cycle();
    if (!cycle_status) {
      std::cerr << "Could not emulate cycle: " << cycle_status.error()
                << std::endl;
      return -1;
    }
    if (chip8.should_draw()) {
      system.draw(chip8);
    }
  }

  return 0;
}
