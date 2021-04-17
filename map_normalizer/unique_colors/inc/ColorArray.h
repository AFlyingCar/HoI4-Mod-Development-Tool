/**
 * @file ColorArray.h
 *
 * @brief Declares all external color arrays used by the shape finder algorithm.
 */

#ifndef COLOR_ARRAY_H
# define COLOR_ARRAY_H

extern "C" {
    //! All LAND province color values.
    extern const unsigned char MN_ALL_LANDS[];
    //! The size of MN_ALL_LANDS.
    extern const unsigned int  MN_ALL_LANDS_SIZE;

    //! All SEA province color values.
    extern const unsigned char MN_ALL_SEAS[];
    //! The size of MN_ALL_SEAS.
    extern const unsigned int  MN_ALL_SEAS_SIZE;

    //! All LAKE province color values.
    extern const unsigned char MN_ALL_LAKES[];
    //! The size of MN_ALL_LAKES.
    extern const unsigned int  MN_ALL_LAKES_SIZE;

    //! All UNKNOWN province color values.
    extern const unsigned char MN_ALL_UNKNOWNS[];
    //! The size of MN_ALL_UNKNOWNS.
    extern const unsigned int  MN_ALL_UNKNOWNS_SIZE;
}

#endif

