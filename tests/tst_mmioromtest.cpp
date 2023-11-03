#include <QTest>

#include <mmioROM.hpp>

using bz80::MmioRom;

class MmioRomTest: public QObject {
    Q_OBJECT

public:
    MmioRomTest();
    ~MmioRomTest();

private:
    std::unique_ptr<MmioRom<15>> rom;

private slots:
    void init();
    void cleanup();

    void test_read8_in_bounds();
    void test_read8_out_of_bounds();
    void test_write8_in_bounds();
    void test_write8_out_of_bounds();
};

MmioRomTest::MmioRomTest() {}
MmioRomTest::~MmioRomTest() {}

void MmioRomTest::init() {
    this->rom = std::unique_ptr<MmioRom<15>>(new MmioRom<15>());
    this->rom->data[0] = 0xbe;
    this->rom->data[1] = 0x69;
}

void MmioRomTest::cleanup() {
    this->rom.reset(nullptr);
}

void MmioRomTest::test_read8_in_bounds() {
    QCOMPARE(this->rom->read8(0), 0xbe);
    QCOMPARE(this->rom->read8(1), 0x69);

    for(int i=2; i<this->rom->data.size(); i++) {
        QCOMPARE(this->rom->read8(i), 0);
    }
}

void MmioRomTest::test_read8_out_of_bounds() {}

void MmioRomTest::test_write8_in_bounds() {}

void MmioRomTest::test_write8_out_of_bounds() {}

QTEST_APPLESS_MAIN(MmioRomTest)

#include "tst_mmioromtest.moc"
