# ------------------------------------------------
#              LAND BINARY COLOR DATA
# ------------------------------------------------

    .section .rodata
    .global HMDT_ALL_LANDS
    .align  1
HMDT_ALL_LANDS:
    .incbin "lands.bin"
HMDT_ALL_LANDS_end:
    .global HMDT_ALL_LANDS_SIZE
    .align  4
HMDT_ALL_LANDS_SIZE:
    .int HMDT_ALL_LANDS_end - HMDT_ALL_LANDS

# ------------------------------------------------
#            SEA BINARY COLOR DATA
# ------------------------------------------------

    .section .rodata
    .global HMDT_ALL_SEAS
    .align  1
HMDT_ALL_SEAS:
    .incbin "seas.bin"
HMDT_ALL_SEAS_end:
    .global HMDT_ALL_SEAS_SIZE
    .align  4
HMDT_ALL_SEAS_SIZE:
    .int HMDT_ALL_SEAS_end - HMDT_ALL_SEAS

# ------------------------------------------------
#            LAKE BINARY COLOR DATA
# ------------------------------------------------

    .section .rodata
    .global HMDT_ALL_LAKES
    .align  1
HMDT_ALL_LAKES:
    .incbin "lakes.bin"
HMDT_ALL_LAKES_end:
    .global HMDT_ALL_LAKES_SIZE
    .align  4
HMDT_ALL_LAKES_SIZE:
    .int HMDT_ALL_LAKES_end - HMDT_ALL_LAKES

# ------------------------------------------------
#            UNKNOWN BINARY COLOR DATA
# ------------------------------------------------

    .section .rodata
    .global HMDT_ALL_UNKNOWNS
    .align  1
HMDT_ALL_UNKNOWNS:
    .incbin "unknowns.bin"
HMDT_ALL_UNKNOWNS_end:
    .global HMDT_ALL_UNKNOWNS_SIZE
    .align  4
HMDT_ALL_UNKNOWNS_SIZE:
    .int HMDT_ALL_UNKNOWNS_end - HMDT_ALL_UNKNOWNS

