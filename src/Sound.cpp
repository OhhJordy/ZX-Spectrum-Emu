#include "sound.h"

Sound::Sound() : currentSoundState(false) {
    // Initialise sound-related variables
}

void Sound::updateSound(uint8_t value) {
    currentSoundState = (value & 0x10) != 0; 

void Sound::generateAudioStream(float* stream, int length) {
    float amplitude = currentSoundState ? 0.5f : 0.0f; // Simple square wave

    for (int i = 0; i < length; ++i) {
        stream[i] = amplitude;
    }
}
