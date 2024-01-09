#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>
#include <array>
#include <vector>

class Memory {
public:
    Memory();
    uint8_t read(uint16_t address) const;
    void write(uint16_t address, uint8_t value);
    void loadROM(const std::vector<uint8_t>& romData);

    // Direct access to specific memory regions
    uint8_t* ROM() { return &data[0x0000]; }
    uint8_t* screenMemory() { return &data[0x4000]; }
    uint8_t* screenColourData() { return &data[0x5800]; }
    uint8_t* printerBuffer() { return &data[0x5B00]; }
    uint8_t* systemVariables() { return &data[0x5C00]; }
    uint8_t* userMemory() { return &data[0x5CCB]; }

private:
    static constexpr size_t MEMORY_SIZE = 65536;
    std::array<uint8_t, MEMORY_SIZE> data; // Z80 has a 16-bit address space
    bool isROMProtected(uint16_t address) const;
};

#endif 
