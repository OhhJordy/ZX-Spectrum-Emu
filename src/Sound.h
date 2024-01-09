#ifndef SOUND_H
#define SOUND_H

#include <vector>

class Sound {
public:
    Sound();
    void updateSound(uint8_t value); 
    void generateAudioStream(float* stream, int length);
    bool currentSoundState;
    std::vector<float> audioBuffer;

};

#endif 
