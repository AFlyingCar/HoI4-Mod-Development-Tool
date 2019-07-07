# ------------------------------------------------
#              LAND BINARY COLOR DATA
# ------------------------------------------------

    .section .rodata
    .global MN_ALL_LANDS
    .type   MN_ALL_LANDS, @object
    .align  1
MN_ALL_LANDS:
    .incbin "lands.bin"
MN_ALL_LANDS_end:
    .global MN_ALL_LANDS_SIZE
    .type   MN_ALL_LANDS_SIZE, @object
    .align  4
MN_ALL_LANDS_SIZE:
    .int MN_ALL_LANDS_end - MN_ALL_LANDS

# ------------------------------------------------
#            SEA BINARY COLOR DATA
# ------------------------------------------------

    .section .rodata
    .global MN_ALL_SEAS
    .type   MN_ALL_SEAS, @object
    .align  1
MN_ALL_SEAS:
    .incbin "seas.bin"
MN_ALL_SEAS_end:
    .global MN_ALL_SEAS_SIZE
    .type   MN_ALL_SEAS_SIZE, @object
    .align  4
MN_ALL_SEAS_SIZE:
    .int MN_ALL_SEAS_end - MN_ALL_SEAS

# ------------------------------------------------
#            LAKE BINARY COLOR DATA
# ------------------------------------------------

    .section .rodata
    .global MN_ALL_LAKES
    .type   MN_ALL_LAKES, @object
    .align  1
MN_ALL_LAKES:
    .incbin "lakes.bin"
MN_ALL_LAKES_end:
    .global MN_ALL_LAKES_SIZE
    .type   MN_ALL_LAKES_SIZE, @object
    .align  4
MN_ALL_LAKES_SIZE:
    .int MN_ALL_LAKES_end - MN_ALL_LAKES

# ------------------------------------------------
#            UNKNOWN BINARY COLOR DATA
# ------------------------------------------------

    .section .rodata
    .global MN_ALL_UNKNOWNS
    .type   MN_ALL_UNKNOWNS, @object
    .align  1
MN_ALL_UNKNOWNS:
    .incbin "unknowns.bin"
MN_ALL_UNKNOWNS_end:
    .global MN_ALL_UNKNOWNS_SIZE
    .type   MN_ALL_UNKNOWNS_SIZE, @object
    .align  4
MN_ALL_UNKNOWNS_SIZE:
    .int MN_ALL_UNKNOWNS_end - MN_ALL_UNKNOWNS

