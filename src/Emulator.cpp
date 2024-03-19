#include "Emulator.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <SDL.h>
#include <algorithm>
#include "Z80.h"

Emulator::Emulator(SDL_Window* window)
    :
    m_memory(), 
    display(&m_memory), 
    input(), 
    sound(), 
    m_window(window),
    m_debugger(), 
    m_ula(),
    m_proc(&m_memory, &m_ula, &m_debugger)


{
    init();
    m_prevFrameTime = std::chrono::high_resolution_clock::now();
    
}

void Emulator::init()
{

    m_proc.init();
    
}

const int EXPECTED_ROM_SIZE = 16384;

void Emulator::loadROM(const std::string filename) 
{
     std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open ROM file." << std::endl;
        return;
    }

    //check file size
     file.seekg(0, std::ios::end);
    int length = file.tellg();
    file.seekg(0, std::ios::beg);

    if (length != EXPECTED_ROM_SIZE) {
        std::cerr << "Unexpected ROM size." << std::endl;
        return;
    }
    file.read(reinterpret_cast<char*>(m_memory.ROM), length);
    file.close();

    m_ROMfile = filename;
}

bool Emulator::loop() 
{
   
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(now - m_prevFrameTime);
    if (m_debugger.shouldBreak() && !m_debugger.shouldBreakNextFrame())
    {
        if (timeSpan.count() >= REFRESH_RATE) 
        {
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            int w, h;
            SDL_GetWindowSize(m_window, &w, &h);
            display.draw(w, h);
            return true;
        }
        return false;
    }
  if ((timeSpan.count() >= REFRESH_RATE) || (m_debugger.shouldBreakNextFrame()))
  
    {
        m_delta = timeSpan;
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_prevFrameTime = std::chrono::high_resolution_clock::now();

        int w, h;
        SDL_GetWindowSize(m_window, &w, &h);

        m_proc.nmi();
        m_proc.simulateFrame();
        display.draw(w, h);
        m_pressedKeys.clear();
        m_debugger.endLoop();
        return true;
    }

    return false;

}

double Emulator::getDeltaTime()
{
    return m_delta.count();
}

void Emulator::reset() {
    //init(); // Only reinitialize components without loading ROM.
    loadROM(m_ROMfile);
}

Display* Emulator::getDisplay()
{
    return &display;
}

Debugger* Emulator::getDebugger()
{
    return &m_debugger;
}

void Emulator::processSDLEvent(SDL_Event e)
{
    switch (e.type) 
    {
        case SDL_KEYDOWN:
            m_pressedKeys.push_back(e.key.keysym.sym);
            break;
        case SDL_KEYUP:
            m_pressedKeys.erase(
                std::remove_if(m_pressedKeys.begin(), m_pressedKeys.end(), 
                               [&e](const SDL_Keycode& keycode) { return keycode == e.key.keysym.sym; }),
                m_pressedKeys.end());
            break;
    }
}




Spectrum48KMemory* Emulator::getMemory()
{
    return &m_memory;
}




