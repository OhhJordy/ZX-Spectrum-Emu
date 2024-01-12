#include "Emulator.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <SDL.h>


Emulator::Emulator(SDL_Window* window)
    : cpu(), memory(), display(&memory), input(), sound(), 
      m_window(window) {

    init();
}

void Emulator::init() {
    cpu.reset();
    // Initialize other components if needed
    m_prevFrameTime = std::chrono::high_resolution_clock::now();
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
                std::remove(m_pressedKeys.begin(), m_pressedKeys.end(), e.key.keysym.sym),
                m_pressedKeys.end());
            break;
    }
}

bool Emulator::loadROM(const std::string& filename) {
    std::ifstream inf;
    inf.open(filename, std::ios::in|std::ios::binary);

    if (!inf.is_open()) {
        std::cerr << "Failed to open ROM file: " << filename << std::endl;
        return false;
    }

    inf.seekg(0, std::ios::end);
    int length = (int)inf.tellg();
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
    return true;
}

bool Emulator::loop() {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(now - m_prevFrameTime);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        processSDLEvent(event);  // Process the SDL event
        if (event.type == SDL_QUIT) {
            return false; // Exit the loop if the window is closed
        }
    }

    if (timeSpan.count() >= REFRESH_RATE) {
        m_prevFrameTime = now;
        int w, h;
        SDL_GetWindowSize(m_window, &w, &h);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cpu.executeNextInstruction();

        // Draw the current state to the display
        display.draw(w, h);

        // Indicate that a frame was rendered
        return true;
    }

    // No frame rendered
    return false;
}


void Emulator::reset()
{
    init();
    loadROM(m_ROMfile);
}



// Other methods (reset, getDisplay, processEvent, etc.) remain similar
