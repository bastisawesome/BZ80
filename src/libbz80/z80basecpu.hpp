#pragma once

#include <cstdint>
#include <array>
#include <functional>

#include "mmioDeviceManager.hpp"
#include "registerpairtype.hpp"

class Bz80BaseCpuInstructionsTest;

namespace bz80 {

typedef std::function<void(uint8_t)> regPairT8Getter;

struct FlagRegister {
    bool carry: 1;
    bool add_sub: 1;
    bool overflow: 1;
    bool unused1: 1;
    bool halfcarry: 1;
    bool unused2: 1;
    bool zero: 1;
    bool sign: 1;
};


class Z80BaseCpu {
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
    uint8_t currentOpcode;
    struct DecodedInstruction currentDecodedInstruction;

    CpuState state;

    std::array<regPairT8Getter, 8> tableRSetters {
        std::bind(&RegisterPairType::setUpper8, this->registerBC,
                  std::placeholders::_1),
        std::bind(&RegisterPairType::setLower8, this->registerBC,
                  std::placeholders::_1),
        std::bind(&RegisterPairType::setUpper8, this->registerDE,
                  std::placeholders::_1),
        std::bind(&RegisterPairType::setLower8, this->registerDE,
                  std::placeholders::_1),
        std::bind(&RegisterPairType::setUpper8, this->registerHL,
                  std::placeholders::_1),
        std::bind(&RegisterPairType::setLower8, this->registerHL,
                  std::placeholders::_1),
    };

    friend class ::Bz80BaseCpuInstructionsTest;

public:
    Z80BaseCpu();
    uint8_t tick(const MmioDeviceManager& bus);

protected:
    void fetch(const MmioDeviceManager& bus);
    void decode();
    uint8_t execute(const MmioDeviceManager& bus);

// Instructions
protected:
    uint8_t ld_r_imm(const MmioDeviceManager& bus, regPairT8Getter reg);
};

} // namespace bz80

