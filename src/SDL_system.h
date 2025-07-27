#ifndef SRC_SDL_SYSTEM_H
#define SRC_SDL_SYSTEM_H

#include "V_system.h"
#include "app_error.h"
#include "chip8.h"

#include <SDL3/SDL.h>
#include <array>
#include <memory>

namespace chip8 {

class SDLSystem {
public:
  SDLSystem(int width, int height, SDL_Window *window = nullptr,
            SDL_Renderer *renderer = nullptr, SDL_Texture *texture = nullptr);
  SDLSystem(const SDLSystem &rhs) = delete;
  SDLSystem(SDLSystem &&rhs) = default;

  ~SDLSystem();
  void poll_events(bool &quit);
  void draw(const Chip8 &chip8);

private:
  int width_;
  int height_;
  std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window_;
  std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer_;
  std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture_;
  std::array<uint32_t, 64 * 32> screen_;
};

// Statically assert that SDLSystem satisfies the AVSystem concept.
static_assert(VSystem<SDLSystem>);

common::StatusOr<SDLSystem> create_sdl_system(int width, int height, int scale);

} // namespace chip8

#endif
