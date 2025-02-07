#!/usr/bin/env python3

from random import random, randint
from calc_flag_h import calc_flag_h, calc_flag_h_16

def gen_flag():
    return True if random() < 0.5 else False

def print_flags():
    print(f"Carry: {gen_flag()}")
    print(f"Add/Sub: {gen_flag()}")
    print(f"Overflow: {gen_flag()}")
    print(f"Half-Carry: {gen_flag()}")
    print(f"Zero: {gen_flag()}")
    print(f"Sign: {gen_flag()}")

def print_address():
    print(f"Address: {hex(randint(1,16))}")

def print_value():
    value = randint(256, 2**16)
    upperValue = value >> 8
    lowerValue = value & 0xff
    print(f"First 16-bit: {hex(value)}")
    print(f"First 8-bit upper: {hex(upperValue)}")
    print(f"First 8-bit lower: {hex(lowerValue)}")

    secondValue = randint(256, 2**16)
    secondUpperValue = value >> 8
    secondLowerValue = value & 0xff
    print(f"Second 16-bit: {hex(secondValue)}")
    print(f"Second 8-bit upper: {hex(secondUpperValue)}")
    print(f"Second 8-bit lower: {hex(secondLowerValue)}")

    print(f"Expected flag h from 8-bit addition: {calc_flag_h(upperValue, secondUpperValue, False)}")
    print(f"Expected flag h from 8-bit subtraction: {calc_flag_h(upperValue, secondUpperValue, True)}")

    print(f"Expected flag h from 16-bit addition: {calc_flag_h_16(value, secondValue, False)}")
    print(f"Expected flag h from 16-bit subtraction: {calc_flag_h_16(value, secondValue, True)}")


print_address()
print_value()
print_flags()
