/**
 * @file ConsoleOutputFunctions.h
 *
 * @brief Defines functions for writing to the console
 */

#ifndef CONSOLE_OUTPUT_FUNCTIONS_H
# define CONSOLE_OUTPUT_FUNCTIONS_H

# include <functional>
# include <ostream>
# include <cstdint>

namespace HMDT::Log {
    class Message;

    bool outputToConsole(const Message&, bool, bool);
    bool outputToStream(const Message&, bool, bool,
                        const std::function<std::ostream&(uint8_t)>&,
                        bool = false,
                        bool = true);

    bool outputWithFormatting(const Message&);
    bool outputNoFormatting(const Message&);
}

#endif

