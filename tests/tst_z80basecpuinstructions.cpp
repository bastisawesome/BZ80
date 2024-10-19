// #include "qtestcase.h"
#include <QTest>
#include <ctime>
#include <memory>

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

FlagRegister genRandomFlags() {
    srand(time(0));
    uint8_t rand = (uint8_t)random();
    return FlagRegister { .carry = (bool)(rand & 0x1),
        .add_sub = (bool)(rand & 0x2),
        .overflow = (bool)(rand & 0x4),
        .unused1 = false,
        .halfcarry = (bool)(rand & 0x10),
        .unused2 = false,
        .zero = (bool)(rand & 0x40),
        .sign = (bool)(rand & 0x80) };
}

class Bz80BaseCpuInstructionsTest : public QObject {
    Q_OBJECT

public:
    Bz80BaseCpuInstructionsTest();
    ~Bz80BaseCpuInstructionsTest();

private:
    std::unique_ptr<Z80BaseCpu> cpu;
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

    // NOP
    void test_nop();

    // DJNZ IMM
    void test_djnz_not_zero();
    void test_djnz_zero();

    // JR IMM
    void test_jr();

    // JR cc, IMM
    void test_jr_cc_imm_data();
    void test_jr_cc_imm();

    // ADD A, r
    void test_add_r_data();
    void test_add_r();
    void test_add_addr_hl();
    void test_add_a();

    // SUB r
    void test_sub_r_data();
    void test_sub_r();
    void test_sub_addr_hl();
    void test_sub_a();

    // LD r, r
    // TODO: Remove flag register from the test data.
    //  The flag register could be seeded with random data, as it is expected
    //  to always contain the same data at the start as it does at the end.
    //  Randomness _should_, in theory, provide enough proof that the flags
    //  are unaltered.
    void test_ld_r_r_data();
    void test_ld_r_r();
    void test_ld_r_addr_hl_data();
    void test_ld_r_addr_hl();
    void test_ld_r_a_data();
    void test_ld_r_a();
    void test_ld_addr_hl_r_data();
    void test_ld_addr_hl_r();
    void test_ld_addr_hl_a();
    void test_ld_a_r_data();
    void test_ld_a_r();
    void test_ld_a_addr_hl();
    void test_ld_a_a();
};

Bz80BaseCpuInstructionsTest::Bz80BaseCpuInstructionsTest() {
    this->bus = MmioDeviceManager();
    std::unique_ptr<MmioRam<16>> ram(new MmioRam<16>());
    ram->write8(0, 15);
    this->bus.addMmioDevice(0, std::move(ram));
    this->cpu.reset(new Z80BaseCpu(bus));
}

Bz80BaseCpuInstructionsTest::~Bz80BaseCpuInstructionsTest() {
}

void Bz80BaseCpuInstructionsTest::init() {}

void Bz80BaseCpuInstructionsTest::cleanup() {
    this->bus.write8(0, 15, false);
    for(uint16_t i=1; i<16; i++) {
        this->bus.write8(i, 0, false);
    }
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

    this->cpu->bus.write8(this->cpu->programCounter, immediate, false);

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

    this->cpu->bus.write8(this->cpu->programCounter, immediate, false);

    this->cpu->currentOpcode = opcode;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    this->cpu->registerF = startingFlags;
    this->cpu->registerHL.set16(15);
    uint8_t foundCycles = this->cpu->tick();

    QCOMPARE(this->bus.read8(15, false), immediate);
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

    this->cpu->bus.write8(this->cpu->programCounter, immediate, false);
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

    this->bus.write8(15, startingValue, false);
    this->cpu->currentOpcode = 0x34;
    this->cpu->registerHL = 15;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->registerF
        = { false, false, false, false, false, false, false, false };
    this->cpu->tick();
    uint8_t foundCycles = this->cpu->tick();

    QCOMPARE(this->bus.read8(15, false), static_cast<uint8_t>(startingValue + 1));
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
    QTest::addRow("DEC D")
        << (uint8_t)0x15 << (uint8_t)0x80 << &this->cpu->registerDE << false
        << FlagRegister { false, false, false, false, false, false, false,
               false }
        << FlagRegister { false, true, true, false, true, false, false, false };
    QTest::addRow("DEC E")
        << (uint8_t)0x1D << (uint8_t)-122 << &this->cpu->registerDE << true
        << FlagRegister { false, false, false, false, false, false, false,
               false }
        << FlagRegister { false, true, false, false, true, false, false, true };
    QTest::addRow("DEC H") << (uint8_t)0x25 << (uint8_t)49
                           << &this->cpu->registerHL << false
                           << FlagRegister { false, false, false, false, false,
                                  false, false, false }
                           << FlagRegister { false, true, false, false, false,
                                  false, false, false };
    QTest::addRow("DEC L")
        << (uint8_t)0x2D << (uint8_t)-25 << &this->cpu->registerHL << true
        << FlagRegister { false, false, false, false, false, false, false,
               false }
        << FlagRegister { false, true, false, false, true, false, false, true };
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
        = FlagRegister { false, true, false, false, true, false, false, true };
    uint8_t expectedCycles = MEMORY_ACCESS_CYCLES;
    uint8_t startingValue = -67;

    this->bus.write8(15, startingValue, false);
    this->cpu->currentOpcode = 0x35;
    this->cpu->registerHL = 15;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->registerF
        = { false, false, false, false, true, false, false, false };
    this->cpu->tick();
    uint8_t foundCycles = this->cpu->tick();

    QCOMPARE(this->bus.read8(15, false), static_cast<uint8_t>(startingValue - 1));
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

void Bz80BaseCpuInstructionsTest::test_nop() {
    uint8_t expRegA = 71;
    auto expRegBC = bz80::RegisterPairType(159);
    auto expRegDE = bz80::RegisterPairType(87);
    auto expRegHL = bz80::RegisterPairType(106);
    auto expF
        = FlagRegister { true, false, true, false, false, false, true, true };

    this->cpu->registerA = expRegA;
    this->cpu->registerBC = expRegBC;
    this->cpu->registerDE = expRegDE;
    this->cpu->registerHL = expRegHL;
    this->cpu->registerF = expF;

    this->cpu->currentOpcode = 0;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    uint8_t foundCycles = this->cpu->tick();

    QCOMPARE(foundCycles, 0);
    QCOMPARE(this->cpu->registerA, expRegA);
    QCOMPARE(this->cpu->registerBC.get16(), expRegBC.get16());
    QCOMPARE(this->cpu->registerDE.get16(), expRegDE.get16());
    QCOMPARE(this->cpu->registerHL.get16(), expRegHL.get16());
    QCOMPARE(this->cpu->registerF, expF);

    QCOMPARE(this->bus.read16(0), 15);
    for(size_t i = 2; i < 16; i += 2) {
        QCOMPARE(this->bus.read16(i), 0);
    }
}

void Bz80BaseCpuInstructionsTest::test_djnz_not_zero() {
    auto expFlags = FlagRegister { false, false, false, false, false, false,
        false, true };
    uint8_t expOffset = 5;
    uint8_t expCycles = 9;

    this->cpu->bus.write8(1, expOffset, false);

    this->cpu->registerBC.setUpper8(15);
    this->cpu->registerF = expFlags;
    this->cpu->programCounter = 1;
    this->cpu->currentOpcode = 0x10;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    uint8_t cycles = this->cpu->tick();

    QCOMPARE(this->cpu->programCounter, expOffset + 2);
    QCOMPARE(this->cpu->registerBC.getUpper8(), 14);
    QCOMPARE(this->cpu->registerF, expFlags);
    QCOMPARE(cycles, expCycles);
}

void Bz80BaseCpuInstructionsTest::test_djnz_zero() {
    auto expFlags
        = FlagRegister { true, false, true, false, false, false, false, true };
    uint8_t expCycles = 4;

    this->cpu->registerBC.setUpper8(1);
    this->cpu->registerF = expFlags;
    this->cpu->programCounter = 1;
    this->cpu->currentOpcode = 0x10;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    uint8_t cycles = this->cpu->tick();

    QCOMPARE(this->cpu->programCounter, 3);
    QCOMPARE(this->cpu->registerBC.getUpper8(), 0);
    QCOMPARE(this->cpu->registerF, expFlags);
    QCOMPARE(cycles, expCycles);
}

void Bz80BaseCpuInstructionsTest::test_jr() {
    auto expFlags
        = FlagRegister { true, false, true, false, false, false, false, true };
    uint8_t expCycles = 8;

    this->bus.write8(11, 3, false);

    this->cpu->programCounter = 11;
    this->cpu->registerF = expFlags;
    this->cpu->currentOpcode = 0x18;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    uint8_t cycles = this->cpu->tick();

    QCOMPARE(this->cpu->programCounter, 15);
    QCOMPARE(this->cpu->registerF, expFlags);
    QCOMPARE(cycles, expCycles);
}

void Bz80BaseCpuInstructionsTest::test_jr_cc_imm_data() {
    QTest::addColumn<uint8_t>("opcode");
    QTest::addColumn<bool>("shouldJump");
    QTest::addColumn<FlagRegister>("startingFlags");
    QTest::addColumn<uint8_t>("offset");
    QTest::addColumn<uint8_t>("expCycles");
    QTest::addColumn<uint16_t>("startingPC");

    QTest::addRow("JR NZ, d (true)")
        << (uint8_t)0x20 << true
        << FlagRegister { false, false, false, false, false, false, false,
               true }
        << (uint8_t)8 << (uint8_t)8 << (uint16_t)14;

    QTest::addRow("JR NZ, d (false)")
        << (uint8_t)0x20 << false
        << FlagRegister { true, true, true, false, true, false, true, false }
        << (uint8_t)46 << (uint8_t)3 << (uint16_t)9;

    QTest::addRow("JR Z, d (true)")
        << (uint8_t)0x28 << true
        << FlagRegister { true, false, false, false, true, false, true, true }
        << (uint8_t)-7 << (uint8_t)8 << (uint16_t)7;

    QTest::addRow("JR Z, d (false)")
        << (uint8_t)0x28 << false
        << FlagRegister { false, true, false, false, false, false, false,
               false }
        << (uint8_t)19 << (uint8_t)3 << (uint16_t)13;

    QTest::addRow("JR NC, d (true)")
        << (uint8_t)0x30 << true
        << FlagRegister { false, true, false, false, false, false, false,
               false }
        << (uint8_t)51 << (uint8_t)8 << (uint16_t)7;

    QTest::addRow("JR NC, d (false)")
        << (uint8_t)0x30 << false
        << FlagRegister { true, false, true, false, true, false, false, true }
        << (uint8_t)24 << (uint8_t)3 << (uint16_t)14;

    QTest::addRow("JR C, d (true)")
        << (uint8_t)0x38 << true
        << FlagRegister { true, false, false, false, false, false, false,
               false }
        << (uint8_t)61 << (uint8_t)8 << (uint16_t)2;

    QTest::addRow("JR C, d (false)")
        << (uint8_t)0x38 << false
        << FlagRegister { false, false, false, false, false, false, false,
               false }
        << (uint8_t)121 << (uint8_t)3 << (uint16_t)14;
}

void Bz80BaseCpuInstructionsTest::test_jr_cc_imm() {
    QFETCH(uint8_t, opcode);
    QFETCH(bool, shouldJump);
    QFETCH(FlagRegister, startingFlags);
    QFETCH(uint8_t, offset);
    QFETCH(uint8_t, expCycles);
    QFETCH(uint16_t, startingPC);

    this->bus.write8(startingPC, offset, false);

    this->cpu->currentOpcode = opcode;
    this->cpu->registerF = startingFlags;
    this->cpu->programCounter = startingPC;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    uint8_t cycles = this->cpu->tick();
    uint8_t expectedPC;

    if(shouldJump) {
        expectedPC = startingPC + offset + 1;
    } else {
        expectedPC = startingPC + 1;
    }

    QCOMPARE(this->cpu->programCounter, expectedPC);
    QCOMPARE(this->cpu->registerF, startingFlags);
    QCOMPARE(cycles, expCycles);
}

void Bz80BaseCpuInstructionsTest::test_add_r_data() {
    QTest::addColumn<uint8_t>("opcode");
    QTest::addColumn<int8_t>("startingAValue");
    QTest::addColumn<RegisterPairType*>("regToAdd");
    QTest::addColumn<bool>("isUpperReg");
    QTest::addColumn<int8_t>("startingRegValue");
    QTest::addColumn<FlagRegister>("expectedFlags");

    QTest::addRow("ADD A, B")
        << (uint8_t)0x80 << (int8_t)0 << &this->cpu->registerBC << true
        << (int8_t)19
        << FlagRegister { .carry = false,
               .add_sub = false,
               .overflow = false,
               .unused1 = false,
               .halfcarry = false,
               .unused2 = false,
               .zero = false,
               .sign = false };

    QTest::addRow("ADD A, C")
        << (uint8_t)0x81 << (int8_t)98 << &this->cpu->registerBC << false
        << (int8_t)70
        << FlagRegister { .carry = false,
               .add_sub = false,
               .overflow = true,
               .unused1 = false,
               .halfcarry = false,
               .unused2 = false,
               .zero = false,
               .sign = true };

    QTest::addRow("ADD A, D")
        << (uint8_t)0x82 << (int8_t)-42 << &this->cpu->registerDE << true
        << (int8_t)42
        << FlagRegister { .carry = false,
               .add_sub = false,
               .overflow = false,
               .unused1 = false,
               .halfcarry = true,
               .unused2 = false,
               .zero = true,
               .sign = false };

    QTest::addRow("ADD A, E")
        << (uint8_t)0x83 << (int8_t)-12 << &this->cpu->registerDE << false
        << (int8_t)-100
        << FlagRegister { .carry = false,
               .add_sub = false,
               .overflow = false,
               .unused1 = false,
               .halfcarry = true,
               .unused2 = false,
               .zero = false,
               .sign = true };

    QTest::addRow("ADD A, H")
        << (uint8_t)0x84 << (int8_t)-113 << &this->cpu->registerHL << true
        << (int8_t)-68
        << FlagRegister { .carry = false,
               .add_sub = false,
               .overflow = true,
               .unused1 = false,
               .halfcarry = true,
               .unused2 = false,
               .zero = false,
               .sign = false };

    QTest::addRow("ADD A, L")
        << (uint8_t)0x85 << (int8_t)26 << &this->cpu->registerHL << false
        << (int8_t)-51
        << FlagRegister { .carry = false,
               .add_sub = false,
               .overflow = false,
               .unused1 = false,
               .halfcarry = true,
               .unused2 = false,
               .zero = false,
               .sign = true };
}

void Bz80BaseCpuInstructionsTest::test_add_r() {
    QFETCH(uint8_t, opcode);
    QFETCH(int8_t, startingAValue);
    QFETCH(RegisterPairType*, regToAdd);
    QFETCH(bool, isUpperReg);
    QFETCH(int8_t, startingRegValue);
    QFETCH(FlagRegister, expectedFlags);

    const int8_t expectedAccValue = startingAValue + startingRegValue;
    const uint8_t expectedCycles = 0;

    if(isUpperReg) {
        regToAdd->setUpper8(startingRegValue);
    } else {
        regToAdd->setLower8(startingRegValue);
    }

    this->cpu->currentOpcode = opcode;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->registerA = startingAValue;
    this->cpu->tick();
    const uint8_t cycles = this->cpu->tick();

    QCOMPARE(this->cpu->registerA, (uint8_t)expectedAccValue);
    QCOMPARE(this->cpu->registerF, expectedFlags);
    QCOMPARE(cycles, expectedCycles);
}

void Bz80BaseCpuInstructionsTest::test_add_addr_hl() {
    this->bus.write8(3, 107, false);

    const int8_t expectedValue = 121;
    const uint8_t expectedCycles = MEMORY_ACCESS_CYCLES;
    const FlagRegister expectedFlags { .carry = false,
        .add_sub = false,
        .overflow = false,
        .unused1 = false,
        .halfcarry = true,
        .unused2 = false,
        .zero = false,
        .sign = false };

    this->cpu->registerA = 14;
    this->cpu->registerHL.set16(3);
    this->cpu->currentOpcode = 0x86;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    const uint8_t cycles = this->cpu->tick();

    QCOMPARE(this->cpu->registerA, expectedValue);
    QCOMPARE(this->cpu->registerF, expectedFlags);
    QCOMPARE(cycles, expectedCycles);
}

void Bz80BaseCpuInstructionsTest::test_add_a() {
    const int8_t expectedValue = (int8_t)-164;
    const uint8_t expectedCycles = 0;
    const FlagRegister expectedFlags { .carry = false,
        .add_sub = false,
        .overflow = true,
        .unused1 = false,
        .halfcarry = true,
        .unused2 = false,
        .zero = false,
        .sign = false };

    this->cpu->registerA = -82;
    this->cpu->currentOpcode = 0x87;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    const uint8_t cycles = this->cpu->tick();

    QCOMPARE(this->cpu->registerA, expectedValue);
    QCOMPARE(this->cpu->registerF, expectedFlags);
    QCOMPARE(cycles, expectedCycles);
}

void Bz80BaseCpuInstructionsTest::test_sub_r_data() {
    QTest::addColumn<uint8_t>("opcode");
    QTest::addColumn<int8_t>("startingAValue");
    QTest::addColumn<RegisterPairType*>("regToAdd");
    QTest::addColumn<bool>("isUpperReg");
    QTest::addColumn<int8_t>("startingRegValue");
    QTest::addColumn<FlagRegister>("expectedFlags");

    QTest::addRow("SUB B")
        << (uint8_t)0x90 << (int8_t)19 << &this->cpu->registerBC << true
        << (int8_t)0
        << FlagRegister { .carry = false,
               .add_sub = true,
               .overflow = false,
               .unused1 = false,
               .halfcarry = false,
               .unused2 = false,
               .zero = false,
               .sign = false };

    QTest::addRow("SUB C")
        << (uint8_t)0x91 << (int8_t)114 << &this->cpu->registerBC << false
        << (int8_t)126
        << FlagRegister { .carry = true,
               .add_sub = true,
               .overflow = false,
               .unused1 = false,
               .halfcarry = true,
               .unused2 = false,
               .zero = false,
               .sign = true };

    QTest::addRow("SUB D")
        << (uint8_t)0x92 << (int8_t)16 << &this->cpu->registerDE << true
        << (int8_t)-28
        << FlagRegister { .carry = false,
               .add_sub = true,
               .overflow = false,
               .unused1 = false,
               .halfcarry = false,
               .unused2 = false,
               .zero = false,
               .sign = false };

    QTest::addRow("SUB E")
        << (uint8_t)0x93 << (int8_t)19 << &this->cpu->registerDE << false
        << (int8_t)19
        << FlagRegister { .carry = false,
               .add_sub = true,
               .overflow = false,
               .unused1 = false,
               .halfcarry = false,
               .unused2 = false,
               .zero = true,
               .sign = false };

    QTest::addRow("SUB H")
        << (uint8_t)0x94 << (int8_t)-41 << &this->cpu->registerHL << true
        << (int8_t)99
        << FlagRegister { .carry = false,
               .add_sub = true,
               .overflow = true,
               .unused1 = false,
               .halfcarry = true,
               .unused2 = false,
               .zero = false,
               .sign = false };

    QTest::addRow("SUB L")
        << (uint8_t)0x95 << (int8_t)3 << &this->cpu->registerHL << false
        << (int8_t)126
        << FlagRegister { .carry = true,
               .add_sub = true,
               .overflow = false,
               .unused1 = false,
               .halfcarry = true,
               .unused2 = false,
               .zero = false,
               .sign = true };
}

void Bz80BaseCpuInstructionsTest::test_sub_r() {
    QFETCH(uint8_t, opcode);
    QFETCH(int8_t, startingAValue);
    QFETCH(RegisterPairType*, regToAdd);
    QFETCH(bool, isUpperReg);
    QFETCH(int8_t, startingRegValue);
    QFETCH(FlagRegister, expectedFlags);

    uint8_t expectedResult = startingAValue - startingRegValue;
    uint8_t expectedCycles = 0;

    if(isUpperReg) {
        regToAdd->setUpper8(startingRegValue);
    } else {
        regToAdd->setLower8(startingRegValue);
    }

    this->cpu->currentOpcode = opcode;
    this->cpu->registerA = startingAValue;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    uint8_t cycles = this->cpu->tick();

    QCOMPARE(this->cpu->registerA, expectedResult);
    QCOMPARE(this->cpu->registerF, expectedFlags);
    QCOMPARE(cycles, expectedCycles);
}

void Bz80BaseCpuInstructionsTest::test_sub_addr_hl() {
    int8_t startingAValue = -126;
    int8_t toSub = 14;
    uint8_t expectedResult = startingAValue - toSub;
    uint8_t expectedCycles = MEMORY_ACCESS_CYCLES;
    auto expectedFlags = FlagRegister { .carry = false,
        .add_sub = true,
        .overflow = true,
        .unused1 = false,
        .halfcarry = true,
        .unused2 = false,
        .zero = false,
        .sign = false };

    this->bus.write8(3, toSub, false);
    this->cpu->registerHL.set16(3);
    this->cpu->registerA = startingAValue;
    this->cpu->currentOpcode = 0x96;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    uint8_t cycles = this->cpu->tick();

    QCOMPARE(this->cpu->registerA, expectedResult);
    QCOMPARE(this->cpu->registerF, expectedFlags);
    QCOMPARE(cycles, expectedCycles);
}

void Bz80BaseCpuInstructionsTest::test_sub_a() {
    int8_t startingAValue = -74;
    uint8_t expectedResult = 0;
    uint8_t expectedCycles = 0;
    auto expectedFlags = FlagRegister { .carry = false,
        .add_sub = true,
        .overflow = false,
        .unused1 = false,
        .halfcarry = false,
        .unused2 = false,
        .zero = true,
        .sign = false };

    this->cpu->registerA = startingAValue;
    this->cpu->currentOpcode = 0x97;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    uint8_t cycles = this->cpu->tick();

    QCOMPARE(this->cpu->registerA, expectedResult);
    QCOMPARE(this->cpu->registerF, expectedFlags);
    QCOMPARE(cycles, expectedCycles);
}

void Bz80BaseCpuInstructionsTest::test_ld_r_r_data() {
    QTest::addColumn<uint8_t>("opcode");
    QTest::addColumn<RegisterPairType*>("toRegister");
    QTest::addColumn<bool>("isLowByte");
    QTest::addColumn<RegisterPairType*>("fromRegisterPair");
    QTest::addColumn<uint16_t>("startingValue");
    QTest::addColumn<FlagRegister>("startingFlags");

    QTest::newRow("ld b, b/ld b, c")
        << (uint8_t)0x40 << &this->cpu->registerBC << false
        << &this->cpu->registerBC << (uint16_t)0x71f3
        << FlagRegister { .carry = false,
               .add_sub = true,
               .overflow = true,
               .unused1 = false,
               .halfcarry = false,
               .unused2 = false,
               .zero = false,
               .sign = true };

    QTest::newRow("ld b, d/ld b, e")
        << (uint8_t)0x42 << &this->cpu->registerBC << false
        << &this->cpu->registerDE << (uint16_t)0xdc05
        << FlagRegister { .carry = true,
               .overflow = false,
               .unused1 = false,
               .halfcarry = true,
               .unused2 = false,
               .zero = true,
               .sign = false };

    QTest::newRow("ld b, h/ld b, l")
        << (uint8_t)0x44 << &this->cpu->registerBC << false
        << &this->cpu->registerHL << (uint16_t)0xcb7c
        << FlagRegister { true, true, true, false, true, false, false, true };

    QTest::newRow("ld c, b/ld c, c")
        << (uint8_t)0x48 << &this->cpu->registerBC << true
        << &this->cpu->registerBC << (uint16_t)0xb09b
        << FlagRegister { false, false, false, false, true, false, false,
               false };

    QTest::newRow("ld c, d/ld c, e")
        << (uint8_t)0x4a << &this->cpu->registerBC << true
        << &this->cpu->registerDE << (uint16_t)0xd54f
        << FlagRegister { true, false, true, false, true, false, false, false };

    QTest::newRow("ld c, h/ld c, l")
        << (uint8_t)0x4c << &this->cpu->registerBC << true
        << &this->cpu->registerHL << (uint16_t)0x9fcd
        << FlagRegister { true, true, true, false, true, false, true, false };

    QTest::newRow("ld d, b/ld d, c")
        << (uint8_t)0x50 << &this->cpu->registerDE << false
        << &this->cpu->registerBC << (uint16_t)0xc6ea
        << FlagRegister { true, true, true, false, false, false, false, false };

    QTest::newRow("ld d, d/ld d, e")
        << (uint8_t)0x52 << &this->cpu->registerDE << false
        << &this->cpu->registerDE << (uint16_t)0x07dd
        << FlagRegister { false, true, false, false, true, false, true, true };

    QTest::newRow("ld d, h/ld d, l")
        << (uint8_t)0x54 << &this->cpu->registerDE << false
        << &this->cpu->registerHL << (uint16_t)0xa9de
        << FlagRegister { false, false, true, false, false, false, true,
               false };

    QTest::newRow("ld e, b/ld e, c")
        << (uint8_t)0x58 << &this->cpu->registerDE << true
        << &this->cpu->registerBC << (uint16_t)0x5df8
        << FlagRegister { false, false, false, false, false, false, true,
               true };

    QTest::newRow("ld e, d/ld e, e")
        << (uint8_t)0x5a << &this->cpu->registerDE << true
        << &this->cpu->registerDE << (uint16_t)0x09e5
        << FlagRegister { true, true, true, false, true, false, true, true };

    QTest::newRow("ld e, h/ld e, l")
        << (uint8_t)0x5c << &this->cpu->registerDE << true
        << &this->cpu->registerHL << (uint16_t)0xe5ad
        << FlagRegister { true, false, false, false, true, false, false, true };

    QTest::newRow("ld h, b/ld h, c")
        << (uint8_t)0x60 << &this->cpu->registerHL << false
        << &this->cpu->registerBC << (uint16_t)0x6392
        << FlagRegister { true, true, true, false, false, false, true, false };

    QTest::newRow("ld h, d/ld h, e")
        << (uint8_t)0x62 << &this->cpu->registerHL << false
        << &this->cpu->registerDE << (uint16_t)0x20ce
        << FlagRegister { true, false, false, false, true, false, false, true };

    QTest::newRow("ld h, h/ld h, l")
        << (uint8_t)0x64 << &this->cpu->registerHL << false
        << &this->cpu->registerHL << (uint16_t)0x43c8
        << FlagRegister { true, true, false, false, true, false, false, true };

    QTest::newRow("ld l, b/ld l, c")
        << (uint8_t)0x68 << &this->cpu->registerHL << true
        << &this->cpu->registerBC << (uint16_t)0x60ef
        << FlagRegister { true, false, true, false, true, false, false, true };

    QTest::newRow("ld l, d/ld l, e")
        << (uint8_t)0x6a << &this->cpu->registerHL << true
        << &this->cpu->registerDE << (uint16_t)0x312b
        << FlagRegister { true, true, true, false, true, false, false, false };

    QTest::newRow("ld l, h/ld l, l")
        << (uint8_t)0x6c << &this->cpu->registerHL << true
        << &this->cpu->registerHL << (uint16_t)0x1583
        << FlagRegister { false, false, true, false, false, false, true, true };
}

void Bz80BaseCpuInstructionsTest::test_ld_r_r() {
    /*
     * The debug output seems to cause problems?
     * Not sure what the issue is, but it segfaults, then spinlocks until
     * timeout. Need to look into this issue later.
     * Except after fixing the test properly, now all the pointers are wrong.
     * It's like it duplicates the CPU class, then forgets about the new one? I
     * cannot figure this out.
     */
    //    qDebug() << QTest::currentDataTag();
    //    QStringList testNames = QString(QTest::currentDataTag()).split("/");
    //    QString testName1 = testNames[0];
    //    QString testName2 = testNames[1];

    QFETCH(uint8_t, opcode);
    QFETCH(RegisterPairType*, toRegister);
    QFETCH(bool, isLowByte);
    QFETCH(RegisterPairType*, fromRegisterPair);
    QFETCH(uint16_t, startingValue);
    QFETCH(FlagRegister, startingFlags);
    this->cpu->registerF = startingFlags;

    const uint8_t expCycles = 0;

    uint8_t cycles;
    uint8_t result;

    //    qDebug() << testName1;
    fromRegisterPair->set16(startingValue);
    this->cpu->currentOpcode = opcode;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    cycles = this->cpu->tick();

    if(isLowByte) {
        result = toRegister->getLower8();
    } else {
        result = toRegister->getUpper8();
    }

    QCOMPARE(result, (startingValue >> 8));
    QCOMPARE(this->cpu->registerF, startingFlags);
    QCOMPARE(cycles, expCycles);

    //    qDebug() << testName2;
    fromRegisterPair->set16(startingValue);
    this->cpu->currentOpcode = opcode + 1;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    cycles = this->cpu->tick();

    if(isLowByte) {
        result = toRegister->getLower8();
    } else {
        result = toRegister->getUpper8();
    }

    QCOMPARE(result, (startingValue & 0xff));
    QCOMPARE(this->cpu->registerF, startingFlags);
    QCOMPARE(cycles, expCycles);
}

void Bz80BaseCpuInstructionsTest::test_ld_r_addr_hl_data() {
    QTest::addColumn<uint8_t>("opcode");
    QTest::addColumn<RegisterPairType*>("toRegister");
    QTest::addColumn<bool>("isLowByte");
    QTest::addColumn<uint16_t>("startingAddress");
    QTest::addColumn<uint8_t>("startingValue");
    QTest::addColumn<FlagRegister>("startingFlags");

    QTest::newRow("ld b, (hl)")
        << (uint8_t)0x46 << &this->cpu->registerBC << false << (uint16_t)0x2
        << (uint8_t)0x9e
        << FlagRegister { true, true, true, false, true, false, true, true };

    QTest::newRow("ld c, (hl)")
        << (uint8_t)0x4e << &this->cpu->registerBC << true << (uint16_t)0xF
        << (uint8_t)0x9a
        << FlagRegister { true, true, false, false, true, false, false, true };

    QTest::newRow("ld d, (hl)")
        << (uint8_t)0x56 << &this->cpu->registerDE << false << (uint16_t)0x8
        << (uint8_t)0xca
        << FlagRegister { true, true, false, true, false, false, true };

    QTest::newRow("ld e, (hl)")
        << (uint8_t)0x5e << &this->cpu->registerDE << true << (uint16_t)0xF
        << (uint8_t)0x68
        << FlagRegister { false, true, true, false, false, false, true, true };

    QTest::newRow("ld h, (hl)")
        << (uint8_t)0x66 << &this->cpu->registerHL << false << (uint16_t)0x6
        << (uint8_t)0xf0
        << FlagRegister { true, true, false, false, true, false, false, true };

    QTest::newRow("ld l, (hl)")
        << (uint8_t)0x6e << &this->cpu->registerHL << true << (uint16_t)0x8
        << (uint8_t)0x5
        << FlagRegister { false, false, false, false, false, false, true,
               false };
}

void Bz80BaseCpuInstructionsTest::test_ld_r_addr_hl() {
    QFETCH(uint8_t, opcode);
    QFETCH(RegisterPairType*, toRegister);
    QFETCH(bool, isLowByte);
    QFETCH(uint16_t, startingAddress);
    QFETCH(uint8_t, startingValue);
    QFETCH(FlagRegister, startingFlags);

    const uint8_t expCycles = MEMORY_ACCESS_CYCLES;

    this->cpu->bus.write8(startingAddress, startingValue, false);
    this->cpu->registerHL.set16(startingAddress);
    this->cpu->registerF = startingFlags;
    this->cpu->currentOpcode = opcode;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    uint8_t cycles = this->cpu->tick();

    uint8_t value;

    if(isLowByte) {
        value = toRegister->getLower8();
    } else {
        value = toRegister->getUpper8();
    }

    QCOMPARE(value, startingValue);
    QCOMPARE(this->cpu->registerF, startingFlags);
    QCOMPARE(cycles, expCycles);
}

void Bz80BaseCpuInstructionsTest::test_ld_r_a_data() {
    QTest::addColumn<uint8_t>("opcode");
    QTest::addColumn<RegisterPairType*>("toRegister");
    QTest::addColumn<bool>("isLowByte");
    QTest::addColumn<uint8_t>("startingValue");

    QTest::addRow("ld b, a")
        << (uint8_t)0x47 << &this->cpu->registerBC << false << (uint8_t)0xc9;

    QTest::addRow("ld c, a")
        << (uint8_t)0x4f << &this->cpu->registerBC << true << (uint8_t)0x6a;

    QTest::addRow("ld d, a")
        << (uint8_t)0x57 << &this->cpu->registerDE << false << (uint8_t)0x63;

    QTest::addRow("ld e, a")
        << (uint8_t)0x5f << &this->cpu->registerDE << true << (uint8_t)0xa4;

    QTest::addRow("ld h, a")
        << (uint8_t)0x67 << &this->cpu->registerHL << false << (uint8_t)0x86;

    QTest::addRow("ld l, a")
        << (uint8_t)0x6f << &this->cpu->registerHL << true << (uint8_t)0xfc;
}

void Bz80BaseCpuInstructionsTest::test_ld_r_a() {
    QFETCH(uint8_t, opcode);
    QFETCH(RegisterPairType*, toRegister);
    QFETCH(bool, isLowByte);
    QFETCH(uint8_t, startingValue);

    uint8_t value;
    const uint8_t expCycles = 0;
    const FlagRegister expFlags = genRandomFlags();

    this->cpu->registerA = startingValue;
    this->cpu->registerF = expFlags;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->currentOpcode = opcode;
    this->cpu->tick();
    uint8_t cycles = this->cpu->tick();

    if(isLowByte) {
        value = toRegister->getLower8();
    } else {
        value = toRegister->getUpper8();
    }

    QCOMPARE(value, startingValue);
    QCOMPARE(this->cpu->registerF, expFlags);
    QCOMPARE(cycles, expCycles);
}

void Bz80BaseCpuInstructionsTest::test_ld_addr_hl_r_data() {
    QTest::addColumn<uint8_t>("opcode");
    QTest::addColumn<RegisterPairType*>("fromRegister");
    QTest::addColumn<bool>("isLowByte");
    QTest::addColumn<uint16_t>("address");
    QTest::addColumn<uint8_t>("startingValue");

    QTest::addRow("ld (hl), b") << (uint8_t)0x70 << &this->cpu->registerBC
                                << false << (uint16_t)0xd << (uint8_t)0xe3;

    QTest::addRow("ld (hl), c") << (uint8_t)0x71 << &this->cpu->registerBC
                                << true << (uint16_t)0xF << (uint8_t)0xad;

    QTest::addRow("ld (hl), d") << (uint8_t)0x72 << &this->cpu->registerDE
                                << false << (uint16_t)0xa << (uint8_t)0x1f;

    QTest::addRow("ld (hl), e") << (uint8_t)0x73 << &this->cpu->registerDE
                                << true << (uint16_t)0xc << (uint8_t)0x6c;

    QTest::addRow("ld (hl), h") << (uint8_t)0x74 << &this->cpu->registerHL
                                << false << (uint16_t)0xb << (uint8_t)0;

    QTest::addRow("ld (hl), l") << (uint8_t)0x75 << &this->cpu->registerHL
                                << true << (uint16_t)0x1 << (uint8_t)0x1;
}

void Bz80BaseCpuInstructionsTest::test_ld_addr_hl_r() {
    QFETCH(uint8_t, opcode);
    QFETCH(RegisterPairType*, fromRegister);
    QFETCH(bool, isLowByte);
    QFETCH(uint16_t, address);
    QFETCH(uint8_t, startingValue);

    const uint8_t expCycles = MEMORY_ACCESS_CYCLES;
    const FlagRegister expFlags = genRandomFlags();

    this->cpu->registerF = expFlags;
    this->cpu->registerHL.set16(address);

    if(isLowByte) {
        fromRegister->setLower8(startingValue);
    } else {
        fromRegister->setUpper8(startingValue);
    }

    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->currentOpcode = opcode;
    this->cpu->tick();
    uint8_t cycles = this->cpu->tick();

    QCOMPARE(this->cpu->bus.read8(address, false), startingValue);
    QCOMPARE(this->cpu->registerF, expFlags);
    QCOMPARE(cycles, expCycles);
}

void Bz80BaseCpuInstructionsTest::test_ld_addr_hl_a() {
    const uint8_t opcode = 0x77;
    const uint16_t startingAddress = 0x5;
    const uint8_t startingValue = 0x71;
    const uint8_t expCycles = MEMORY_ACCESS_CYCLES;
    const FlagRegister startingFlags = genRandomFlags();

    this->cpu->registerA = startingValue;
    this->cpu->registerF = startingFlags;
    this->cpu->registerHL.set16(startingAddress);
    this->cpu->currentOpcode = opcode;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    uint8_t cycles = this->cpu->tick();

    QCOMPARE(this->cpu->bus.read8(startingAddress, false), startingValue);
    QCOMPARE(this->cpu->registerF, startingFlags);
    QCOMPARE(cycles, expCycles);
}

void Bz80BaseCpuInstructionsTest::test_ld_a_r_data() {
    QTest::addColumn<uint8_t>("opcode");
    QTest::addColumn<RegisterPairType*>("fromRegister");
    QTest::addColumn<bool>("isLowByte");
    QTest::addColumn<uint8_t>("startingValue");

    QTest::addRow("ld a, b")
        << (uint8_t)0x78 << &this->cpu->registerBC << false << (uint8_t)0xdf;
    QTest::addRow("ld a, c")
        << (uint8_t)0x79 << &this->cpu->registerBC << true << (uint8_t)0x81;
    QTest::addRow("ld a, d")
        << (uint8_t)0x7a << &this->cpu->registerDE << false << (uint8_t)0x90;
    QTest::addRow("ld a, e")
        << (uint8_t)0x7b << &this->cpu->registerDE << true << (uint8_t)0xd2;
    QTest::addRow("ld a, h")
        << (uint8_t)0x7c << &this->cpu->registerHL << false << (uint8_t)0xcc;
    QTest::addRow("ld a, l")
        << (uint8_t)0x7d << &this->cpu->registerHL << true << (uint8_t)0xd3;
}

void Bz80BaseCpuInstructionsTest::test_ld_a_r() {
    QFETCH(uint8_t, opcode);
    QFETCH(RegisterPairType*, fromRegister);
    QFETCH(bool, isLowByte);
    QFETCH(uint8_t, startingValue);

    const uint8_t expCycles = 0;
    const FlagRegister expFlags = genRandomFlags();

    if(isLowByte) {
        fromRegister->setLower8(startingValue);
    } else {
        fromRegister->setUpper8(startingValue);
    }

    this->cpu->registerF = expFlags;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->currentOpcode = opcode;
    this->cpu->tick();
    uint8_t cycles = this->cpu->tick();

    QCOMPARE(this->cpu->registerA, startingValue);
    QCOMPARE(this->cpu->registerF, expFlags);
    QCOMPARE(cycles, expCycles);
}

void Bz80BaseCpuInstructionsTest::test_ld_a_addr_hl() {
    const uint8_t opcode = 0x7e;
    const uint8_t startingValue = 0xa2;
    const uint16_t address = 0x1;
    const uint8_t expCycles = MEMORY_ACCESS_CYCLES;
    const FlagRegister expFlags = genRandomFlags();

    this->bus.write8(address, startingValue, false);
    this->cpu->registerHL.set16(address);
    this->cpu->registerF = expFlags;
    this->cpu->currentOpcode = opcode;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    uint8_t cycles = this->cpu->tick();

    QCOMPARE(this->cpu->registerA, startingValue);
    QCOMPARE(this->cpu->registerF, expFlags);
    QCOMPARE(cycles, expCycles);
}

void Bz80BaseCpuInstructionsTest::test_ld_a_a() {
    const uint8_t opcode = 0x7f;
    const uint8_t startingValue = 0xfe;
    const uint8_t expCycles = 0;
    const FlagRegister expFlags = genRandomFlags();

    this->cpu->registerA = startingValue;
    this->cpu->registerF = expFlags;
    this->cpu->currentOpcode = opcode;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    uint8_t cycles = this->cpu->tick();

    QCOMPARE(this->cpu->registerA, startingValue);
    QCOMPARE(this->cpu->registerF, expFlags);
    QCOMPARE(cycles, expCycles);
}

QTEST_APPLESS_MAIN(Bz80BaseCpuInstructionsTest)

#include "tst_z80basecpuinstructions.moc"
