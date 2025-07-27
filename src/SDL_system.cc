#include "SDL_system.h"
#include "app_error.h"

#include <SDL3/SDL.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace chip8 {

namespace {

void handle_quit_signals(int sig) {
  SDL_Event event;
  event.type = SDL_EVENT_QUIT;
  SDL_PushEvent(&event);
}

} // namespace

SDLSystem::SDLSystem(int width, int height, SDL_Window *window,
                     SDL_Renderer *renderer, SDL_Texture *texture)
    : width_(width), height_(height), window_(window, SDL_DestroyWindow),
      renderer_(renderer, SDL_DestroyRenderer),
      texture_(texture, SDL_DestroyTexture) {}

SDLSystem::~SDLSystem() {
  std::cout << "Destructor is invoked" << std::endl;
  SDL_Quit();
  exit(1);
}

void SDLSystem::poll_events(bool &quit) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      quit = true;
    }
  }
  SDL_Delay(2);
}

void SDLSystem::draw(const Chip8 &chip8) {
  for (int i = 0; i < width_ * height_; ++i) {
    screen_[i] = (chip8.display_[i] == 1) ? 0xFFFFFFFF : 0xFF000000;
  }

  SDL_UpdateTexture(texture_.get(), NULL, screen_.data(),
                    width_ * sizeof(uint32_t));
  SDL_RenderClear(renderer_.get());
  SDL_RenderTexture(renderer_.get(), texture_.get(), NULL, NULL);
  SDL_RenderPresent(renderer_.get());
}

common::StatusOr<SDLSystem> create_sdl_system(int width, int height,
                                              int scale) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window =
      SDL_CreateWindow("CHIP-8 Emulator", width * scale, height * scale, 0);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
  SDL_Texture *texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STATIC, width, height);
  signal(SIGINT, handle_quit_signals);
  signal(SIGTERM, handle_quit_signals);
  return common::StatusOr<SDLSystem>(std::in_place, width, height, window,
                                     renderer, texture);
}

} // namespace chip8
