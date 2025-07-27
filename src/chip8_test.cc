#include "src/chip8.h"
#include "gtest/gtest.h"

namespace chip8 {

TEST(Chip8Test, InitialState) {
  Chip8 chip8;
  chip8.loadRom("../roms/test_opcode.ch8");
  EXPECT_EQ(chip8.program_counter_, 0x200);
  EXPECT_EQ(chip8.index_register_, 0);
  EXPECT_EQ(chip8.stack_pointer_, 0);
}

} // namespace chip8
