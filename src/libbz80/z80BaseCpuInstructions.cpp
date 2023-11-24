#include "z80basecpu.hpp"

#include <cstdint>

namespace bz80 {

uint8_t Z80BaseCpu::ld_r_imm(const MmioDeviceManager& bus) {
    uint8_t cycles = MEMORY_ACCESS_CYCLES;
    uint8_t value = bus.read8(this->programCounter++);
    switch(this->currentDecodedInstruction.y) {
    case 0:
        this->registerBC.setUpper8(value);
        return cycles;
    case 1:
        this->registerBC.setLower8(value);
        return cycles;
    case 2:
        this->registerDE.setUpper8(value);
        return cycles;
    case 3:
        this->registerDE.setLower8(value);
        return cycles;
    case 4:
        this->registerHL.setUpper8(value);
        return cycles;
    case 5:
        this->registerHL.setLower8(value);
        return cycles;
    case 6:
        bus.write8(this->registerHL.get16(), value);
        return cycles + MEMORY_ACCESS_CYCLES;
    case 7:
        this->registerA = value;
        return cycles;
    default: return UINT8_MAX;
    }
}

};
