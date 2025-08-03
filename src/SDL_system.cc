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

SDLSystem::SDLSystem(int width, int height, const AudioState &audio_state,
                     SDL_Window *window, SDL_Renderer *renderer,
                     SDL_Texture *texture)
    : width_(width), height_(height), audio_state_(audio_state),
      window_(window, SDL_DestroyWindow),
      renderer_(renderer, SDL_DestroyRenderer),
      texture_(texture, SDL_DestroyTexture) {}

SDLSystem::~SDLSystem() {
  std::cout << "Destructor is invoked" << std::endl;
  SDL_Quit();
}

void SDLSystem::poll_events(bool &quit, Chip8 &chip8) {
  SDL_Event event;
  std::array<uint8_t, 16u> &keys = chip8.keypad_;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      quit = true;
    } else if (event.type == SDL_EVENT_KEY_DOWN ||
               event.type == SDL_EVENT_KEY_UP) {
      if (event.type == SDL_EVENT_KEY_DOWN)
        chip8.waiting_for_key_press_ = false;
      if (event.type == SDL_EVENT_KEY_UP)
        chip8.waiting_for_key_release_ = false;
      const uint8_t v = (event.type == SDL_EVENT_KEY_DOWN) ? 1u : 0u;
      switch (event.key.key) {
      case SDLK_X:
        keys[0u] = v;
        break;
      case SDLK_1:
        keys[1u] = v;
        break;
      case SDLK_2:
        keys[2u] = v;
        break;
      case SDLK_3:
        keys[3u] = v;
        break;
      case SDLK_Q:
        keys[4u] = v;
        break;
      case SDLK_W:
        keys[5u] = v;
        break;
      case SDLK_E:
        keys[6u] = v;
        break;
      case SDLK_A:
        keys[7u] = v;
        break;
      case SDLK_S:
        keys[8u] = v;
        break;
      case SDLK_D:
        keys[9u] = v;
        break;
      case SDLK_Z:
        keys[0xAu] = v;
        break;
      case SDLK_C:
        keys[0xBu] = v;
        break;
      case SDLK_4:
        keys[0xCu] = v;
        break;
      case SDLK_R:
        keys[0xDu] = v;
        break;
      case SDLK_F:
        keys[0xEu] = v;
        break;
      case SDLK_V:
        keys[0xFu] = v;
        break;
      default:
        break;
      }
    }
  }
}

void SDLSystem::audio_callback(const Chip8 &chip8, Uint8 *stream, int len) {
  Sint16 *target_buffer = (Sint16 *)stream;
  int num_samples_to_write = len / sizeof(Sint16);

  for (int i = 0; i < num_samples_to_write; i++) {
    if (chip8.should_beep_.load()) {
      target_buffer[i] = ((audio_state_.running_sample_index /
                           audio_state_.half_square_wave_period) %
                          2)
                             ? audio_state_.tone_volume
                             : -audio_state_.tone_volume;
    } else {
      target_buffer[i] = 0;
    }
    audio_state_.running_sample_index++;
  }
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
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  SDL_Window *window =
      SDL_CreateWindow("CHIP-8 Emulator", width * scale, height * scale, 0);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
  SDL_Texture *texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STATIC, width, height);
  signal(SIGINT, handle_quit_signals);
  signal(SIGTERM, handle_quit_signals);
  AudioState audio_state = {};
  audio_state.samples_per_second = 44100;
  audio_state.tone_hz = 440;
  audio_state.tone_volume = 3000;
  audio_state.square_wave_period =
      audio_state.samples_per_second / audio_state.tone_hz;
  audio_state.half_square_wave_period = audio_state.square_wave_period / 2;
  audio_state.running_sample_index = 0;
  return common::StatusOr<SDLSystem>(std::in_place, width, height, audio_state,
                                     window, renderer, texture);
}

} // namespace chip8
