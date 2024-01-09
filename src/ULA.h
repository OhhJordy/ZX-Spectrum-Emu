#ifndef ULA_H
#define ULA_H

#include "Memory.h"
#include "Display.h"
#include "Sound.h"

class ULA {
public:
    ULA(Memory* memory, Display* display, Sound* sound);

    void update(); // Main update function called every cycle

private:
    Memory* memory;
    Display* display;
    Sound* sound;

    void refreshScreen(); // Handle screen refresh logic
    void updateBorderColour(); // Update the border colour
    void updateSoundTiming(); // Update sound generation timing

};

#endif 
