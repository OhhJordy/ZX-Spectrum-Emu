#ifndef Z80_H
#define Z80_H

#include <cstdint>
#include <array>
#include <functional>
#include <iostream>

class Z80 {
public:
    Z80();
    void reset();
    void executeNextInstruction();
    uint8_t getRegisterState(char reg);
    void interrupt(uint8_t interruptData);

private:
    // Member Variables
    uint8_t A, B, C, D, E, H, L, F, I, R;
    uint16_t SP, PC;
    bool interruptsEnabled;
    bool halted;
    bool interruptRequest;
    uint8_t interruptVector;
    int interruptMode;

    static constexpr uint8_t Z_FLAG = 0x80;
    static constexpr uint8_t N_FLAG = 0x40;
    static constexpr uint8_t H_FLAG = 0x20;
    static constexpr uint8_t C_FLAG = 0x10;
    static constexpr uint8_t S_FLAG = 0x80;
    static constexpr uint8_t PV_FLAG = 0x04;

    std::array<std::function<void()>, 256> opcodeTable;

    // Memory class instance (assuming Memory is a class you have defined)
    Memory memory;

    // Private methods
    void InitialiseOpcodeTable();
    void InitialiseRegisters();
    void unimplementedOpcode();
    void handleExtendedOpcode(uint8_t opcode);
    void handleInterrupts();
    void handleCBPrefix();

    void setFlag(uint8_t flag, bool value);
    void setZeroFlag(uint8_t result);
    void setSignFlag(uint8_t result);
    void setHalfCarryFlag(uint8_t a, uint8_t b, uint8_t result);
    void setCarryFlag(uint8_t a, uint8_t b, uint16_t result);
    void setParityOverflowFlag(uint8_t result);

    uint8_t fetch();
    uint16_t fetch16();
    uint16_t getBC() const;
    uint16_t getDE() const;
    uint16_t getHL() const;
    void setBC(uint16_t value);
    void setDE(uint16_t value);
    void setHL(uint16_t value);

    void push16(uint16_t value);
    uint16_t pop16();
};

int main() {
    Z80 cpu;
    cpu.executeNextInstruction();

    std::cout << "A: " << static_cast<int>(cpu.A) << std::endl;
    std::cout << "B: " << static_cast<int>(cpu.B) << std::endl;
    std::cout << "C: " << static_cast<int>(cpu.C) << std::endl;
    std::cout << "D: " << static_cast<int>(cpu.D) << std::endl;
    std::cout << "E: " << static_cast<int>(cpu.E) << std::endl;
    std::cout << "H: " << static_cast<int>(cpu.H) << std::endl;
    std::cout << "L: " << static_cast<int>(cpu.L) << std::endl;
    std::cout << "SP: " << cpu.SP << std::endl;
    std::cout << "PC: " << cpu.PC << std::endl;

    return 0;
}

#endif 
