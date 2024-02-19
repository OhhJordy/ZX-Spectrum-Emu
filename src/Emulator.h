#ifndef EMULATOR_H
#define EMULATOR_H

#include "Z80.h"
#include "Memory.h"
#include "Display.h"
#include "Input.h"
#include "Sound.h"
#include <SDL.h>
#include <string>
#include <vector>
#include <chrono>

#define REFRESH_RATE (1.0 / 50.0) // 50Hz refresh rate

class Emulator {
public:
    Emulator(SDL_Window* window);
    bool loadROM(const std::string& filename);
    bool loop();
    void reset(const std::string& romFilePath);
    Display* getDisplay();
    void processSDLEvent(SDL_Event e);

private:
    void init();
    Z80 cpu;
    Spectrum48KMemory memory;
    Display display;
    Input input;
    Sound sound;
    std::string m_ROMfile;
    SDL_Window* m_window;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_prevFrameTime;
    std::vector<SDL_Keycode> m_pressedKeys;
};

#endif // EMULATOR_H
