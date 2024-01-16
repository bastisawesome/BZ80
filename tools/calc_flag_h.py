#!/usr/bin/env python3

import sys

def main():
    args = sys.argv
    if len(args) != 4:
        print(f'Usage: {args[0]} <bool isSubtraction> <int originalValue> <int toAdd>')
        sys.exit(1)

    isAdd = eval(args[1])
    origValue = int(args[2])
    toAdd = int(args[3])

    print(f'Expected H flag: {calc_flag_h(origValue, toAdd, isAdd)}')

def calc_flag_h(origVal: int, toAdd: int, isSub: bool):
    if not isSub:
        return ((origVal & 0xf) + (toAdd & 0xf)) > 0xf

    return origVal - toAdd < 0

if __name__ == '__main__':
    main()
