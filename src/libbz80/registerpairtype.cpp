#include "registerpairtype.hpp"

#include <cstdint>

namespace bz80 {

RegisterPairType::RegisterPairType(uint16_t initialValue):
    combinedValue(initialValue) {
    this->calculate8();
}

RegisterPairType::RegisterPairType(const RegisterPairType& source) {
    this->lower8Register = source.lower8Register;
    this->upper8Register = source.upper8Register;
    this->combinedValue = source.combinedValue;
}

void RegisterPairType::setUpper8(uint8_t value) {
    this->upper8Register = value;
    this->calculateCombined();
}

void RegisterPairType::setLower8(uint8_t value) {
    this->lower8Register = value;
    this->calculateCombined();
}

void RegisterPairType::addUpper8(uint8_t value) {
    this->upper8Register += value;
    this->calculateCombined();
}

void RegisterPairType::addLower8(uint8_t value) {
    this->lower8Register += value;
    this->calculateCombined();
}

uint8_t RegisterPairType::getUpper8() const {
    return this->upper8Register;
}

uint8_t RegisterPairType::getLower8() const {
    return this->lower8Register;
}

void RegisterPairType::set16(uint16_t value) {
    this->combinedValue = value;
    this->calculate8();
}

void RegisterPairType::add16(uint16_t value) {
    this->combinedValue += value;
    this->calculate8();
}

uint16_t RegisterPairType::get16() const {
    return this->combinedValue;
}

void RegisterPairType::calculateCombined() {
    this->combinedValue = this->lower8Register;
    this->combinedValue |= this->upper8Register << 8;
}

void RegisterPairType::calculate8() {
    this->lower8Register = static_cast<uint8_t>(this->combinedValue);
    this->upper8Register = static_cast<uint8_t>(this->combinedValue >> 8);
}

} // namespace bz80
