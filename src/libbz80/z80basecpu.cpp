#include "z80basecpu.hpp"

#include <cassert>

#define FETCH_CYCLES 4
#define DECODE_CYCLES 0

namespace bz80 {

Z80BaseCpu::Z80BaseCpu():
    registerA(0), registerBC(0), registerDE(0), registerHL(0), registerF({0}),
    registerA_alt(0), registerBC_alt(0), registerDE_alt(0), registerHL_alt(0),
    registerF_alt({0}), programCounter(0), stackPointer(0), indexX(0),
    indexY(0), interruptVector(0), memoryRefresh(0), isHalted(false),
    state(CpuState::FETCH), currentOpcode(0) {}

uint8_t Z80BaseCpu::tick(const MmioDeviceManager& bus) {
    uint8_t cycles;
    switch(this->state) {
    case CpuState::FETCH: {
        cycles = FETCH_CYCLES;
        this->fetch(bus);
        this->state = CpuState::DECODE;
        break;
    }
    case CpuState::DECODE: {
        cycles = DECODE_CYCLES;
        this->decode();
        this->state = CpuState::EXECUTE;
        break;
    }
    case CpuState::EXECUTE: {
        cycles = this->execute(bus);
        this->state = CpuState::FETCH;
        break;
    }
    }

    return cycles;
}

void Z80BaseCpu::fetch(const MmioDeviceManager& bus) {
    this->currentOpcode = bus.read8(this->programCounter++);
}

void Z80BaseCpu::decode() {
    switch(this->currentOpcode) {
    case 0xcb:
    case 0xdd:
    case 0xed:
    case 0xfd:
        assert(false && "Cannot handle prefixed-opcodes yet.");
    }

    struct DecodedInstruction decodedOpcode;

    decodedOpcode.x = this->currentOpcode >> 6;
    decodedOpcode.y = 0b00111 & (this->currentOpcode >> 3);
    decodedOpcode.z = 0b00000111 & this->currentOpcode;
    decodedOpcode.p = decodedOpcode.y >> 1;
    decodedOpcode.q = decodedOpcode.y % 2;

    this->currentDecodedInstruction = decodedOpcode;
}

uint8_t Z80BaseCpu::execute(const MmioDeviceManager& bus) {
    auto decodedInst = this->currentDecodedInstruction;
    switch(decodedInst.x) {
    case 0: {
        switch(decodedInst.z) {
        case 6: {
            this->ld_r_imm(bus, tableRSetters[decodedInst.y]);
        }

        default:
            assert(false && "Cannot handle this instruction yet.");
        }
        break;
    }
    default:
        assert(false && "Cannot handle this instruction yet.");
    }

    return 0;
}

} // namespace bz80
