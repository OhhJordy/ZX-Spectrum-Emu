#include "Emulator.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <SDL.h>
#include <algorithm>


Emulator::Emulator(SDL_Window* window)
    : cpu(), memory(), display(&memory), input(), sound(), m_window(window), m_ROMfile() {
    init();
}

void Emulator::init() {
    try {
        cpu.reset();
        m_prevFrameTime = std::chrono::high_resolution_clock::now();
    } catch (const std::exception& e) {
        std::cerr << "Initialization error: " << e.what() << std::endl;
    }
}

void Emulator::processSDLEvent(SDL_Event e)
{
   switch (e.type)
    {
        case SDL_KEYDOWN:
            m_pressedKeys.push_back(e.key.keysym.sym);
            break;
        case SDL_KEYUP:
            for (auto it = m_pressedKeys.begin(); it != m_pressedKeys.end(); ++it) {
                if (*it == e.key.keysym.sym) {
                    m_pressedKeys.erase(it);
                    break; // Found and erased, exit loop
                }
            }
            break;
    }
}


bool Emulator::loadROM(const std::string& filename) {
    std::ifstream inf(filename, std::ios::in | std::ios::binary);
    if (!inf.is_open()) {
        std::cerr << "Failed to open ROM file: " << filename << std::endl;
        return false;
    }

    inf.seekg(0, std::ios::end);
    size_t length = inf.tellg();
    inf.seekg(0, std::ios::beg);

    if (length > memory.size) {
        std::cerr << "ROM file too large: " << filename << std::endl;
        return false;
    }

    inf.read(reinterpret_cast<char*>(memory.ROM), length);
    if (!inf) {
        std::cerr << "Failed to read ROM file: " << filename << std::endl;
        return false;
    }

    inf.close();
    reset(filename); // Reset CPU and components to ensure consistent state
    return true;
}

bool Emulator::loop() {
    try {
        auto now = std::chrono::high_resolution_clock::now();
        auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(now - m_prevFrameTime);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            processSDLEvent(event);
            if (event.type == SDL_QUIT) return false;
        }

        if (timeSpan.count() >= REFRESH_RATE) {
            m_prevFrameTime = now;
            int w, h;
            SDL_GetWindowSize(m_window, &w, &h);

            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            cpu.executeNextInstruction(); 

            display.draw(w, h); 

            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Loop error: " << e.what() << std::endl;
        return false; 
    }
    return false; // No frame rendered
}



void Emulator::reset(const std::string& romFilePath) {
    m_ROMfile = romFilePath;
    init(); // Reinitialize components
    loadROM(m_ROMfile); // Reload ROM to reset state
}



// Other methods (reset, getDisplay, processEvent, etc.) remain similar
