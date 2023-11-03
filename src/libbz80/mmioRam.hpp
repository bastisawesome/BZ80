#pragma once

#include <cstdint>
#include <array>

#include "mmioDevice.hpp"

class MmioRamTest;

namespace bz80 {

template<uint8_t N>
class MmioRam: public MmioDevice {
private:
    std::array<uint8_t, N> data;

    friend class ::MmioRamTest;

public:
    MmioRam():
        data(std::array<uint8_t, N>()){}

    /**
     * @brief read8 Returns value stored in memory at `addr`.
     * @param addr Address to read from, adjusted for the module.
     * @return The value stored at address `addr`.
     *
     * `addr` is assumed to be adjusted before being passed.
     * Returns the value at `addr` or 0 if out of bounds.
     */
    uint8_t read8(const uint16_t addr) const override {
        if(addr > this->data.size()) {
            return 0;
        }

        return this->data[addr];
    }

    /**
     * @brief write8 Writes `value` to `addr`.
     * @param addr Address to write to.
     * @param value Value to write at the specified address.
     *
     * Writes the specified `value` to `addr`. `addr` is assumed to be
     * adjusted before being passed. If `addr` is out of bounds, write is
     * ignored.
     */
    void write8(const uint16_t addr, const uint8_t value) override {
        if(addr > this->data.size()) {
            return;
        }

        this->data[addr] = value;
    }
};

}
