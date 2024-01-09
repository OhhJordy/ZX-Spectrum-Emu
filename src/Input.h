#ifndef INPUT_H
#define INPUT_H

#include <unordered_map>
#include <string>
#include <vector>

class Input {
public:
    Input();
    void processSDLEvent(const SDL_Event& event);
    bool isKeyPressed(const std::string& key) const;

private:
    static const SDL_Keycode keyCodes[8][5];
    static const std::string keyStrings[8][5];
    std::unordered_map<std::string, bool> keyState; 

    std::string getKeyStringFromSDLKeycode(SDL_Keycode keycode) const;
};

#endif 