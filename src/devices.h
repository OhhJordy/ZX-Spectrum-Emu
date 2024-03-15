#pragma once

#include "defines.h"

#include <type_traits>

class IDevice {
    public:
    
    virtual void receiveData(uint8_t data, uint16_t port) = 0;

    virtual bool sendData(uint8_t& out, uint16_t port) = 0;
};