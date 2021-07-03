#ifndef CONSOLE_OUTPUT_FUNCTIONS_H
# define CONSOLE_OUTPUT_FUNCTIONS_H

# include <functional>
# include <ostream>

namespace MapNormalizer::Log {
    class Message;

    bool outputToConsole(const Message&, bool, bool);
    bool outputToStream(const Message&, bool, bool,
                        const std::function<std::ostream&(uint8_t)>&);

    bool outputWithFormatting(const Message&);
    bool outputNoFormatting(const Message&);
}

#endif

