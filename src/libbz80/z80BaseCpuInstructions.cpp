#include "z80basecpu.hpp"

#include <cstdint>

namespace bz80 {

bool calcFlagH(uint8_t origValue, uint8_t toAdd, bool isSub = false) {
    if(!isSub) {
        return ((origValue & 0xf) + (toAdd & 0xf)) > 0xf;
    }

    return (toAdd & 0xf) <= (origValue & 0xf);
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
    default:
        return UINT8_MAX;
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

    this->registerF.sign = 0b10000000 & postIncValue;
    this->registerF.zero = postIncValue == 0;
    this->registerF.overflow = preIncValue == 0x7f;
    this->registerF.add_sub = 0;
    this->registerF.halfcarry = calcFlagH(preIncValue, 1);

    return cycles;
}

uint8_t Z80BaseCpu::dec_r(const MmioDeviceManager& bus) {
    uint8_t cycles = 0, preDecValue;

    switch(this->currentDecodedInstruction.y) {
    case 0:
        preDecValue = this->registerBC.getUpper8();
        this->registerBC.addUpper8(-1);
        break;
    case 1:
        preDecValue = this->registerBC.getLower8();
        this->registerBC.addLower8(-1);
        break;
    case 2:
        preDecValue = this->registerDE.getUpper8();
        this->registerDE.addUpper8(-1);
        break;
    case 3:
        preDecValue = this->registerDE.getLower8();
        this->registerDE.addLower8(-1);
        break;
    case 4:
        preDecValue = this->registerHL.getUpper8();
        this->registerHL.addUpper8(-1);
        break;
    case 5:
        preDecValue = this->registerHL.getLower8();
        this->registerHL.addLower8(-1);
        break;
    case 6:
        cycles += MEMORY_ACCESS_CYCLES;
        preDecValue = this->bus.read8(this->registerHL.get16());
        this->bus.write8(this->registerHL.get16(), preDecValue - 1);
        break;
    case 7:
        preDecValue = this->registerA--;
        break;
    default:
        return 255;
    }

    uint8_t postDecValue = preDecValue - 1;

    this->registerF.sign = 0b10000000 & postDecValue;
    this->registerF.zero = postDecValue == 0;
    this->registerF.halfcarry = calcFlagH(preDecValue, -1, true);
    this->registerF.overflow = preDecValue == 0x80;
    this->registerF.add_sub = true;

    return cycles;
}

uint8_t Z80BaseCpu::nop() {
    return 0;
}

uint8_t Z80BaseCpu::djnz(const MmioDeviceManager& bus) {
    uint8_t cycles = 0;

    this->registerBC.addUpper8(-1);
    cycles += INC_DEC_REG_CYCLES;

    int8_t jumpAmt = bus.read8(this->programCounter++);

    if(this->registerBC.getUpper8() != 0) {
        this->programCounter += jumpAmt;
        cycles += MEMORY_ACCESS_CYCLES;
        cycles += INC_DEC_REG_CYCLES;
        cycles += INC_DEC_REG_CYCLES;
    } else {
        this->programCounter++;
    }

    cycles += TEST_REG_CYCLES;

    return cycles;
}

uint8_t Z80BaseCpu::jr_imm(const MmioDeviceManager& bus) {
    uint8_t cycles = 0;

    int8_t jumpAmt = this->bus.read8(this->programCounter++);
    cycles += MEMORY_ACCESS_CYCLES;
    this->programCounter += jumpAmt;
    cycles += 5; // TODO: Figure out why this is 5?

    return cycles;
}

uint8_t Z80BaseCpu::jr_cc_imm(const MmioDeviceManager& bus) {
    uint8_t cycles = 0;
    bool willJump = false;

    switch(this->currentDecodedInstruction.y - 4) {
    case 0:
        // NZ
        if(!this->registerF.zero) {
            willJump = true;
        }
        break;
    case 1:
        // Z
        if(this->registerF.zero) {
            willJump = true;
        }
        break;
    case 2:
        // NC
        if(!this->registerF.carry) {
            willJump = true;
        }
        break;
    case 3:
        // C
        if(this->registerF.carry) {
            willJump = true;
        }
        break;
    }

    if(willJump) {
        int8_t value = bus.read8(this->programCounter++);
        this->programCounter += value;
        cycles += MEMORY_ACCESS_CYCLES;
        cycles += INC_DEC_REG_CYCLES;
        cycles += INC_DEC_REG_CYCLES;
    } else {
        this->programCounter++;
    }

    cycles += TEST_REG_CYCLES;

    return cycles;
}
};
