#ifndef SOUND_H
#define SOUND_H

#include <cstdint>
#include <vector>

class Sound {
public:
    Sound();
    void updateSound(uint8_t value); 
    void generateAudioStream(float* stream, int length);
    void setFrequency(int freq);
    void setSampleRate(int rate);
    int getSampleRate() const;

private:
    bool currentSoundState;
    int frequency;  
    int sampleRate; 
};

#endif
