#ifndef MN_MESSAGE_H
# define MN_MESSAGE_H

# include <string>
# include <chrono>
# include <variant>
# include <vector>

# include "Source.h"
# include "Format.h"

namespace MapNormalizer::Log {
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
                STDOUT,
                DEBUG,
                ERROR,
                WARN,
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
}

std::ostream& operator<<(std::ostream&,
                         const MapNormalizer::Log::Message::Level&);

namespace std {
    string to_string(const MapNormalizer::Log::Message::Level&);
}

#endif

