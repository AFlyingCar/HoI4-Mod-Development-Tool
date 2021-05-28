#!/usr/bin/env python3

import sys
import random
import colorsys
from array import array

colors = []

# Maximum possible legal value for hue
MAX_HUE_VALUE = 360

MIN_VAL_VALUE = 20
MIN_SAT_VALUE = 20

MAX_SAT_VALUE = 100
MAX_VAL_VALUE = 80

def genColor(hue, sat, val):
    return [int(255 * i) for i in colorsys.hsv_to_rgb(hue / MAX_HUE_VALUE,
                                                      sat / 100.0,
                                                      val / 100.0)]

def genAllColors(h_range, s_range, v_range):
    for hue in range(*h_range):
        for sat in range(*s_range):
            for val in range(*v_range):
                color = genColor(hue, sat, val)
                colors.append(array('B', color))

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

print("Shuffling...")
random.shuffle(colors)

colors = b"".join(colors)

file = open(filename, "wb")
file.write(colors)

