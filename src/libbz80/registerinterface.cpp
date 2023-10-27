#include "registerinterface.hpp"

namespace bz80 {

uint8_t RegisterInterface::registerA() const {
    return this->registerA_int;
}
uint8_t RegisterInterface::registerB() const {
    return this->registerBC_int.getUpper8();
}
uint8_t RegisterInterface::registerC() const {
    return this->registerBC_int.getLower8();
}
uint8_t RegisterInterface::registerD() const {
    return this->registerDE_int.getUpper8();
}
uint8_t RegisterInterface::registerE() const {
    return this->registerDE_int.getLower8();
}
uint8_t RegisterInterface::registerH() const {
    return this->registerHL_int.getUpper8();
}
uint8_t RegisterInterface::registerL() const {
    return this->registerHL_int.getLower8();
}

void RegisterInterface::registerA(uint8_t value){
    this->registerA_int = value;
}

void RegisterInterface::registerB(uint8_t value){
    this->registerBC_int.setUpper8(value);
}

void RegisterInterface::registerC(uint8_t value){
    this->registerBC_int.setLower8(value);
}

void RegisterInterface::registerD(uint8_t value){
    this->registerDE_int.setUpper8(value);
}

void RegisterInterface::registerE(uint8_t value){
    this->registerDE_int.setLower8(value);
}

void RegisterInterface::registerH(uint8_t value){
    this->registerHL_int.setUpper8(value);
}

void RegisterInterface::registerL(uint8_t value){
    this->registerHL_int.setLower8(value);
}

uint16_t RegisterInterface::registerBC() const {
    return this->registerBC_int.get16();
}
uint16_t RegisterInterface::registerDE() const {
    return this->registerDE_int.get16();
}
uint16_t RegisterInterface::registerHL() const {
    return this->registerHL_int.get16();
}

void RegisterInterface::registerBC(uint16_t value) {
    this->registerBC_int.set16(value);
}

void RegisterInterface::registerDE(uint16_t value) {
    this->registerDE_int.set16(value);
}

void RegisterInterface::registerHL(uint16_t value) {
    this->registerHL_int.set16(value);
}

} // namespace bz80
