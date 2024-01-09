#include "Emulator.h"
#include <iostream>
#include <fstream>

Emulator::Emulator() : cpu(), memory(), display(&memory), input(), sound() {
    // Initialise the emulator components
}

Emulator::~Emulator() {
    stop();
}

void Emulator::Initialise() {
    cpu.reset();
}

bool Emulator::loadROM(const std::string& romFile) {
    std::ifstream file(romFile, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open ROM file: " << romFile << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(size);
    if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        memory.loadROM(buffer);
        return true;
    }

    std::cerr << "Failed to read ROM file: " << romFile << std::endl;
    return false;
}

void Emulator::run() {
    while (true) { // Main emulation loop
        cpu.executeNextInstruction(); // Execute a CPU instruction
        updateInput(); // Update input state
        updateSound(); // Update sound state
        display.draw(); // Update the display
        // Handle timing and synchronization
    }
}


void Emulator::stop() {
    isRunning = false;
}

void Emulator::updateInput() {
    

    input.processEvents();

    
}

void Emulator::updateSound() {
  

    uint8_t soundValue = cpu.getSoundValue(); 
    sound.updateSound(soundValue);

    
}



void Emulator::mainLoop() {
    while (isRunning) {
        cpu.executeNextInstruction();
        
        display.refresh(); 
}
