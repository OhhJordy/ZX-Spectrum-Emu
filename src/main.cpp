#include "Emulator.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <GL/glew.h>

#include <string>
#include "Display.h"
#include "defines.h"
#include "ULA.h"
#include "window.h"
#include "Z80.h"
#include "instructions.h"

#include <iostream>
#include <bitset>

#include <fstream>
#include <random>
#include <functional>
#include <string>
#include <chrono>
#include <memory>
#include <iomanip> 
#include <sstream> 

#include "utils.h"
#include <shellapi.h>
#include <SDL_events.h>

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    try {
        static bool showImguiDemo = false;

        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            return -1;
        }
        std::cout << "SDL initialized successfully." << std::endl;

        SDL_Window* window = createWindow(800, 600, "ZX++");
        if (!window) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return -1;
        }
        std::cout << "SDL window created successfully." << std::endl;

        SDL_GLContext glContext = SDL_GL_CreateContext(window);
        if (!glContext) {
            std::cerr << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(window);
            SDL_Quit();
            return -1;
        }
        std::cout << "OpenGL context created successfully." << std::endl;

        // Initialize GLEW
        glewExperimental = GL_TRUE;
        GLenum glewError = glewInit();
        if (glewError != GLEW_OK) {
            std::cerr << "Error initializing GLEW: " << glewGetErrorString(glewError) << std::endl;
            SDL_GL_DeleteContext(glContext);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return -1;
        }
        std::cout << "GLEW initialized successfully." << std::endl;

        Emulator emu(window);

        std::string romFilePath = "C:/Users/Jordan/ROM/48k.rom";
        emu.loadROM(romFilePath);

        // Main loop
        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                } else if (e.type == SDL_WINDOWEVENT) {
                    switch (e.window.event) {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                        case SDL_WINDOWEVENT_RESIZED:
                            int w, h;
                            SDL_GetWindowSize(window, &w, &h);
                            glViewport(0, 0, w, h);
                            break;
                    }
                } else if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_F1) {
                        showImguiDemo = !showImguiDemo;
                    }
                }
                emu.processSDLEvent(e);
            }

            if (emu.loop()) {
                std::stringstream stream;
                stream << std::fixed << std::setprecision(1) << 1.0f / (float)emu.getDeltaTime();
                std::string fps = stream.str();
                fps = "ZX Spectrum | FPS: " + fps;
                SDL_SetWindowTitle(window, fps.c_str());

                if (showImguiDemo) {
                    // ImGui demo code goes here
                }

                SDL_GL_SwapWindow(window);
            }
        }

        std::cout << "Shutting down emulator..." << std::endl;
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        std::cout << "Emulator shut down successfully." << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception caught in WinMain: " << e.what() << std::endl;
        // Perform necessary cleanup
        SDL_Quit();
        return -1; // Return an error code
    } catch (...) {
        std::cerr << "Unknown exception caught in WinMain." << std::endl;
        // Perform necessary cleanup
        SDL_Quit();
        return -1; // Return an error code
    }
}

