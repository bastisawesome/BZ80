#include <QtTest>

// add necessary includes here
#include "registerpairtype.hpp"

using bz80::RegisterPairType;

class RegisterPairTypeTest : public QObject {
    Q_OBJECT

public:
    RegisterPairTypeTest();
    ~RegisterPairTypeTest();

private:
    RegisterPairType rp;

private slots:
    void init();
    void cleanup();

    // Implementation Details
    void test_DefaultConstructor();
    void test_U16Constructor();
    void test_CopyConstructor();
    void test_calculateCombined();
    void test_calculate8();

    // Interface Tests
    void test_setUpper8();
    void test_setLower8();
    void test_addUpper8();
    void test_addLower8();
    void test_getUpper8();
    void test_getLower8();
    void test_set16();
    void test_add16();
    void test_get16();

    // Operator tests
    void test_assignU16();
    void test_assignRegisterPair();
    void test_addU16();
    void test_addRegisterPair();
    void test_subtractU16();
    void test_subtractRegisterPair();
    void test_addAssignU16();
    void test_addAssignRegisterPair();
    void test_subtractAssignU16();
    void test_subtractAssignRegisterPair();
    void test_preIncrement();
    void test_postIncrement();
    void test_preDecrement();
    void test_postDecrement();
};

RegisterPairTypeTest::RegisterPairTypeTest() {}

RegisterPairTypeTest::~RegisterPairTypeTest() {}

void RegisterPairTypeTest::init() {
    this->rp = RegisterPairType();
}

void RegisterPairTypeTest::cleanup() {}

void RegisterPairTypeTest::test_DefaultConstructor() {
    QCOMPARE(this->rp.upper8Register, 0);
    QCOMPARE(this->rp.lower8Register, 0);
    QCOMPARE(this->rp.combinedValue, 0);
}

void RegisterPairTypeTest::test_U16Constructor() {
    auto rp = RegisterPairType(0xcafe);

    QCOMPARE(rp.upper8Register, 0xca);
    QCOMPARE(rp.lower8Register, 0xfe);
    QCOMPARE(rp.combinedValue, 0xcafe);
}

void RegisterPairTypeTest::test_CopyConstructor() {
    auto rp1 = RegisterPairType();
    rp1.upper8Register = 0xde;
    rp1.lower8Register = 0xad;
    rp1.combinedValue = 0xdead;

    auto rp2 = RegisterPairType(rp1);

    QCOMPARE(rp1.upper8Register, rp2.upper8Register);
    QCOMPARE(rp1.lower8Register, rp2.lower8Register);
    QCOMPARE(rp1.combinedValue, rp2.combinedValue);
    QVERIFY(!(&rp1 == &rp2));
}

void RegisterPairTypeTest::test_calculateCombined() {
    this->rp.upper8Register = 0xbe;
    this->rp.lower8Register = 0xef;
    this->rp.calculateCombined();

    QCOMPARE(this->rp.combinedValue, 0xbeef);
}

void RegisterPairTypeTest::test_calculate8() {
    this->rp.combinedValue = 0xbeef;
    this->rp.calculate8();

    QCOMPARE(this->rp.upper8Register, 0xbe);
    QCOMPARE(this->rp.lower8Register, 0xef);
}

void RegisterPairTypeTest::test_setUpper8() {
    this->rp.setUpper8(0x69);

    QCOMPARE(this->rp.upper8Register, 0x69);
    QCOMPARE(this->rp.lower8Register, 0);
    QCOMPARE(this->rp.combinedValue, 0x6900);
}

void RegisterPairTypeTest::test_setLower8() {
    this->rp.setLower8(0x42);

    QCOMPARE(this->rp.upper8Register, 0);
    QCOMPARE(this->rp.lower8Register, 0x42);
    QCOMPARE(this->rp.combinedValue, 0x42);
}

void RegisterPairTypeTest::test_addUpper8() {
    this->rp.addUpper8(0x69);

    QCOMPARE(this->rp.upper8Register, 0x69);
    QCOMPARE(this->rp.lower8Register, 0);
    QCOMPARE(this->rp.combinedValue, 0x6900);

    this->rp.addUpper8(-0x27);

    QCOMPARE(this->rp.upper8Register, 0x42);
    QCOMPARE(this->rp.lower8Register, 0);
    QCOMPARE(this->rp.combinedValue, 0x4200);
}

void RegisterPairTypeTest::test_addLower8() {
    this->rp.addLower8(0xbe);

    QCOMPARE(this->rp.upper8Register, 0);
    QCOMPARE(this->rp.lower8Register, 0xbe);
    QCOMPARE(this->rp.combinedValue, 0xbe);

    this->rp.addLower8(-0x11);

    QCOMPARE(this->rp.upper8Register, 0);
    QCOMPARE(this->rp.lower8Register, 0xad);
    QCOMPARE(this->rp.combinedValue, 0xad);
}

void RegisterPairTypeTest::test_getUpper8() {
    auto rp = RegisterPairType(0xcafe);

    QCOMPARE(rp.getUpper8(), 0xca);
}

void RegisterPairTypeTest::test_getLower8() {
    auto rp = RegisterPairType(0xcafe);

    QCOMPARE(rp.getLower8(), 0xfe);
}

void RegisterPairTypeTest::test_set16() {
    this->rp.set16(0xdead);

    QCOMPARE(this->rp.upper8Register, 0xde);
    QCOMPARE(this->rp.lower8Register, 0xad);
    QCOMPARE(this->rp.combinedValue, 0xdead);
}

void RegisterPairTypeTest::test_add16() {
    this->rp.add16(0x420);

    QCOMPARE(this->rp.upper8Register, 0x04);
    QCOMPARE(this->rp.lower8Register, 0x20);
    QCOMPARE(this->rp.combinedValue, 0x420);

    this->rp.add16(-0x3b7);

    QCOMPARE(this->rp.upper8Register, 0);
    QCOMPARE(this->rp.lower8Register, 0x69);
    QCOMPARE(this->rp.combinedValue, 0x69);
}

void RegisterPairTypeTest::test_get16() {
    auto rp = RegisterPairType(0xbeef);

    QCOMPARE(rp.get16(), 0xbeef);
}

void RegisterPairTypeTest::test_assignU16() {
    this->rp = 0xacdc;

    QCOMPARE(this->rp.upper8Register, 0xac);
    QCOMPARE(this->rp.lower8Register, 0xdc);
    QCOMPARE(this->rp.combinedValue, 0xacdc);
}

void RegisterPairTypeTest::test_assignRegisterPair() {
    auto rp1 = RegisterPairType(0xabcd);
    this->rp = rp1;

    QCOMPARE(this->rp.upper8Register, rp1.upper8Register);
    QCOMPARE(this->rp.lower8Register, rp1.lower8Register);
    QCOMPARE(this->rp.combinedValue, rp1.combinedValue);
    QVERIFY(!(&this->rp == &rp1));
}

void RegisterPairTypeTest::test_addU16() {
    auto rp1 = this->rp + 0xbad;

    QCOMPARE(rp1.upper8Register, 0xb);
    QCOMPARE(rp1.lower8Register, 0xad);
    QCOMPARE(rp1.combinedValue, 0xbad);
}

void RegisterPairTypeTest::test_addRegisterPair() {
    auto rp1 = RegisterPairType(0xdad);
    auto rp2 = this->rp + rp1;

    QCOMPARE(rp2.upper8Register, 0xd);
    QCOMPARE(rp2.lower8Register, 0xad);
    QCOMPARE(rp2.combinedValue, 0xdad);
}

void RegisterPairTypeTest::test_subtractU16() {
    this->rp = RegisterPairType(0xdb3e);
    auto rp1 = this->rp - 0x1040;

    QCOMPARE(rp1.upper8Register, 0xca);
    QCOMPARE(rp1.lower8Register, 0xfe);
    QCOMPARE(rp1.combinedValue, 0xcafe);
}

void RegisterPairTypeTest::test_subtractRegisterPair() {
    this->rp = RegisterPairType(0xdb6e);
    auto rp1 = RegisterPairType(0x1070);
    auto rp2 = this->rp - rp1;

    QCOMPARE(rp2.upper8Register, 0xca);
    QCOMPARE(rp2.lower8Register, 0xfe);
    QCOMPARE(rp2.combinedValue, 0xcafe);
}

void RegisterPairTypeTest::test_addAssignU16() {
    this->rp = RegisterPairType(0xbabe);
    this->rp += 0x1040;

    QCOMPARE(this->rp.upper8Register, 0xca);
    QCOMPARE(this->rp.lower8Register, 0xfe);
    QCOMPARE(this->rp.combinedValue, 0xcafe);
}

void RegisterPairTypeTest::test_addAssignRegisterPair() {
    this->rp = RegisterPairType(0x1c2f);
    auto rp1 = RegisterPairType(0xdeaf);

    this->rp += rp1;

    QCOMPARE(this->rp.upper8Register, 0xfa);
    QCOMPARE(this->rp.lower8Register, 0xde);
    QCOMPARE(this->rp.combinedValue, 0xfade);
}

void RegisterPairTypeTest::test_subtractAssignU16() {
    this->rp = RegisterPairType(0x5678);
    auto rp1 = RegisterPairType(0x1234);

    this->rp -= rp1;

    QCOMPARE(this->rp.upper8Register, 0x44);
    QCOMPARE(this->rp.lower8Register, 0x44);
    QCOMPARE(this->rp.combinedValue, 0x4444);
}

void RegisterPairTypeTest::test_subtractAssignRegisterPair() {
    this->rp = RegisterPairType(0xfade);
    auto rp1 = RegisterPairType(0x1c2f);

    this->rp -= rp1;

    QCOMPARE(this->rp.upper8Register, 0xde);
    QCOMPARE(this->rp.lower8Register, 0xaf);
    QCOMPARE(this->rp.combinedValue, 0xdeaf);
}

void RegisterPairTypeTest::test_preIncrement() {
    this->rp = RegisterPairType(0x12ff);
    auto rp1 = ++this->rp;

    QCOMPARE(this->rp.upper8Register, 0x13);
    QCOMPARE(this->rp.lower8Register, 0x00);
    QCOMPARE(this->rp.combinedValue, 0x1300);

    QCOMPARE(rp1.upper8Register, 0x13);
    QCOMPARE(rp1.lower8Register, 0x00);
    QCOMPARE(rp1.combinedValue, 0x1300);
}

void RegisterPairTypeTest::test_postIncrement() {
    this->rp = RegisterPairType(0x12ff);
    auto rp1 = this->rp++;

    QCOMPARE(this->rp.upper8Register, 0x13);
    QCOMPARE(this->rp.lower8Register, 0x00);
    QCOMPARE(this->rp.combinedValue, 0x1300);

    QCOMPARE(rp1.upper8Register, 0x12);
    QCOMPARE(rp1.lower8Register, 0xff);
    QCOMPARE(rp1.combinedValue, 0x12ff);
}

void RegisterPairTypeTest::test_preDecrement() {
    this->rp = RegisterPairType(0x1300);
    auto rp1 = --this->rp;

    QCOMPARE(this->rp.upper8Register, 0x12);
    QCOMPARE(this->rp.lower8Register, 0xff);
    QCOMPARE(this->rp.combinedValue, 0x12ff);

    QCOMPARE(rp1.upper8Register, 0x12);
    QCOMPARE(rp1.lower8Register, 0xff);
    QCOMPARE(rp1.combinedValue, 0x12ff);
}

void RegisterPairTypeTest::test_postDecrement() {
    this->rp = RegisterPairType(0x1300);
    auto rp1 = this->rp--;

    QCOMPARE(this->rp.upper8Register, 0x12);
    QCOMPARE(this->rp.lower8Register, 0xff);
    QCOMPARE(this->rp.combinedValue, 0x12ff);

    QCOMPARE(rp1.upper8Register, 0x13);
    QCOMPARE(rp1.lower8Register, 0x00);
    QCOMPARE(rp1.combinedValue, 0x1300);
}

QTEST_APPLESS_MAIN(RegisterPairTypeTest)

#include "tst_registerpairtype.moc"
