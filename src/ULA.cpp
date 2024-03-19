#include "ULA.h"
#include "Input.h"

ULA::ULA(Input* input) : input(input) {
    // Initialize the keyboard matrix to all keys not pressed
    for (int i = 0; i < 8; ++i) {
        keyboardMatrix[i] = 0xFF; // All bits set to 1
    }
}

void ULA::processKeyboardInput() {
    if (!input) return; // Return if input is null

    // Iterate over the keyboard matrix rows
    for (int row = 0; row < 8; ++row) {
        // Reset the current row state to all keys not pressed
        keyboardMatrix[row] = 0xFF;

        // For each key in the row...
        for (int col = 0; col < 5; ++col) {
            // Use the public method to get the key string
            std::string keyStr = Input::getKeyString(row, col);
            // Determine if the key is pressed
            if (input->isKeyPressed(keyStr)) {
                // Clear the bit for the pressed key
                keyboardMatrix[row] &= ~(1 << col);
            }
        }
    }
}
uint8_t ULA::readKeyboard(int row) {
    // Return the state of the specified row
    return keyboardMatrix[row];
}
