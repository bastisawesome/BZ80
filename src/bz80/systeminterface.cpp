#include <cstdint>
#include <memory>
#include <mmioDevice.hpp>
#include <mmioDeviceManager.hpp>
#include <z80basecpu.hpp>

#include "systeminterface.hpp"

SystemInterface::SystemInterface(std::unique_ptr<Z80BaseCpu> cpu,
    std::unique_ptr<MmioDeviceManager> mmioManager)
    : cpu(std::move(cpu))
    , mmioManager(std::move(mmioManager)) { }

SystemInterface::SystemInterface(std::unique_ptr<Z80BaseCpu> cpu)
    : cpu(std::move(cpu)) {
    this->mmioManager
        = std::unique_ptr<MmioDeviceManager>(new MmioDeviceManager());
}

void SystemInterface::addMmioDevice(
    uint16_t const addr, std::unique_ptr<MmioDevice> device) const {
    this->mmioManager->addMmioDevice(addr, std::move(device));
}

void SystemInterface::run() { }
