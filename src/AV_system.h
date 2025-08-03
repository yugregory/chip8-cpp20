#ifndef SRC_AV_SYSTEM_H
#define SRC_AV_SYSTEM_H

#include "chip8.h"
#include <array>
#include <concepts>
#include <ostream>

namespace chip8 {

struct AudioState {
  int samples_per_second;
  int tone_hz;
  int tone_volume;
  uint32_t running_sample_index;
  int square_wave_period;
  int half_square_wave_period;
};

template <typename T>
concept AVSystem = requires(T t, const Chip8 &chip8, int width, int height,
                            AudioState &audio_state) {
  { T(width, height, audio_state) };

  {
    t.poll_events(std::declval<bool &>(), std::declval<Chip8 &>())
  } -> std::same_as<void>;

  { t.draw(chip8) } -> std::same_as<void>;
};

} // namespace chip8

#endif
