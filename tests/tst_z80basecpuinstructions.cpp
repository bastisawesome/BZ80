#include "qtestcase.h"
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

    this->bus.write8(15, startingValue);
    this->cpu->currentOpcode = 0x35;
    this->cpu->registerHL = 15;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->registerF
        = { false, false, false, false, true, false, false, false };
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

    this->cpu->bus.write8(1, expOffset);

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

    this->bus.write8(11, 3);

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

    this->bus.write8(startingPC, offset);

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
    this->bus.write8(3, 107);

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

    this->bus.write8(3, toSub);
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

QTEST_APPLESS_MAIN(Bz80BaseCpuInstructionsTest)

#include "tst_z80basecpuinstructions.moc"
