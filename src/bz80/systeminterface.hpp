#pragma once

#include <memory>
#include <cstdint>
#include <mmioDeviceManager.hpp>
#include <mmioDevice.hpp>
#include <z80BaseCpu.hpp>

class SystemInterface {
private:
    std::unique_ptr<MmioDeviceManager> mmioManager;
    std::unique_ptr<Z80BaseCpu> cpu;

public:
    SystemInterface(std::unique_ptr<Z80BaseCpu>, std::unique_ptr<MmioDeviceManager>);
    SystemInterface(std::unique_ptr<Z80BaseCpu>);
    void addMmioDevice(uint16_t const addr, std::unique_ptr<MmioDevice> device) const;
    void run();
};
