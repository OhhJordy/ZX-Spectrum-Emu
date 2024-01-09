#ifndef EMULATOR_H
#define EMULATOR_H

#include "Z80.h"
#include "Memory.h"
#include "Display.h"
#include "Input.h"
#include "Sound.h"

class Emulator {
public:
    Emulator();
    void loadROM(const std::string& romPath);
    void run(); // Main emulation loop
    void updateInput(); // Update input state
    void updateSound(); // Update sound state

private:
    Z80 cpu;
    Memory memory;
    Display display;
    Input input;
    Sound sound;

};

#endif 
