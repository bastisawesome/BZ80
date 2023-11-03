#include <QtTest>

// add necessary includes here
#include <memory>
#include <mmioDeviceManager.hpp>
#include <mmioRam.hpp>
#include <mmioDevice.hpp>
#include <map>

using bz80::MmioDeviceManager;
using bz80::MmioRam;
using bz80::MmioDevice;

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
    void test_read8_no_devices();
    void test_read16();
    void test_read16_no_devices();
    void test_write8();
    void test_write8_no_devices();
    void test_write16();
    void test_write16_no_devices();
    void test_add_device();
};

MmioDeviceManagerTest::MmioDeviceManagerTest() {}

MmioDeviceManagerTest::~MmioDeviceManagerTest() {}

void MmioDeviceManagerTest::init() {
    this->deviceManager = std::unique_ptr<MmioDeviceManager>(new MmioDeviceManager());
    std::unique_ptr<MmioDevice> ram(new MmioRam<15>());
    ram->write8(0, 42);
    ram->write8(1, 69);

    this->deviceManager->addDevice(0, std::move(ram));
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
    auto const whatIsThis = &device->second;

    QCOMPARE(device, this->deviceManager->devices.end());
}

void MmioDeviceManagerTest::test_read8() {
    QCOMPARE(this->deviceManager->read8(0), 42);
    QCOMPARE(this->deviceManager->read8(1), 69);
}

void MmioDeviceManagerTest::test_read8_no_devices() {
    this->deviceManager->devices.erase(0);
    this->deviceManager->read8(0);
}

void MmioDeviceManagerTest::test_read16() {
    // 17706 is the value of decimal 69 shifted up 8-bits, "or"ed with 42
    QCOMPARE(this->deviceManager->read16(0), 17706);
}

void MmioDeviceManagerTest::test_read16_no_devices() {
    this->deviceManager->devices.erase(0);
    this->deviceManager->read16(0);
}

void MmioDeviceManagerTest::test_write8() {
    this->deviceManager->devices[0x8000] = std::make_unique<MmioRam<15>>();
    this->deviceManager->write8(3, 0xad);
    this->deviceManager->write8(0x8000, 0xbe);

    QCOMPARE(this->deviceManager->devices[0]->read8(3), 0xad);
    QCOMPARE(this->deviceManager->devices[0x8000]->read8(0), 0xbe);
}

void MmioDeviceManagerTest::test_write8_no_devices() {
    this->deviceManager->devices.erase(0);
    this->deviceManager->write8(0, 1);
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

void MmioDeviceManagerTest::test_add_device() {
    auto newDevice = std::make_unique<MmioRam<15>>();
    newDevice->write8(0, 0xef);
    newDevice->write8(1, 0xbe);
    this->deviceManager->addDevice(0x16, std::move(newDevice));

    QVERIFY(this->deviceManager->devices[0x16]);
    QCOMPARE(this->deviceManager->read16(0x16), 0xbeef);
}

QTEST_APPLESS_MAIN(MmioDeviceManagerTest)

#include "tst_mmiodevicemanager.moc"
