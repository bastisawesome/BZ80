#pragma once

#include <QDebug>
#include <array>
#include <cstdint>
#include <functional>

#include "mmioDeviceManager.hpp"
#include "registerpairtype.hpp"

#define FETCH_CYCLES (uint8_t)4
#define DECODE_CYCLES (uint8_t)0
#define MEMORY_ACCESS_CYCLES (uint8_t)3
#define INC_DEC_REG_CYCLES (uint8_t)1
#define TEST_REG_CYCLES (uint8_t)3

/*
 * TODO: Remove `bus` from instruction methods.
 */

class Bz80BaseCpuInstructionsTest;

namespace bz80 {

class UnimplementedInstructionException : public std::runtime_error {
public:
    UnimplementedInstructionException()
        : std::runtime_error("") {
        this->message = "Instruction not implemented (unknown instruction)";
    }
    UnimplementedInstructionException(const std::string& instrName)
        : std::runtime_error(instrName) {
        this->message
            = std::string("Instruction not implemented: ") + instrName;
    }

    const char* what() noexcept {
        return message.c_str();
    }

private:
    std::string message;
};

struct FlagRegister {
    bool carry : 1;
    bool add_sub : 1;
    bool overflow : 1;
    bool unused1 : 1;
    bool halfcarry : 1;
    bool unused2 : 1;
    bool zero : 1;
    bool sign : 1;

    bool operator==(const FlagRegister&) const = default;

    operator QString() const {
        QString repr;
        repr += "FlagRegister {\n\tCarry: " + QString::number(this->carry)
            + "\n\tAdd/Sub: " + QString::number(this->add_sub)
            + "\n\tOverflow: " + QString::number(this->overflow)
            + "\n\tUnused: " + QString::number(this->unused1)
            + "\n\tHalf-carry: " + QString::number(this->halfcarry)
            + "\n\tUnused: " + QString::number(this->unused2)
            + "\n\tZero: " + QString::number(this->zero)
            + "\n\tNegative: " + QString::number(this->sign) + "}";
        return repr;
    }
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
    uint8_t dec_r(const MmioDeviceManager& bus);
    uint8_t nop();
    uint8_t djnz(const MmioDeviceManager& bus);
    uint8_t jr_imm(const MmioDeviceManager& bus);
    uint8_t jr_cc_imm(const MmioDeviceManager& bus);
    uint8_t add_a_r(const MmioDeviceManager& bus);
    uint8_t sub_r(const MmioDeviceManager& bus);
    uint8_t ld_r_r(const MmioDeviceManager& bus);
};

} // namespace bz80
