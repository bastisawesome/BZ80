#include "z80basecpu.hpp"

namespace bz80 {

Z80BaseCpu::Z80BaseCpu(const MmioDeviceManager& bus) :
    registerA(0), registerBC(0), registerDE(0), registerHL(0), registerF({ 0 }),
    registerA_alt(0), registerBC_alt(0), registerDE_alt(0), registerHL_alt(0),
    registerF_alt({ 0 }), programCounter(0), stackPointer(0), indexX(0),
    indexY(0), interruptVector(0), memoryRefresh(0), isHalted(false),
    state(CpuState::FETCH), currentOpcode(0), bus(bus) { }

uint8_t Z80BaseCpu::tick() {
    uint8_t cycles;
    switch(this->state) {
    case CpuState::FETCH: {
        cycles = FETCH_CYCLES;
        this->fetch();
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
        cycles = this->execute();
        this->state = CpuState::FETCH;
        break;
    }
    }

    return cycles;
}

void Z80BaseCpu::fetch() {
    this->currentOpcode = this->bus.read8(this->programCounter++);
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

uint8_t Z80BaseCpu::execute() {
    uint8_t cycles = 0;
    auto decodedInst = this->currentDecodedInstruction;
    switch(decodedInst.x) {
    case 0: {
        switch(decodedInst.z) {
        case 0: {
            switch(decodedInst.y) {
            case 0:
                return this->nop();
            case 2:
                return this->djnz(bus);
            case 3:
                return this->jr_imm(bus);
            default:
                if(4 <= decodedInst.y && decodedInst.y <= 7) {
                    return this->jr_cc_imm(bus);
                }
                throw UnimplementedInstructionException(
                    std::string("EX AF,AF'"));
            }
        }
        case 4: {
            cycles += this->inc_r(bus);
            return cycles;
        }
        case 5: {
            cycles += this->dec_r(bus);
            return cycles;
        }
        case 6: {
            cycles += this->ld_r_imm(bus);
            return cycles;
        }

        default:
            throw UnimplementedInstructionException();
        }
        break;
    }
    case 1: {
        if(this->currentDecodedInstruction.z == 6
            && this->currentDecodedInstruction.y == 6) {
            throw UnimplementedInstructionException("HALT");
        }
        return this->ld_r_r(bus);
    }
    case 2: {
        switch(this->currentDecodedInstruction.y) {
        case 0:
            return this->add_a_r(bus);
        case 2:
            return this->sub_r(bus);
        default:
            throw UnimplementedInstructionException("ALU instruction...");
        }
    }
    default:
        throw UnimplementedInstructionException();
    }

    return cycles;
}

} // namespace bz80
