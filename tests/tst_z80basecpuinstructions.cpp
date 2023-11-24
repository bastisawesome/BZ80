#include <QTest>

#include <z80basecpu.hpp>
#include <mmioDeviceManager.hpp>
#include <mmioRam.hpp>
#include <registerpairtype.hpp>

using bz80::Z80BaseCpu;
using bz80::MmioDeviceManager;
using bz80::MmioRam;
using bz80::RegisterPairType;
using bz80::FlagRegister;

class Bz80BaseCpuInstructionsTest: public QObject {
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

    void test_ld_r_imm_data();
    void test_ld_r_imm();
    void test_ld_addr_hl_imm();
    void test_ld_a_imm();
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
    QTest::addColumn<RegisterPairType*>("expectedRegisterPair");
    QTest::addColumn<bool>("isLowByte");
    QTest::addColumn<FlagRegister>("startingFlags");
    QTest::addColumn<uint8_t>("expectedCycles");

    QTest::newRow("LD B, 15") << (uint8_t)0x06
                              << (uint8_t)15
                              << &this->cpu->registerBC
                              << false
                              << FlagRegister {true, false, false, false,
                                 false, false, false, false}
                              << MEMORY_ACCESS_CYCLES;
    QTest::newRow("LD C, 26") << (uint8_t)0x0E
                              << (uint8_t)26
                              << &this->cpu->registerBC
                              << true
                              << FlagRegister {false, true, false, false,
                                 false, false, false, false}
                              << MEMORY_ACCESS_CYCLES;
    QTest::newRow("LD D, 92") << (uint8_t)0x16
                              << (uint8_t)92
                              << &this->cpu->registerDE
                              << false
                              << FlagRegister {false, false, true, false,
                                 false, false, false, false}
                              << MEMORY_ACCESS_CYCLES;
    QTest::newRow("LD E, 0xad") << (uint8_t)0x1E
                                << (uint8_t)0xad
                                << &this->cpu->registerDE
                                << true
                                << FlagRegister {false, false, false, true,
                                   false, false, false, false}
                                << MEMORY_ACCESS_CYCLES;
    QTest::newRow("LD H, 69") << (uint8_t)0x26
                              << (uint8_t)69
                              << &this->cpu->registerHL
                              << false
                              << FlagRegister {false, false, false, false,
                                 true, false, false, false}
                              << MEMORY_ACCESS_CYCLES;
    QTest::newRow("LD L, 142") << (uint8_t)0x2E
                               << (uint8_t)142
                               << &this->cpu->registerHL
                               << true
                               << FlagRegister {false, false, false, false,
                                  false, true, false, false}
                               << MEMORY_ACCESS_CYCLES;
}

void Bz80BaseCpuInstructionsTest::test_ld_r_imm() {
    QFETCH(uint8_t, opcode);
    QFETCH(uint8_t, immediate);
    QFETCH(RegisterPairType*, expectedRegisterPair);
    QFETCH(bool, isLowByte);
    QFETCH(FlagRegister, startingFlags);
    QFETCH(uint8_t, expectedCycles);

    this->cpu->bus.write8(this->cpu->programCounter, immediate);

    this->cpu->currentOpcode = opcode;
    this->cpu->state = Z80BaseCpu::CpuState::DECODE;
    this->cpu->tick();
    this->cpu->registerF = startingFlags;
    uint8_t foundCycles = this->cpu->tick();

    uint8_t foundValue = isLowByte ?
                expectedRegisterPair->getLower8() :
                expectedRegisterPair->getUpper8();

    QCOMPARE(foundValue, immediate);
    auto foundFlags = this->cpu->registerF;
    QCOMPARE(foundFlags, startingFlags);
    QCOMPARE(foundCycles, expectedCycles);
}

void Bz80BaseCpuInstructionsTest::test_ld_addr_hl_imm() {
    uint8_t opcode = 0x36;
    uint8_t immediate = 7;
    FlagRegister startingFlags {false, false, false, false,
                                false, false, true, false};
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
    FlagRegister startingFlags {false, false, false, false,
                                false, false, false, true};
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

QTEST_APPLESS_MAIN(Bz80BaseCpuInstructionsTest)

#include "tst_z80basecpuinstructions.moc"
