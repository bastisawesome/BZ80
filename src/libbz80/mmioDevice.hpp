#pragma once

#include <cstdint>

namespace bz80 {

class MmioDevice {
public:
    virtual uint8_t read8(const uint16_t addr) const = 0;
    virtual void write8(const uint16_t addr, const uint8_t value) = 0;
};

}
