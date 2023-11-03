#pragma once

#include <map>
#include <memory>
#include <cstdint>
#include <utility>

#include "mmioDevice.hpp"

class MmioDeviceManagerTest;

namespace bz80 {

class MmioDeviceManager {
private:
    std::map<uint16_t, std::unique_ptr<MmioDevice>> devices;

    std::map<uint16_t, std::unique_ptr<MmioDevice>>::const_iterator getNearestDevice(uint16_t addr) const {
        auto found = this->devices.lower_bound(addr);

        if(found == this->devices.end()) {
            if(this->devices.empty()) {
                return found;
            }

            return --found;
        }

        if(found->first == addr) {
            return found;
        }

        return --found;
    }

    friend class ::MmioDeviceManagerTest;

public:
    MmioDeviceManager():
    devices(std::map<uint16_t, std::unique_ptr<MmioDevice>>()){}

    /**
     * @brief read8 Returns the value at `addr` from the device mapped to `addr`.
     * @param addr Memory address of the device.
     * @return Value stored at that memory address.
     *
     * Finds the nearest device mapped to `addr` and returns the value stored
     * in that device at `addr`.
     */
    uint8_t read8(uint16_t const addr) const {
        if(this->devices.empty()) {
            return 0;
        }

        auto const devIter = this->getNearestDevice(addr);
        uint16_t adjustedAddr = addr & ~devIter->first;
        return devIter->second->read8(adjustedAddr);
    }

    /**
     * @brief read8 Returns 16-bit value stored at device(s) at `addr`/`addr`+1.
     * @param addr Memory address of the device(s).
     * @return Value stored at the `addr`/`addr`+1 of the device(s)
     *
     * Finds the nearest device mapped to `addr` and returns the value stored
     * in that device at `addr`.
     */
    uint16_t read16(uint16_t const addr) const {
        if(this->devices.empty()) {
            return 0;
        }

        uint16_t value;

        value = this->read8(addr);
        value |= this->read8(addr+1) << 8;

        return value;
    }

    /**
     * @brief write8 Writes `value` to device at address `addr`.
     * @param addr Address of the device to write data to.
     * @param value Value to be written to the device.
     *
     * Finds the nearest device mapped to `addr` and writes `value` to that
     * device based on the local, adjusted address.
     */
    void write8(uint16_t const addr, uint8_t const value) const {
        if(this->devices.empty()) {
            return;
        }

        auto const devIter = this->getNearestDevice(addr);
        uint16_t adjustedAddr = addr & ~devIter->first;
        devIter->second->write8(adjustedAddr, value);
    }

    /**
     * @brief write16 Writes `value` to the device(s) located at `addr`/`addr`+1.
     * @param addr Address of the device(s) to write to.
     * @param value Value to be written to the device(s).
     *
     * Writes the upper 8-bits of `value` to device mapped to `addr`, then
     * writes the lower 8-bits of `value` to device mapped to `addr`+1.
     */
    void write16(uint16_t const addr, uint16_t const value) const {
        if(this->devices.empty()) {
            return;
        }

        write8(addr, static_cast<uint8_t>(value));
        write8(addr+1, value >> 8);
    }

    /**
     * @brief addDevice Adds a new `MmioDevice` to the manager.
     * @param addr Base address to assign to the device. `addr` = device[0].
     * @param device Unique pointer to device to add to the manager.
     *
     * Adds a new `MmioDevice` to the manager. Assumes ownership of `device`. If
     * the base address overlaps with another base address, some address space
     * may become inaccessible. If the address is the same as another device,
     * that device will be replaced.
     */
    void addDevice(uint16_t const addr, std::unique_ptr<MmioDevice> device) {
        this->devices[addr] = std::move(device);
    }
};

}
