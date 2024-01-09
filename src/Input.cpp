#include "input.h"
#include <algorithm>

const SDL_Keycode Input::keyCodes[8][5] = {
    // ... (same as provided)
};

const std::string Input::keyStrings[8][5] = {
    // ... (same as provided)
};

Input::Input() {
    // Initialise keyState with all keys set to false (not pressed)
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
