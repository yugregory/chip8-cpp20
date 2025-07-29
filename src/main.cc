#include <SDL3/SDL.h>
#include <iostream>

#include "SDL_system.h"
#include "app_error.h"
#include "chip8.h"
#include "signal.h"

namespace {

constexpr int k_width = 64;
constexpr int k_height = 32;
constexpr int k_scale = 16;

common::Status run(chip8::Chip8 &chip8, chip8::SDLSystem &system) {
  while (true) {
    bool quit = false;
    system.poll_events(quit, chip8.keypad_);
    if (quit) {
      return {};
    }
    common::Status status = chip8.execute_cycle();
    if (!status) {
      return status;
    }
    if (chip8.redraw_) {
      system.draw(chip8);
    }
  }
  return {};
}

} // namespace

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "No ROM passed into emulator." << std::endl;
    return -1;
  }

  chip8::Chip8 chip8;
  common::Status load_status = chip8.loadRom(std::filesystem::path(argv[1]));
  if (!load_status) {
    std::cerr << "Error when loading rom: " << load_status.error() << std::endl;
    return -1;
  }

  common::StatusOr<chip8::SDLSystem> system =
      chip8::create_sdl_system(k_width, k_height, k_scale);
  if (!system.has_value()) {
    std::cerr << "Error when setting up sdl system: " << system.error()
              << std::endl;
  }

  common::Status run_status = run(chip8, system.value());
  if (!run_status) {
    std::cerr << "Error when running the rom: " << run_status.error()
              << std::endl;
    return -1;
  }
  return 0;
}
