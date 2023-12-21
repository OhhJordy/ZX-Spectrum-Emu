#ifndef Z80_H
#define Z80_H

#include <cstdint>
#include <array>

class Z80 {
public:
    Z80();
    void executeNextInstruction();

private:
    uint8_t A;
    uint8_t B;
    uint8_t C;
    uint8_t D;
    uint8_t E;
    uint8_t H;
    uint8_t L;
    uint16_t AF;
    uint16_t BC;
    uint16_t DE;
    uint16_t HL;
    uint16_t SP;
    uint16_t PC;
    uint8_t I;
    uint8_t R;
    static constexpr uint8_t Z_FLAG = 0x80;
    static constexpr uint8_t N_FLAG = 0x40;
    static constexpr uint8_t H_FLAG = 0x20;
    static constexpr uint8_t C_FLAG = 0x10;
    static constexpr uint8_t S_FLAG = 0x80;
    static constexpr uint8_t PV_FLAG = 0x04;
    std::array<uint8_t, 65536> memory;

    void setFlag(uint8_t flag, bool value);
    void setZeroFlag(uint8_t result);
    void setSignFlag(uint8_t result);
    void setHalfCarryFlag(uint8_t a, uint8_t b, uint8_t result);
    void setCarryFlag(uint8_t a, uint8_t b, uint16_t result);
    void setParityOverflowFlag(uint8_t result);

    void LD_B_A();
    void LD_B_n();
    void LD_C_n();
    void LD_D_n();
    void LD_E_n();
    void LD_H_n();
    void LD_L_n();
    void ADD_A_B();
    void ADD_A_C();
    void SUB_B();
    void SUB_C();
    void ADD_A_D();
    void ADD_A_E();
    void ADD_A_H();
    void ADD_A_L();
    void ADD_A_HL();
    void ADD_A_A();
    void ADC_A_B();
    void ADC_A_C();

    uint8_t fetch();
};

#endif // Z80_H
