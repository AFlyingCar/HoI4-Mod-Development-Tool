#!/usr/bin/env python3

import random
import colorsys
from array import array

RED_START = [255, 0, 0]
GREEN_START = [0, 255, 0]
BLUE_START = [0, 0, 255]

land_colors = []
sea_colors = []
lake_colors = []
unknown_colors = []

# Maximum possible legal value for hue
MAX_HUE_VALUE = 360

MIN_VAL_VALUE = 20
MIN_SAT_VALUE = 20

print("Generating land colors...")

# Generate land colors (70 < saturation < 155)
for hue in range(70, 155):
    for sat in range(MIN_SAT_VALUE, 100):
        for val in range(MIN_VAL_VALUE, 100):
            color = [int(255 * i) for i in colorsys.hsv_to_rgb(hue / MAX_HUE_VALUE,
                                                               sat / 100.0,
                                                               val / 100.0)]

            land_colors.append(array('B', color))

print("Generating sea colors...")
# Generate sea colors (175 < saturation < 255)
for hue in range(175, 255):
    for sat in range(MIN_SAT_VALUE, 100):
        for val in range(MIN_VAL_VALUE, 100):
            color = [int(255 * i) for i in colorsys.hsv_to_rgb(hue / MAX_HUE_VALUE,
                                                               sat / 100.0,
                                                               val / 100.0)]

            sea_colors.append(array('B', color))

print("Generating lake colors...")
# Generate lake colors (256 < saturation < 335)
for hue in range(256, 335):
    for sat in range(MIN_SAT_VALUE, 100):
        for val in range(MIN_VAL_VALUE, 100):
            color = [int(255 * i) for i in colorsys.hsv_to_rgb(hue / MAX_HUE_VALUE,
                                                               sat / 100.0,
                                                               val / 100.0)]

            lake_colors.append(array('B', color))

print("Generating unknown colors...")
# Generate all unknown colors (0 < saturation < 69
for hue in range(0, 69):
    for sat in range(MIN_SAT_VALUE, 100):
        for val in range(MIN_VAL_VALUE, 100):
            color = [int(255 * i) for i in colorsys.hsv_to_rgb(hue / MAX_HUE_VALUE,
                                                               sat / 100.0,
                                                               val / 100.0)]

            unknown_colors.append(array('B', color))

print(f"Generated {len(land_colors)} land colors.")
print(f"Generated {len(sea_colors)} sea colors.")
print(f"Generated {len(lake_colors)} lake colors.")
print(f"Generated {len(unknown_colors)} unknown colors.")

print("Shuffling all color lists...")
random.shuffle(land_colors)
random.shuffle(sea_colors)
random.shuffle(lake_colors)
random.shuffle(unknown_colors)

land_colors = b"".join(land_colors)
sea_colors = b"".join(sea_colors)
lake_colors = b"".join(lake_colors)
unknown_colors = b"".join(unknown_colors)

land_file = open("lands.bin", "wb")
sea_file = open("seas.bin", "wb")
lake_file = open("lakes.bin", "wb")
unknown_file = open("unknowns.bin", "wb")

land_file.write(land_colors)
sea_file.write(sea_colors)
lake_file.write(lake_colors)
unknown_file.write(unknown_colors)

