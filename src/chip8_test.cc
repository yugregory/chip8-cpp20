#include "src/chip8.h"
#include "gtest/gtest.h"
#include <cstddef>

namespace chip8 {

class Chip8Test : public ::testing::Test {
protected:
  Chip8 chip8;
};

TEST_F(Chip8Test, InitialState) {
  EXPECT_EQ(chip8.program_counter_, 0x200);
  EXPECT_EQ(chip8.index_register_, 0);
  EXPECT_EQ(chip8.stack_pointer_, 0);
  EXPECT_EQ(chip8.memory_[0x50], std::byte{0xF0});
  EXPECT_EQ(chip8.memory_[0x51], std::byte{0x90});
}

TEST_F(Chip8Test, Opcode00E0_CLS) {
  for (auto &pixel : chip8.display_) {
    pixel = 1;
  }
  chip8.redraw_ = false;
  chip8.memory_[chip8.program_counter_] = std::byte{0x00};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0xE0};
  chip8.execute_cycle();
  for (const auto &pixel : chip8.display_) {
    EXPECT_EQ(pixel, 0);
  }
  EXPECT_TRUE(chip8.redraw_);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode00EE_RET) {
  chip8.stack_[0] = 0x300;
  chip8.stack_pointer_ = 1;
  chip8.program_counter_ = 0x400;
  chip8.memory_[chip8.program_counter_] = std::byte{0x00};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0xEE};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.program_counter_, 0x300);
  EXPECT_EQ(chip8.stack_pointer_, 0);
}

TEST_F(Chip8Test, Opcode1nnn_JP) {
  chip8.memory_[chip8.program_counter_] = std::byte{0x12};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x34};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.program_counter_, 0x234);
}

TEST_F(Chip8Test, Opcode2nnn_CALL) {
  uint16_t initial_pc = chip8.program_counter_;
  chip8.memory_[chip8.program_counter_] = std::byte{0x23};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x45};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.program_counter_, 0x345);
  EXPECT_EQ(chip8.stack_pointer_, 1);
  EXPECT_EQ(chip8.stack_[0], initial_pc + 2);
}

TEST_F(Chip8Test, Opcode3xkk_SE_True) {
  uint16_t initial_pc = chip8.program_counter_;
  chip8.registers_[3] = 0xAB;
  chip8.memory_[chip8.program_counter_] = std::byte{0x33};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0xAB};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.program_counter_, initial_pc + 4);
}

TEST_F(Chip8Test, Opcode3xkk_SE_False) {
  uint16_t initial_pc = chip8.program_counter_;
  chip8.registers_[3] = 0xAC;
  chip8.memory_[chip8.program_counter_] = std::byte{0x33};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0xAB};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.program_counter_, initial_pc + 2);
}

TEST_F(Chip8Test, Opcode4xkk_SNE_True) {
  uint16_t initial_pc = chip8.program_counter_;
  chip8.registers_[3] = 0xAC;
  chip8.memory_[chip8.program_counter_] = std::byte{0x43};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0xAB};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.program_counter_, initial_pc + 4);
}

TEST_F(Chip8Test, Opcode4xkk_SNE_False) {
  uint16_t initial_pc = chip8.program_counter_;
  chip8.registers_[3] = 0xAB;
  chip8.memory_[chip8.program_counter_] = std::byte{0x43};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0xAB};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.program_counter_, initial_pc + 2);
}

TEST_F(Chip8Test, Opcode5xy0_SE_Reg_True) {
  uint16_t initial_pc = chip8.program_counter_;
  chip8.registers_[1] = 0xA;
  chip8.registers_[2] = 0xA;
  chip8.memory_[chip8.program_counter_] = std::byte{0x51};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x20};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.program_counter_, initial_pc + 4);
}

TEST_F(Chip8Test, Opcode5xy0_SE_Reg_False) {
  uint16_t initial_pc = chip8.program_counter_;
  chip8.registers_[1] = 0xA;
  chip8.registers_[2] = 0xB;
  chip8.memory_[chip8.program_counter_] = std::byte{0x51};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x20};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.program_counter_, initial_pc + 2);
}

TEST_F(Chip8Test, Opcode6xkk_LD) {
  chip8.memory_[chip8.program_counter_] = std::byte{0x65};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0xF1};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[5], 0xF1);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode7xkk_ADD) {
  chip8.registers_[5] = 0x10;
  chip8.memory_[chip8.program_counter_] = std::byte{0x75};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x05};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[5], 0x15);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode8xy0_LD_Reg) {
  chip8.registers_[1] = 10;
  chip8.registers_[2] = 20;
  chip8.memory_[chip8.program_counter_] = std::byte{0x81};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x20};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[1], 20);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode8xy1_OR) {
  chip8.registers_[1] = 0b1010;
  chip8.registers_[2] = 0b0101;
  chip8.memory_[chip8.program_counter_] = std::byte{0x81};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x21};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[1], 0b1111);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode8xy2_AND) {
  chip8.registers_[1] = 0b1010;
  chip8.registers_[2] = 0b1100;
  chip8.memory_[chip8.program_counter_] = std::byte{0x81};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x22};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[1], 0b1000);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode8xy3_XOR) {
  chip8.registers_[1] = 0b1010;
  chip8.registers_[2] = 0b1100;
  chip8.memory_[chip8.program_counter_] = std::byte{0x81};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x23};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[1], 0b0110);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode8xy4_ADD_Reg_NoCarry) {
  chip8.registers_[1] = 10;
  chip8.registers_[2] = 20;
  chip8.memory_[chip8.program_counter_] = std::byte{0x81};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x24};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[1], 30);
  EXPECT_EQ(chip8.registers_[0xF], 0);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode8xy4_ADD_Reg_Carry) {
  chip8.registers_[1] = 250;
  chip8.registers_[2] = 10;
  chip8.memory_[chip8.program_counter_] = std::byte{0x81};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x24};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[1], 4); // 260 % 256
  EXPECT_EQ(chip8.registers_[0xF], 1);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode8xy5_SUB_NoBorrow) {
  chip8.registers_[1] = 30;
  chip8.registers_[2] = 10;
  chip8.memory_[chip8.program_counter_] = std::byte{0x81};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x25};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[1], 20);
  EXPECT_EQ(chip8.registers_[0xF], 1);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode8xy5_SUB_Borrow) {
  chip8.registers_[1] = 10;
  chip8.registers_[2] = 20;
  chip8.memory_[chip8.program_counter_] = std::byte{0x81};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x25};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[1], 246);
  EXPECT_EQ(chip8.registers_[0xF], 0);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode8xy6_SHR_LSB_0) {
  chip8.registers_[2] = 0b1010;
  chip8.registers_[1] = 0;
  chip8.memory_[chip8.program_counter_] = std::byte{0x81};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x26};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[1], 0b0101);
  EXPECT_EQ(chip8.registers_[0xF], 0);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode8xy6_SHR_LSB_1) {
  chip8.registers_[2] = 0b1011;
  chip8.memory_[chip8.program_counter_] = std::byte{0x81};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x26};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[1], 0b0101);
  EXPECT_EQ(chip8.registers_[0xF], 1);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode8xy7_SUBN_NoBorrow) {
  chip8.registers_[1] = 10;
  chip8.registers_[2] = 30;
  chip8.memory_[chip8.program_counter_] = std::byte{0x81};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x27};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[1], 20);
  EXPECT_EQ(chip8.registers_[0xF], 1);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode8xy7_SUBN_Borrow) {
  chip8.registers_[1] = 20;
  chip8.registers_[2] = 10;
  chip8.memory_[chip8.program_counter_] = std::byte{0x81};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x27};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[1], 246);
  EXPECT_EQ(chip8.registers_[0xF], 0);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode8xyE_SHL_MSB_0) {
  chip8.registers_[2] = 0b01010101;
  chip8.memory_[chip8.program_counter_] = std::byte{0x81};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x2E};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[1], 0b10101010);
  EXPECT_EQ(chip8.registers_[0xF], 0);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode8xyE_SHL_MSB_1) {
  chip8.registers_[2] = 0b10101010;
  chip8.memory_[chip8.program_counter_] = std::byte{0x81};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x2E};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[1], 0b01010100);
  EXPECT_EQ(chip8.registers_[0xF], 1);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, Opcode9xy0_SNE_Reg_True) {
  uint16_t initial_pc = chip8.program_counter_;
  chip8.registers_[1] = 0xA;
  chip8.registers_[2] = 0xB;
  chip8.memory_[chip8.program_counter_] = std::byte{0x91};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x20};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.program_counter_, initial_pc + 4);
}

TEST_F(Chip8Test, Opcode9xy0_SNE_Reg_False) {
  uint16_t initial_pc = chip8.program_counter_;
  chip8.registers_[1] = 0xA;
  chip8.registers_[2] = 0xA;
  chip8.memory_[chip8.program_counter_] = std::byte{0x91};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x20};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.program_counter_, initial_pc + 2);
}

TEST_F(Chip8Test, OpcodeAnnn_LD_I) {
  chip8.memory_[chip8.program_counter_] = std::byte{0xA1};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x23};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.index_register_, 0x123);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, OpcodeBnnn_JP_V0) {
  chip8.registers_[0] = 0x10;
  chip8.memory_[chip8.program_counter_] = std::byte{0xB1};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x23};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.program_counter_, 0x123 + 0x10);
}

TEST_F(Chip8Test, OpcodeDxyn_DRW) {
  chip8.registers_[0] = 0;
  chip8.registers_[1] = 0;
  chip8.index_register_ = 0x300;
  // Sprite (singular pixel)
  chip8.memory_[0x300] = std::byte{0x80}; // 10000000
  chip8.memory_[chip8.program_counter_] = std::byte{0xD0};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x11};

  chip8.execute_cycle();

  // First draw, no collission
  EXPECT_EQ(chip8.display_[0], 1);
  EXPECT_EQ(chip8.registers_[0xF], 0);
  EXPECT_TRUE(chip8.redraw_);
  EXPECT_EQ(chip8.program_counter_, 0x202);

  // Second draw, collision
  chip8.redraw_ = false;
  chip8.program_counter_ = 0x200;
  chip8.execute_cycle();
  EXPECT_EQ(chip8.display_[0], 0);
  EXPECT_EQ(chip8.registers_[0xF], 1);
  EXPECT_TRUE(chip8.redraw_);
}

TEST_F(Chip8Test, OpcodeEx9E_SKP_True) {
  uint16_t initial_pc = chip8.program_counter_;
  chip8.registers_[5] = 0xA;
  chip8.keypad_[0xA] = 1;
  chip8.memory_[chip8.program_counter_] = std::byte{0xE5};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x9E};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.program_counter_, initial_pc + 4);
}

TEST_F(Chip8Test, OpcodeEx9E_SKP_False) {
  uint16_t initial_pc = chip8.program_counter_;
  chip8.registers_[5] = 0xA;
  chip8.keypad_[0xA] = 0;
  chip8.memory_[chip8.program_counter_] = std::byte{0xE5};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x9E};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.program_counter_, initial_pc + 2);
}

TEST_F(Chip8Test, OpcodeExA1_SKNP_True) {
  uint16_t initial_pc = chip8.program_counter_;
  chip8.registers_[5] = 0xA;
  chip8.keypad_[0xA] = 0;
  chip8.memory_[chip8.program_counter_] = std::byte{0xE5};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0xA1};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.program_counter_, initial_pc + 4);
}

TEST_F(Chip8Test, OpcodeExA1_SKNP_False) {
  uint16_t initial_pc = chip8.program_counter_;
  chip8.registers_[5] = 0xA;
  chip8.keypad_[0xA] = 1;
  chip8.memory_[chip8.program_counter_] = std::byte{0xE5};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0xA1};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.program_counter_, initial_pc + 2);
}

TEST_F(Chip8Test, OpcodeFx07_LD_Vx_DT) {
  chip8.delay_timer_ = 42;
  chip8.memory_[chip8.program_counter_] = std::byte{0xF1};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x07};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.registers_[1], 42);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, OpcodeFx0A_LD_Vx_K_NoPress) {
  uint16_t initial_pc = chip8.program_counter_;
  chip8.memory_[chip8.program_counter_] = std::byte{0xF1};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x0A};
  chip8.execute_cycle();
  EXPECT_TRUE(chip8.waiting_for_key_press_);
  EXPECT_EQ(chip8.program_counter_, initial_pc);
}

TEST_F(Chip8Test, OpcodeFx0A_LD_Vx_K_Press) {
  chip8.keypad_[5] = 1;
  chip8.memory_[chip8.program_counter_] = std::byte{0xF1};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x0A};
  chip8.execute_cycle();
  EXPECT_FALSE(chip8.waiting_for_key_press_);
  EXPECT_TRUE(chip8.waiting_for_key_release_);
  EXPECT_EQ(chip8.registers_[1], 5);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, OpcodeFx15_LD_DT_Vx) {
  chip8.registers_[1] = 42;
  chip8.memory_[chip8.program_counter_] = std::byte{0xF1};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x15};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.delay_timer_, 42);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, OpcodeFx18_LD_ST_Vx) {
  chip8.registers_[1] = 42;
  chip8.memory_[chip8.program_counter_] = std::byte{0xF1};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x18};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.sound_timer_, 42);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, OpcodeFx1E_ADD_I_Vx) {
  chip8.index_register_ = 0x100;
  chip8.registers_[1] = 0x23;
  chip8.memory_[chip8.program_counter_] = std::byte{0xF1};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x1E};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.index_register_, 0x123);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, OpcodeFx29_LD_F_Vx) {
  chip8.registers_[1] = 0xA;
  chip8.memory_[chip8.program_counter_] = std::byte{0xF1};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x29};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.index_register_, 0x50 + 5 * 0xA);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, OpcodeFx33_LD_B_Vx) {
  chip8.registers_[1] = 123;
  chip8.index_register_ = 0x300;
  chip8.memory_[chip8.program_counter_] = std::byte{0xF1};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x33};
  chip8.execute_cycle();
  EXPECT_EQ(chip8.memory_[0x300], std::byte{1});
  EXPECT_EQ(chip8.memory_[0x301], std::byte{2});
  EXPECT_EQ(chip8.memory_[0x302], std::byte{3});
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, OpcodeFx55_LD_I_Vx) {
  chip8.index_register_ = 0x300;
  for (int i = 0; i <= 5; ++i) {
    chip8.registers_[i] = i * 10;
  }
  chip8.memory_[chip8.program_counter_] = std::byte{0xF5};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x55};
  chip8.execute_cycle();
  for (int i = 0; i <= 5; ++i) {
    EXPECT_EQ(chip8.memory_[0x300 + i],
              std::byte{static_cast<uint8_t>(i * 10)});
  }
  EXPECT_EQ(chip8.index_register_, 0x300 + 5 + 1);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

TEST_F(Chip8Test, OpcodeFx65_LD_Vx_I) {
  chip8.index_register_ = 0x300;
  for (int i = 0; i <= 5; ++i) {
    chip8.memory_[0x300 + i] = std::byte{static_cast<uint8_t>(i * 10)};
  }
  chip8.memory_[chip8.program_counter_] = std::byte{0xF5};
  chip8.memory_[chip8.program_counter_ + 1] = std::byte{0x65};
  chip8.execute_cycle();
  for (int i = 0; i <= 5; ++i) {
    EXPECT_EQ(chip8.registers_[i], i * 10);
  }
  EXPECT_EQ(chip8.index_register_, 0x300 + 5 + 1);
  EXPECT_EQ(chip8.program_counter_, 0x202);
}

} // namespace chip8
