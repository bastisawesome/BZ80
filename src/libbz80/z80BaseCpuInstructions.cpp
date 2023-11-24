#include "z80basecpu.hpp"

#include <cstdint>

namespace bz80 {

bool calcFlagH(uint8_t origValue, uint8_t toAdd) {
    return (((origValue & 0xf) + (toAdd & 0xf)) > 0xf);
}

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

uint8_t Z80BaseCpu::inc_r(const MmioDeviceManager& bus) {
    uint8_t cycles = 0;
    uint8_t preIncValue;

    switch(this->currentDecodedInstruction.y) {
    case 0:
        preIncValue = this->registerBC.getUpper8();
        this->registerBC.addUpper8(1);
        break;
    case 1:
        preIncValue = this->registerBC.getLower8();
        this->registerBC.addLower8(1);
        break;
    case 2:
        preIncValue = this->registerDE.getUpper8();
        this->registerDE.addUpper8(1);
        break;
    case 3:
        preIncValue = this->registerDE.getLower8();
        this->registerDE.addLower8(1);
        break;
    case 4:
        preIncValue = this->registerHL.getUpper8();
        this->registerHL.addUpper8(1);
        break;
    case 5:
        preIncValue = this->registerHL.getLower8();
        this->registerHL.addLower8(1);
        break;
    case 6:
        cycles += MEMORY_ACCESS_CYCLES;
        preIncValue = bus.read8(this->registerHL.get16());
        bus.write8(this->registerHL.get16(), preIncValue + 1);
        break;
    case 7:
        preIncValue = this->registerA;
        this->registerA++;
        break;
    default:
        return 255;
    }

    uint8_t postIncValue = preIncValue + 1;

    this->registerF.sign = 0b10000000 & (postIncValue);
    this->registerF.zero = (postIncValue) == 0;
    this->registerF.overflow = preIncValue == 0x7f;
    this->registerF.add_sub = 0;
    this->registerF.halfcarry = calcFlagH(preIncValue, 1);

    return cycles;
}

};
