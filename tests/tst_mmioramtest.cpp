#include <QtTest>

// add necessary includes here
#include <memory>
#include <mmioRam.hpp>

using bz80::MmioRam;

namespace bz80 {
class MmioRamTest : public QObject {
    Q_OBJECT

public:
    MmioRamTest();
    ~MmioRamTest();

private:
    std::unique_ptr<MmioRam<15>> ramModule;

private slots:
    void init();
    void cleanup();

    void test_read8_in_bounds();
    void test_read8_out_of_bounds();
    void test_write8_in_bounds();
    void test_write8_out_of_bounds();
    void test_write8_overwrite();
};

MmioRamTest::MmioRamTest() {}

MmioRamTest::~MmioRamTest() {}

void MmioRamTest::init() {
    this->ramModule = std::unique_ptr<MmioRam<15>>(new MmioRam<15>);
    this->ramModule->data[0] = 0xca;
    this->ramModule->data[1] = 0xfe;
}

void MmioRamTest::cleanup() {
    this->ramModule.reset(nullptr);
}

void MmioRamTest::test_read8_in_bounds() {
    QCOMPARE(this->ramModule->read8(0), 0xca);
    QCOMPARE(this->ramModule->read8(1), 0xfe);
}

void MmioRamTest::test_read8_out_of_bounds() {
    QCOMPARE(this->ramModule->read8(197), 0x0);
}

void MmioRamTest::test_write8_in_bounds() {
    this->ramModule->write8(4, 69);

    QCOMPARE(this->ramModule->data[4], 69);
    QCOMPARE(this->ramModule->data[0], 0xca);
    QCOMPARE(this->ramModule->data[1], 0xfe);
}

void MmioRamTest::test_write8_out_of_bounds() {
    this->ramModule->write8(600, 42);

    QCOMPARE(this->ramModule->data[0], 0xca);
    QCOMPARE(this->ramModule->data[1], 0xfe);

    for(int i=2; i < this->ramModule->data.size(); i++) {
        QCOMPARE(this->ramModule->data[i], 0);
    }
}

void MmioRamTest::test_write8_overwrite() {
    this->ramModule->write8(0, 69);

    QCOMPARE(this->ramModule->data[0], 69);
}

}
QTEST_APPLESS_MAIN(bz80::MmioRamTest)

#include "tst_mmioramtest.moc"
