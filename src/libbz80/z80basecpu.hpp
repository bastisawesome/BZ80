#pragma once

#include <cstdint>
#include <array>
#include <functional>

#include "mmioDeviceManager.hpp"
#include "registerpairtype.hpp"

#define FETCH_CYCLES (uint8_t)4
#define DECODE_CYCLES (uint8_t)0
#define MEMORY_ACCESS_CYCLES (uint8_t)3

class Bz80BaseCpuInstructionsTest;

namespace bz80 {

struct FlagRegister {
    bool carry: 1;
    bool add_sub: 1;
    bool overflow: 1;
    bool unused1: 1;
    bool halfcarry: 1;
    bool unused2: 1;
    bool zero: 1;
    bool sign: 1;

    bool operator==(const FlagRegister&) const = default;
};

class Z80BaseCpu {
protected:
    void fetch();
    void decode();
    uint8_t execute();

    void setRegisterA(uint8_t value);
    void setMemoryAddressAtHL(uint8_t value);
    uint8_t getRegisterA();
    uint8_t getMemoryAddressAtHl();

protected:
    enum class CpuState {
        FETCH,
        DECODE,
        EXECUTE,
    };

    /*
     * x = the opcode's 1st octal digit (i.e. bits 7-6)
     * y = the opcode's 2nd octal digit (i.e. bits 5-3)
     * z = the opcode's 3rd octal digit (i.e. bits 2-0)
     * p = y rightshifted one position (i.e. bits 5-4)
     * q = y modulo 2 (i.e. bit 3)
     */
    struct DecodedInstruction {
        uint8_t x, y, z, p, q;
    };

    // Registers
    uint8_t registerA;
    RegisterPairType registerBC, registerDE, registerHL;
    struct FlagRegister registerF;

    uint8_t registerA_alt;
    RegisterPairType registerBC_alt, registerDE_alt, registerHL_alt;
    struct FlagRegister registerF_alt;

    uint16_t programCounter, stackPointer, indexX, indexY;
    uint8_t interruptVector, memoryRefresh;

    bool isHalted;
    const MmioDeviceManager& bus;
    uint8_t currentOpcode;
    struct DecodedInstruction currentDecodedInstruction;

    CpuState state;

    friend class ::Bz80BaseCpuInstructionsTest;

public:
    Z80BaseCpu(const MmioDeviceManager& bus);
    uint8_t tick();

// Instructions
protected:
    uint8_t ld_r_imm(const MmioDeviceManager& bus);
    uint8_t inc_r(const MmioDeviceManager& bus);
};

} // namespace bz80

