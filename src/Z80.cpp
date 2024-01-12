#include "Memory.h"
#include "Z80.h"
#include <cstdint>
#include <array>
#include <stdexcept>
#include <iostream>

// Constructor
Z80::Z80() : memory() {
    InitialiseRegisters();
    InitialiseOpcodeTable();
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

// Execute next instruction
void Z80::executeNextInstruction() {
    if (halted) return;

    uint8_t opcode = fetch();

    if (opcode == 0xCB || opcode == 0xDD || opcode == 0xED || opcode == 0xFD) {
        handleExtendedOpcode(opcode);
    } else {
        auto handler = opcodeTable[opcode];
        if (handler) {
            handler();
        } else {
            unimplementedOpcode();
        }
    }
}

    void Z80::setFlagsForAddition(uint8_t a, uint8_t b, uint16_t result) {
        F = 0;
        if ((result & 0x100) != 0) F |= C_FLAG; // Carry flag
        if ((result & 0xFF) == 0) F |= Z_FLAG;  // Zero flag
        if (result & 0x80) F |= S_FLAG;         // Sign flag
        if (((a ^ b ^ 0x80) & (a ^ result) & 0x80) != 0) F |= PV_FLAG; // Parity/Overflow flag
        if (((a & 0x0F) + (b & 0x0F) + (F & C_FLAG ? 1 : 0)) & 0x10) F |= H_FLAG; // Half-carry flag
    }

    // Helper function to set flags for SUB operations
    void Z80::setFlagsForSubtraction(uint8_t a, uint8_t b) {
        uint16_t result = a - b;
        F = N_FLAG; // SUB operations always set N_FLAG
        if ((result & 0xFF) == 0) F |= Z_FLAG;  // Zero flag
        if (result & 0x80) F |= S_FLAG;         // Sign flag
        if ((a ^ b) & (a ^ result) & 0x80) F |= PV_FLAG; // Parity/Overflow flag
        if ((a & 0x0F) < (b & 0x0F)) F |= H_FLAG; // Half-carry flag
        if (result & 0x100) F |= C_FLAG;        // Carry flag
    }

     // Helper function to perform SBC operation with a given register
        void Z80::SBC_A_Register(uint8_t value) {
            uint16_t result = A - value - (F & C_FLAG ? 1 : 0);
            setFlagsForSubtraction(A, value + (F & C_FLAG ? 1 : 0));
            A = static_cast<uint8_t>(result);
        }

        // Helper function to perform AND operation with a given register
        void Z80::AND_A_Register(uint8_t value) {
            A &= value;
            setFlagsForLogicalOperation(A);
        }
        
          // Function to calculate parity of the result
        uint8_t Z80::parity(uint8_t value) const {
            value ^= value >> 4;
            value ^= value >> 2;
            value ^= value >> 1;
            return (value & 1) ? 0 : PV_FLAG;
        }

        // Helper function to set flags for AND operations
        void Z80::setFlagsForLogicalOperation(uint8_t result) {
            F = result ? 0 : Z_FLAG; // Zero flag
            F |= H_FLAG;             // AND operations always set H_FLAG
            if (result & 0x80) F |= S_FLAG; // Sign flag
            F &= ~N_FLAG;            // Reset N_FLAG
            F &= ~C_FLAG;            // Reset C_FLAG
            F |= parity(result);     // Parity flag
        }

        // Helper function to update flags after logical operations
        void Z80::updateLogicalFlags(uint8_t result) {
            // Reset N and C flags
            F &= ~(N_FLAG | C_FLAG);

            // Set H flag
            F |= H_FLAG;

            // Update Z and S flags
            F = (result == 0) ? Z_FLAG : 0;
            F |= (result & 0x80) ? S_FLAG : 0;

            // Update P/V flag (parity)
            F |= parity(result) ? PV_FLAG : 0;
        }

        // Helper function to compare a register with the accumulator
        void Z80::compareWithA(uint8_t value) {
            uint8_t result = A - value;

            // Set flags
            F = (result == 0) ? Z_FLAG : 0;  // Zero flag
            F |= (result & S_FLAG);          // Sign flag
            F |= (A < value) ? C_FLAG : 0;   // Carry flag
            F |= N_FLAG;                     // Subtract flag

            // Half-Carry Flag
            F |= ((A & 0x0F) < (value & 0x0F)) ? H_FLAG : 0;

            // Parity/Overflow Flag
            F |= (((A ^ value) & (A ^ result) & 0x80) != 0) ? PV_FLAG : 0;
        }

        // Helper function to increment a register and set flags
        void Z80::incrementRegister(uint8_t& reg) {
            // Increment register
            reg++;

            // Update flags
            F &= C_FLAG; // Keep carry flag, clear others
            F |= (reg == 0) ? Z_FLAG : 0; // Set Zero flag if result is 0
            F |= (reg & S_FLAG); // Set Sign flag based on bit 7 of the result

            // Set Half-Carry Flag
            F |= ((reg & 0x0F) == 0) ? H_FLAG : 0;

            // Parity/Overflow Flag
            F |= (reg == 0x80) ? PV_FLAG : 0; // Overflow occurs if result is 0x80 (128 in decimal)
        }

        // Helper function to decrement a register and set flags
        void Z80::decrementRegister(uint8_t& reg) {
            // Decrement register
            reg--;

            // Update flags
            F &= C_FLAG; // Keep carry flag, clear others
            F |= (reg == 0) ? Z_FLAG : 0; // Set Zero flag if result is 0
            F |= (reg & S_FLAG); // Set Sign flag based on bit 7 of the result

            // Set Half-Carry Flag
            F |= ((reg & 0x0F) == 0x0F) ? H_FLAG : 0;

            // Parity/Overflow Flag
            F |= (reg == 0x7F) ? PV_FLAG : 0; // Overflow occurs if result is 0x7F (127 in decimal)

            // Set N Flag as operation is subtraction
            F |= N_FLAG;
        }

    void Z80::SBC_A_Register(uint8_t value) {
        uint16_t result = A - value - (F & C_FLAG ? 1 : 0);
        setFlagsForSubtraction(A, value + (F & C_FLAG ? 1 : 0));
        A = static_cast<uint8_t>(result);
    }
    
        // Opcode Implementations
        void Z80::NOP() {
           
        }

        void Z80::halt() {
         halted = true;
        }


    void Z80::ADC_A_B() {
        uint16_t result = A + B + (F & C_FLAG ? 1 : 0);
        setFlagsForAddition(A, B, result);
        A = result & 0xFF;
    }

    void Z80::ADC_A_C() {
        uint16_t result = A + C + (F & C_FLAG ? 1 : 0);
        setFlagsForAddition(A, C, result);
        A = result & 0xFF;
    }

    void Z80::ADC_A_D() {
        uint16_t result = A + D + (F & C_FLAG ? 1 : 0);
        setFlagsForAddition(A, D, result);
        A = result & 0xFF;
    }

    void Z80::ADC_A_E() {
        uint16_t result = A + E + (F & C_FLAG ? 1 : 0);
        setFlagsForAddition(A, E, result);
        A = result & 0xFF;
    }

    void Z80::ADC_A_H() {
        uint16_t result = A + H + (F & C_FLAG ? 1 : 0);
        setFlagsForAddition(A, H, result);
        A = result & 0xFF;
    }

    void Z80::ADC_A_L() {
        uint16_t result = A + L + (F & C_FLAG ? 1 : 0);
        setFlagsForAddition(A, L, result);
        A = result & 0xFF;
    }

        void Z80::ADC_A_HL() {
            uint16_t hl_address = (H << 8) | L;
            uint8_t value = memory.read(hl_address);
            A = A + value + (F & C_FLAG ? 1 : 0);
       
        }

        void Z80::ADC_A_A() {
            A = A + A + (F & C_FLAG ? 1 : 0);
           
        }

        void Z80::SUB_B() {
        setFlagsForSubtraction(A, B);
        A -= B;
    }

        void Z80::SUB_C() {
            setFlagsForSubtraction(A, C);
            A -= C;
        }

        void Z80::SUB_D() {
            setFlagsForSubtraction(A, D);
            A -= D;
        }

        void Z80::SUB_E() {
            setFlagsForSubtraction(A, E);
            A -= E;
        }

        void Z80::SUB_H() {
            setFlagsForSubtraction(A, H);
            A -= H;
        }

        void Z80::SUB_L() {
            setFlagsForSubtraction(A, L);
            A -= L;
        }

        void Z80::SUB_HL() {
            uint8_t value = memory.read(getHL());
            setFlagsForSubtraction(A, value);
            A -= value;
        }

        void Z80::SUB_A() {
            setFlagsForSubtraction(A, A);
            A = 0;
        }

        void Z80::SBC_A_B() {
            SBC_A_Register(B);
        }

        void Z80::SBC_A_C() {
            SBC_A_Register(C);
        }

        void Z80::SBC_A_D() {
            SBC_A_Register(D);
        }

        void Z80::SBC_A_E() {
            SBC_A_Register(E);
        }

        void Z80::SBC_A_H() {
            SBC_A_Register(H);
        }

        void Z80::SBC_A_L() {
            SBC_A_Register(L);
        }

        void Z80::SBC_A_HL() {
            SBC_A_Register(memory.read(getHL()));
        }

        void Z80::SBC_A_A() {
            SBC_A_Register(A);
        }


        void Z80::AND_B() {
            A &= B;
            updateLogicalFlags(A);
        }

        void Z80::AND_C() {
            A &= C;
            updateLogicalFlags(A);
        }

        void Z80::AND_D() {
            A &= D;
            updateLogicalFlags(A);
        }

        void Z80::AND_E() {
            A &= E;
            updateLogicalFlags(A);
        }

        void Z80::AND_H() {
            A &= H;
            updateLogicalFlags(A);
        }

        void Z80::AND_L() {
            A &= L;
            updateLogicalFlags(A);
        }

        void Z80::AND_HL() {
            A &= memory.read((H << 8) | L);
            updateLogicalFlags(A);
        }

        void Z80::AND_A() {
            updateLogicalFlags(A); // A & A = A, so just update flags based on A
        }

        void Z80::XOR_B() {
            A ^= B;
            updateLogicalFlags(A);
        }

        void Z80::XOR_C() {
            A ^= C;
            updateLogicalFlags(A);
        }

        void Z80::XOR_D() {
            A ^= D;
            updateLogicalFlags(A);
        }

        void Z80::XOR_E() {
            A ^= E;
            updateLogicalFlags(A);
        }

        void Z80::XOR_H() {
            A ^= H;
            updateLogicalFlags(A);
        }

        void Z80::XOR_L() {
            A ^= L;
            updateLogicalFlags(A);
        }

        void Z80::XOR_HL() {
            A ^= memory.read((H << 8) | L);
            updateLogicalFlags(A);
        }

        void Z80::XOR_A() {
            A = 0; // A XOR A is always 0
            updateLogicalFlags(A);
        }

        void Z80::OR_B() {
            A |= B;
            updateLogicalFlags(A);
        }

        void Z80::OR_C() {
            A |= C;
            updateLogicalFlags(A);
        }

        void Z80::OR_D() {
            A |= D;
            updateLogicalFlags(A);
        }

        void Z80::OR_E() {
            A |= E;
            updateLogicalFlags(A);
        }

        void Z80::OR_H() {
            A |= H;
            updateLogicalFlags(A);
        }

        void Z80::OR_L() {
            A |= L;
            updateLogicalFlags(A);
        }

        void Z80::OR_HL() {
            A |= memory.read((H << 8) | L);
            updateLogicalFlags(A);
        }

        void Z80::OR_A() {
            // OR A with itself doesn't change A, but it affects the flags
            updateLogicalFlags(A);
        }

        void Z80::CP_B() {
            compareWithA(B);
        }

        void Z80::CP_C() {
            compareWithA(C);
        }

        void Z80::CP_D() {
            compareWithA(D);
        }

        void Z80::CP_E() {
            compareWithA(E);
        }

        void Z80::CP_H() {
            compareWithA(H);
        }

        void Z80::CP_L() {
            compareWithA(L);
        }

        void Z80::CP_HL() {
            compareWithA(memory.read((H << 8) | L));
        }

        void Z80::CP_A() {
            compareWithA(A);
        }

        void Z80::INC_B() {
            incrementRegister(B);
        }

        void Z80::INC_C() {
            incrementRegister(C);
        }

        void Z80::INC_D() {
            incrementRegister(D);
        }

        void Z80::INC_E() {
            incrementRegister(E);
        }

        void Z80::INC_H() {
            incrementRegister(H);
        }

        void Z80::INC_L() {
            incrementRegister(L);
        }

        void Z80::INC_A() {
            incrementRegister(A);
        }

        void Z80::DEC_B() {
            decrementRegister(B);
        }

        void Z80::DEC_C() {
            decrementRegister(C);
        }

        void Z80::DEC_D() {
            decrementRegister(D);
        }

        void Z80::DEC_E() {
            decrementRegister(E);
        }

        void Z80::DEC_H() {
            decrementRegister(H);
        }

        void Z80::DEC_L() {
            decrementRegister(L);
        }

        void Z80::DEC_A() {
            decrementRegister(A);
        }

        void Z80::LD_B_n() {
            B = fetch();
        }

        void Z80::LD_C_n() {
            C = fetch();
        }

        void Z80::LD_D_n() {
            D = fetch();
        }

        void Z80::LD_E_n() {
            E = fetch();
        }

        void Z80::LD_H_n() {
            H = fetch();
        }

        void Z80::LD_L_n() {
            L = fetch();
        }

        void Z80::LD_HL_n() {
            uint16_t address = getHL();
            uint8_t value = fetch();
            memory.write(address, value);
        }

        void Z80::LD_A_n() {
            A = fetch();
        }

        void Z80::LD_B_C() {
            B = C;
        }

        void Z80::LD_B_D() {
            B = D;
        }

        void Z80::LD_B_E() {
            B = E;
        }

        void Z80::LD_B_H() {
            B = H;
        }

        void Z80::LD_B_L() {
            B = L;
        }

        void Z80::LD_B_HL() {
            uint16_t address = getHL();
            B = memory.read(address);
        }

        void Z80::LD_C_B() {
            C = B;
        }

        void Z80::LD_C_C() {
            // C = C; This is redundant
        }

        void Z80::LD_C_D() {
            C = D;
        }

        void Z80::LD_C_E() {
            C = E;
        }

        void Z80::LD_C_H() {
            C = H;
        }

        void Z80::LD_C_L() {
            C = L;
        }

        void Z80::LD_C_HL() {
            uint16_t address = getHL();
            C = memory.read(address);
        }

        void Z80::LD_A_HL() {
            uint16_t address = getHL();
            A = memory.read(address);
        }

        void Z80::LD_B_HL() {
            uint16_t address = getHL();
            B = memory.read(address);
        }

        void Z80::LD_C_HL() {
            uint16_t address = getHL();
            C = memory.read(address);
        }

        void Z80::LD_D_HL() {
            uint16_t address = getHL();
            D = memory.read(address);
        }

        void Z80::LD_E_HL() {
            uint16_t address = getHL();
            E = memory.read(address);
        }

        void Z80::LD_H_HL() {
            uint16_t address = getHL();
            H = memory.read(address);
        }

        void Z80::LD_L_HL() {
            uint16_t address = getHL();
            L = memory.read(address);
        }

        void Z80::LD_HL_B() {
            uint16_t address = getHL();
            memory.write(address, B);
        }

        void Z80::LD_HL_C() {
            uint16_t address = getHL();
            memory.write(address, C);
        }

        void Z80::LD_HL_D() {
            uint16_t address = getHL();
            memory.write(address, D);
        }

        void Z80::LD_HL_E() {
            uint16_t address = getHL();
            memory.write(address, E);
        }

        void Z80::LD_HL_H() {
            uint16_t address = getHL();
            memory.write(address, H);
        }

        void Z80::LD_HL_L() {
            uint16_t address = getHL();
            memory.write(address, L);
        }

        void Z80::LD_HL_A() {
            uint16_t address = getHL();
            memory.write(address, A);
        }

        void Z80::LD_BC_nn() {
            uint16_t value = fetch16();
            setBC(value);
        }

        void Z80::LD_DE_nn() {
            uint16_t value = fetch16();
            setDE(value);
        }

        void Z80::LD_HL_nn() {
            uint16_t value = fetch16();
            setHL(value);
        }

        void Z80::LD_BC_A() {
            uint16_t address = getBC();
            memory.write(address, A);
        }

        void Z80::LD_DE_A() {
            uint16_t address = getDE();
            memory.write(address, A);
        }

        void Z80::EX_DE_HL() {
            std::swap(D, H);
            std::swap(E, L);
        }

        void Z80::RL_D() {
            uint16_t result = (D << 1) | (F & C_FLAG ? 1 : 0);
            F = (result & 0x100) ? C_FLAG : 0; // Set carry flag if bit 8 is set
            D = result & 0xFF;
            setZeroFlag(D);
            setSignFlag(D);
            setHalfCarryFlag(0, 0, D);
            setParityOverflowFlag(D);
        }

        void Z80::RR_E() {
            uint16_t result = (E >> 1) | (F & C_FLAG ? 0x80 : 0);
            F = (E & 0x01) ? C_FLAG : 0; // Set carry flag if bit 0 is set
            E = result;
            setZeroFlag(E);
            setSignFlag(E);
            setHalfCarryFlag(0, 0, E);
            setParityOverflowFlag(E);
        }

        void Z80::SLA_C() {
            uint16_t result = C << 1;
            F = (result & 0x100) ? C_FLAG : 0; // Set carry flag if bit 8 is set
            C = result & 0xFF;
            setZeroFlag(C);
            setSignFlag(C);
            setHalfCarryFlag(0, 0, C);
            setParityOverflowFlag(C);
        }

        void Z80::SRA_B() {
            F = (B & 0x01) ? C_FLAG : 0; // Set carry flag if bit 0 is set
            B = (B & 0x80) | (B >> 1); // Preserve the sign bit
            setZeroFlag(B);
            setSignFlag(B);
            setHalfCarryFlag(0, 0, B);
            setParityOverflowFlag(B);
        }

        void Z80::RLC_B() {
            F = (B & 0x80) ? C_FLAG : 0; // Set carry flag if bit 7 of B is set
            B = (B << 1) | (B >> 7);
            setZeroFlag(B);
            setSignFlag(B);
            setHalfCarryFlag(0, 0, B);
            setParityOverflowFlag(B);
        }

        void Z80::RLC_C() {
            F = (C & 0x80) ? C_FLAG : 0;
            C = (C << 1) | (C >> 7);
            setZeroFlag(C);
            setSignFlag(C);
            setHalfCarryFlag(0, 0, C);
            setParityOverflowFlag(C);
        }

        void Z80::RLC_D() {
            F = (D & 0x80) ? C_FLAG : 0;
            D = (D << 1) | (D >> 7);
            setZeroFlag(D);
            setSignFlag(D);
            setHalfCarryFlag(0, 0, D);
            setParityOverflowFlag(D);
        }

        void Z80::RLC_E() {
            F = (E & 0x80) ? C_FLAG : 0;
            E = (E << 1) | (E >> 7);
            setZeroFlag(E);
            setSignFlag(E);
            setHalfCarryFlag(0, 0, E);
            setParityOverflowFlag(E);
        }

        void Z80::RLC_H() {
            F = (H & 0x80) ? C_FLAG : 0;
            H = (H << 1) | (H >> 7);
            setZeroFlag(H);
            setSignFlag(H);
            setHalfCarryFlag(0, 0, H);
            setParityOverflowFlag(H);
        }

        void Z80::RLC_L() {
            F = (L & 0x80) ? C_FLAG : 0;
            L = (L << 1) | (L >> 7);
            setZeroFlag(L);
            setSignFlag(L);
            setHalfCarryFlag(0, 0, L);
            setParityOverflowFlag(L);
        }

        void Z80::RLC_HL() {
            uint16_t address = getHL();
            uint8_t value = memory.read(address);
            F = (value & 0x80) ? C_FLAG : 0;
            value = (value << 1) | (value >> 7);
            memory.write(address, value);
            setZeroFlag(value);
            setSignFlag(value);
            setHalfCarryFlag(0, 0, value);
            setParityOverflowFlag(value);
        }

        void Z80::RRC_B() {
            F = (B & 0x01) ? C_FLAG : 0;
            B = (B >> 1) | (B << 7);
            setZeroFlag(B);
            setSignFlag(B);
            setHalfCarryFlag(0, 0, B);
            setParityOverflowFlag(B);
        }

        void Z80::RRC_C() {
            F = (C & 0x01) ? C_FLAG : 0;
            C = (C >> 1) | (C << 7);
            setZeroFlag(C);
            setSignFlag(C);
            setHalfCarryFlag(0, 0, C);
            setParityOverflowFlag(C);
        }

        void Z80::RRC_D() {
            F = (D & 0x01) ? C_FLAG : 0;
            D = (D >> 1) | (D << 7);
            setZeroFlag(D);
            setSignFlag(D);
            setHalfCarryFlag(0, 0, D);
            setParityOverflowFlag(D);
        }

        void Z80::RRC_E() {
            F = (E & 0x01) ? C_FLAG : 0;
            E = (E >> 1) | (E << 7);
            setZeroFlag(E);
            setSignFlag(E);
            setHalfCarryFlag(0, 0, E);
            setParityOverflowFlag(E);
        }

        void Z80::RRC_H() {
            F = (H & 0x01) ? C_FLAG : 0;
            H = (H >> 1) | (H << 7);
            setZeroFlag(H);
            setSignFlag(H);
            setHalfCarryFlag(0, 0, H);
            setParityOverflowFlag(H);
        }

        void Z80::RRC_L() {
            F = (L & 0x01) ? C_FLAG : 0;
            L = (L >> 1) | (L << 7);
            setZeroFlag(L);
            setSignFlag(L);
            setHalfCarryFlag(0, 0, L);
            setParityOverflowFlag(L);
        }

        void Z80::RRC_HL() {
            uint16_t address = getHL();
            uint8_t value = memory.read(address);
            F = (value & 0x01) ? C_FLAG : 0;
            value = (value >> 1) | (value << 7);
            memory.write(address, value);
            setZeroFlag(value);
            setSignFlag(value);
            setHalfCarryFlag(0, 0, value);
            setParityOverflowFlag(value);
        }

        void Z80::RL_B() {
            uint8_t carry = F & C_FLAG;
            F = (B & 0x80) ? C_FLAG : 0;
            B = (B << 1) | carry;
            setZeroFlag(B);
            setSignFlag(B);
            setHalfCarryFlag(0, 0, B);
            setParityOverflowFlag(B);
        }

        void Z80::RL_C() {
            uint8_t carry = F & C_FLAG;
            F = (C & 0x80) ? C_FLAG : 0;
            C = (C << 1) | carry;
            setZeroFlag(C);
            setSignFlag(C);
            setHalfCarryFlag(0, 0, C);
            setParityOverflowFlag(C);
        }

        void Z80::RL_E() {
            uint8_t carry = F & C_FLAG;
            F = (E & 0x80) ? C_FLAG : 0;
            E = (E << 1) | carry;
            setZeroFlag(E);
            setSignFlag(E);
            setHalfCarryFlag(0, 0, E);
            setParityOverflowFlag(E);
        }

        void Z80::RL_H() {
            uint8_t carry = F & C_FLAG;
            F = (H & 0x80) ? C_FLAG : 0;
            H = (H << 1) | carry;
            setZeroFlag(H);
            setSignFlag(H);
            setHalfCarryFlag(0, 0, H);
            setParityOverflowFlag(H);
        }

        void Z80::RL_L() {
            uint8_t carry = F & C_FLAG;
            F = (L & 0x80) ? C_FLAG : 0;
            L = (L << 1) | carry;
            setZeroFlag(L);
            setSignFlag(L);
            setHalfCarryFlag(0, 0, L);
            setParityOverflowFlag(L);
        }

        void Z80::RL_HL() {
            uint16_t address = getHL();
            uint8_t value = memory.read(address);
            uint8_t carry = F & C_FLAG;
            F = (value & 0x80) ? C_FLAG : 0;
            value = (value << 1) | carry;
            memory.write(address, value);
            setZeroFlag(value);
            setSignFlag(value);
            setHalfCarryFlag(0, 0, value);
            setParityOverflowFlag(value);
        }

        void Z80::RR_B() {
            uint8_t carry = F & C_FLAG;
            F = (B & 0x01) ? C_FLAG : 0;
            B = (B >> 1) | (carry << 7);
            setZeroFlag(B);
            setSignFlag(B);
            setHalfCarryFlag(0, 0, B);
            setParityOverflowFlag(B);
        }

        void Z80::RR_C() {
            uint8_t carry = F & C_FLAG;
            F = (C & 0x01) ? C_FLAG : 0;
            C = (C >> 1) | (carry << 7);
            setZeroFlag(C);
            setSignFlag(C);
            setHalfCarryFlag(0, 0, C);
            setParityOverflowFlag(C);
        }

        void Z80::RR_D() {
            uint8_t carry = F & C_FLAG;
            F = (D & 0x01) ? C_FLAG : 0;
            D = (D >> 1) | (carry << 7);
            setZeroFlag(D);
            setSignFlag(D);
            setHalfCarryFlag(0, 0, D);
            setParityOverflowFlag(D);
        }

        void Z80::RR_H() {
            uint8_t carry = F & C_FLAG;
            F = (H & 0x01) ? C_FLAG : 0;
            H = (H >> 1) | (carry << 7);
            setZeroFlag(H);
            setSignFlag(H);
            setHalfCarryFlag(0, 0, H);
            setParityOverflowFlag(H);
        }

        void Z80::RR_L() {
            uint8_t carry = F & C_FLAG;
            F = (L & 0x01) ? C_FLAG : 0;
            L = (L >> 1) | (carry << 7);
            setZeroFlag(L);
            setSignFlag(L);
            setHalfCarryFlag(0, 0, L);
            setParityOverflowFlag(L);
        }

        void Z80::RR_HL() {
            uint16_t address = getHL();
            uint8_t value = memory.read(address);
            uint8_t carry = F & C_FLAG;
            F = (value & 0x01) ? C_FLAG : 0;
            value = (value >> 1) | (carry << 7);
            memory.write(address, value);
            setZeroFlag(value);
            setSignFlag(value);
            setHalfCarryFlag(0, 0, value);
            setParityOverflowFlag(value);
        }

        void Z80::SLA_B() {
            F = (B & 0x80) ? C_FLAG : 0;
            B <<= 1;
            setZeroFlag(B);
            setSignFlag(B);
            setHalfCarryFlag(0, 0, B);
            setParityOverflowFlag(B);
        }

        void Z80::SLA_D() {
            F = (D & 0x80) ? C_FLAG : 0;
            D <<= 1;
            setZeroFlag(D);
            setSignFlag(D);
            setHalfCarryFlag(0, 0, D);
            setParityOverflowFlag(D);
        }

        void Z80::SLA_E() {
            F = (E & 0x80) ? C_FLAG : 0;
            E <<= 1;
            setZeroFlag(E);
            setSignFlag(E);
            setHalfCarryFlag(0, 0, E);
            setParityOverflowFlag(E);
        }

        void Z80::SLA_H() {
            F = (H & 0x80) ? C_FLAG : 0;
            H <<= 1;
            setZeroFlag(H);
            setSignFlag(H);
            setHalfCarryFlag(0, 0, H);
            setParityOverflowFlag(H);
        }

        void Z80::SLA_L() {
            F = (L & 0x80) ? C_FLAG : 0;
            L <<= 1;
            setZeroFlag(L);
            setSignFlag(L);
            setHalfCarryFlag(0, 0, L);
            setParityOverflowFlag(L);
        }

        void Z80::SLA_HL() {
            uint16_t address = getHL();
            uint8_t value = memory.read(address);
            F = (value & 0x80) ? C_FLAG : 0;
            value <<= 1;
            memory.write(address, value);
            setZeroFlag(value);
            setSignFlag(value);
            setHalfCarryFlag(0, 0, value);
            setParityOverflowFlag(value);
        }

        void Z80::SLA_A() {
            F = (A & 0x80) ? C_FLAG : 0;
            A <<= 1;
            setZeroFlag(A);
            setSignFlag(A);
            setHalfCarryFlag(0, 0, A);
            setParityOverflowFlag(A);
        }

        void Z80::SRA_C() {
            F = (C & 0x01) ? C_FLAG : 0;
            C = (C >> 1) | (C & 0x80);
            setZeroFlag(C);
            setSignFlag(C);
            setHalfCarryFlag(0, 0, C);
            setParityOverflowFlag(C);
        }

        void Z80::SRA_D() {
            F = (D & 0x01) ? C_FLAG : 0;
            D = (D >> 1) | (D & 0x80);
            setZeroFlag(D);
            setSignFlag(D);
            setHalfCarryFlag(0, 0, D);
            setParityOverflowFlag(D);
        }

        void Z80::SRA_E() {
            F = (E & 0x01) ? C_FLAG : 0;
            E = (E >> 1) | (E & 0x80);
            setZeroFlag(E);
            setSignFlag(E);
            setHalfCarryFlag(0, 0, E);
            setParityOverflowFlag(E);
        }

        void Z80::SRA_H() {
            F = (H & 0x01) ? C_FLAG : 0;
            H = (H >> 1) | (H & 0x80);
            setZeroFlag(H);
            setSignFlag(H);
            setHalfCarryFlag(0, 0, H);
            setParityOverflowFlag(H);
        }

        void Z80::SRA_L() {
            F = (L & 0x01) ? C_FLAG : 0;
            L = (L >> 1) | (L & 0x80);
            setZeroFlag(L);
            setSignFlag(L);
            setHalfCarryFlag(0, 0, L);
            setParityOverflowFlag(L);
        }

        void Z80::SRA_HL() {
            uint16_t address = getHL();
            uint8_t value = memory.read(address);
            F = (value & 0x01) ? C_FLAG : 0;
            value = (value >> 1) | (value & 0x80);
            memory.write(address, value);
            setZeroFlag(value);
            setSignFlag(value);
            setHalfCarryFlag(0, 0, value);
            setParityOverflowFlag(value);
        }

        void Z80::SRA_A() {
            F = (A & 0x01) ? C_FLAG : 0;
            A = (A >> 1) | (A & 0x80);
            setZeroFlag(A);
            setSignFlag(A);
            setHalfCarryFlag(0, 0, A);
            setParityOverflowFlag(A);
        }

        void Z80::SLL_B() {
            F = (B & 0x80) ? C_FLAG : 0;
            B <<= 1;
            setZeroFlag(B);
            setSignFlag(B);
            setHalfCarryFlag(0, 0, B);
            setParityOverflowFlag(B);
        }

        void Z80::SLL_C() {
            F = (C & 0x80) ? C_FLAG : 0;
            C <<= 1;
            setZeroFlag(C);
            setSignFlag(C);
            setHalfCarryFlag(0, 0, C);
            setParityOverflowFlag(C);
        }

        void Z80::SLL_D() {
            F = (D & 0x80) ? C_FLAG : 0;
            D <<= 1;
            setZeroFlag(D);
            setSignFlag(D);
            setHalfCarryFlag(0, 0, D);
            setParityOverflowFlag(D);
        }

        void Z80::SLL_E() {
            F = (E & 0x80) ? C_FLAG : 0;
            E <<= 1;
            setZeroFlag(E);
            setSignFlag(E);
            setHalfCarryFlag(0, 0, E);
            setParityOverflowFlag(E);
        }

        void Z80::SLL_H() {
            F = (H & 0x80) ? C_FLAG : 0;
            H <<= 1;
            setZeroFlag(H);
            setSignFlag(H);
            setHalfCarryFlag(0, 0, H);
            setParityOverflowFlag(H);
        }

        void Z80::SLL_L() {
            F = (L & 0x80) ? C_FLAG : 0;
            L <<= 1;
            setZeroFlag(L);
            setSignFlag(L);
            setHalfCarryFlag(0, 0, L);
            setParityOverflowFlag(L);
        }

        void Z80::SLL_HL() {
            uint16_t address = getHL();
            uint8_t value = memory.read(address);
            F = (value & 0x80) ? C_FLAG : 0;
            value <<= 1;
            memory.write(address, value);
            setZeroFlag(value);
            setSignFlag(value);
            setHalfCarryFlag(0, 0, value);
            setParityOverflowFlag(value);
        }

         void Z80::SLL_A() {
            F = (A & 0x80) ? C_FLAG : 0;
            A <<= 1;
            setZeroFlag(A);
            setSignFlag(A);
            setHalfCarryFlag(0, 0, A);
            setParityOverflowFlag(A);
        }

            void Z80::SRL_B() {
            F = (B & 0x01) ? C_FLAG : 0;
            B >>= 1;
            setZeroFlag(B);
            setSignFlag(B);
            setHalfCarryFlag(0, 0, B);
            setParityOverflowFlag(B);
        }

        void Z80::SRL_C() {
            F = (C & 0x01) ? C_FLAG : 0;
            C >>= 1;
            setZeroFlag(C);
            setSignFlag(C);
            setHalfCarryFlag(0, 0, C);
            setParityOverflowFlag(C);
        }

        void Z80::SRL_D() {
            F = (D & 0x01) ? C_FLAG : 0;
            D >>= 1;
            setZeroFlag(D);
            setSignFlag(D);
            setHalfCarryFlag(0, 0, D);
            setParityOverflowFlag(D);
        }

        void Z80::SRL_E() {
            F = (E & 0x01) ? C_FLAG : 0;
            E >>= 1;
            setZeroFlag(E);
            setSignFlag(E);
            setHalfCarryFlag(0, 0, E);
            setParityOverflowFlag(E);
        }

        void Z80::SRL_H() {
            F = (H & 0x01) ? C_FLAG : 0;
            H >>= 1;
            setZeroFlag(H);
            setSignFlag(H);
            setHalfCarryFlag(0, 0, H);
            setParityOverflowFlag(H);
        }

        void Z80::SRL_L() {
            F = (L & 0x01) ? C_FLAG : 0;
            L >>= 1;
            setZeroFlag(L);
            setSignFlag(L);
            setHalfCarryFlag(0, 0, L);
            setParityOverflowFlag(L);
        }

        void Z80::SRL_HL() {
            uint16_t address = getHL();
            uint8_t value = memory.read(address);
            F = (value & 0x01) ? C_FLAG : 0;
            value >>= 1;
            memory.write(address, value);
            setZeroFlag(value);
            setSignFlag(value);
            setHalfCarryFlag(0, 0, value);
            setParityOverflowFlag(value);
        }

        void Z80::SRL_A() {
            F = (A & 0x01) ? C_FLAG : 0;
            A >>= 1;
            setZeroFlag(A);
            setSignFlag(A);
            setHalfCarryFlag(0, 0, A);
            setParityOverflowFlag(A);
        }

        void Z80::BIT_3_E() {
            F = (E & 0x08) ? 0 : Z_FLAG;
            setHalfCarryFlag(0, 0, F);
        }

        void Z80::BIT_5_H() {
            F = (H & 0x20) ? 0 : Z_FLAG;
            setHalfCarryFlag(0, 0, F);
        }

        void Z80::SET_6_L() {
            L |= 0x40;
        }

        void Z80::SET_2_H() {
            H |= 0x04;
        }

        void Z80::RES_7_A() {
            A &= ~0x80;
        }

        void Z80::RES_1_L() {
            L &= ~0x02;
        }

        void Z80::BIT_4_C() {
            F = (C & 0x10) ? 0 : Z_FLAG;
            setHalfCarryFlag(0, 0, F);
        }

        void Z80::RES_2_D() {
            D &= ~0x04;
        }

        void Z80::SET_3_E() {
            E |= 0x08;
        }


        void Z80::DI() {
            interruptsEnabled = false;
        }

        void Z80::EI() {
            interruptsEnabled = true;
        }

        void Z80::JR_e() {
            int8_t offset = static_cast<int8_t>(fetch());
            PC += offset;
        }

        void Z80::JR_NZ_e() {
            if (!(F & Z_FLAG)) {
                JR_e();
            }
        }

        void Z80::JR_Z_e() {
            if (F & Z_FLAG) {
                JR_e();
            }
        }

        void Z80::JR_NC_e() {
            if (!(F & C_FLAG)) {
                JR_e();
            }
        }

        void Z80::JR_C_e() {
            if (F & C_FLAG) {
                JR_e();
            }
        }

        void Z80::DJNZ() {
            B--;
            if (B != 0) {
                JR_e();
            }
        }

        void Z80::JP_HL() {
            PC = getHL();
        }

        void Z80::JP_nn() {
            uint16_t address = fetch16();
            PC = address;
        }

        void Z80::JP_Z_nn() {
            if (F & Z_FLAG) {
                JP_nn();
            } else {
                PC += 2; // Skip over the two bytes of the nn address
            }
        }

        void Z80::JP_NC_nn() {
            if (!(F & C_FLAG)) {
                JP_nn();
            } else {
                PC += 2;
            }
        }

        void Z80::JP_PO_nn() {
            if (!(F & PV_FLAG)) {
                JP_nn();
            } else {
                PC += 2;
            }
        }

        void Z80::CALL_Z_nn() {
            if (F & Z_FLAG) {
                CALL_nn();
            } else {
                PC += 2;
            }
        }

        void Z80::CALL_NZ_nn() {
            if (!(F & Z_FLAG)) {
                CALL_nn();
            } else {
                PC += 2;
            }
        }

        void Z80::CALL_nn() {
            uint16_t address = fetch16();
            push16(PC);
            PC = address;
        }

        void Z80::CALL_PE_nn() {
            if (F & PV_FLAG) {
                CALL_nn();
            } else {
                PC += 2;
            }
        }

        void Z80::RET() {
            PC = pop16();
        }

        void Z80::RET_NZ() {
            if (!(F & Z_FLAG)) {
                RET();
            }
        }

        void Z80::RET_Z() {
            if (F & Z_FLAG) {
                RET();
            }
        }

        void Z80::RET_P() {
            if (!(F & S_FLAG)) { 
                RET();
            }
        }

        void Z80::RST_00H() {
            push16(PC);
            PC = 0x0000;
        }

        void Z80::RST_08H() {
            push16(PC);
            PC = 0x0008;
        }

        void Z80::RST_10H() {
            push16(PC);
            PC = 0x0010;
        }

        void Z80::RST_18H() {
            push16(PC);
            PC = 0x0018;
        }

        void Z80::RST_20H() {
            push16(PC);
            PC = 0x0020;
        }

        void Z80::RST_28H() {
            push16(PC);
            PC = 0x0028;
        }

        void Z80::RST_30H() {
            push16(PC);
            PC = 0x0030;
        }

        void Z80::RST_38H() {
            push16(PC);
            PC = 0x0038;
        }

        void Z80::PUSH_BC() {
            push16((B << 8) | C);
        }

        void Z80::POP_DE() {
            uint16_t value = pop16();
            D = value >> 8;
            E = value & 0xFF;
        }

        void Z80::PUSH_DE() {
            push16((D << 8) | E);
        }

        void Z80::PUSH_HL() {
            push16((H << 8) | L);
        }

        void Z80::PUSH_AF() {
            push16((A << 8) | F);
        }

        void Z80::POP_BC() {
            uint16_t value = pop16();
            B = value >> 8;
            C = value & 0xFF;
        }

        void Z80::POP_HL() {
            uint16_t value = pop16();
            H = value >> 8;
            L = value & 0xFF;
        }

        void Z80::POP_AF() {
            uint16_t value = pop16();
            A = value >> 8;
            F = value & 0xFF;
        }

        void Z80::ADD_HL_BC() {
            uint16_t hl = (H << 8) | L;
            uint16_t bc = (B << 8) | C;
            uint32_t result = hl + bc;

            H = (result >> 8) & 0xFF;
            L = result & 0xFF;

            setFlag(H_FLAG, ((hl ^ bc ^ (result & 0xFFFF)) & 0x1000) != 0);
            setFlag(C_FLAG, result > 0xFFFF);
        }

       void Z80::ADD_HL_DE() {
            uint16_t hl = (H << 8) | L;
            uint16_t de = (D << 8) | E;
            uint32_t result = hl + de;
            
            H = (result >> 8) & 0xFF;
            L = result & 0xFF;

            setFlag(H_FLAG, ((hl ^ de ^ (result & 0xFFFF)) & 0x1000) != 0);
            setFlag(C_FLAG, result > 0xFFFF);
        }


        void Z80::ADD_HL_SP() {
            uint16_t hl = (H << 8) | L;
            uint32_t result = hl + SP;
            
            H = (result >> 8) & 0xFF;
            L = result & 0xFF;

            setFlag(H_FLAG, ((hl ^ SP ^ (result & 0xFFFF)) & 0x1000) != 0);
            setFlag(C_FLAG, result > 0xFFFF);
        }


       void Z80::ADD_HL_HL() {
            uint16_t hl = (H << 8) | L;
            uint32_t result = hl * 2;
            
            H = (result >> 8) & 0xFF;
            L = result & 0xFF;

            setFlag(H_FLAG, ((hl ^ hl ^ (result & 0xFFFF)) & 0x1000) != 0);
            setFlag(C_FLAG, result > 0xFFFF);
        }


        void Z80::LDI_HL() {
            // Increment HL
            uint16_t hl = (H << 8) | L;
            hl++;
            H = (hl >> 8) & 0xFF;
            L = hl & 0xFF;
        }

        void Z80::DEC_HL() {
            // Decrement HL
            uint16_t hl = (H << 8) | L;
            hl--;
            H = (hl >> 8) & 0xFF;
            L = hl & 0xFF;
        }

       void Z80::INC_HLm() {
            uint16_t hl = (H << 8) | L;
            uint8_t value = memory.read(hl);
            uint8_t result = value + 1;

            memory.write(hl, result);

            setFlag(H_FLAG, ((value ^ 1 ^ result) & 0x10) != 0);
        }

        void Z80::DEC_HLm() {
            uint16_t hl = (H << 8) | L;
            uint8_t value = memory.read(hl);
            uint8_t result = value - 1;

            memory.write(hl, result);

            setFlag(H_FLAG, ((value ^ 0xFF ^ result) & 0x10) != 0);
        }

        uint8_t Z80::getA() const {
    return A;
}

        uint8_t Z80::getB() const {
            return B;
        }

        uint8_t Z80::getC() const {
            return C;
        }

        uint8_t Z80::getD() const {
            return D;
        }

        uint8_t Z80::getE() const {
            return E;
        }

        uint8_t Z80::getH() const {
            return H;
        }

        uint8_t Z80::getL() const {
            return L;
        }

        uint16_t Z80::getSP() const {
            return SP;
        }

        uint16_t Z80::getPC() const {
            return PC;
        }

      void Z80::InitialiseRegisters() {
        A = B = C = D = E = H = L = F = I = R = 0;
        SP = PC = 0;
        halted = interruptsEnabled = false;
    }

        void Z80::InitialiseOpcodeTable() {
        // Initialise all opcodes to a default handler (can be a NOP or unimplemented instruction handler)
        for (auto &opcode_handler : opcodeTable) {
            opcode_handler = [this]() { this->unimplementedOpcode(); };

        }

        // Basic Operations
        opcodeTable[0x00] = [this]() { NOP(); };
        opcodeTable[0x76] = [this]() { halt(); };

        // Arithmetic and Logical Operations
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
        opcodeTable[0x41] = [this]() { LD_B_C(); };
        opcodeTable[0x42] = [this]() { LD_B_D(); };
        opcodeTable[0x43] = [this]() { LD_B_E(); };
        opcodeTable[0x4A] = [this]() { LD_C_D(); };
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
        opcodeTable[0x77] = [this]() { LD_HL_A(); };




        // Rotate and Shift Operations
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
    }

    void Z80::handleExtendedOpcode(uint8_t opcode) {
        // Handle extended opcodes (0xCB, 0xDD, 0xED, 0xFD)
        // This should include the logic to handle the extended opcode space

        // Example:
        switch (opcode) {
            case 0xCB:
                handleCBPrefix();
                break;
            case 0xDD:
            case 0xED:
            case 0xFD:
                // Handle other prefixes
                break;
            default:
                unimplementedOpcode();
                break;
        }
    }

    // Handle CB prefix
     void Z80::handleCBPrefix() {
         // Implementation...
     }

    void Z80::setFlag(uint8_t flag, bool value) {
        if (value) {
            F |= flag;
        }
        else {
            F &= ~flag;
        }
    }

    void Z80::setZeroFlag(uint8_t result) {
        setFlag(Z_FLAG, result == 0);
    }

    void Z80::setSignFlag(uint8_t result) {
        setFlag(S_FLAG, (result & 0x80) != 0);
    }

     // Set half carry flag
     void Z80::setHalfCarryFlag(uint8_t a, uint8_t b, uint8_t result) {
         // Implementation...
     }

     // Set carry flag
     void Z80::setCarryFlag(uint8_t a, uint8_t b, uint16_t result) {
         // Implementation...
     }

     // Set parity/overflow flag
     void Z80::setParityOverflowFlag(uint8_t result) {
         // Implementation...
     }

     // Fetch a byte
     uint8_t Z80::fetch() {
        return memory.read(PC++);
     }

     // Fetch a 16-bit value
     uint16_t Z80::fetch16() {
         uint8_t lowByte = memory.read(PC++);
        uint8_t highByte = memory.read(PC++);
     return (static_cast<uint16_t>(highByte) << 8) | lowByte;
     }

     // Get BC register
     uint16_t Z80::getBC() const {
         // Implementation...
     }

     // Other register access methods...


// Handle interrupts
     void Z80::handleInterrupts() {
         // Implementation...
     }

  
    void Z80::setHalfCarryFlag(uint8_t a, uint8_t b, uint8_t result) {
        setFlag(H_FLAG, ((a & 0x0F) + (b & 0x0F)) > 0x0F);
    }

    void Z80::setCarryFlag(uint8_t a, uint8_t b, uint16_t result) {
        setFlag(C_FLAG, result > 0xFF);
    }

   void Z80::setParityOverflowFlag(uint8_t result) {
    // Parity flag is set if the number of set bits in 'result' is even.
    uint8_t parity = 0;
    for (parity = 0; result; parity++) {
        result &= result - 1;
    }
    setFlag(PV_FLAG, (parity % 2) == 0);
}
    void Z80::push16(uint16_t value) {
    memory.write(--SP, value >> 8);
    memory.write(--SP, value & 0xFF);
    }

   uint16_t Z80::pop16() {
    uint16_t low = memory.read(SP++);
    uint16_t high = memory.read(SP++);
    return (high << 8) | low;
}

    void Z80::PUSH_AF() {
        push16((A << 8) | F);
    }

    void Z80::POP_AF() {
        uint16_t af = pop16();
        A = af >> 8;
        F = af & 0xFF;
    }

   void Z80::unimplementedOpcode() {
    std::cerr << "Unimplemented opcode at address 0x" << std::hex << PC-1 << std::endl;
    halted = true; // Halt the CPU to prevent executing further instructions
    }
