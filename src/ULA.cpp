#include "ULA.h"

ULA::ULA(Memory* mem, Display* disp, Sound* snd)
    : memory(mem), display(disp), sound(snd) {

}

void ULA::update() {
    refreshScreen();
    updateBorderColour();
    updateSoundTiming();
}

void ULA::refreshScreen() {


    for (int y = 0; y < 192; y++) {
        for (int x = 0; x < 32; x++) { /
            uint16_t address = 0x4000 + y * 32 + x;
            uint8_t byte = memory->read(address);
            display->setPixels(x * 8, y, byte); 
        }
    }

    display->refreshScreen(); 
}

void ULA::updateBorderColour() {
   
    uint8_t borderColour = memory->readBorderColour(); 
    display->setBorderColour(borderColour); 

void ULA::updateSoundTiming() {
   
    sound->update(); 
}

