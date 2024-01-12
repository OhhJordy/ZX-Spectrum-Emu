#include "Input.h"
#include <SDL.h>

const SDL_Keycode Input::keyCodes[8][5] = {
    { SDLK_LSHIFT, SDLK_z, SDLK_x, SDLK_c, SDLK_v },
    { SDLK_a, SDLK_s, SDLK_d, SDLK_f, SDLK_g },
    { SDLK_q, SDLK_w, SDLK_e, SDLK_r, SDLK_t },
    { SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5 },
    { SDLK_0, SDLK_9, SDLK_8, SDLK_7, SDLK_6 },
    { SDLK_p, SDLK_o, SDLK_i, SDLK_u, SDLK_y },
    { SDLK_RETURN, SDLK_l, SDLK_k, SDLK_j, SDLK_h },
    { SDLK_SPACE, SDLK_RSHIFT, SDLK_m, SDLK_n, SDLK_b }
};

const std::string Input::keyStrings[8][5] = {
    { "ZX_CAPS_SHIFT", "ZX_Z", "ZX_X", "ZX_C", "ZX_V" },
    { "ZX_A", "ZX_S", "ZX_D", "ZX_F", "ZX_G" },
    { "ZX_Q", "ZX_W", "ZX_E", "ZX_R", "ZX_T" },
    { "ZX_1", "ZX_2", "ZX_3", "ZX_4", "ZX_5" },
    { "ZX_0", "ZX_9", "ZX_8", "ZX_7", "ZX_6" },
    { "ZX_P", "ZX_O", "ZX_I", "ZX_U", "ZX_Y" },
    { "ZX_ENTER", "ZX_L", "ZX_K", "ZX_J", "ZX_H" },
    { "ZX_SPACE", "ZX_SYMBOL_SHIFT", "ZX_M", "ZX_N", "ZX_B" }
};

Input::Input() {
    // Initialize the keyState for all keys
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 5; ++j) {
            keyState[keyStrings[i][j]] = false;
        }
    }
}



void Input::processSDLEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        std::string keyStr = getKeyStringFromSDLKeycode(event.key.keysym.sym);
        if (keyStr != "ZX_UNKNOWN") {
            keyState[keyStr] = (event.type == SDL_KEYDOWN);
        }
    }
}

bool Input::isKeyPressed(const std::string& key) const {
    auto it = keyState.find(key);
    return it != keyState.end() && it->second;
}

std::string Input::getKeyStringFromSDLKeycode(SDL_Keycode keycode) const {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 5; ++j) {
            if (keyCodes[i][j] == keycode) {
                return keyStrings[i][j];
            }
        }
    }
    return "ZX_UNKNOWN";
}
