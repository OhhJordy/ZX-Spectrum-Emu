#include <cstdint>
#include <array>
#include <stdexcept>
#include <iostream>

class Z80 {
public:
    Z80() {
        initializeRegisters();
        SP = 0;
        PC = 0;
        I = 0;
        R = 0;
        interruptsEnabled = false;
        memory.fill(0);
    }

    void executeNextInstruction() {
        if (halted) return;
        uint8_t opcode = fetch();
        switch (opcode) {
            case 0x00: NOP();
                break;
            case 0x02: LD_BC_A(); 
                break;
            case 0x12: LD_DE_A(); 
                break;
            case 0x77: LD_HL_A(); 
                break;
            case 0x21: LD_HL_nn(); 
                break;
            case 0x7E: LD_A_HL(); 
                break;
            case 0x0A: LD_A_BC(); 
                break;
            case 0x1A: LD_A_DE(); 
                break;
            case 0x36: LD_HL_n(); 
                break;
            case 0x05: DEC_B(); 
                break;
            case 0x0C: INC_C(); 
                break;
            case 0x11: LD_DE_nn(); 
                break;
            case 0x7F: LD_A_A(); 
                break;
            case 0xBE: CP_HL(); 
                break;
            case 0x76:
                halt();
                break;
            case 0x3E:
                A = fetch();
                setZeroFlag(A);
                setSignFlag(A);
                break;
            case 0x47:
                LD_B_A();
                break;
            case 0x06:
                LD_B_n();
                break;
            case 0x0E:
                LD_C_n();
                break;
            case 0x16:
                LD_D_n();
                break;
            case 0x1E:
                LD_E_n();
                break;
            case 0x26:
                LD_H_n();
                break;
            case 0x2E:
                LD_L_n();
                break;
            case 0x80:
                ADD_A_B();
                break;
            case 0x81:
                ADD_A_C();
                break;
            case 0x90:
                SUB_B();
                break;
            case 0x91:
                SUB_C();
                break;
            case 0x82:
                ADD_A_D();
                break;
            case 0x83:
                ADD_A_E();
                break;
            case 0x84:
                ADD_A_H();
                break;
            case 0x85:
                ADD_A_L();
                break;
            case 0x86:
                ADD_A_HL();
                break;
            case 0x87:
                ADD_A_A();
                break;
            case 0x88:
                ADC_A_B();
                break;
            case 0x89:
                ADC_A_C();
                break;
            case 0x01: LD_BC_nn(); 
                break;
            default:
                throw std::runtime_error("Unimplemented opcode encountered");
        }
    }

private:
   uint8_t A, B, C, D, E, H, L, F, I, R;
   uint16_t SP, PC;
   bool interruptsEnabled;
   bool halted = false;

    static constexpr uint8_t Z_FLAG = 0x80;
    static constexpr uint8_t N_FLAG = 0x40;
    static constexpr uint8_t H_FLAG = 0x20;
    static constexpr uint8_t C_FLAG = 0x10;
    static constexpr uint8_t S_FLAG = 0x80;
    static constexpr uint8_t PV_FLAG = 0x04;

    std::array<uint8_t, 65536> memory;

     void initializeRegisters() {
        A = B = C = D = E = H = L = F = I = R = 0;
        SP = PC = 0;
        halted = interruptsEnabled = false;
    }

   

    uint16_t fetch16() {
        uint8_t lowByte = fetch();
        uint8_t highByte = fetch();
        return (static_cast<uint16_t>(highByte) << 8) | lowByte;
    }

    void setFlag(uint8_t flag, bool value) {
        if (value) {
            F |= flag;
        } else {
            F &= ~flag;
        }
    }

    void NOP() { }

    void halt() { halted = true; }

    void DI() { interruptsEnabled = false; }

    void EI() { interruptsEnabled = true; }

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
        
    }

    uint8_t fetch() {
        uint8_t opcode = memory[PC++];
        return opcode;
    }
    

    uint16_t fetch16() {
    uint8_t lowByte = fetch();
    uint8_t highByte = fetch();
    return (static_cast<uint16_t>(highByte) << 8) | lowByte;
}

    uint16_t getBC() const { return (B << 8) | C; }
    uint16_t getDE() const { return (D << 8) | E; }
    uint16_t getHL() const { return (H << 8) | L; }

    void setBC(uint16_t value) { B = value >> 8; C = value & 0xFF; }

    // Stack operations
    void push16(uint16_t value) {
        memory[--SP] = value >> 8;
        memory[--SP] = value & 0xFF;
    }

    uint16_t pop16() {
        uint16_t value = memory[SP++];
        value |= memory[SP++] << 8;
        return value;
    }



    void LD_B_A() {
        B = A;
    }

    void LD_A_HL() {
    A = memory[getHL()];
    }

    void LD_A_BC() {
        A = memory[getBC()];
    }

    void LD_A_DE() {
        A = memory[getDE()];
    }

    void LD_HL_n() {
        uint8_t value = fetch();
        memory[getHL()] = value;
    }

    void LD_BC_A() {
    memory[getBC()] = A;
    }

    void LD_DE_A() {
        memory[getDE()] = A;
    }

    void LD_HL_A() {
        memory[getHL()] = A;
    }

    void LD_HL_nn() {
        uint16_t value = fetch16();
        setHL(value);
    }

        };

    void LD_B_n() {
        B = fetch();
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

    void LD_BC_nn() {
        BC = fetch16();
    }
    void LD_DE_nn() {
        DE = fetch16();
    }
    void LD_HL_nn() {
        HL = fetch16();
    }
    void LD_SP_nn() {
        SP = fetch16();
    }

    void LD_B_C() {
        B = C;
    }

    void LD_B_n() {
        B = fetch();
    }

    void LD_B_HL() {
        B = memory[HL];
    }

    void LD_HL_B() {
        memory[HL] = B;
    }

    void INC_B() {
        B++;
        setZeroFlag(B);
        setSignFlag(B);
    }

    void DEC_B() {
        B--;
        setZeroFlag(B);
        setSignFlag(B);
    }

    void DEC_B() {
    B--;
    setZeroFlag(B == 0);
    setSignFlag((B & 0x80) != 0);
    setHalfCarryFlag(1, B, B); // Half carry for subtraction
    setFlag(N_FLAG, true);
    }

    void INC_C() {
        C++;
        setZeroFlag(C == 0);
        setSignFlag((C & 0x80) != 0);
        setHalfCarryFlag(C - 1, 1, C); // Half carry for addition
        setFlag(N_FLAG, false);
    }

    void LD_DE_nn() {
        DE = fetch16();
    }

    void LD_A_A() {
    }

    void CP_HL() {
        uint8_t value = memory[getHL()];
        uint16_t result = static_cast<uint16_t>(A) - static_cast<uint16_t>(value);
        setZeroFlag(static_cast<uint8_t>(result) == 0);
        setSignFlag((result & 0x80) != 0);
        setHalfCarryFlag(A, value, static_cast<uint8_t>(result));
        setCarryFlag(A, value, result);
        setFlag(N_FLAG, true);
    }    

    void ADD_A_B() {
        uint16_t result = static_cast<uint16_t>(A) + static_cast<uint16_t>(B);
        setZeroFlag(static_cast<uint8_t>(result));
        setSignFlag(static_cast<uint8_t>(result));
        setHalfCarryFlag(A, B, static_cast<uint8_t>(result));
        setCarryFlag(A, B, result);
        setParityOverflowFlag(static_cast<uint8_t>(result));
        A = static_cast<uint8_t>(result);
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

    void SUB_B() {
        uint16_t result = static_cast<uint16_t>(A) - static_cast<uint16_t>(B);
        setZeroFlag(static_cast<uint8_t>(result));
        setSignFlag(static_cast<uint8_t>(result));
        setHalfCarryFlag(A, static_cast<uint8_t>(-B), static_cast<uint8_t>(result));
        setCarryFlag(A, static_cast<uint8_t>(-B), result);
        setParityOverflowFlag(static_cast<uint8_t>(result));
        A = static_cast<uint8_t>(result);
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

    void ADD_A_HL() {
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

    void JP_nn() {
        uint16_t address = fetch16();  // Fetch a 16-bit address
        PC = address;
    }

    void CALL_nn() {
        uint16_t address = fetch16();  // Fetch a 16-bit address
        // Push the return address (PC) onto the stack
        push16(PC);
        PC = address;
    }

    void BIT_0_B() {
        setZeroFlag((B & (1 << 0)) == 0);
    }

    // SET b,r (Set bit in register)
    void SET_0_B() {
        B |= (1 << 0);
    }

    // RES b,r (Reset bit in register)
    void RES_0_B() {
        B &= ~(1 << 0);
    }

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
