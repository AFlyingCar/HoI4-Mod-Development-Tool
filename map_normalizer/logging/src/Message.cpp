
#include "Message.h"

MapNormalizer::Log::Message::Message(const Level& debug_level,
                                     const PieceList& pieces,
                                     const Timestamp& timestamp,
                                     const Source& source):
    m_level(debug_level),
    m_pieces(pieces),
    m_timestamp(timestamp),
    m_source(source)
{ }

bool MapNormalizer::Log::Message::operator==(const Message& message) const {
    return m_level == message.m_level &&
           m_pieces == message.m_pieces &&
           m_timestamp == message.m_timestamp &&
           m_source == message.m_source;
}

bool MapNormalizer::Log::Message::operator!=(const Message& message) const {
    return !operator==(message);
}

auto MapNormalizer::Log::Message::getSource() const -> const Source& {
    return m_source;
}

auto MapNormalizer::Log::Message::getTimestamp() const -> const Timestamp& {
    return m_timestamp;
}

auto MapNormalizer::Log::Message::getDebugLevel() const -> const Level& {
    return m_level;
}

auto MapNormalizer::Log::Message::getPieces() const -> const PieceList& {
    return m_pieces;
}

std::ostream& operator<<(std::ostream& o,
                         const MapNormalizer::Log::Message::Level& level)
{
    return (o << std::to_string(level));
}

std::string std::to_string(const MapNormalizer::Log::Message::Level& level) {
    switch(level) {
        case MapNormalizer::Log::Message::Level::ERROR:
            return "ERR";
        case MapNormalizer::Log::Message::Level::WARN:
            return "WRN";
        case MapNormalizer::Log::Message::Level::STDOUT:
            return "OUT";
        case MapNormalizer::Log::Message::Level::DEBUG:
            return "DBG";
    }

    return "";
}

