#!/usr/bin/env python3

import sys
import struct
import random
import colorsys

from collections import OrderedDict

colors = []

RANDOM_SEED = 1622487670

# Maximum possible legal value for hue
MAX_HUE_VALUE = 360

MIN_VAL_VALUE = 50
MIN_SAT_VALUE = 50

MAX_SAT_VALUE = 80
MAX_VAL_VALUE = 100

def genColor(hue, sat, val):
    return [int(255 * i) for i in colorsys.hsv_to_rgb(hue / MAX_HUE_VALUE,
                                                      sat / 100.0,
                                                      val / 100.0)]

def genAllColors(h_range, s_range, v_range):
    for hue in range(*h_range):
        for sat in range(*s_range):
            for val in range(*v_range):
                color = genColor(hue, sat, val)
                colors.append(struct.pack("<BBB", color[0], color[1], color[2]))

if "lands" in sys.argv:
    filename = "lands.bin"
    # Land colors (70 < saturation < 155)
    hue_range = (70, 155)
elif "seas" in sys.argv:
    filename = "seas.bin"
    # Sea colors (175 < saturation < 255)
    hue_range = (175, 255)
elif "lakes" in sys.argv:
    filename = "lakes.bin"
    # Lake colors (256 < saturation < 335)
    hue_range = (256, 335)
elif "unknowns" in sys.argv:
    filename = "unknowns.bin"
    # All unknown colors (0 < saturation < 69
    hue_range = (0, 69)
else:
    print("Missing parameter.", file=sys.stderr)
    exit(1)

print(f"Generating {filename[:-5]} colors...")
genAllColors(hue_range, (MIN_SAT_VALUE, MAX_SAT_VALUE), (MIN_VAL_VALUE, MAX_VAL_VALUE))

print(f"Generated {len(colors)} colors.")

print("Removing duplicates...")
colors = list(OrderedDict.fromkeys(colors))

print(f"Shuffling {len(colors)} colors...")
random.Random(RANDOM_SEED).shuffle(colors)

colors = b"".join(colors)

file = open(filename, "wb")
file.write(colors)

