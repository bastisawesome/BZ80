#include <QTest>

#include <mmioDeviceManager.hpp>
#include <mmioRam.hpp>
#include <registerpairtype.hpp>
#include <z80basecpu.hpp>

using bz80::FlagRegister;
using bz80::MmioDeviceManager;
using bz80::MmioRam;
using bz80::RegisterPairType;
using bz80::Z80BaseCpu;

/**
 * TODO: Merge INC r and DEC r instruction tests, they are identical-ish.
 */

class Bz80BaseCpuInstructionsTest : public QObject {
    Q_OBJECT

public:
    Bz80BaseCpuInstructionsTest();
    ~Bz80BaseCpuInstructionsTest();

private:
    Z80BaseCpu* cpu;
    MmioDeviceManager bus;
private slots:
    void init();
    void cleanup();

    // LD r, IMM
    void test_ld_r_imm_data();
    void test_ld_r_imm();
    void test_ld_addr_hl_imm();
    void test_ld_a_imm();

    // INC r
    void test_inc_r_data();
    void test_inc_r();
    void test_inc_addr_hl();
    void test_inc_a();

    // DEC r
    void test_dec_r_data();
    void test_dec_r();
    void test_dec_addr_hl();
    void test_dec_a();
};

Bz80BaseCpuInstructionsTest::Bz80BaseCpuInstructionsTest() {
    this->bus = MmioDeviceManager();
    this->cpu = new Z80BaseCpu(bus);
}

Bz80BaseCpuInstructionsTest::~Bz80BaseCpuInstructionsTest() {
    delete this->cpu;
}

void Bz80BaseCpuInstructionsTest::init() {
    std::unique_ptr<MmioRam<16>> ram(new MmioRam<16>());
    ram->write8(0, 15);
    this->bus.addDevice(0, std::move(ram));
}

void Bz80BaseCpuInstructionsTest::cleanup() {
    delete this->cpu;

    this->cpu = new Z80BaseCpu(bus);
}

void Bz80BaseCpuInstructionsTest::test_ld_r_imm_data() {
    QTest::addColumn<uint8_t>("opcode");
    QTest::addColumn<uint8_t>("immediate");
    QTest::addColumn<RegisterPairType*>("expectedRegister");
    QTest::addColumn<bool>("isLowByte");
    QTest::addColumn<FlagRegister>("startingFlags");
    QTest::addColumn<uint8_t>("expectedCycles");

    QTest::newRow("LD B, 15")
        << (uint8_t)0x06 << (uint8_t)15 << &this->cpu->registerBC << false
        << FlagRegister { true, false, false, false, false, false, false,
               false }
        << MEMORY_ACCESS_CYCLES;
    QTest::newRow("LD C, 26")
        << (uint8_t)0x0E << (uint8_t)26 << &this->cpu->registerBC << true
        << FlagRegister { false, true, false, false, false, false, false,
               false }
        << MEMORY_ACCESS_CYCLES;
    QTest::newRow("LD D, 92")
        << (uint8_t)0x16 << (uint8_t)92 << &this->cpu->registerDE << false
        << FlagRegister { false, false, true, false, false, false, false,
               false }
        << MEMORY_ACCESS_CYCLES;
    QTest::newRow("LD E, 0xad")
        << (uint8_t)0x1E << (uint8_t)0xad << &this->cpu->registerDE << true
        << FlagRegister { false, false, false, true, false, false, false,
               false }
        << MEMORY_ACCESS_CYCLES;
    QTest::newRow("LD H, 69")
        << (uint8_t)0x26 << (uint8_t)69 << &this->cpu->registerHL << false
        << FlagRegister { false, false, false, false, true, false, false,
               false }
        << MEMORY_ACCESS_CYCLES;
    QTest::newRow("LD L, 142")
        << (uint8_t)0x2E << (uint8_t)142 << &this->cpu->registerHL << true
        << FlagRegister { false, false, false, false, false, true, false,
               false }
        << MEMORY_ACCESS_CYCLES;
}

void Bz80BaseCpuInstructionsTest::test_ld_r_imm() {
    QFETCH(uint8_t, opcode);
    QFETCH(uint8_t, immediate);
    QFETCH(RegisterPairType*, expectedRegister);
    QFETCH(bool, isLowByte);
    QFETCH(FlagRegister, startingFlags);
    QFETCH(uint8_t, expectedCycles);

    this->cpu->bus.write8(this->cpu->programCounter, immediate);

    this->cpu->currentOpcode = opcode;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    this->cpu->registerF = startingFlags;
    uint8_t foundCycles = this->cpu->tick();

    uint8_t foundValue = isLowByte ? expectedRegister->getLower8()
                                   : expectedRegister->getUpper8();

    QCOMPARE(foundValue, immediate);
    auto foundFlags = this->cpu->registerF;
    QCOMPARE(foundFlags, startingFlags);
    QCOMPARE(foundCycles, expectedCycles);
}

void Bz80BaseCpuInstructionsTest::test_ld_addr_hl_imm() {
    uint8_t opcode = 0x36;
    uint8_t immediate = 7;
    FlagRegister startingFlags { false, false, false, false, false, false, true,
        false };
    uint8_t expectedCycles = MEMORY_ACCESS_CYCLES * 2;

    this->cpu->bus.write8(this->cpu->programCounter, immediate);

    this->cpu->currentOpcode = opcode;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    this->cpu->registerF = startingFlags;
    this->cpu->registerHL.set16(15);
    uint8_t foundCycles = this->cpu->tick();

    QCOMPARE(this->bus.read8(15), immediate);
    auto foundFlags = this->cpu->registerF;
    QCOMPARE(foundFlags, startingFlags);
    QCOMPARE(foundCycles, expectedCycles);
}

void Bz80BaseCpuInstructionsTest::test_ld_a_imm() {
    uint8_t opcode = 0x3e;
    uint8_t immediate = 72;
    FlagRegister startingFlags { false, false, false, false, false, false,
        false, true };
    uint8_t expectedCycles = MEMORY_ACCESS_CYCLES;

    this->cpu->bus.write8(this->cpu->programCounter, immediate);
    this->cpu->currentOpcode = opcode;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    this->cpu->registerF = startingFlags;
    uint8_t foundCycles = this->cpu->tick();

    QCOMPARE(this->cpu->registerA, immediate);
    auto foundFlags = this->cpu->registerF;
    QCOMPARE(foundFlags, startingFlags);
    QCOMPARE(foundCycles, expectedCycles);
}

void Bz80BaseCpuInstructionsTest::test_inc_r_data() {
    QTest::addColumn<uint8_t>("opcode");
    QTest::addColumn<uint8_t>("startingValue");
    QTest::addColumn<RegisterPairType*>("expectedRegister");
    QTest::addColumn<bool>("isLowByte");
    QTest::addColumn<FlagRegister>("startingFlags");
    QTest::addColumn<FlagRegister>("expectedFlags");

    QTest::newRow("INC B")
        << (uint8_t)0x04 << (uint8_t)0 << &this->cpu->registerBC << false
        << FlagRegister { true, true, true, false, false, false, false, false }
        << FlagRegister { true, false, false, false, false, false, false,
               false };
    QTest::newRow("INC C") << (uint8_t)0x0c << (uint8_t)15
                           << &this->cpu->registerBC << true
                           << FlagRegister { 0 }
                           << FlagRegister { false, false, false, false, true,
                                  false, false, false };
    QTest::newRow("INC D")
        << (uint8_t)0x14 << (uint8_t)127 << &this->cpu->registerDE << false
        << FlagRegister { 0 }
        << FlagRegister { false, false, true, false, true, false, false, true };
    QTest::newRow("INC E") << (uint8_t)0x1c << (uint8_t)255
                           << &this->cpu->registerDE << true
                           << FlagRegister { 0 }
                           << FlagRegister { false, false, false, false, true,
                                  false, true, false };
    QTest::newRow("INC H") << (uint8_t)0x24 << (uint8_t)-17
                           << &this->cpu->registerHL << false
                           << FlagRegister { 0 }
                           << FlagRegister { false, false, false, false, true,
                                  false, false, true };
    QTest::newRow("INC L") << (uint8_t)0x2c << (uint8_t)153
                           << &this->cpu->registerHL << true
                           << FlagRegister { 0 }
                           << FlagRegister { false, false, false, false, false,
                                  false, false, true };
}

void Bz80BaseCpuInstructionsTest::test_inc_r() {
    QFETCH(uint8_t, opcode);
    QFETCH(uint8_t, startingValue);
    QFETCH(RegisterPairType*, expectedRegister);
    QFETCH(bool, isLowByte);
    QFETCH(FlagRegister, startingFlags);
    QFETCH(FlagRegister, expectedFlags);

    if(isLowByte) {
        expectedRegister->setLower8(startingValue);
    } else {
        expectedRegister->setUpper8(startingValue);
    }

    this->cpu->currentOpcode = opcode;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->registerF = startingFlags;
    this->cpu->tick();
    uint8_t foundCycles = this->cpu->tick();
    uint8_t foundRegisterValue = isLowByte
        ? expectedRegister->getLower8()
        : expectedRegister->getUpper8();

    QCOMPARE(foundRegisterValue, static_cast<uint8_t>(startingValue + 1));
    QCOMPARE(foundCycles, 0); // INC r should not use any extra cycles.
    QCOMPARE(this->cpu->registerF, expectedFlags);
}

void Bz80BaseCpuInstructionsTest::test_inc_addr_hl() {
    auto expectedFlags = FlagRegister { false, false, false, false, false,
        false, false, false };
    uint8_t expectedCycles = MEMORY_ACCESS_CYCLES;
    uint8_t startingValue = 96;

    this->bus.write8(15, startingValue);
    this->cpu->currentOpcode = 0x34;
    this->cpu->registerHL = 15;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->registerF
        = { false, false, false, false, false, false, false, false };
    this->cpu->tick();
    uint8_t foundCycles = this->cpu->tick();

    QCOMPARE(this->bus.read8(15), static_cast<uint8_t>(startingValue + 1));
    QCOMPARE(foundCycles, expectedCycles);
    QCOMPARE(this->cpu->registerF, expectedFlags);
}

void Bz80BaseCpuInstructionsTest::test_inc_a() {
    auto expectedFlags = FlagRegister { false, false, false, false, false,
        false, false, true };
    uint8_t expectedCycles = 0;
    uint8_t startingValue = 169;

    this->cpu->registerA = startingValue;
    this->cpu->currentOpcode = 0x3C;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->registerF
        = { false, false, false, false, false, false, false, true };
    this->cpu->tick();
    uint8_t foundCycles = this->cpu->tick();

    QCOMPARE(this->cpu->registerA, static_cast<uint8_t>(startingValue + 1));
    QCOMPARE(foundCycles, expectedCycles);
    QCOMPARE(this->cpu->registerF, expectedFlags);
}

void Bz80BaseCpuInstructionsTest::test_dec_r_data() {
    QTest::addColumn<uint8_t>("opcode");
    QTest::addColumn<uint8_t>("startingValue");
    QTest::addColumn<RegisterPairType*>("expectedRegister");
    QTest::addColumn<bool>("isLowByte");
    QTest::addColumn<FlagRegister>("startingFlags");
    QTest::addColumn<FlagRegister>("expectedFlags");

    QTest::newRow("DEC B") << (uint8_t)0x05 << (uint8_t)1 << &this->cpu->registerBC << false
                           << FlagRegister { true, false, false, false,
                                             false, false, false, false,}
                           << FlagRegister { true, true, false, false,
                                             false, false, true, false};
    QTest::addRow("DEC C") << (uint8_t)0x0D << (uint8_t)0
                           << &this->cpu->registerBC << true
                           << FlagRegister { false, false, false, false, false,
                                  false, false, false }
                           << FlagRegister { false, true, false, false, false,
                                  false, false, true };
    QTest::addRow("DEC D") << (uint8_t)0x15 << (uint8_t)0x80
                           << &this->cpu->registerDE << false
                           << FlagRegister { false, false, false, false, false,
                                  false, false, false }
                           << FlagRegister { false, true, true, false, false,
                                  false, false, false };
    QTest::addRow("DEC E") << (uint8_t)0x1D << (uint8_t)-122
                           << &this->cpu->registerDE << true
                           << FlagRegister { false, false, false, false, false,
                                  false, false, false }
                           << FlagRegister { false, true, false, false, false,
                                  false, false, true };
    QTest::addRow("DEC H") << (uint8_t)0x25 << (uint8_t)49
                           << &this->cpu->registerHL << false
                           << FlagRegister { false, false, false, false, false,
                                  false, false, false }
                           << FlagRegister { false, true, false, false, false,
                                  false, false, false };
    QTest::addRow("DEC L") << (uint8_t)0x2D << (uint8_t)-25
                           << &this->cpu->registerHL << true
                           << FlagRegister { false, false, false, false, false,
                                  false, false, false }
                           << FlagRegister { false, true, false, false, false,
                                  false, false, true };
}

void Bz80BaseCpuInstructionsTest::test_dec_r() {
    QFETCH(uint8_t, opcode);
    QFETCH(uint8_t, startingValue);
    QFETCH(RegisterPairType*, expectedRegister);
    QFETCH(bool, isLowByte);
    QFETCH(FlagRegister, startingFlags);
    QFETCH(FlagRegister, expectedFlags);

    if(isLowByte) {
        expectedRegister->setLower8(startingValue);
    } else {
        expectedRegister->setUpper8(startingValue);
    }

    this->cpu->currentOpcode = opcode;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->registerF = startingFlags;
    this->cpu->tick();
    uint8_t foundCycles = this->cpu->tick();
    uint8_t foundRegisterValue = isLowByte
        ? expectedRegister->getLower8()
        : expectedRegister->getUpper8();

    QCOMPARE(foundRegisterValue, static_cast<uint8_t>(startingValue - 1));
    QCOMPARE(foundCycles, 0); // INC r should not use any extra cycles.
    QCOMPARE(this->cpu->registerF, expectedFlags);
}

void Bz80BaseCpuInstructionsTest::test_dec_addr_hl() {
    auto expectedFlags
        = FlagRegister { false, true, false, false, false, false, false, true };
    uint8_t expectedCycles = MEMORY_ACCESS_CYCLES;
    uint8_t startingValue = -67;

    this->bus.write8(15, startingValue);
    this->cpu->currentOpcode = 0x35;
    this->cpu->registerHL = 15;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->registerF
        = { false, false, false, false, false, false, false, false };
    this->cpu->tick();
    uint8_t foundCycles = this->cpu->tick();

    QCOMPARE(this->bus.read8(15), static_cast<uint8_t>(startingValue - 1));
    QCOMPARE(foundCycles, expectedCycles);
    QCOMPARE(this->cpu->registerF, expectedFlags);
}

void Bz80BaseCpuInstructionsTest::test_dec_a() {
    auto expectedFlags = FlagRegister { false, true, false, false, false, false,
        false, false };
    uint8_t expectedCycles = 0;
    uint8_t startingValue = 110;

    this->cpu->registerA = startingValue;
    this->cpu->currentOpcode = 0x3D;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->registerF
        = { false, false, false, false, false, false, false, false };
    this->cpu->tick();
    uint8_t foundCycles = this->cpu->tick();

    QCOMPARE(this->cpu->registerA, static_cast<uint8_t>(startingValue - 1));
    QCOMPARE(foundCycles, expectedCycles);
    QCOMPARE(this->cpu->registerF, expectedFlags);
}

QTEST_APPLESS_MAIN(Bz80BaseCpuInstructionsTest)

#include "tst_z80basecpuinstructions.moc"
