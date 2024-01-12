#include "sound.h"

Sound::Sound() : currentSoundState(false), sampleRate(44100), frequency(1000) {
}

void Sound::updateSound(uint8_t value) {
    currentSoundState = (value & 0x10) != 0; 
}

void Sound::generateAudioStream(float* stream, int length) {
    float amplitude = currentSoundState ? 0.5f : 0.0f;
    float waveLength = static_cast<float>(sampleRate) / frequency;

    for (int i = 0; i < length; ++i) {
        float t = static_cast<float>(i) / waveLength;
        stream[i] = (static_cast<int>(t) % 2 == 0) ? amplitude : -amplitude;
    }
}

void Sound::setFrequency(int freq) {
    frequency = freq;
}

int Sound::getSampleRate() const {
    return sampleRate;
}

void Sound::setSampleRate(int rate) {
    sampleRate = rate;
}
