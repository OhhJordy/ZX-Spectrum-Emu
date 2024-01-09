#include "Memory.h"
#include <algorithm>
#include <stdexcept>

Memory::Memory() {
    data.fill(0); 
   
}

uint8_t Memory::read(uint16_t address) const {
    return data[address];
}

void Memory::write(uint16_t address, uint8_t value) {
    if (!isROMProtected(address)) {
        data[address] = value;
    } else {
       
    }
}

void Memory::loadROM(const std::vector<uint8_t>& romData) {
    if (romData.size() > 16 * 1024) {
        throw std::runtime_error("ROM data is too large for memory");
    }
    std::copy(romData.begin(), romData.end(), data.begin());
}

bool Memory::isROMProtected(uint16_t address) const {
    return address < 16 * 1024; 
}
