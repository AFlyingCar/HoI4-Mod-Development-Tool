
#include "Message.h"

MapNormalizer::Log::Message::Message(const Level& debug_level,
                                     const std::vector<Piece>& pieces,
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

auto MapNormalizer::Log::Message::getPieces() const -> const std::vector<Piece>&
{
    return m_pieces;
}

