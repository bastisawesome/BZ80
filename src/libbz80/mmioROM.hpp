#pragma once

#include <cstdint>
#include <array>

#include "mmioDevice.hpp"

template<uint16_t N>
class MmioRom: MmioDevice {
private:
    std::array<uint8_t, N> data;

public:
    MmioRom():
        data(std::array<uint8_t, N>()) {}

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
     * Exists only to satisfy the requirements for MmioDevice. ROM cannot be
     * written to.
     */
    void write8(const uint16_t addr, const uint8_t value) override {
        // Do nothing.
    }
};
