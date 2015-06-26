#!/usr/bin/python3

import math

print("{")
for i in range(128):
    print('\t{},'.format(round(math.sin(i / 256 * math.pi) * (1 << 15))))

print("}")
