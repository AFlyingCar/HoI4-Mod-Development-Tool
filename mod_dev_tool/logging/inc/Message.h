#ifndef HMDT_MESSAGE_H
# define HMDT_MESSAGE_H

# include <string>
# include <chrono>
# include <variant>
# include <vector>

# include "Source.h"
# include "Format.h"

namespace HMDT::Log {
    using Timestamp = std::chrono::time_point<std::chrono::system_clock>;

    /**
     * @brief A loggable message
     */
    class Message {
        public:

            //! A message piece can be either text or formatting.
            using Piece = std::variant<std::string, Format>;
            using PieceList = std::vector<Piece>;

            /**
             * @brief The debug level
             */
            enum class Level {
                INFO,  //!< Standard Information
                DEBUG, //!< Debugging Information
                ERROR, //!< Error messages
                WARN,  //!< Warning messages
            };

            Message(const Level&, const PieceList&, const Timestamp&,
                    const Source& = Source());

            bool operator==(const Message&) const;
            bool operator!=(const Message&) const;

            const Source& getSource() const;
            const Timestamp& getTimestamp() const;
            const Level& getDebugLevel() const;
            const PieceList& getPieces() const;

            std::string getTimestampAsString(const std::string& = "%Y-%m-%d %X") const;

        private:
            //! The debug level of the message
            Level m_level;

            //! The pieces of this message
            PieceList m_pieces;

            //! When the message was generated
            Timestamp m_timestamp;

            //! Where the message came from
            Source m_source;
    };

    /**
     * @brief Gets the default color for the given message level.
     *
     * @param level The message level
     *
     * @return The default 8-bit color code for the given level.
     */
    constexpr uint8_t getLevelDefaultColor(Message::Level level) {
        switch(level) {
            case Message::Level::INFO:
                return 15;
            case Message::Level::DEBUG:
                return 12;
            case Message::Level::ERROR:
                return 9;
            case Message::Level::WARN:
                return 11;
        }

        return 0;
    }

    std::ostream& operator<<(std::ostream&, const Message::Level&);
}

namespace std {
    string to_string(const HMDT::Log::Message::Level&);
}

#endif

