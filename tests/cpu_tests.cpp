#include <gtest/gtest.h>
#include "Z80.h"
#include "Z80.cpp"

// Test ADD Instruction
TEST(Z80Test, AddInstruction) {
    Z80 cpu;
    cpu.reset();
    cpu.A = 0x10;  // Set register A
    cpu.B = 0x20;  // Set register B
    // Assume 0x80 is opcode for ADD A, B
    cpu.memory.write(0x0000, 0x80);
    cpu.executeNextInstruction();
    ASSERT_EQ(cpu.A, 0x30);
}

// Other tests...

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}