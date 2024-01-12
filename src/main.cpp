#include "Emulator.h"
#include <SDL.h>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create an SDL window
    SDL_Window* window = SDL_CreateWindow("ZX Spectrum Emulator",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          640, 480, SDL_WINDOW_OPENGL);

    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create emulator instance with the SDL window
    Emulator emulator(window);

    // Load ROM (specify the path to your ROM file)
    std::string romFilePath = "path/to/your/romfile.rom";
    if (!emulator.loadROM(romFilePath)) {
        std::cerr << "Failed to load ROM: " << romFilePath << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Main loop
    while (true) {
        // Check for emulator loop exit condition
        if (!emulator.loop()) {
            break;
        }

        // Handle SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    emulator.reset(); 
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    // Process keyboard input events
                    emulator.processSDLEvent(event);
                    break;
            }
        }

    }

    // Clean up and shut down
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
