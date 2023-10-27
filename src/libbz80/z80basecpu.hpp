#pragma once

#include <cstdint>

#include "registerinterface.hpp"
#include "mmioDeviceManager.hpp"

namespace bz80 {

class Z80BaseCpu {
protected:
    enum class CpuState {
        FETCH,
        DECODE,
        EXECUTE,
    };

    struct DecodedInstruction {};

    uint16_t programCounter, stackPointer, indexX, indexY;
    uint8_t interruptVector, memoryRefresh;
    bool isHalted;

    RegisterInterface registers, registersAlt;
    CpuState state;

public:
    Z80BaseCpu();
    uint8_t tick(const MmioDeviceManager& bus);

private:
    uint8_t fetch(MmioDeviceManager& bus);
    struct DecodedInstruction decode(uint8_t instruction);
    uint8_t execute(MmioDeviceManager& bus, DecodedInstruction instruction);
};

} // namespace bz80

