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
#include "ULA.h"
#include "debugger.h"

#define REFRESH_RATE (1.0 / 50.0) // 50Hz refresh rate

class Emulator {
    public:
        Emulator(SDL_Window* window);


        void loadROM(std::string filename);

        bool loop();

        double getDeltaTime();

        void reset();



    Display* getDisplay();
    Debugger* getDebugger();
    Spectrum48KMemory* getMemory();

    void processSDLEvent(SDL_Event e);

protected:
    void init();

private:
    Z80 m_proc;
    Spectrum48KMemory memory;
    Display display;
    Input input;
    Sound sound;
    Debugger m_debugger;
    ULA m_ula;
    std::string m_ROMfile;
    SDL_Window* m_window;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_prevFrameTime;
    std::vector<SDL_Keycode> m_pressedKeys;
    std::chrono::duration<double> m_delta;

};

#endif 
