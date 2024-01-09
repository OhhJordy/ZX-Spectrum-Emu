#include "Memory.h"
#include "Z80.h"
#include <cstdint>
#include <array>
#include <stdexcept>
#include <iostream>


    Z80() : memory() {
        InitialiseRegisters();
        InitialiseOpcodeTable();
        interruptsEnabled = false;
        halted = false;
        interruptRequest = false;
        interruptMode = 0;
    }

void Z80::executeNextInstruction() {
    if (halted) return;

    uint8_t opcode = fetch();

    // Check for extended opcode prefixes
    if (opcode == 0xCB || opcode == 0xDD || opcode == 0xED || opcode == 0xFD) {
        handleExtendedOpcode(opcode);
    } else {
        auto handler = opcodeTable[opcode];
        if (handler) {
            handler(); // Execute the handler function for the opcode
        } else {
            std::cerr << "Unimplemented opcode: 0x" << std::hex << static_cast<int>(opcode) << std::endl;
            throw std::runtime_error("Unimplemented opcode encountered");
        }
    }
}

    
void handleExtendedOpcode(uint8_t opcode) {
    switch (opcode) {
        case 0xCB:
            handleCBPrefix();
            break;
        default:
            unimplementedOpcode();
            break;
    }
}


    void interrupt(uint8_t interruptData) {
    if (!interruptsEnabled) return;

    interruptRequest = true;
    interruptVector = interruptData; 

}

   // Resets the CPU to its initial state
    void Z80::reset() {
    A = B = C = D = E = H = L = F = I = R = 0;
    SP = 0xFFFF;  // Stack pointer often starts at the top of memory
    PC = 0x0000;  // Program counter starts at 0
    interruptsEnabled = false;
    halted = false;
    interruptRequest = false;
    interruptMode = 1; // Default interrupt mode can vary based on the system
}


    uint8_t getRegisterState(char reg) {
    switch (reg) {
        case 'A': return A;
        case 'B': return B;
        case 'C': return C;
        case 'D': return D;
        case 'E': return E;
        case 'H': return H;
        case 'L': return L;
        case 'F': return F;
        default:  return 0;  // Return 0 or an error code for unknown register
    }
}


    Memory memory;
    uint8_t A, B, C, D, E, H, L, F, I, R;
    uint16_t SP, PC;
    bool interruptsEnabled;
    bool halted;

    static constexpr uint8_t Z_FLAG = 0x80;
    static constexpr uint8_t N_FLAG = 0x40;
    static constexpr uint8_t H_FLAG = 0x20;
    static constexpr uint8_t C_FLAG = 0x10;
    static constexpr uint8_t S_FLAG = 0x80;
    static constexpr uint8_t PV_FLAG = 0x04;

    std::array<std::function<void()>, 256> opcodeTable;

    void InitialiseOpcodeTable() {
        // Initialise all opcodes to a default handler (can be a NOP or unimplemented instruction handler)
        for (auto &opcode_handler : opcodeTable) {
            opcode_handler = [this]() { unimplementedOpcode(); };
        }

        // Basic Operations
        opcodeTable[0x00] = [this]() { NOP(); };
        opcodeTable[0x76] = [this]() { halt(); };

        // Arithmetic and Logical Operations
        opcodeTable[0x80] = [this]() { ADD_A_B(); };
        opcodeTable[0x81] = [this]() { ADD_A_C(); };
        opcodeTable[0x82] = [this]() { ADD_A_D(); };
        opcodeTable[0x83] = [this]() { ADD_A_E(); };
        opcodeTable[0x84] = [this]() { ADD_A_H(); };
        opcodeTable[0x85] = [this]() { ADD_A_L(); };
        opcodeTable[0x86] = [this]() { ADD_A_HL(); };
        opcodeTable[0x87] = [this]() { ADD_A_A(); };
        opcodeTable[0x88] = [this]() { ADC_A_B(); };
        opcodeTable[0x89] = [this]() { ADC_A_C(); };
        opcodeTable[0x8B] = [this]() { ADC_A_E(); };
        opcodeTable[0x90] = [this]() { SUB_B(); };
        opcodeTable[0x91] = [this]() { SUB_C(); };
        opcodeTable[0x92] = [this]() { SUB_D(); };
        opcodeTable[0x93] = [this]() { SUB_E(); };
        opcodeTable[0x98] = [this]() { SBC_A_B(); };
        opcodeTable[0x9A] = [this]() { SBC_A_D(); };
        opcodeTable[0xA0] = [this]() { AND_B(); };
        opcodeTable[0xA2] = [this]() { AND_D(); };
        opcodeTable[0xAF] = [this]() { XOR_A(); };
        opcodeTable[0xB1] = [this]() { OR_C(); };
        opcodeTable[0xB3] = [this]() { OR_E(); };
        opcodeTable[0xB7] = [this]() { OR_A(); };
        opcodeTable[0xBA] = [this]() { CP_D(); };
        opcodeTable[0xBE] = [this]() { CP_HL(); };
        opcodeTable[0xBF] = [this]() { CP_A(); };
        opcodeTable[0x04] = [this]() { INC_B(); };
        opcodeTable[0x0C] = [this]() { INC_C(); };
        opcodeTable[0x14] = [this]() { INC_D(); };
        opcodeTable[0x1C] = [this]() { INC_E(); };
        opcodeTable[0x3C] = [this]() { INC_A(); };
        opcodeTable[0x05] = [this]() { DEC_B(); };
        opcodeTable[0x0D] = [this]() { DEC_C(); };
        opcodeTable[0x15] = [this]() { DEC_D(); };
        opcodeTable[0x1D] = [this]() { DEC_E(); };
        opcodeTable[0x3D] = [this]() { DEC_A(); };
        opcodeTable[0x8A] = [this]() { ADC_A_D(); };
        opcodeTable[0x8C] = [this]() { ADC_A_H(); };
        opcodeTable[0x8D] = [this]() { ADC_A_L(); };
        opcodeTable[0x8E] = [this]() { ADC_A_HL(); };
        opcodeTable[0x8F] = [this]() { ADC_A_A(); };
        opcodeTable[0x94] = [this]() { SUB_H(); };
        opcodeTable[0x95] = [this]() { SUB_L(); };
        opcodeTable[0x96] = [this]() { SUB_HL(); };
        opcodeTable[0x97] = [this]() { SUB_A(); };
        opcodeTable[0x9B] = [this]() { SBC_A_E(); };
        opcodeTable[0x9C] = [this]() { SBC_A_H(); };
        opcodeTable[0x9D] = [this]() { SBC_A_L(); };
        opcodeTable[0x9E] = [this]() { SBC_A_HL(); };
        opcodeTable[0x9F] = [this]() { SBC_A_A(); };
        opcodeTable[0xA1] = [this]() { AND_C(); };
        opcodeTable[0xA3] = [this]() { AND_E(); };
        opcodeTable[0xA4] = [this]() { AND_H(); };
        opcodeTable[0xA5] = [this]() { AND_L(); };
        opcodeTable[0xA6] = [this]() { AND_HL(); };
        opcodeTable[0xA7] = [this]() { AND_A(); };
        opcodeTable[0xA8] = [this]() { XOR_B(); };
        opcodeTable[0xA9] = [this]() { XOR_C(); };
        opcodeTable[0xAA] = [this]() { XOR_D(); };
        opcodeTable[0xAB] = [this]() { XOR_E(); };
        opcodeTable[0xAC] = [this]() { XOR_H(); };
        opcodeTable[0xAD] = [this]() { XOR_L(); };
        opcodeTable[0xAE] = [this]() { XOR_HL(); };
        opcodeTable[0xB0] = [this]() { OR_B(); };
        opcodeTable[0xB2] = [this]() { OR_D(); };
        opcodeTable[0xB4] = [this]() { OR_H(); };
        opcodeTable[0xB5] = [this]() { OR_L(); };
        opcodeTable[0xB6] = [this]() { OR_HL(); };
        opcodeTable[0xB8] = [this]() { CP_B(); };
        opcodeTable[0xB9] = [this]() { CP_C(); };
        opcodeTable[0xBB] = [this]() { CP_E(); };
        opcodeTable[0xBC] = [this]() { CP_H(); };
        opcodeTable[0xBD] = [this]() { CP_L(); };

        // Load Operations
        opcodeTable[0x06] = [this]() { LD_B_n(); };
        opcodeTable[0x0E] = [this]() { LD_C_n(); };
        opcodeTable[0x16] = [this]() { LD_D_n(); };
        opcodeTable[0x1E] = [this]() { LD_E_n(); };
        opcodeTable[0x26] = [this]() { LD_H_n(); };
        opcodeTable[0x2E] = [this]() { LD_L_n(); };
        opcodeTable[0x36] = [this]() { LD_HL_n(); };
        opcodeTable[0x3E] = [this]() { LD_A_n(); };
        opcodeTable[0x40] = [this]() { LD_B_B(); };
        opcodeTable[0x41] = [this]() { LD_B_C(); };
        opcodeTable[0x42] = [this]() { LD_B_D(); };
        opcodeTable[0x43] = [this]() { LD_B_E(); };
        opcodeTable[0x4A] = [this]() { LD_C_D(); };
        opcodeTable[0x54] = [this]() { LD_D_H(); };
        opcodeTable[0x5D] = [this]() { LD_E_L(); };
        opcodeTable[0x60] = [this]() { LD_H_B(); };
        opcodeTable[0x69] = [this]() { LD_L_C(); };
        opcodeTable[0x70] = [this]() { LD_HL_B(); };
        opcodeTable[0x71] = [this]() { LD_HL_C(); };
        opcodeTable[0x7E] = [this]() { LD_A_HL(); };
        opcodeTable[0x77] = [this]() { LD_HL_A(); };
        opcodeTable[0x02] = [this]() { LD_BC_A(); };
        opcodeTable[0x12] = [this]() { LD_DE_A(); };
        opcodeTable[0x01] = [this]() { LD_BC_nn(); };
        opcodeTable[0x11] = [this]() { LD_DE_nn(); };
        opcodeTable[0x21] = [this]() { LD_HL_nn(); };
        opcodeTable[0xEB] = [this]() { EX_DE_HL(); };
        opcodeTable[0x44] = [this]() { LD_B_H(); };
        opcodeTable[0x45] = [this]() { LD_B_L(); };
        opcodeTable[0x46] = [this]() { LD_B_HL(); };
        opcodeTable[0x48] = [this]() { LD_C_B(); };
        opcodeTable[0x49] = [this]() { LD_C_C(); };
        opcodeTable[0x4B] = [this]() { LD_C_E(); };
        opcodeTable[0x4C] = [this]() { LD_C_H(); };
        opcodeTable[0x4D] = [this]() { LD_C_L(); };
        opcodeTable[0x4E] = [this]() { LD_C_HL(); };
        opcodeTable[0x50] = [this]() { LD_D_B(); };
        opcodeTable[0x51] = [this]() { LD_D_C(); };
        opcodeTable[0x53] = [this]() { LD_D_E(); };
        opcodeTable[0x55] = [this]() { LD_D_L(); };
        opcodeTable[0x56] = [this]() { LD_D_HL(); };
        opcodeTable[0x57] = [this]() { LD_D_A(); };
        opcodeTable[0x58] = [this]() { LD_E_B(); };
        opcodeTable[0x59] = [this]() { LD_E_C(); };
        opcodeTable[0x5A] = [this]() { LD_E_D(); };
        opcodeTable[0x5B] = [this]() { LD_E_E(); };
        opcodeTable[0x5C] = [this]() { LD_E_H(); };
        opcodeTable[0x5E] = [this]() { LD_E_HL(); };
        opcodeTable[0x5F] = [this]() { LD_E_A(); };
        opcodeTable[0x61] = [this]() { LD_H_C(); };
        opcodeTable[0x62] = [this]() { LD_H_D(); };
        opcodeTable[0x63] = [this]() { LD_H_E(); };
        opcodeTable[0x64] = [this]() { LD_H_H(); };
        opcodeTable[0x65] = [this]() { LD_H_L(); };
        opcodeTable[0x66] = [this]() { LD_H_HL(); };
        opcodeTable[0x67] = [this]() { LD_H_A(); };
        opcodeTable[0x68] = [this]() { LD_L_B(); };
        opcodeTable[0x6A] = [this]() { LD_L_D(); };
        opcodeTable[0x6B] = [this]() { LD_L_E(); };
        opcodeTable[0x6C] = [this]() { LD_L_H(); };
        opcodeTable[0x6D] = [this]() { LD_L_L(); };
        opcodeTable[0x6E] = [this]() { LD_L_HL(); };
        opcodeTable[0x6F] = [this]() { LD_L_A(); };
        opcodeTable[0x72] = [this]() { LD_HL_D(); };
        opcodeTable[0x73] = [this]() { LD_HL_E(); };
        opcodeTable[0x74] = [this]() { LD_HL_H(); };
        opcodeTable[0x75] = [this]() { LD_HL_L(); };
        opcodeTable[0x77] = [this]() { LD_HL_A(); };
        opcodeTable[0x78] = [this]() { LD_A_B(); };
        opcodeTable[0x79] = [this]() { LD_A_C(); };
        opcodeTable[0x7A] = [this]() { LD_A_D(); };
        opcodeTable[0x7B] = [this]() { LD_A_E(); };
        opcodeTable[0x7C] = [this]() { LD_A_H(); };
        opcodeTable[0x7D] = [this]() { LD_A_L(); };
        opcodeTable[0x7F] = [this]() { LD_A_A(); };


        // Rotate and Shift Operations
        opcodeTable[0x07] = [this]() { RLC_A(); };
        opcodeTable[0x0F] = [this]() { RRCA(); };
        opcodeTable[0x17] = [this]() { RLA(); };
        opcodeTable[0x1F] = [this]() { RRA(); };
        opcodeTable[0xCB12] = [this]() { RL_D(); };
        opcodeTable[0xCB1B] = [this]() { RR_E(); };
        opcodeTable[0xCB21] = [this]() { SLA_C(); };
        opcodeTable[0xCB28] = [this]() { SRA_B(); };
        opcodeTable[0xCB00] = [this]() { RLC_B(); };
        opcodeTable[0xCB01] = [this]() { RLC_C(); };
        opcodeTable[0xCB02] = [this]() { RLC_D(); };
        opcodeTable[0xCB03] = [this]() { RLC_E(); };
        opcodeTable[0xCB04] = [this]() { RLC_H(); };
        opcodeTable[0xCB05] = [this]() { RLC_L(); };
        opcodeTable[0xCB06] = [this]() { RLC_HL(); };  // Rotate memory location HL left through carry

        opcodeTable[0xCB08] = [this]() { RRC_B(); };
        opcodeTable[0xCB09] = [this]() { RRC_C(); };
        opcodeTable[0xCB0A] = [this]() { RRC_D(); };
        opcodeTable[0xCB0B] = [this]() { RRC_E(); };
        opcodeTable[0xCB0C] = [this]() { RRC_H(); };
        opcodeTable[0xCB0D] = [this]() { RRC_L(); };
        opcodeTable[0xCB0E] = [this]() { RRC_HL(); };  // Rotate memory location HL right through carry

        opcodeTable[0xCB10] = [this]() { RL_B(); };
        opcodeTable[0xCB11] = [this]() { RL_C(); };
        opcodeTable[0xCB13] = [this]() { RL_E(); };
        opcodeTable[0xCB14] = [this]() { RL_H(); };
        opcodeTable[0xCB15] = [this]() { RL_L(); };
        opcodeTable[0xCB16] = [this]() { RL_HL(); };   // Rotate memory location HL left

        opcodeTable[0xCB18] = [this]() { RR_B(); };
        opcodeTable[0xCB19] = [this]() { RR_C(); };
        opcodeTable[0xCB1A] = [this]() { RR_D(); };
        opcodeTable[0xCB1C] = [this]() { RR_H(); };
        opcodeTable[0xCB1D] = [this]() { RR_L(); };
        opcodeTable[0xCB1E] = [this]() { RR_HL(); };   // Rotate memory location HL right

        opcodeTable[0xCB20] = [this]() { SLA_B(); };
        opcodeTable[0xCB22] = [this]() { SLA_D(); };
        opcodeTable[0xCB23] = [this]() { SLA_E(); };
        opcodeTable[0xCB24] = [this]() { SLA_H(); };
        opcodeTable[0xCB25] = [this]() { SLA_L(); };
        opcodeTable[0xCB26] = [this]() { SLA_HL(); };  // Shift memory location HL left

        opcodeTable[0xCB27] = [this]() { SLA_A(); };

        opcodeTable[0xCB29] = [this]() { SRA_C(); };
        opcodeTable[0xCB2A] = [this]() { SRA_D(); };
        opcodeTable[0xCB2B] = [this]() { SRA_E(); };
        opcodeTable[0xCB2C] = [this]() { SRA_H(); };
        opcodeTable[0xCB2D] = [this]() { SRA_L(); };
        opcodeTable[0xCB2E] = [this]() { SRA_HL(); };  // Shift memory location HL right

        opcodeTable[0xCB2F] = [this]() { SRA_A(); };

        opcodeTable[0xCB30] = [this]() { SLL_B(); };  
        opcodeTable[0xCB31] = [this]() { SLL_C(); };
        opcodeTable[0xCB32] = [this]() { SLL_D(); };
        opcodeTable[0xCB33] = [this]() { SLL_E(); };
        opcodeTable[0xCB34] = [this]() { SLL_H(); };
        opcodeTable[0xCB35] = [this]() { SLL_L(); };
        opcodeTable[0xCB36] = [this]() { SLL_HL(); };
        opcodeTable[0xCB37] = [this]() { SLL_A(); };

        opcodeTable[0xCB38] = [this]() { SRL_B(); };
        opcodeTable[0xCB39] = [this]() { SRL_C(); };
        opcodeTable[0xCB3A] = [this]() { SRL_D(); };
        opcodeTable[0xCB3B] = [this]() { SRL_E(); };
        opcodeTable[0xCB3C] = [this]() { SRL_H(); };
        opcodeTable[0xCB3D] = [this]() { SRL_L(); };
        opcodeTable[0xCB3E] = [this]() { SRL_HL(); };  // Shift memory location HL right logical
        opcodeTable[0xCB3F] = [this]() { SRL_A(); };


        // Bit Set, Reset, and Test Operations
        opcodeTable[0xCB53] = [this]() { BIT_3_E(); };
        opcodeTable[0xCB74] = [this]() { BIT_5_H(); };
        opcodeTable[0xCB75] = [this]() { SET_6_L(); };
        opcodeTable[0xCB76] = [this]() { SET_2_H(); };
        opcodeTable[0xCB87] = [this]() { RES_7_A(); };
        opcodeTable[0xCB8D] = [this]() { RES_1_L(); };
        opcodeTable[0xCB61] = [this]() { BIT_4_C(); };
        opcodeTable[0xCB92] = [this]() { RES_2_D(); };
        opcodeTable[0xCB9B] = [this]() { SET_3_E(); };

        // Control Instructions
        opcodeTable[0xF3] = [this]() { DI(); };
        opcodeTable[0xFB] = [this]() { EI(); };
        opcodeTable[0x18] = [this]() { JR_e(); };
        opcodeTable[0x20] = [this]() { JR_NZ_e(); };
        opcodeTable[0x28] = [this]() { JR_Z_e(); };
        opcodeTable[0x30] = [this]() { JR_NC_e(); };
        opcodeTable[0x38] = [this]() { JR_C_e(); };
        opcodeTable[0x10] = [this]() { DJNZ(); };

        // Jump, Call, and Return Operations
        opcodeTable[0xE9] = [this]() { JP_HL(); };
        opcodeTable[0xC3] = [this]() { JP_nn(); };
        opcodeTable[0xC2] = [this]() { JP_Z_nn(); };
        opcodeTable[0xD2] = [this]() { JP_NC_nn(); };
        opcodeTable[0xE2] = [this]() { JP_PO_nn(); };
        opcodeTable[0xCC] = [this]() { CALL_Z_nn(); };
        opcodeTable[0xC4] = [this]() { CALL_NZ_nn(); };
        opcodeTable[0xCD] = [this]() { CALL_nn(); };
        opcodeTable[0xEC] = [this]() { CALL_PE_nn(); };
        opcodeTable[0xC9] = [this]() { RET(); };
        opcodeTable[0xC0] = [this]() { RET_NZ(); };
        opcodeTable[0xC8] = [this]() { RET_Z(); };
        opcodeTable[0xF2] = [this]() { RET_P(); };
        opcodeTable[0xC7] = [this]() { RST_00H(); };
        opcodeTable[0xCF] = [this]() { RST_08H(); };
        opcodeTable[0xD7] = [this]() { RST_10H(); };
        opcodeTable[0xDF] = [this]() { RST_18H(); };
        opcodeTable[0xE7] = [this]() { RST_20H(); };
        opcodeTable[0xEF] = [this]() { RST_28H(); };
        opcodeTable[0xF7] = [this]() { RST_30H(); };
        opcodeTable[0xFF] = [this]() { RST_38H(); };

        // Stack Operations
        opcodeTable[0xC5] = [this]() { PUSH_BC(); };
        opcodeTable[0xD1] = [this]() { POP_DE(); };
        opcodeTable[0xD5] = [this]() { PUSH_DE(); };
        opcodeTable[0xE5] = [this]() { PUSH_HL(); };
        opcodeTable[0xF5] = [this]() { PUSH_AF(); };
        opcodeTable[0xC1] = [this]() { POP_BC(); };
        opcodeTable[0xE1] = [this]() { POP_HL(); };
        opcodeTable[0xF1] = [this]() { POP_AF(); };

        // 16-bit Arithmetic Operations
        opcodeTable[0x09] = [this]() { ADD_HL_BC(); };
        opcodeTable[0x19] = [this]() { ADD_HL_DE(); };
        opcodeTable[0x39] = [this]() { ADD_HL_SP(); };
        opcodeTable[0x29] = [this]() { ADD_HL_HL(); };
        opcodeTable[0x2A] = [this]() { LDI_HL(); };
        opcodeTable[0x2B] = [this]() { DEC_HL(); };
        opcodeTable[0x34] = [this]() { INC_HLm(); };  // Increment memory location pointed by HL
        opcodeTable[0x35] = [this]() { DEC_HLm(); };  // Decrement memory location pointed by HL


   void unimplementedOpcode() {
    std::cerr << "Unimplemented opcode at address 0x" << std::hex << PC-1 << std::endl;
    halted = true; // Halt the CPU to prevent executing further instructions


}



     void InitialiseRegisters() {
        A = B = C = D = E = H = L = F = I = R = 0;
        SP = PC = 0;
        halted = interruptsEnabled = false;
    }


    void setFlag(uint8_t flag, bool value) {
        if (value) {
            F |= flag;
        } else {
            F &= ~flag;
        }
    }

    void setZeroFlag(uint8_t result) {
        setFlag(Z_FLAG, result == 0);
    }

    void setFlagsForCompare(uint8_t result, uint8_t operand) {
    setZeroFlag(result == 0);
    setSignFlag((result & 0x80) != 0);
    setHalfCarryFlag((A & 0x0F) < (operand & 0x0F));
    setCarryFlag(A < operand);
    setFlag(N_FLAG, true);
}


    void setSignFlag(uint8_t result) {
        setFlag(S_FLAG, (result & 0x80) != 0);
    }

    void setHalfCarryFlag(uint8_t a, uint8_t b, uint8_t result) {
        setFlag(H_FLAG, ((a & 0x0F) + (b & 0x0F)) > 0x0F);
    }

    void setCarryFlag(uint8_t a, uint8_t b, uint16_t result) {
        setFlag(C_FLAG, result > 0xFF);
    }

   void setParityOverflowFlag(uint8_t result) {
    // Parity flag is set if the number of set bits in 'result' is even.
    uint8_t parity = 0;
    for (parity = 0; result; parity++) {
        result &= result - 1;
    }
    setFlag(PV_FLAG, (parity % 2) == 0);
}

    uint8_t fetch() {
    return memory.read(PC++);
    }

    uint16_t fetch16() {
        uint8_t lowByte = memory.read(PC++);
        uint8_t highByte = memory.read(PC++);
        return (static_cast<uint16_t>(highByte) << 8) | lowByte;
    }

    uint16_t getBC() const { return (B << 8) | C; }
    uint16_t getDE() const { return (D << 8) | E; }
    uint16_t getHL() const { return (H << 8) | L; }

    void setBC(uint16_t value) { B = value >> 8; C = value & 0xFF; }

    void push16(uint16_t value) {
    memory.write(--SP, value >> 8);
    memory.write(--SP, value & 0xFF);
    }

    uint16_t pop16() {
        uint16_t low = memory.read(SP++);
        uint16_t high = memory.read(SP++);
        return (high << 8) | low;
    }

    void PUSH_AF() {
        push16((A << 8) | F);
    }

    void POP_AF() {
        uint16_t af = pop16();
        A = af >> 8;
        F = af & 0xFF;
    }

     void ADD_HL_BC() {
        uint32_t result = getHL() + getBC();
        setHL(static_cast<uint16_t>(result));
    }


    void ADD_A_B() {
    uint16_t result = static_cast<uint16_t>(A) + static_cast<uint16_t>(B);
    A = static_cast<uint8_t>(result);
    setZeroFlag(A == 0);
    setSignFlag((A & 0x80) != 0);
    setHalfCarryFlag(A - B, B, A);
    setCarryFlag(A - B, B, result);
    setFlag(N_FLAG, false); // N flag is reset
}

   void ADD_A_HL() {
        uint16_t hlValue = memory.read(getHL());
        uint16_t result = static_cast<uint16_t>(A) + static_cast<uint16_t>(hlValue);
        A = static_cast<uint8_t>(result);
        setZeroFlag(A == 0);
        setSignFlag((A & 0x80) != 0);
        setHalfCarryFlag(A - hlValue, hlValue, A);
        setCarryFlag(result > 0xFF);
        setFlag(N_FLAG, false);
    }

    void ADD_A_C() {
        uint16_t result = static_cast<uint16_t>(A) + static_cast<uint16_t>(C);
        setZeroFlag(static_cast<uint8_t>(result));
        setSignFlag(static_cast<uint8_t>(result));
        setHalfCarryFlag(A, C, static_cast<uint8_t>(result));
        setCarryFlag(A, C, result);
        setParityOverflowFlag(static_cast<uint8_t>(result));
        A = static_cast<uint8_t>(result);
    }

    void ADD_A_D() {
            uint16_t result = static_cast<uint16_t>(A) + static_cast<uint16_t>(D);
            setZeroFlag(static_cast<uint8_t>(result));
            setSignFlag(static_cast<uint8_t>(result));
            setHalfCarryFlag(A, D, static_cast<uint8_t>(result));
            setCarryFlag(A, D, result);
            setParityOverflowFlag(static_cast<uint8_t>(result));
            A = static_cast<uint8_t>(result);
        }

    void ADD_A_E() {
        uint16_t result = static_cast<uint16_t>(A) + static_cast<uint16_t>(E);
        setZeroFlag(static_cast<uint8_t>(result));
        setSignFlag(static_cast<uint8_t>(result));
        setHalfCarryFlag(A, E, static_cast<uint8_t>(result));
        setCarryFlag(A, E, result);
        setParityOverflowFlag(static_cast<uint8_t>(result));
        A = static_cast<uint8_t>(result);
    }

    void ADD_A_H() {
        uint16_t result = static_cast<uint16_t>(A) + static_cast<uint16_t>(H);
        setZeroFlag(static_cast<uint8_t>(result));
        setSignFlag(static_cast<uint8_t>(result));
        setHalfCarryFlag(A, H, static_cast<uint8_t>(result));
        setCarryFlag(A, H, result);
        setParityOverflowFlag(static_cast<uint8_t>(result));
        A = static_cast<uint8_t>(result);
    }

    void ADD_A_L() {
        uint16_t result = static_cast<uint16_t>(A) + static_cast<uint16_t>(L);
        setZeroFlag(static_cast<uint8_t>(result));
        setSignFlag(static_cast<uint8_t>(result));
        setHalfCarryFlag(A, L, static_cast<uint8_t>(result));
        setCarryFlag(A, L, result);
        setParityOverflowFlag(static_cast<uint8_t>(result));
        A = static_cast<uint8_t>(result);
    }

   

    void ADD_A_A() {
        uint16_t result = static_cast<uint16_t>(A) + static_cast<uint16_t>(A);
        setZeroFlag(static_cast<uint8_t>(result));
        setSignFlag(static_cast<uint8_t>(result));
        setHalfCarryFlag(A, A, static_cast<uint8_t>(result));
        setCarryFlag(A, A, result);
        setParityOverflowFlag(static_cast<uint8_t>(result));
        A = static_cast<uint8_t>(result);
    }

    void SUB_n(uint8_t n) {
            uint16_t result = A - n;
            A = static_cast<uint8_t>(result);

        }

    void SUB_B() {
        uint16_t result = static_cast<uint16_t>(A) - static_cast<uint16_t>(B);
        A = static_cast<uint8_t>(result);
        setZeroFlag(A == 0);
        setSignFlag((A & 0x80) != 0);
        setHalfCarryFlag(A, static_cast<uint8_t>(-B), A);
        setCarryFlag(A, static_cast<uint8_t>(-B), result);
        setFlag(N_FLAG, true); // N flag is set
    }


    void SUB_C() {
        uint16_t result = static_cast<uint16_t>(A) - static_cast<uint16_t>(C);
        setZeroFlag(static_cast<uint8_t>(result));
        setSignFlag(static_cast<uint8_t>(result));
        setHalfCarryFlag(A, static_cast<uint8_t>(-C), static_cast<uint8_t>(result));
        setCarryFlag(A, static_cast<uint8_t>(-C), result);
        setParityOverflowFlag(static_cast<uint8_t>(result));
        A = static_cast<uint8_t>(result);
    }


    void ADC_A_B() {
        uint16_t result = static_cast<uint16_t>(A) + static_cast<uint16_t>(B) + (F & C_FLAG);
        setZeroFlag(static_cast<uint8_t>(result));
        setSignFlag(static_cast<uint8_t>(result));
        setHalfCarryFlag(A, B, static_cast<uint8_t>(result));
        setCarryFlag(A, B, result);
        setParityOverflowFlag(static_cast<uint8_t>(result));
        A = static_cast<uint8_t>(result);
    }

    void ADC_A_C() {
        uint16_t result = static_cast<uint16_t>(A) + static_cast<uint16_t>(C) + (F & C_FLAG);
        setZeroFlag(static_cast<uint8_t>(result));
        setSignFlag(static_cast<uint8_t>(result));
        setHalfCarryFlag(A, C, static_cast<uint8_t>(result));
        setCarryFlag(A, C, result);
        setParityOverflowFlag(static_cast<uint8_t>(result));
        A = static_cast<uint8_t>(result);
    }

    void ADC_A_E() {
    uint16_t result = static_cast<uint16_t>(A) + static_cast<uint16_t>(E) + (F & C_FLAG ? 1 : 0);

    // Set flags based on the result
    setZeroFlag(static_cast<uint8_t>(result) == 0); // Set if result is zero
    setSignFlag(result & 0x80); // Set if bit 7 of the result is set
    setHalfCarryFlag((A & 0x0F) + (E & 0x0F) + (F & C_FLAG ? 1 : 0) > 0x0F); 
    setParityOverflowFlag(((A ^ ~E) & (A ^ result)) & 0x80);
    setCarryFlag(result > 0xFF); 

    A = static_cast<uint8_t>(result);
}

    void XOR_n(uint8_t n) {
        A ^= n;
        }

    void XOR_A() {
        A ^= A;
        setZeroFlag(A == 0);
        setSignFlag(false); // XOR with itself always results in 0
        setHalfCarryFlag(false);
        setCarryFlag(false);
        setParityOverflowFlag(A == 0); // Parity is true if result is zero
    }

    void OR_A() {
        A |= A;
        setZeroFlag(A == 0);
        setSignFlag(false); // OR with itself does not change the sign
        setHalfCarryFlag(false);
        setCarryFlag(false);
        setParityOverflowFlag(A == 0); // Parity is true if result is zero
    }

     void OR_C() {
        A |= C;
    }

    void AND_A() {
    A &= A;
    setZeroFlag(A == 0);
    setSignFlag((A & 0x80) != 0);
    setHalfCarryFlag(true);
    setCarryFlag(false);
    setParityOverflowFlag(A == 0); // Parity is true if result is zero
    }
    void AND_B() {
            A &= B;
        }

    void CP_A() {
        // Comparing A with itself always results in zero
        setZeroFlag(true);
        setSignFlag(false);
        setHalfCarryFlag(false);
        setCarryFlag(false);
        setFlag(N_FLAG, true); // CP instructions set N flag
    }

    void CP_D() {
    uint8_t result = A - D;
    setFlagsForCompare(result, D);
}

    void CP_HL() {
            uint8_t value = memory.read(getHL());
            uint16_t result = static_cast<uint16_t>(A) - static_cast<uint16_t>(value);
            setZeroFlag(static_cast<uint8_t>(result) == 0);
            setSignFlag((result & 0x80) != 0);
            setHalfCarryFlag(A, value, static_cast<uint8_t>(result));
            setCarryFlag(A, value, result);
            setFlag(N_FLAG, true);
        }  

    void CP_n(uint8_t n) {
    uint8_t result = A - n;
    setFlagsForCompare(result, n);
}


        void LD_HL_n() {
        uint8_t value = fetch();
        memory.write(getHL(), value);
        }

        void LD_BC_A() {
            memory.write(getBC(), A);
        }

        void LD_DE_A() {
            memory.write(getDE(), A);
        }

        void LD_HL_A() {
            memory.write(getHL(), A);
        }

        void LD_HL_B() {
            memory.write(getHL(), B);
        }

      void LD_BC_nn() {
    uint16_t nn = fetch16();
    setBC(nn);
    }

    void LD_DE_nn() {
        uint16_t nn = fetch16();
        setDE(nn);
    }

    void LD_HL_nn() {
        uint16_t nn = fetch16();
        setHL(nn);
    }


        void LD_SP_nn() {
            SP = fetch16();
        }

        void LD_A_HLi() {
            A = memory.read(HL++);
        }

        void LD_HLd_A() {
            memory.write(HL--, A);
        }

        void LD_A_HLd() {
            A = memory.read(HL--);
        }

        void LD_B_B() {
        }

        void LD_B_C() {
            B = C; /
        }

        void LD_B_D() {
            B = D; 
        }

        void LD_C_n() {
        C = fetch(); 
        }

        void LD_D_n() {
            D = fetch(); 
        }

        void LD_E_n() {
            E = fetch(); 
        }

        void LD_H_n() {
            H = fetch(); 
        }

        void LD_L_n() {
            L = fetch(); 
        }

          void BIT_0_B() {
        setZeroFlag((B & (1 << 0)) == 0);
        }

       void BIT_b_r(uint8_t bit, uint8_t& reg) {
    bool bitSet = reg & (1 << bit);
    setZeroFlag(!bitSet);
    setFlag(N_FLAG, false);
    setFlag(H_FLAG, true);
}

        void BIT_3_E() {
        }
        void SET_2_H() {
        H |= (1 << 2);
    }

        void SET_b_r(uint8_t bit, uint8_t& reg) {
    
            }

        void SET_0_B() {
                B |= (1 << 0);
            }


        void SET_7_H() {
                H |= (1 << 7);
            }
        void RES_7_L() {
        L &= ~(1 << 7);
        }
        void RES_0_B() {
            B &= ~(1 << 0);
        }

        void RES_b_r(uint8_t bit, uint8_t& reg) {
    
        }
        void RES_1_L() {
            L &= ~(1 << 1);
        }

        void JP_Z_nn() {
        uint16_t address = fetch16();
        if (F & Z_FLAG) {
            PC = address;
        }
            }
        void JP_nn() {
                uint16_t address = fetch16();  
                PC = address;
            }
        void JR_n(int8_t offset) {
        PC += offset;
        }

        void JR_e() {
            int8_t offset = static_cast<int8_t>(fetch());
            PC += offset;
        }

        void JR_NZ_e() {
            int8_t offset = static_cast<int8_t>(fetch());
            if (!(F & Z_FLAG)) {
                PC += offset;
            }
        }

        void JR_Z_e() {
            int8_t offset = static_cast<int8_t>(fetch());
            if (F & Z_FLAG) {
                PC += offset;
            }
        }

        void JR_NC_e() {
            int8_t offset = static_cast<int8_t>(fetch());
            if (!(F & C_FLAG)) {
                PC += offset;
            }
        }

        void JR_C_e() {
            int8_t offset = static_cast<int8_t>(fetch());
            if (F & C_FLAG) {
                PC += offset;
            }
        }
         void RET() {
        PC = pop16();
        }

        void RET_NZ() {
            if (!(F & Z_FLAG)) {
                PC = pop16();
            }
        }

        void INC_B() {
            B++;
            setZeroFlag(B == 0);
            setSignFlag((B & 0x80) != 0);
            setHalfCarryFlag(B - 1, 1, B); /
            setFlag(N_FLAG, false); 
        }
        void INC_C() {
                C++;
                setZeroFlag(C == 0);
                setSignFlag((C & 0x80) != 0);
                setHalfCarryFlag(C - 1, 1, C); 
                setFlag(N_FLAG, false);
            }
        void INC_DE() {
                DE++;
            }
        void INC_DE() { 
            DE++;
        }
        void INC_HL() { 
                HL++;
            }

        void INC_A() { 
                A++;
                setZeroFlag(A == 0);
                setSignFlag((A & 0x80) != 0);
                setHalfCarryFlag((A & 0x0F) == 0); 
                setParityOverflowFlag(A == 0x80); 
            }
        void DEC_B() {
            B--;
            setZeroFlag(B == 0);
            setSignFlag((B & 0x80) != 0);
            setHalfCarryFlag(1, B, B); 
            setFlag(N_FLAG, true);
            }
        void DEC_HL() {
                HL--;
            }
        void DEC_C() {
            C--;
            setZeroFlag(C == 0);
            setSignFlag((C & 0x80) != 0);
            setHalfCarryFlag(1, C, C); 
            setFlag(N_FLAG, true); 
        }
        void DEC_DE() { 
                DE--;
                
            }
        void DEC_A() { 
                A--;
                setZeroFlag(A == 0);
                setSignFlag((A & 0x80) != 0);
                setHalfCarryFlag((A & 0x0F) == 0x0F); 
                setFlag(N_FLAG, true); 
                setParityOverflowFlag(A == 0x7F); 
            }

        void RLC_A() {
        bool carry = (A & 0x80) != 0;
        A = (A << 1) | (carry ? 1 : 0);
        setZeroFlag(A == 0);
        setSignFlag((A & 0x80) != 0);
        setHalfCarryFlag(false);
        setCarryFlag(carry);
     }
        void RLCA() {
        }

        void RRCA() {
        bool carry = (A & 0x01) != 0;
        A = (A >> 1) | (carry ? 0x80 : 0);
        setZeroFlag(A == 0);
        setSignFlag((A & 0x80) != 0);
        setHalfCarryFlag(false);
        setCarryFlag(carry);    }
        void RLA() {
        bool oldCarry = (F & C_FLAG) != 0;
        bool newCarry = (A & 0x80) != 0;
        A = (A << 1) | (oldCarry ? 1 : 0);
        setZeroFlag(false); 
        setSignFlag(false); 
        setHalfCarryFlag(false); 
        setCarryFlag(newCarry);
    }
        void RRA() {
        bool oldCarry = (F & C_FLAG) != 0;
        bool newCarry = (A & 0x01) != 0;
        A = (A >> 1) | (oldCarry ? 0x80 : 0);
        setZeroFlag(false); 
        setSignFlag(false); 
        setHalfCarryFlag(false); 
        setCarryFlag(newCarry);
    }
   
   void handleInterrupts() {
    if (!interruptsEnabled) return;

    if (interruptRequest) {
        interruptRequest = false;
        push16(PC);

        switch (interruptMode) {
            case 0:
                break;
            case 1:
                PC = 0x0038;
                break;
            case 2:
                uint16_t vectorAddress = ((static_cast<uint16_t>(I) << 8) | interruptVector) & 0xFF;
                PC = memory.read16(vectorAddress);
                break;
        }
    }

}


void handleCBPrefix() {
    uint8_t opcode = fetch();
    switch (opcode) {
        default:
            unimplementedOpcode();
            break;
    }
}



};


