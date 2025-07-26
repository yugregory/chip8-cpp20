#include <SDL3/SDL.h>
#include <iostream>

#include "chip8.h"
#include "core.h"
#include "signal.h"

namespace {

void handle_quit_signals(int sig) {
  SDL_Event event;
  event.type = SDL_EVENT_QUIT;
  SDL_PushEvent(&event);
}

void draw_new_screen(const chip8::Chip8 &emulator,
                     std::array<uint32_t, 64 * 32> &screen,
                     SDL_Renderer *renderer, SDL_Texture *texture) {
  for (int i = 0; i < 64 * 32; ++i) {
    screen[i] = (emulator.display()[i] == 1) ? 0xFFFFFFFF : 0xFF000000;
  }

  SDL_UpdateTexture(texture, NULL, screen.data(), 64 * sizeof(uint32_t));
  SDL_RenderClear(renderer);
  SDL_RenderTexture(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

} // namespace

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "No ROM passed into emulator." << std::endl;
    return -1;
  }
  chip8::Chip8 emulator;
  if (!emulator.loadRom(std::filesystem::path(argv[1]))) {
    return -1;
  }

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("CHIP-8 Emulator",
                                        /*width=*/1024, /*height=*/512, 0);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
  SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                           SDL_TEXTUREACCESS_STATIC, 64, 32);
  signal(SIGINT, handle_quit_signals);
  signal(SIGTERM, handle_quit_signals);

  std::array<uint32_t, 64 * 32> screen;
  while (true) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        return 0;
      }
    }
    bool success = emulator.execute_cycle();
    if (!success) {
      std::cerr << "Could not emulate cycle" << std::endl;
      return -1;
    }
    if (emulator.should_draw()) {
      draw_new_screen(emulator, screen, renderer, texture);
    }
    SDL_Delay(1);
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
