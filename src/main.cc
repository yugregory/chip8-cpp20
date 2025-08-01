#include <SDL3/SDL.h>
#include <chrono>
#include <iostream>
#include <thread>

#include "SDL_system.h"
#include "app_error.h"
#include "chip8.h"
#include "signal.h"

namespace {

constexpr int k_width = 64;
constexpr int k_height = 32;
constexpr int k_scale = 16;

constexpr int k_cycles_per_second = 7200;
constexpr int k_timer_frequency = 60;
constexpr int k_cycles_per_frame = k_cycles_per_second / k_timer_frequency;

constexpr auto k_time_per_frame_ms =
    std::chrono::duration<double, std::milli>(1000.0 / k_timer_frequency);

common::Status run(chip8::Chip8 &chip8, chip8::SDLSystem &system) {
  bool quit = false;
  while (true) {
    std::chrono::time_point frame_start = std::chrono::system_clock::now();
    system.poll_events(quit, chip8);
    if (quit)
      return {};
    for (int i = 0; i < k_cycles_per_frame; i++) {
      common::Status status = chip8.execute_cycle();
      if (!status)
        return status;
    }
    if (chip8.redraw_) {
      chip8.redraw_ = false;
      system.draw(chip8);
    }
    std::chrono::time_point frame_end = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> d = frame_end - frame_start;
    std::chrono::duration<double, std::milli> sleep_time_ms =
        k_time_per_frame_ms - d;
    if (sleep_time_ms.count() > 0) {
      std::this_thread::sleep_for(sleep_time_ms);
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
