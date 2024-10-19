#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <utility>
#include <array>

#include "mmioDevice.hpp"

class MmioDeviceManagerTest;

namespace bz80 {

/**
 * @brief Indicates that a port-mapped device has been assigned to a port
 *      already in use.
 */
class PortOccupiedException : std::exception {};

class MmioDeviceManager {
private:
    std::map<uint16_t, std::unique_ptr<MmioDevice>> devices;
    std::array<std::unique_ptr<MmioDevice>, 256> portDevices;

    std::map<uint16_t, std::unique_ptr<MmioDevice>>::const_iterator
    getNearestDevice(uint16_t addr) const {
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

    /**
     * @brief read8Port Returns the value from device mapped to `addr`
     * @param addr Port of the device to read from
     * @return Data emitted by the device, or 0 if no device found
     *
     * Finds the device mapped to port `addr`. Unlike `read8Mmio`, each device
     * must be mapped directly to an address with no overlaps. Devices cannot
     * share address space. Returns the value that device is emitting on the
     * data bus, or 0 if the device is not found or is not emitting.
     */
    uint8_t read8Port(uint8_t const addr) const {
        try {
            // I've found out that `at` does bounds-checking but does not ensure
            // non-null pointers. So have some null pointer checking!
            if(this->portDevices.at(addr) != nullptr) {
                return this->portDevices[addr]->read8(0);
            }
            return 0;
        } catch(const std::out_of_range &ex) {
            return 0;
        }
    }

    /**
     * @brief read8 Returns the value at `addr` from the device mapped to
     * `addr`.
     * @param addr Memory address of the device.
     * @return Value stored at that memory address.
     *
     * Finds the nearest device mapped to `addr` and returns the value stored
     * in that device at `addr`.
     */
    uint8_t read8Mmio(uint16_t const addr) const {
        if(this->devices.empty()) {
            return 0;
        }

        auto const devIter = this->getNearestDevice(addr);
        uint16_t adjustedAddr = addr & ~devIter->first;
        return devIter->second->read8(adjustedAddr);
    }

    /**
     * @brief write8Port Writes `value` to port-mapped device at `addr`
     * @param addr Port number of the device
     * @param value Value to write to the device. Ignored if no device mapped.
     *
     * Finds the device mapped to port `addr` and sends it the value. The value
     * is dropped if there is no available device. Like `read8Port`, only one
     * device may be mapped to `addr`, with no overlap.
     */
    void write8Port(uint8_t const addr, uint8_t const value) const {
        try {
            // Do nothing if no device is attached.
            if(this->portDevices.at(addr) != nullptr) {
                this->portDevices[addr]->write8(0, value);
            }
        } catch(std::out_of_range &ex) {
            // Do nothing.
        }
    }

    /**
     * @brief write8 Writes `value` to device at address `addr`.
     * @param addr Address of the device to write data to.
     * @param value Value to be written to the device.
     *
     * Finds the nearest device mapped to `addr` and writes `value` to that
     * device based on the local, adjusted address.
     */
    void write8Mmio(uint16_t const addr, uint8_t const value) const {
        if(this->devices.empty()) {
            return;
        }

        auto const devIter = this->getNearestDevice(addr);
        uint16_t adjustedAddr = addr & ~devIter->first;
        devIter->second->write8(adjustedAddr, value);
    }

    /**
     * @brief addPortDevice Adds a new port-mapped device.
     * @param addr Port number to assign to the device.
     * @param device Device to be assigned to the port.
     * @throws bz80::PortOccupiedException Thrown if `addr` is already in use by
     *      another device.
     *
     * Adds a new `MmioDevice` as a port-mapped device, taking ownership of it.
     * Port-mapped devices cannot overlap, each device can take up a single byte
     * in the address space. Up to 256 devices can be mapped.
     *
     * Attempting to map a device to a port already in use will raise a
     * `PortOccupiedException`.
     */
    void addPortDevice(uint8_t const addr, std::unique_ptr<MmioDevice> device) {
        if(this->portDevices[addr] != nullptr) {
            throw PortOccupiedException();
        }

        this->portDevices[addr] = std::move(device);
    }

    friend class ::MmioDeviceManagerTest;

public:
    MmioDeviceManager()
        : devices(std::map<uint16_t, std::unique_ptr<MmioDevice>>()),
        portDevices({0}) { }

    /**
     * @brief read8 Returns the value at MMIO or port-mapped device at `addr`
     * @param addr Address or port number of the device to read from
     * @param iorq Set if the device is port-mapped
     * @return Value read from the device
     *
     * Finds the memory-mapped or port-mapped device at `addr` and returns the
     * value from the device. If the device is not found, returns 0.
     */
    uint8_t read8(uint16_t const addr, bool const iorq) const {
        if(iorq) {
            return read8Port((uint8_t)addr & 0xFF);
        }

        return read8Mmio(addr);
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

        value = this->read8Mmio(addr);
        value |= this->read8Mmio(addr + 1) << 8;

        return value;
    }

    void write8(uint16_t const addr, uint8_t const value, bool const iorq)
    const {
        if(iorq) {
            this->write8Port((uint8_t)addr & 0xFF, value);
        }

        return write8Mmio(addr, value);
    }

    /**
     * @brief write16 Writes `value` to the device(s) located at
     * `addr`/`addr`+1.
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

        write8Mmio(addr, static_cast<uint8_t>(value));
        write8Mmio(addr + 1, value >> 8);
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
    void addMmioDevice(uint16_t const addr, std::unique_ptr<MmioDevice> device) {
        this->devices[addr] = std::move(device);
    }
};

}
