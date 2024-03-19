#ifndef ULA_H
#define ULA_H

#include <cstdint>
class Input; 

class ULA {
public:
    ULA(Input* input);
    void processKeyboardInput();
    uint8_t readKeyboard(int row);

private:
    Input* input;
    uint8_t keyboardMatrix[8]; // Assuming 8 rows for the keyboard matrix
};

#endif // ULA_H
