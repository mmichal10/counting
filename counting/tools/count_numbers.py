#!/usr/bin/env python3

import sys

BITMAP_SIZE = 1 << 29  # 2^32 bits / 8 = 2^29 bytes = 512 MB

def bit_get(bm, i):
    return (bm[i >> 3] >> (i & 7)) & 1

def bit_set(bm, i):
    bm[i >> 3] |= (1 << (i & 7))

def bit_clear(bm, i):
    bm[i >> 3] &= ~(1 << (i & 7))

def popcount_bytearray(bm):
    # Python 3.10+: int.bit_count() is very fast
    return sum(b.bit_count() for b in bm)

def main(path):
    seen_once = bytearray(BITMAP_SIZE)
    seen_multiple = bytearray(BITMAP_SIZE)

    with open(path, "r", buffering=1024 * 1024) as f:
        for line in f:
            x = int(line.strip())

            if bit_get(seen_multiple, x):
                continue

            if bit_get(seen_once, x):
                bit_clear(seen_once, x)
                bit_set(seen_multiple, x)
            else:
                bit_set(seen_once, x)

    unique = popcount_bytearray(seen_once)
    multiple = popcount_bytearray(seen_multiple)
    distinct = unique + multiple

    print(f"Distinct numbers: {distinct}")
    print(f"Unique numbers (occur once): {unique}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: count_numbers.py <file>")
        sys.exit(1)
    main(sys.argv[1])

