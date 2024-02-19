#ifndef Z80_H
#define Z80_H

#include <cstdint>
#include <array>
#include <functional>
#include <iostream>
#include "Memory.h" 

struct Word {                 
    uint8_t low;
    uint8_t high;
};

struct WordFlags {
    union {
        uint8_t byte;           
        struct {                
            bool CF : 1;        
            bool NF : 1;        // Last operation was addition (0) or subtraction (1)
            bool PF : 1;        // Parity or signed overflow flag
            bool XF : 1;        // Undocumented: Copy of bit 3 of the result
            bool HF : 1;        // Half carry
            bool YF : 1;        // Undocumented: Copy of bit 5 of the result
            bool ZF : 1;        // Zero flag
            bool SF : 1;        // Is result negative? (Copy of MSB)
        };
    } low;
    uint8_t high;
};

 struct Z80Registers {
    uint16_t PC;                // Program counter
    uint16_t SP;                // Stack pointer

    // Index registers
    union {
        uint16_t word;
        Word bytes;
    } IX;
    union {
        uint16_t word;
        Word bytes;
    } IY;

    union {                     // Interrupt register and memory refresh register
        uint16_t word;
        Word bytes;
    } IR;

    // Accumulator and flags
    union {
        uint16_t word;
        WordFlags bytes;
    } AF;

    // General purpose registers
    union {
        uint16_t word;
        Word bytes;
    } BC;

    union {
        uint16_t word;
        Word bytes;
    } DE;

    union {
        uint16_t word;
        Word bytes;
    } HL;

    // Alternate register set
    union {
        uint16_t word;
        WordFlags bytes;
    } AFx;

    union {
        uint16_t word;
        Word bytes;
    } BCx;

    union {
        uint16_t word;
        Word bytes;
    } DEx;

    union {
        uint16_t word;
        Word bytes;
    } HLx;
};



class Z80 {
public:
    Z80();
    void reset();
    void executeNextInstruction();
    void setFlagsForAddition(uint8_t a, uint8_t b, uint16_t result);
    void setFlagsForSubtraction(uint8_t a, uint8_t b);
    void SBC_A_Register(uint8_t value);
    void AND_A_Register(uint8_t value);
    uint8_t parity(uint8_t value) const;
    void setFlagsForLogicalOperation(uint8_t result);
    void decrementRegister(uint8_t& reg);
    void incrementRegister(uint8_t& reg);
    void updateLogicalFlags(uint8_t result);
    void compareWithA(uint8_t value);
    void InitialiseOpcodeTable();
    void InitialiseRegisters();
    Z80Registers* getRegisters();

    uint8_t getRegisterState(char reg);
    void interrupt(uint8_t interruptData);

    // Opcode methods
    void NOP();
    void halt();
       // ADC Instructions
    void ADC_A_B();
    void ADC_A_C();
    void ADC_A_D();
    void ADC_A_E();
    void ADC_A_H();
    void ADC_A_L();
    void ADC_A_HL();
    void ADC_A_A();

    // SUB Instructions
    void SUB_B();
    void SUB_C();
    void SUB_D();
    void SUB_E();
    void SUB_H();
    void SUB_L();
    void SUB_HL();
    void SUB_A();

    // SBC Instructions
    void SBC_A_B();
    void SBC_A_C();
    void SBC_A_D();
    void SBC_A_E();
    void SBC_A_H();
    void SBC_A_L();
    void SBC_A_HL();
    void SBC_A_A();

    // AND Instructions
    void AND_B();
    void AND_C();
    void AND_D();
    void AND_E();
    void AND_H();
    void AND_L();
    void AND_HL();
    void AND_A();

    // XOR Instructions
    void XOR_B();
    void XOR_C();
    void XOR_D();
    void XOR_E();
    void XOR_H();
    void XOR_L();
    void XOR_HL();
    void XOR_A();

    // OR Instructions
    void OR_B();
    void OR_C();
    void OR_D();
    void OR_E();
    void OR_H();
    void OR_L();
    void OR_HL();
    void OR_A();

    // CP (Compare) Instructions
    void CP_B();
    void CP_C();
    void CP_D();
    void CP_E();
    void CP_H();
    void CP_L();
    void CP_HL();
    void CP_A();

    // INC Instructions
    void INC_B();
    void INC_C();
    void INC_D();
    void INC_E();
    void INC_H();
    void INC_L();
    void INC_A();

    // DEC Instructions
    void DEC_B();
    void DEC_C();
    void DEC_D();
    void DEC_E();
    void DEC_H();
    void DEC_L();
    void DEC_A();

    
    // LD n, r Instructions (Load immediate value n into register r)
    void LD_B_n();
    void LD_C_n();
    void LD_D_n();
    void LD_E_n();
    void LD_H_n();
    void LD_L_n();
    void LD_HL_n(); 
    void LD_A_n();

    // LD r, r Instructions (Load the contents of one register into another)
    void LD_B_C();
    void LD_B_D();
    void LD_B_E();
    void LD_B_H();
    void LD_B_L();
    void LD_B_HL(); 
    void LD_C_B();
    void LD_C_C();
    void LD_C_D();
    void LD_C_E();
    void LD_C_H();
    void LD_C_L();
    void LD_C_HL(); 
    

    // LD r, (HL) Instructions (Load the contents of memory location HL into register r)
    void LD_D_HL();
    void LD_E_HL();
    void LD_H_HL();
    void LD_L_HL();

    // LD (HL), r Instructions (Load the contents of register r into memory location HL)
    void LD_HL_B();
    void LD_HL_C();
    void LD_HL_D();
    void LD_HL_E();
    void LD_HL_H();
    void LD_HL_L();
    void LD_HL_A();

    // LD r, (nn) and LD (nn), r Instructions (Load between register r and memory location nn)
    void LD_BC_nn();
    void LD_DE_nn();
    void LD_HL_nn();
    void LD_BC_A(); 
    void LD_DE_A(); 
    
    void EX_DE_HL();
    void RL_D();  
    void RR_E();  
    void SLA_C();  
    void SRA_B(); 

    // Rotate Left Circular (RLC) Instructions
    void RLC_B();  // Rotate B left circular
    void RLC_C();  // Rotate C left circular
    void RLC_D();  // Rotate D left circular
    void RLC_E();  // Rotate E left circular
    void RLC_H();  // Rotate H left circular
    void RLC_L();  // Rotate L left circular
    void RLC_HL(); // Rotate memory location pointed by HL left circular

    
    // Rotate Right Circular (RRC) Instructions
    void RRC_B();  // Rotate B right circular
    void RRC_C();  // Rotate C right circular
    void RRC_D();  // Rotate D right circular
    void RRC_E();  // Rotate E right circular
    void RRC_H();  // Rotate H right circular
    void RRC_L();  // Rotate L right circular
    void RRC_HL(); // Rotate memory location pointed by HL right circular

    // Rotate Left (RL) Instructions
    void RL_B();   // Rotate B left
    void RL_C();   // Rotate C left
    void RL_E();   // Rotate E left
    void RL_H();   // Rotate H left
    void RL_L();   // Rotate L left
    void RL_HL();  // Rotate memory location pointed by HL left

    // Rotate Right (RR) Instructions
    void RR_B();   // Rotate B right
    void RR_C();   // Rotate C right
    void RR_D();   // Rotate D right
    void RR_H();   // Rotate H right
    void RR_L();   // Rotate L right
    void RR_HL();  // Rotate memory location pointed by HL right

    // Shift Left Arithmetic (SLA) Instructions
    void SLA_B();  // Shift B left arithmetic
    void SLA_D();  // Shift D left arithmetic
    void SLA_E();  // Shift E left arithmetic
    void SLA_H();  // Shift H left arithmetic
    void SLA_L();  // Shift L left arithmetic
    void SLA_HL(); // Shift memory location HL left arithmetic
    void SLA_A();  // Shift A left arithmetic

    // Shift Right Arithmetic (SRA) Instructions
    void SRA_C();  // Shift C right arithmetic
    void SRA_D();  // Shift D right arithmetic
    void SRA_E();  // Shift E right arithmetic
    void SRA_H();  // Shift H right arithmetic
    void SRA_L();  // Shift L right arithmetic
    void SRA_HL(); // Shift memory location HL right arithmetic
    void SRA_A();  // Shift A right arithmetic

    // Shift Left Logical (SLL) Instructions
    void SLL_B();  // Shift B left logical
    void SLL_C();  // Shift C left logical
    void SLL_D();  // Shift D left logical
    void SLL_E();  // Shift E left logical
    void SLL_H();  // Shift H left logical
    void SLL_L();  // Shift L left logical
    void SLL_HL(); // Shift memory location HL left logical
    void SLL_A();  // Shift A left logical

    // Shift Right Logical (SRL) Instructions
    void SRL_B();  // Shift B right logical
    void SRL_C();  // Shift C right logical
    void SRL_D();  // Shift D right logical
    void SRL_E();  // Shift E right logical
    void SRL_H();  // Shift H right logical
    void SRL_L();  // Shift L right logical
    void SRL_HL(); // Shift memory location HL right logical
    void SRL_A();  // Shift A right logical

        // Bit Set, Reset, and Test Operations
    void BIT_3_E();
    void BIT_5_H();
    void SET_6_L();
    void SET_2_H();
    void RES_7_A();
    void RES_1_L();
    void BIT_4_C();
    void RES_2_D();
    void SET_3_E();

    // Control Instructions
    void DI();     // Disable Interrupts
    void EI();     // Enable Interrupts
    void JR_e();   // Jump relative
    void JR_NZ_e();// Jump relative if not zero
    void JR_Z_e(); // Jump relative if zero
    void JR_NC_e();// Jump relative if no carry
    void JR_C_e(); // Jump relative if carry
    void DJNZ();   // Decrement B and jump if not zero

    // Jump, Call, and Return Operations
    void JP_HL();    // Jump to address contained in HL
    void JP_nn();    // Jump to address nn
    void JP_Z_nn();  // Jump to address nn if zero
    void JP_NC_nn(); // Jump to address nn if no carry
    void JP_PO_nn(); // Jump to address nn if parity odd
    void CALL_Z_nn();// Call address nn if zero
    void CALL_NZ_nn();// Call address nn if not zero
    void CALL_nn();  // Call address nn
    void CALL_PE_nn();// Call address nn if parity even
    void RET();      // Return from subroutine
    void RET_NZ();   // Return if not zero
    void RET_Z();    // Return if zero
    void RET_P();    // Return if positive
    void RST_00H();  // Call routine at address 00H
    void RST_08H();  // Call routine at address 08H
    void RST_10H();  // Call routine at address 10H
    void RST_18H();  // Call routine at address 18H
    void RST_20H();  // Call routine at address 20H
    void RST_28H();  // Call routine at address 28H
    void RST_30H();  // Call routine at address 30H
    void RST_38H();  // Call routine at address 38H

    // Stack Operations
    void PUSH_BC();
    void POP_DE();
    void PUSH_DE();
    void PUSH_HL();
    void PUSH_AF();
    void POP_BC();
    void POP_HL();
    void POP_AF();

    // 16-bit Arithmetic Operations
    void ADD_HL_BC();
    void ADD_HL_DE();
    void ADD_HL_SP();
    void ADD_HL_HL();
    void LDI_HL();    // Load Increment (HL)
    void DEC_HL();    // Decrement HL
    void INC_HLm();   // Increment memory location pointed by HL
    void DEC_HLm();   // Decrement memory location pointed by HL

    // Getter methods for registers and flags
    uint8_t getA() const;
    uint8_t getB() const;
    uint8_t getC() const;
    uint8_t getD() const;
    uint8_t getE() const;
    uint8_t getH() const;
    uint8_t getL() const;
    uint16_t getSP() const;
    uint16_t getPC() const;

   
private:
    // Member Variables
    uint8_t A, B, C, D, E, H, L, F, I, R;
    uint16_t SP, PC;
    bool interruptsEnabled;
    bool halted;
    bool interruptRequest;
    uint8_t interruptVector;
    int interruptMode;

    Spectrum48KMemory memory;

    static constexpr uint8_t Z_FLAG = 0x80;
    static constexpr uint8_t N_FLAG = 0x40;
    static constexpr uint8_t H_FLAG = 0x20;
    static constexpr uint8_t C_FLAG = 0x10;
    static constexpr uint8_t S_FLAG = 0x80;
    static constexpr uint8_t PV_FLAG = 0x04;

    std::array<std::function<void()>, 256> opcodeTable;

    // Memory class instance
    

    // Private methods
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

#endif // Z80_H
