
#include "ConsoleOutputFunctions.h"

#include <iostream>

#include "Message.h"

#define STR(X) #X

#define ANSI_ESCAPE "\33["
#define ANSI_ESCAPE_END "m"

#define ANSI_FOREGROUND_COLOR_START ANSI_ESCAPE "38;5;"
#define ANSI_BACKGROUND_COLOR_START ANSI_ESCAPE "48;5;"

#define BUILD_ANSI_CODE(CODE) ANSI_ESCAPE STR(CODE) ANSI_ESCAPE_END

#define ANSI_RESET BUILD_ANSI_CODE(0)
#define ANSI_BOLD BUILD_ANSI_CODE(1)
#define ANSI_FAINT BUILD_ANSI_CODE(2)
#define ANSI_ITALIC BUILD_ANSI_CODE(3)
#define ANSI_UNDERLINE BUILD_ANSI_CODE(4)
#define ANSI_SLOW_BLINK BUILD_ANSI_CODE(5)
#define ANSI_RAPID_BLINK BUILD_ANSI_CODE(6)
#define ANSI_INVISIBLE BUILD_ANSI_CODE(8)
#define ANSI_STRIKE BUILD_ANSI_CODE(9)

bool MapNormalizer::Log::outputToConsole(const Message& message,
                                         bool do_formatting,
                                         bool do_prefix)
{
    return outputToStream(message, do_formatting, do_prefix,
            [](uint8_t debug_level) -> std::ostream& {
                switch(static_cast<Message::Level>(debug_level)) {
                    case Message::Level::STDOUT:
                    case Message::Level::DEBUG:
                        return std::cout;
                    case Message::Level::ERROR:
                    case Message::Level::WARN:
                        return std::cerr;
                }
            });
}

/**
 * @brief Outputs the given message to a stream
 *
 * @param message The message to output
 * @param do_formatting Whether to output with formatting or not
 * @param do_prefix Whether or not to output a prefix with the message
 * @param get_ostream A function which returns a std::ostream& for the given
 *                    Message::Level
 *
 * @return true
 */
bool MapNormalizer::Log::outputToStream(const Message& message,
                                        bool do_formatting,
                                        bool do_prefix,
                                        const std::function<std::ostream&(uint8_t)>& get_ostream)
{
    const char* default_color = "";
    std::ostream& out = get_ostream(static_cast<uint8_t>(message.getDebugLevel()));

    // Color mapping
    // https://i.stack.imgur.com/KTSQa.png
    switch(message.getDebugLevel()) {
        case Message::Level::STDOUT:
            if(do_formatting) {
                default_color = ANSI_FOREGROUND_COLOR_START "15" ANSI_ESCAPE_END;
            }
            break;
        case Message::Level::DEBUG:
            if(do_formatting) {
                default_color = ANSI_FOREGROUND_COLOR_START "12" ANSI_ESCAPE_END;
            }
            break;
        case Message::Level::ERROR:
            if(do_formatting) {
                default_color = ANSI_FOREGROUND_COLOR_START "9" ANSI_ESCAPE_END;
            }
            break;
        case Message::Level::WARN:
            if(do_formatting) {
                default_color = ANSI_FOREGROUND_COLOR_START "11" ANSI_ESCAPE_END;
            }
            break;
    }

    // Output the formatting codes if applicable
    if(do_formatting) {
        out << default_color;
    }

    // Output the message prefix
    if(do_prefix) {
        out << "[" << message.getDebugLevel() << "] ~ ";
    }

    // Output every piece
    for(auto&& piece : message.getPieces()) {
        // Output ANSI formatting codes
        if(do_formatting && std::holds_alternative<Format>(piece)) {
            const auto& format = std::get<Format>(piece);

            if(format.type == Format::CLEAR) {
                out << ANSI_RESET;
            } else if(format.type == Format::RESET) {
                // Reset to normal and re-apply default formatting
                out << ANSI_RESET << default_color;
            } else if(format.type == Format::BOLD) {
                // Bold
                out << ANSI_BOLD;
            } else if(format.type == Format::FAINT) {
                // Fainting
                out << ANSI_FAINT;
            } else if(format.type == Format::ITALIC) {
                // Italicize
                out << ANSI_ITALIC;
            } else if(format.type == Format::UNDERLINE) {
                // Underline
                out << ANSI_UNDERLINE;
            } else if(format.type == Format::BLINK) {
                // Fast or Slow blinking
                if(format.data[0] == 0) {
                    out << ANSI_SLOW_BLINK;
                } else if(format.data[0] == 1) {
                    out << ANSI_RAPID_BLINK;
                }
            } else if(format.type == Format::INVISIBLE) {
                // Make invisible
                out << ANSI_INVISIBLE;
            } else if(format.type == Format::STRIKE) {
                // Strikethrough
                out << ANSI_STRIKE;
            } else if(format.type == Format::COLOR) {
                // Change text color
                out << ANSI_FOREGROUND_COLOR_START
                    << std::to_string(format.data[0])
                    << ANSI_ESCAPE_END;
            }
        } else {
            out << std::get<std::string>(piece);
        }
    }

    // Reset the formatting
    if(do_formatting) {
        out << ANSI_RESET;
    }

    out << "\n";

    return true;
}

bool MapNormalizer::Log::outputWithFormatting(const Message& message) {
    return outputToConsole(message, true, true);
}

bool MapNormalizer::Log::outputNoFormatting(const Message& message) {
    return outputToConsole(message, false, true);
}

