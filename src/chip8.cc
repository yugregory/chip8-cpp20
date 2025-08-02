#include "chip8.h"
#include "app_error.h"

#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

namespace chip8 {
namespace {

constexpr size_t k_program_start = 0x200;
constexpr size_t k_font_address = 0x50;

constexpr size_t k_font_space = 80;
constexpr std::array<uint16_t, k_font_space> k_fontset = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void print_instructions(uint8_t b1, uint8_t b2, uint16_t program_counter) {
  std::cout << "The instruction for this cycle is: " << std::hex
            << std::uppercase << std::setfill('0') << std::setw(2) << (int)b1
            << " " << std::hex << std::uppercase << std::setfill('0')
            << std::setw(2) << (int)b2 << " at address: " << program_counter
            << std::endl;
}

common::Status zero(chip8::Chip8 &em, uint8_t b1, uint8_t b2) {
  if (b1 == 0x00u) {
    if (b2 == 0xE0u) {
      em.display_ = std::array<uint8_t, 64 * 32>{};
      em.redraw_ = true;
    } else if (b2 == 0xEEu) {
      if (em.stack_pointer_ - 1 < 0) {
        return std::unexpected(
            common::AppError{common::ErrorCode::InternalError,
                             "Unable to decrement stack_pointer further"});
      }
      em.stack_pointer_ -= 1;
      em.program_counter_ = em.stack_[em.stack_pointer_];
    }
  }
  return {};
}

common::Status jp(chip8::Chip8 &em, uint8_t b1, uint8_t b2) {
  em.program_counter_ = ((b1 & 0x0Fu) << 8u) | b2;
  return {};
}

common::Status ldi(chip8::Chip8 &em, uint8_t b1, uint8_t b2) {
  em.index_register_ = ((b1 & 0x0Fu) << 8u) | b2;
  return {};
}

common::Status ldv(chip8::Chip8 &em, uint8_t b1, uint8_t b2) {
  uint8_t Vx = b1 & 0x0Fu;
  em.registers_[Vx] = b2;
  return {};
}

common::Status addv(chip8::Chip8 &em, uint8_t b1, uint8_t b2) {
  uint8_t Vx = b1 & 0x0Fu;
  em.registers_[Vx] += b2;
  return {};
}

common::Status draw(chip8::Chip8 &em, uint8_t b1, uint8_t b2) {
  uint8_t Vx = b1 & 0x0Fu;
  uint8_t Vy = (b2 & 0xF0u) >> 4u;
  uint8_t height = b2 & 0x0Fu;
  uint8_t x_coord = em.registers_[Vx] % 64;
  uint8_t y_coord = em.registers_[Vy] % 32;
  em.registers_[0xFu] = 0u;
  for (uint8_t i = 0u; i < height; i++) {
    uint8_t sprite_byte =
        static_cast<uint8_t>(em.memory_[em.index_register_ + i]);
    for (uint8_t j = 0u; j < 8u; j++) {
      if ((sprite_byte & (0x80u >> j)) != 0u) {
        int x = (x_coord + j) % 64;
        int y = (y_coord + i) % 32;
        uint16_t pix = (y * 64) + x;
        if (em.display_[pix] == 1u) {
          em.registers_[0xFu] = 1u;
        }
        em.display_[pix] ^= 1u;
      }
    }
  }
  em.redraw_ = true;
  return {};
}

common::Status call(chip8::Chip8 &em, uint8_t b1, uint8_t b2) {
  if (em.stack_pointer_ > 15) {
    return std::unexpected(
        common::AppError{common::ErrorCode::InternalError, "stack overflow"});
  }
  uint16_t address = ((b1 & 0x0Fu) << 8u) | b2;
  em.stack_[em.stack_pointer_] = em.program_counter_;
  em.stack_pointer_ += 1;
  em.program_counter_ = address;
  return {};
}

common::Status skip_instr_equal(chip8::Chip8 &em, uint8_t b1, uint8_t b2) {
  uint8_t Vx = b1 & 0x0Fu;
  if (em.registers_[Vx] == b2) {
    em.program_counter_ += 2;
  }
  return {};
}

common::Status skip_instr_not_equal(chip8::Chip8 &em, uint8_t b1, uint8_t b2) {
  uint8_t Vx = b1 & 0x0Fu;
  if (em.registers_[Vx] != b2) {
    em.program_counter_ += 2;
  }
  return {};
}

common::Status skip_reg_equal(chip8::Chip8 &em, uint8_t b1, uint8_t b2) {
  uint8_t Vx = b1 & 0x0Fu;
  uint8_t Vy = (b2 & 0xF0u) >> 4u;
  if (em.registers_[Vx] == em.registers_[Vy]) {
    em.program_counter_ += 2;
  }
  return {};
}

common::Status skip_reg_not_equal(chip8::Chip8 &em, uint8_t b1, uint8_t b2) {
  uint8_t Vx = b1 & 0x0Fu;
  uint8_t Vy = (b2 & 0xF0u) >> 4u;
  if (em.registers_[Vx] != em.registers_[Vy]) {
    em.program_counter_ += 2;
  }
  return {};
}

common::Status register_ops(chip8::Chip8 &em, uint8_t b1, uint8_t b2) {
  uint8_t Vx = b1 & 0x0Fu;
  uint8_t Vy = (b2 & 0xF0u) >> 4u;
  uint8_t op = (b2 & 0x0Fu);
  switch (op) {
  case 0x0u:
    em.registers_[Vx] = em.registers_[Vy];
    break;
  case 0x1u:
    em.registers_[Vx] |= em.registers_[Vy];
    em.registers_[0xFu] = 0u;
    break;
  case 0x2u:
    em.registers_[Vx] &= em.registers_[Vy];
    em.registers_[0xFu] = 0u;
    break;
  case 0x3u:
    em.registers_[Vx] ^= em.registers_[Vy];
    em.registers_[0xFu] = 0u;
    break;
  case 0x4u: {
    uint16_t sum = em.registers_[Vx] + em.registers_[Vy];
    em.registers_[Vx] = (sum & 0xFFu);
    em.registers_[0xFu] = (sum > 255u) ? 1u : 0u;
    break;
  }
  case 0x5u: {
    int diff = em.registers_[Vx] - em.registers_[Vy];
    em.registers_[Vx] -= em.registers_[Vy];
    em.registers_[0xFu] = (diff >= 0) ? 1u : 0u;
    break;
  }
  case 0x6u: {
    em.registers_[Vx] = em.registers_[Vy];
    bool last_bit_set = em.registers_[Vx] & 0x1u;
    em.registers_[Vx] >>= 1;
    em.registers_[0xFu] = last_bit_set ? 1 : 0;
    break;
  }
  case 0x7u: {
    int diff = em.registers_[Vy] - em.registers_[Vx];
    em.registers_[Vx] = em.registers_[Vy] - em.registers_[Vx];
    em.registers_[0xFu] = (diff >= 0) ? 1u : 0u;
    break;
  }
  case 0xEu: {
    em.registers_[Vx] = em.registers_[Vy];
    bool last_bit_set = em.registers_[Vx] >> 7u;
    em.registers_[Vx] <<= 1;
    em.registers_[0xFu] = last_bit_set ? 1 : 0;
    break;
  }
  default:
    return std::unexpected(common::AppError{common::ErrorCode::InternalError,
                                            "Register op is invalid"});
  }
  return {};
}

common::Status jp_offset(chip8::Chip8 &em, uint8_t b1, uint8_t b2) {
  em.program_counter_ = em.registers_[0] + (((b1 & 0x0Fu) << 8u) | b2);
  return {};
}

common::Status reg_random_plus_offset(chip8::Chip8 &em, uint8_t b1,
                                      uint8_t b2) {
  uint8_t Vx = b1 & 0x0Fu;
  em.registers_[Vx] = em.rand_byte_(em.rand_gen_) & b2;
  return {};
}

common::Status skip_key(chip8::Chip8 &em, uint8_t b1, uint8_t b2) {
  uint8_t Vx = b1 & 0x0Fu;
  uint8_t key = em.registers_[Vx];
  switch (b2) {
  case 0x9Eu:
    if (em.keypad_[key] == 1u)
      em.program_counter_ += 2;
    break;
  case 0xA1u:
    if (em.keypad_[key] != 1u)
      em.program_counter_ += 2;
    break;
  default:
    return std::unexpected(common::AppError{
        common::ErrorCode::InternalError, "keypad skip operation is invalid"});
  }
  return {};
}

common::Status finstr(chip8::Chip8 &em, uint8_t b1, uint8_t b2) {
  uint8_t Vx = b1 & 0x0Fu;
  switch (b2) {
  case 0x07u:
    em.registers_[Vx] = em.delay_timer_;
    break;
  case 0x0Au: {
    bool set = false;
    for (uint8_t i = 0; i < 16u; i++) {
      if (em.keypad_[i] == 1u) {
        em.waiting_for_key_press_ = false;
        em.waiting_for_key_release_ = true;
        em.registers_[Vx] = i;
        set = true;
        break;
      }
    }
    if (!set) {
      em.waiting_for_key_press_ = true;
      em.waiting_for_key_release_ = false;
      em.program_counter_ -= 2;
    }
    break;
  }
  case 0x15:
    em.delay_timer_ = em.registers_[Vx];
    break;
  case 0x18:
    em.sound_timer_ = em.registers_[Vx];
    break;
  case 0x1E:
    em.index_register_ += em.registers_[Vx];
    break;
  case 0x29:
    em.index_register_ = k_font_address + (5 * em.registers_[Vx]);
    break;
  case 0x33: {
    uint16_t value = em.registers_[Vx];
    em.memory_[em.index_register_ + 2] = static_cast<std::byte>(value % 10);
    value /= 10;
    em.memory_[em.index_register_ + 1] = static_cast<std::byte>(value % 10);
    value /= 10;
    em.memory_[em.index_register_] = static_cast<std::byte>(value % 10);
    break;
  }
  case 0x55:
    for (uint8_t i = 0; i <= Vx; i++) {
      em.memory_[em.index_register_ + i] =
          static_cast<std::byte>(em.registers_[i]);
    }
    em.index_register_ += (Vx + 1);
    break;
  case 0x65:
    for (uint8_t i = 0; i <= Vx; i++) {
      em.registers_[i] =
          static_cast<uint8_t>(em.memory_[em.index_register_ + i]);
    }
    em.index_register_ += (Vx + 1);
    break;
  default:
    return std::unexpected(common::AppError{common::ErrorCode::InternalError,
                                            "Invalid finstruction"});
  }
  return {};
}

} // namespace

Chip8::Chip8()
    : rand_gen_(std::chrono::system_clock::now().time_since_epoch().count()),
      rand_byte_(std::uniform_int_distribution<uint8_t>(0, 255U)),
      program_counter_(k_program_start) {
  execute_[0x0u] = &zero;
  execute_[0x1u] = &jp;
  execute_[0x2u] = &call;
  execute_[0x3u] = &skip_instr_equal;
  execute_[0x4u] = &skip_instr_not_equal;
  execute_[0x5u] = &skip_reg_equal;
  execute_[0x6u] = &ldv;
  execute_[0x7u] = &addv;
  execute_[0x8u] = &register_ops;
  execute_[0x9u] = &skip_reg_not_equal;
  execute_[0xAu] = &ldi;
  execute_[0xBu] = &jp_offset;
  execute_[0xCu] = &reg_random_plus_offset;
  execute_[0xDu] = &draw;
  execute_[0xEu] = &skip_key;
  execute_[0xFu] = &finstr;

  for (size_t i = 0; i < k_font_space; i++) {
    memory_[k_font_address + i] = static_cast<std::byte>(k_fontset[i]);
  }
  for (size_t i = 0; i < 16; i++) {
    keypad_[i] = 0u;
  }
}

common::Status Chip8::loadRom(const std::filesystem::path &path) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file) {
    return std::unexpected(
        common::AppError{common::ErrorCode::IOError, "Unable to open file"});
  }
  std::streampos file_size = file.tellg();
  file.seekg(0, std::ios::beg);
  file.read(reinterpret_cast<char *>(&memory_[program_counter_]), file_size);
  if (!file) {
    return std::unexpected(common::AppError{
        common::ErrorCode::IOError, "Failed to read ROM bytes from file"});
  }
  file.close();
  return {};
}

common::Status Chip8::execute_cycle() {
  if (delay_timer_ > 0) {
    --delay_timer_;
  }
  if (sound_timer_ > 0) {
    --sound_timer_;
  }
  if (waiting_for_key_press_ || waiting_for_key_release_) {
    return {};
  }
  uint8_t b1 = static_cast<uint8_t>(memory_[program_counter_]);
  uint8_t b2 = static_cast<uint8_t>(memory_[program_counter_ + 1]);
  // print_instructions(b1, b2, program_counter_);
  program_counter_ += 2;
  uint8_t opcode = b1 >> 4u;
  common::Status status = execute_[opcode](*this, b1, b2);
  return status;
}

} // namespace chip8
