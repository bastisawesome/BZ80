#pragma once

#include <cstdint>

#include "registerpairtype.hpp"

namespace bz80 {

struct FlagRegister {
    uint8_t carry: 1;
    uint8_t add_sub: 1;
    uint8_t overflow: 1;
    uint8_t unused1: 1;
    uint8_t halfcarry: 1;
    uint8_t unused2: 1;
    uint8_t zero: 1;
    uint8_t sign: 1;
};

class RegisterInterface {
private:
    uint8_t registerA_int;
    RegisterPairType registerBC_int, registerDE_int, registerHL_int;

public:
    struct FlagRegister registerF;

    RegisterInterface();

    uint8_t registerA() const;
    uint8_t registerB() const;
    uint8_t registerC() const;
    uint8_t registerD() const;
    uint8_t registerE() const;
    uint8_t registerH() const;
    uint8_t registerL() const;

    void registerA(uint8_t value);
    void registerB(uint8_t value);
    void registerC(uint8_t value);
    void registerD(uint8_t value);
    void registerE(uint8_t value);
    void registerH(uint8_t value);
    void registerL(uint8_t value);

    uint16_t registerBC() const;
    uint16_t registerDE() const;
    uint16_t registerHL() const;

    void registerBC(uint16_t value);
    void registerDE(uint16_t value);
    void registerHL(uint16_t value);
};

} // namespace bz80

