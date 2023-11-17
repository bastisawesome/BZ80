#include "z80basecpu.hpp"

#define LD_R_IMM_CYCLES 3

namespace bz80 {

uint8_t Z80BaseCpu::ld_r_imm(const MmioDeviceManager& bus, regPairT8Getter reg) {
    uint8_t immValue = bus.read8(this->programCounter++);
    reg(immValue);
    return LD_R_IMM_CYCLES;
}

};
