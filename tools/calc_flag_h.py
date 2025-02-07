#!/usr/bin/env python3

import sys

def main():
    args = sys.argv
    if len(args) < 4 or len(args) > 5:
        print(f'Usage: {args[0]} <bool isSubtraction> <int originalValue> <int toAdd> [bool 16-bit]')
        sys.exit(1)

    isAdd = eval(args[1])
    origValue = int(args[2])
    toAdd = int(args[3])
    is16Bit = False

    if(len(args) == 5):
        is16Bit = eval(args[4])

    if not is16Bit:
        result = calc_flag_h(origValue, toAdd, isAdd)
    else:
        result = calc_flag_h_16(origValue, toAdd, isAdd)

    print(f'Expected H flag: {result}')

def calc_flag_h(origVal: int, toAdd: int, isSub: bool):
    if not isSub:
        return ((origVal & 0xf) + (toAdd & 0xf)) > 0xf

    return origVal - toAdd < 0

def calc_flag_h_16(origVal: int, toAdd: int, isSub: bool):
    if not isSub:
        return ((origVal & 0xfff) + (toAdd & 0xfff)) > 0xfff

    return origVal - toAdd < 0

if __name__ == '__main__':
    main()
