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

constexpr size_t program_start = 0x200;

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
      em.program_counter_ += 2;
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
  uint8_t x_coord = em.registers_[Vx];
  uint8_t y_coord = em.registers_[Vy];
  em.registers_[0xF] = 0;
  for (uint8_t i = 0; i < height; i++) {
    uint8_t sprite_byte =
        static_cast<uint8_t>(em.memory_[em.index_register_ + i]);
    for (uint8_t j = 0; j < 8; j++) {
      if ((sprite_byte & (0x80 >> j)) != 0) {
        int x = (x_coord + j) % 64;
        int y = (y_coord + i) % 32;
        int pix = (y * 64) + x;
        if (em.display_[pix] == 1) {
          em.registers_[0xF] = 1;
        }
        em.display_[pix] ^= 1;
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
  em.stack_[em.stack_pointer_] = em.program_counter_ + 2;
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
  case 0x0:
    em.registers_[Vx] = em.registers_[Vy];
    break;
  case 0x1:
    em.registers_[Vx] |= em.registers_[Vy];
    break;
  case 0x2:
    em.registers_[Vx] &= em.registers_[Vy];
    break;
  case 0x3:
    em.registers_[Vx] ^= em.registers_[Vy];
    break;
  case 0x4:
    break;
  case 0x5:
    break;
  case 0x6:
    break;
  case 0x7:
    break;
  case 0xE:
    break;
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

} // namespace

Chip8::Chip8()
    : rand_gen_(std::chrono::system_clock::now().time_since_epoch().count()),
      rand_byte_(std::uniform_int_distribution<uint8_t>(0, 255U)) {
  execute_[0x0] = &zero;
  execute_[0x1] = &jp;
  execute_[0x2] = &call;
  execute_[0x3] = &skip_instr_equal;
  execute_[0x4] = &skip_instr_not_equal;
  execute_[0x4] = &skip_reg_equal;
  execute_[0x6] = &ldv;
  execute_[0x7] = &addv;
  execute_[0x8] = &register_ops;
  execute_[0x8] = &skip_reg_not_equal;
  execute_[0xA] = &ldi;
  execute_[0xB] = &jp_offset;
  execute_[0xC] = &reg_random_plus_offset;
  execute_[0xD] = &draw;
}

common::Status Chip8::loadRom(const std::filesystem::path &path) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file) {
    return std::unexpected(
        common::AppError{common::ErrorCode::IOError, "Unable to open file"});
  }
  std::streampos file_size = file.tellg();
  file.seekg(0, std::ios::beg);
  file.read(reinterpret_cast<char *>(&memory_[program_start]), file_size);
  if (!file) {
    return std::unexpected(common::AppError{
        common::ErrorCode::IOError, "Failed to read ROM bytes from file"});
  }
  file.close();
  program_counter_ = program_start;
  return {};
}

common::Status Chip8::execute_cycle() {
  redraw_ = false;
  uint8_t b1 = static_cast<uint8_t>(memory_[program_counter_]);
  uint8_t b2 = static_cast<uint8_t>(memory_[program_counter_ + 1]);
  print_instructions(b1, b2, program_counter_);
  program_counter_ += 2;
  uint8_t opcode = b1 >> 4u;
  return execute_[opcode](*this, b1, b2);
}

} // namespace chip8
