
#include "Message.h"

#include <chrono>
#include <iomanip>
#include <sstream>

#include "Logger.h"

HMDT::Log::Message::Message(const Level& debug_level, const PieceList& pieces,
                            const Timestamp& timestamp, const Source& source):
    m_level(debug_level),
    m_pieces(pieces),
    m_timestamp(timestamp),
    m_source(source)
{ }

bool HMDT::Log::Message::operator==(const Message& message) const {
    return m_level == message.m_level &&
           m_pieces == message.m_pieces &&
           m_timestamp == message.m_timestamp &&
           m_source == message.m_source;
}

bool HMDT::Log::Message::operator!=(const Message& message) const {
    return !operator==(message);
}

auto HMDT::Log::Message::getSource() const -> const Source& {
    return m_source;
}

auto HMDT::Log::Message::getTimestamp() const -> const Timestamp& {
    return m_timestamp;
}

auto HMDT::Log::Message::getDebugLevel() const -> const Level& {
    return m_level;
}

auto HMDT::Log::Message::getPieces() const -> const PieceList& {
    return m_pieces;
}

std::string HMDT::Log::Message::getTimestampAsString(const std::string& timestamp_format) const
{
    return Logger::getTimestampAsString(m_timestamp, timestamp_format);
}

std::ostream& HMDT::Log::operator<<(std::ostream& o,
                                    const Message::Level& level)
{
    return (o << std::to_string(level));
}

std::string std::to_string(const HMDT::Log::Message::Level& level) {
    switch(level) {
        case HMDT::Log::Message::Level::ERROR:
            return "ERROR";
        case HMDT::Log::Message::Level::WARN:
            return "WARN";
        case HMDT::Log::Message::Level::INFO:
            return "INFO";
        case HMDT::Log::Message::Level::DEBUG:
            return "DEBUG";
    }

    return "";
}

