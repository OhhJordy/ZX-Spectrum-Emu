#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>
#include <vector>
#include <string>

class Z80;

#include "Memory.h"


enum class MachineCycleType { UNUSED, M1R, MRD, MWR, IOR, IOW, NON };

struct Instruction {
    int cycles;                 
    int cyclesOnJump;          
    int numDataBytes;           

    void (*execute)(Z80*, Spectrum48KMemory*, std::vector<uint8_t>);

    int cntMachineCycles;       

    MachineCycleType machineCycles[7];  
    int machineCycleTimes[7];   
    std::string mnemonic;
};

#endif