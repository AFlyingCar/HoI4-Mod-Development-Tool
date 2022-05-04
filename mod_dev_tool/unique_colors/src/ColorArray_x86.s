# ------------------------------------------------
#              LAND BINARY COLOR DATA
# ------------------------------------------------

    .section .rodata
    .global MN_ALL_LANDS
    .align  1
MN_ALL_LANDS:
    .incbin "lands.bin"
MN_ALL_LANDS_end:
    .global MN_ALL_LANDS_SIZE
    .align  4
MN_ALL_LANDS_SIZE:
    .int MN_ALL_LANDS_end - MN_ALL_LANDS

# ------------------------------------------------
#            SEA BINARY COLOR DATA
# ------------------------------------------------

    .section .rodata
    .global MN_ALL_SEAS
    .align  1
MN_ALL_SEAS:
    .incbin "seas.bin"
MN_ALL_SEAS_end:
    .global MN_ALL_SEAS_SIZE
    .align  4
MN_ALL_SEAS_SIZE:
    .int MN_ALL_SEAS_end - MN_ALL_SEAS

# ------------------------------------------------
#            LAKE BINARY COLOR DATA
# ------------------------------------------------

    .section .rodata
    .global MN_ALL_LAKES
    .align  1
MN_ALL_LAKES:
    .incbin "lakes.bin"
MN_ALL_LAKES_end:
    .global MN_ALL_LAKES_SIZE
    .align  4
MN_ALL_LAKES_SIZE:
    .int MN_ALL_LAKES_end - MN_ALL_LAKES

# ------------------------------------------------
#            UNKNOWN BINARY COLOR DATA
# ------------------------------------------------

    .section .rodata
    .global MN_ALL_UNKNOWNS
    .align  1
MN_ALL_UNKNOWNS:
    .incbin "unknowns.bin"
MN_ALL_UNKNOWNS_end:
    .global MN_ALL_UNKNOWNS_SIZE
    .align  4
MN_ALL_UNKNOWNS_SIZE:
    .int MN_ALL_UNKNOWNS_end - MN_ALL_UNKNOWNS

