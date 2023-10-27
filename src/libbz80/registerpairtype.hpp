#pragma once

#include <cstdint>

namespace bz80 {

class RegisterPairType {
private:
    uint8_t upper8Register, lower8Register;
    uint16_t combinedValue;

public:
    RegisterPairType(uint16_t initialValue = 0);
    RegisterPairType(const RegisterPairType& source);

    /**
     * @brief setUpper8 Assigns `value` to the upper 8-bit register.
     * @param value Value to assign to the upper 8-bit register.
     *
     * Recalculates the value in the register pair.
     */
    void setUpper8(uint8_t value);

    /**
     * @brief setLower8 Assigns `value` to the lower 8-bit register.
     * @param value Value to assign to the lower 8-bit register.
     *
     * Recalculates the value in the register pair.
     */
    void setLower8(uint8_t value);

    /**
     * @brief addUpper8 Adds the 8-bit `value` to the upper 8-bit register.
     * @param value Value to be added to the register.
     *
     * For subtraction, values passed in should be in two's complement format.
     * Recalculates the value in the register pair.
     */
    void addUpper8(uint8_t value);

    /**
     * @brief addLower8 Adds the 8-bit `value` to the lower 8-bit register.
     * @param value Value to be added to the register.
     *
     * For subtraction, values passed in should be in two's complement format.
     * Recalculates the value in the register pair.
     */
    void addLower8(uint8_t value);

    /**
     * @brief getUpper8 Returns the value in the upper 8-bit register.
     * @return Value stored in the register.
     */
    uint8_t getUpper8() const;

    /**
     * @brief getLower8 Returns the value in the lower 8-bit register.
     * @return Value stored in the register.
     */
    uint8_t getLower8() const;

    /**
     * @brief set16 Assigns the 16-bit `value` into the register pair.
     * @param value Value to assign to the register pair.
     *
     * Recalculates the values store in the 8-bit registers.
     */
    void set16(uint16_t value);

    /**
     * @brief add16 Adds the 16-bit `value` to the register pair.
     * @param value Value to be added to the register pair.
     *
     * For subtraction, values passed in should be in two's complement format.
     * Recalculates the values store in the 8-bit registers.
     */
    void add16(uint16_t value);

    /**
     * @brief get16 Returns the value stored in the register pair.
     * @return Value stored in the register pair.
     */
    uint16_t get16() const;

    // Operator Overloads
    RegisterPairType& operator=(const uint16_t& value) {
        this->set16(value);

        return *this;
    }

    RegisterPairType& operator=(const RegisterPairType& source) {
        this->lower8Register = source.lower8Register;
        this->upper8Register = source.upper8Register;
        this->combinedValue = source.combinedValue;

        return *this;
    }

    RegisterPairType operator+(const uint16_t value) {
        return RegisterPairType(this->combinedValue + value);
    }

    RegisterPairType operator+(const RegisterPairType& source) {
        return RegisterPairType(this->combinedValue + source.combinedValue);
    }

    RegisterPairType operator-(const uint16_t value) {
        return RegisterPairType(this->combinedValue - value);
    }

    RegisterPairType operator-(const RegisterPairType& source) {
        return RegisterPairType(this->combinedValue - source.combinedValue);
    }

    RegisterPairType& operator+=(const uint16_t& value) {
        this->combinedValue += value;
        this->calculate8();

        return *this;
    }

    RegisterPairType& operator+=(const RegisterPairType& source) {
        this->combinedValue += source.combinedValue;
        this->calculate8();

        return *this;
    }

    RegisterPairType& operator-=(const uint16_t value) {
        this->combinedValue -= value;
        this->calculate8();

        return *this;
    }

    RegisterPairType& operator-=(const RegisterPairType& source) {
        this->combinedValue -= source.combinedValue;
        this->calculate8();

        return *this;
    }

    RegisterPairType& operator++() {
        this->add16(1);

        return *this;
    }

    RegisterPairType operator++(int) {
        auto tmp = RegisterPairType(*this);

        this->add16(1);

        return tmp;
    }

    RegisterPairType& operator--() {
        this->add16(-1);

        return *this;
    }

    RegisterPairType operator--(int) {
        auto tmp = RegisterPairType(*this);

        this->add16(-1);

        return tmp;
    }

private:
    /**
     * @brief calculateCombined Calculates the register pair based on the
     * values stored in the 8-bit registers.
     */
    void calculateCombined();

    /**
     * @brief calculate8 Calculates the 8-bit registers based on the value in
     * the register pair.
     */
    void calculate8();
};

}
