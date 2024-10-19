#include <QtTest>

// add necessary includes here
#include <map>
#include <memory>
#include <mmioDevice.hpp>
#include <mmioDeviceManager.hpp>
#include <mmioRam.hpp>

using bz80::MmioDevice;
using bz80::MmioDeviceManager;
using bz80::MmioRam;
using bz80::PortOccupiedException;

class MockPortDevice : public MmioDevice {
public:
    uint8_t data;

    MockPortDevice(): data(0) {}

    uint8_t read8(const uint16_t addr) const override {
        return this->data;
    }

    void write8(const uint16_t addr, const uint8_t value) override {
        this->data = value;
    }

};

class MmioDeviceManagerTest : public QObject {
    Q_OBJECT

public:
    MmioDeviceManagerTest();
    ~MmioDeviceManagerTest();

private:
    std::unique_ptr<MmioDeviceManager> deviceManager;

private slots:
    void init();
    void cleanup();

    void test_get_nearest_device_exact();
    void test_get_nearest_device_not_exact();
    void test_get_nearest_device_one_device();
    void test_get_nearest_device_no_devices();
    void test_read8();
    void test_read8Port();
    void test_read8Port_no_devices();
    void test_read8Mmio();
    void test_read8Mmio_no_devices();
    void test_read16();
    void test_read16_no_devices();
    void test_write8();
    void test_write8Mmio();
    void test_write8Mmio_no_devices();
    void test_write8Port();
    void test_write8Port_no_devices();
    void test_write16();
    void test_write16_no_devices();
    void test_addMmioDevice();
    void test_addPortDevice();
    void test_addPortDevice_occupied();
};

MmioDeviceManagerTest::MmioDeviceManagerTest() { }

MmioDeviceManagerTest::~MmioDeviceManagerTest() { }

void MmioDeviceManagerTest::init() {
    this->deviceManager
        = std::unique_ptr<MmioDeviceManager>(new MmioDeviceManager());
    std::unique_ptr<MmioDevice> ram(new MmioRam<15>());
    ram->write8(0, 42);
    ram->write8(1, 69);

    this->deviceManager->addMmioDevice(0, std::move(ram));

    std::unique_ptr<MockPortDevice> portDevice(new MockPortDevice());
    portDevice->data = 10;
    this->deviceManager->portDevices[0] = std::move(portDevice);

    std::unique_ptr<MockPortDevice> portDevice2(new MockPortDevice());
    portDevice2->data = 74;
    this->deviceManager->portDevices[2] = std::move(portDevice2);
}

void MmioDeviceManagerTest::cleanup() {
    this->deviceManager.reset(nullptr);
}

void MmioDeviceManagerTest::test_get_nearest_device_exact() {
    this->deviceManager->devices[0x8000] = std::make_unique<MmioRam<15>>();
    this->deviceManager->devices[0x8000]->write8(0, 0xbe);
    auto device1 = this->deviceManager->getNearestDevice(0);
    auto device2 = this->deviceManager->getNearestDevice(0x8000);

    QCOMPARE(device1->first, 0);
    QCOMPARE(device2->first, 0x8000);
    QCOMPARE(device1->second->read8(0), 42);
    QCOMPARE(device2->second->read8(0), 0xbe);
}

void MmioDeviceManagerTest::test_get_nearest_device_not_exact() {
    this->deviceManager->devices[0x8000] = std::make_unique<MmioRam<15>>();
    this->deviceManager->devices[0x8000]->write8(5, 0xbe);
    auto const device1 = this->deviceManager->getNearestDevice(1);
    auto const device2 = this->deviceManager->getNearestDevice(0x8005);

    QCOMPARE(device1->first, 0);
    QCOMPARE(device2->first, 0x8000);
    QCOMPARE(device1->second->read8(1), 69);
    QCOMPARE(device2->second->read8(5), 0xbe);
}

void MmioDeviceManagerTest::test_get_nearest_device_one_device() {
    auto const device = this->deviceManager->getNearestDevice(1);

    QCOMPARE(device->first, 0);
    QCOMPARE(device->second->read8(1), 69);
}

void MmioDeviceManagerTest::test_get_nearest_device_no_devices() {
    this->deviceManager->devices.erase(0);
    auto const device = this->deviceManager->getNearestDevice(0);

    QCOMPARE(device, this->deviceManager->devices.end());
}

void MmioDeviceManagerTest::test_read8Mmio() {
    QCOMPARE(this->deviceManager->read8Mmio(0), 42);
    QCOMPARE(this->deviceManager->read8Mmio(1), 69);
}

void MmioDeviceManagerTest::test_read8Mmio_no_devices() {
    this->deviceManager->devices.erase(0);
    this->deviceManager->read8Mmio(0);
}

void MmioDeviceManagerTest::test_read16() {
    // 17706 is the value of decimal 69 shifted up 8-bits, "or"ed with 42
    QCOMPARE(this->deviceManager->read16(0), 17706);
}

void MmioDeviceManagerTest::test_read16_no_devices() {
    this->deviceManager->devices.erase(0);
    this->deviceManager->read16(0);
}

void MmioDeviceManagerTest::test_write8Mmio() {
    this->deviceManager->devices[0x8000] = std::make_unique<MmioRam<15>>();
    this->deviceManager->write8Mmio(3, 0xad);
    this->deviceManager->write8Mmio(0x8000, 0xbe);

    QCOMPARE(this->deviceManager->devices[0]->read8(3), 0xad);
    QCOMPARE(this->deviceManager->devices[0x8000]->read8(0), 0xbe);
}

void MmioDeviceManagerTest::test_write8Mmio_no_devices() {
    this->deviceManager->devices.erase(0);
    this->deviceManager->write8Mmio(0, 1);
}

void MmioDeviceManagerTest::test_write16() {
    this->deviceManager->devices[0x8000] = std::make_unique<MmioRam<15>>();
    this->deviceManager->write16(3, 0xdead);
    this->deviceManager->write16(0x8008, 0xbeef);

    QCOMPARE(this->deviceManager->devices[0]->read8(3), 0xad);
    QCOMPARE(this->deviceManager->devices[0]->read8(4), 0xde);
    QCOMPARE(this->deviceManager->devices[0x8000]->read8(8), 0xef);
    QCOMPARE(this->deviceManager->devices[0x8000]->read8(9), 0xbe);
}

void MmioDeviceManagerTest::test_write16_no_devices() {
    this->deviceManager->devices.erase(0);
    this->deviceManager->write16(0, 1);
}

void MmioDeviceManagerTest::test_addMmioDevice() {
    auto newDevice = std::make_unique<MmioRam<15>>();
    newDevice->write8(0, 0xef);
    newDevice->write8(1, 0xbe);
    this->deviceManager->addMmioDevice(0x16, std::move(newDevice));

    QVERIFY(this->deviceManager->devices[0x16]);
    QCOMPARE(this->deviceManager->read16(0x16), 0xbeef);
}

void MmioDeviceManagerTest::test_read8() {
    uint8_t data;

    // Test memory-mapped device lookup.
    data = this->deviceManager->read8(0, false);
    QCOMPARE(data, 42);
    data = this->deviceManager->read8(1, false);
    QCOMPARE(data, 69);

    // Test port-mapped device lookup.
    data = this->deviceManager->read8(0, true);
    QCOMPARE(data, 10);

    std::unique_ptr<MockPortDevice> newDevice(new MockPortDevice());
    newDevice->data = 65;
    this->deviceManager->portDevices[1] = std::move(newDevice);

    data = this->deviceManager->read8(1, true);
    QCOMPARE(data, 65);
}

void MmioDeviceManagerTest::test_read8Port() {
    QCOMPARE(this->deviceManager->read8Port(0), 10);
    QCOMPARE(this->deviceManager->read8Port(2), 74);
}

void MmioDeviceManagerTest::test_read8Port_no_devices() {
    QCOMPARE(this->deviceManager->read8Port(1), 0);
}

void MmioDeviceManagerTest::test_write8() {
    this->deviceManager->write8(3, 100, false);
    QCOMPARE(this->deviceManager->devices[0]->read8(3), 100);

    this->deviceManager->write8(0, 254, true);
    MockPortDevice* device = (MockPortDevice*)this->deviceManager->portDevices[0].get();
    QCOMPARE(device->data, 254);
}

void MmioDeviceManagerTest::test_write8Port() {
    this->deviceManager->write8Port(0, 200);
    QCOMPARE(((MockPortDevice*)this->deviceManager->portDevices[0].get())->data, 200);

    this->deviceManager->write8Port(2, 92);
    QCOMPARE(((MockPortDevice*)this->deviceManager->portDevices[2].get())->data, 92);
}

void MmioDeviceManagerTest::test_write8Port_no_devices() {
    this->deviceManager->write8Port(1, 39);
    this->deviceManager->write8Port(243, 184);

    QCOMPARE(((MockPortDevice*)this->deviceManager->portDevices[0].get())->data, 10);
    QCOMPARE(((MockPortDevice*)this->deviceManager->portDevices[2].get())->data, 74);
}

void MmioDeviceManagerTest::test_addPortDevice() {
    std::unique_ptr<MockPortDevice> portDevice(new MockPortDevice);
    portDevice->data = 29;
    this->deviceManager->addPortDevice(1, std::move(portDevice));

    QVERIFY(this->deviceManager->portDevices[1] != nullptr);
    QCOMPARE(((MockPortDevice*)this->deviceManager->portDevices[1].get())->data, 29);
}

void MmioDeviceManagerTest::test_addPortDevice_occupied() {
    std::unique_ptr<MockPortDevice> portDevice(new MockPortDevice());
    QVERIFY_THROWS_EXCEPTION(PortOccupiedException,
        this->deviceManager->addPortDevice(0, std::move(portDevice)));

    portDevice.reset(new MockPortDevice());
    QVERIFY_THROWS_EXCEPTION(PortOccupiedException,
        this->deviceManager->addPortDevice(2, std::move(portDevice)));

    // Finally, ensure there isn't something silly going on. Let's add the
    // device to a known-good port, then add it again!
    portDevice.reset(new MockPortDevice());
    QVERIFY_THROWS_NO_EXCEPTION(this->deviceManager->addPortDevice(200,
        std::move(portDevice)));
    portDevice.reset(new MockPortDevice());
    QVERIFY_THROWS_EXCEPTION(PortOccupiedException,
        this->deviceManager->addPortDevice(200, std::move(portDevice)));

}

QTEST_APPLESS_MAIN(MmioDeviceManagerTest)

#include "tst_mmiodevicemanager.moc"
