#include "SDL_system.h"

#include <SDL3/SDL.h>
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

SDLSystem::SDLSystem(int width, int height, int scale)
    : width_(width), height_(height), scale_(scale),
      window_(nullptr, SDL_DestroyWindow),
      renderer_(nullptr, SDL_DestroyRenderer),
      texture_(nullptr, SDL_DestroyTexture) {
  SDL_Init(SDL_INIT_VIDEO);
  int screen_width = width_ * scale;
  int screen_height = height_ * scale;
  std::cout << "screen_width: " << screen_width
            << " screen_height: " << screen_height << std::endl;
  window_.reset(
      SDL_CreateWindow("CHIP-8 Emulator", screen_width, screen_height, 0));
  renderer_.reset(SDL_CreateRenderer(window_.get(), nullptr));
  texture_.reset(SDL_CreateTexture(renderer_.get(), SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STATIC, width, height));
  signal(SIGINT, handle_quit_signals);
  signal(SIGTERM, handle_quit_signals);
}

SDLSystem::~SDLSystem() { SDL_Quit(); }

void SDLSystem::poll_events(bool &quit) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      quit = true;
    }
  }
}

void SDLSystem::draw(const Chip8 &chip8) {
  for (int i = 0; i < width_ * height_; ++i) {
    screen_[i] = (chip8.display()[i] == 1) ? 0xFFFFFFFF : 0xFF000000;
  }

  SDL_UpdateTexture(texture_.get(), NULL, screen_.data(),
                    width_ * sizeof(uint32_t));
  SDL_RenderClear(renderer_.get());
  SDL_RenderTexture(renderer_.get(), texture_.get(), NULL, NULL);
  SDL_RenderPresent(renderer_.get());
  SDL_Delay(2);
}

} // namespace chip8
