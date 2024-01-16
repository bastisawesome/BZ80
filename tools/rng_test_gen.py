#!/usr/bin/env python3

from random import random, randint

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
    print(f"16-bit: {hex(value)}")
    print(f"8-bit upper: {hex(upperValue)}")
    print(f"8-bit lower: {hex(lowerValue)}")

print_address()
print_value()
print_flags()
