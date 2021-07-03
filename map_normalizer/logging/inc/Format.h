#ifndef MN_FORMAT_H
# define MN_FORMAT_H

# include <cstdint>

namespace MapNormalizer::Log {
    /**
     * @brief Defines a single formatting operation
     */
    struct Format {
        static constexpr std::size_t DATA_SIZE = 1;

        /**
         * @brief The type of operation to perform
         */
        enum Type: uint8_t {
            CLEAR     = 0b00000000, // Clear formatting
            RESET     = 0b11111111, // Reset formatting

            BOLD      = 0b00000001, // Bold
            FAINT     = 0b00000010, // Faint/dimming
            ITALIC    = 0b00000100, // Italics
            UNDERLINE = 0b00001000, // Underline
            BLINK     = 0b00010000, // Blink
            INVISIBLE = 0b00100000, // Invisible
            STRIKE    = 0b01000000, // Strike-through
            COLOR     = 0b10000000, // Color
        } type;

        /**
         * @brief Extra data for formatting options that need it
         *
         * @details Defined Formatting options that make use of this:
         *     BLINK:
         *       0 == Slow
         *       1 == Rapid
         *     COLOR:
         *       Full byte is used for 256-color: https://i.stack.imgur.com/KTSQa.png
         */
        uint8_t data[DATA_SIZE] = { 0 };

        Format operator|(const MapNormalizer::Log::Format&) const;
        bool operator==(const MapNormalizer::Log::Format&) const;
    };

    /**
     * @brief Builds a single Format from multiple Formats
     *
     * @tparam Formats The list of Format types
     * @param formats The list of formats
     *
     * @return A single Format
     */
    template<typename... Formats>
    Format buildFormat(Formats&&... formats) {
        return (formats | ...);
    }
}

/**
 * @brief Creates a Format for the given type
 *
 * @param TYPE The format type
 * @param R The red color
 * @param G The green color
 * @param B The blue color
 */
# define FTYPE(TYPE, COLOR) \
    MapNormalizer::Log::Format { MapNormalizer::Log::Format::Type::TYPE, { COLOR } }

// Formatting types
# define FCLEAR     FTYPE(CLEAR, 0)
# define FRESET     FTYPE(RESET, 0)
# define FBOLD      FTYPE(BOLD, 0)
# define FFAINT     FTYPE(FAINT, 0)
# define FITALIC    FTYPE(ITALIC, 0)
# define FUNDERLINE FTYPE(UNDERLINE, 0)
# define FBLINKSLOW FTYPE(BLINK, 0)
# define FBLINKFAST FTYPE(BLINK, 1)
# define FINVISIBLE FTYPE(INVISIBLE, 0)
# define FSTRIKE    FTYPE(STRIKE, 0)
# define FCOLOR(C)  FTYPE(COLOR, C)

/**
 * @brief Builds a single Format option
 */
# define FORMAT(...) MapNormalizer::Log::buildFormat(__VA_ARGS__)

#endif

