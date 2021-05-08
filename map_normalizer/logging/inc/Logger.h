/**
 * @file Logger.h
 *
 * @brief Defines all functions used for logging information.
 */

#ifndef LOGGER_H
# define LOGGER_H

# include <string>
# include <sstream>
# include <vector>
# include <optional>
# include <queue>
# include <mutex>
# include <chrono>

# include "Options.h"

namespace {
    std::stringstream s_log_builder;
}

namespace MapNormalizer { void init();

    void deleteInfoLine();
    void setInfoLineImpl(std::string&&);
    void writeErrorImpl(const std::string&, bool = true);
    void writeWarningImpl(const std::string&, bool = true);
    void writeStdoutImpl(const std::string&, bool = true);
    void writeDebugImpl(const std::string&, bool = true);

    namespace Log {
        enum class Destination {
            INFOLINE,
            STDOUT,
            DEBUG,
            ERROR,
            WARNING
        };

        class Message {
            public:
                Message(const std::string&, const Destination&, bool, bool);
                Message(const Message&);

                bool operator<(const Message&) const;
                bool operator>(const Message&) const;

                Destination getDestination() const;

                const std::string& getMessage() const;

                bool doWritePrefix() const;

                bool doAllowEmpty() const;

            private:
                std::chrono::time_point<std::chrono::system_clock> m_timestamp;

                std::string m_message;

                Destination m_destination;

                // Properties of the message
                bool m_write_prefix;
                bool m_allow_empty;
        };

        enum class OutputModType {
            NONE,
            WRITE_NO_PREFIX,
            ALLOW_EMPTY
        };

        struct OutputMod {
            OutputModType type;
        };

        namespace Mod {
            const OutputMod NONE = OutputMod{ OutputModType::NONE };
            const OutputMod WRITE_NO_PREFIX = OutputMod{ OutputModType::WRITE_NO_PREFIX };
            const OutputMod ALLOW_EMPTY = OutputMod{ OutputModType::ALLOW_EMPTY };
        }

        class Logger {
            public:
                static Logger& getInstance();

                void work(bool&);

                bool tryPushIntoQueue(const std::vector<Message>&);
                void pushIntoQueue(const std::vector<Message>&);

                // void pushInfoLine(const Message&);

            protected:
                void outputMessage(const Message&);

            private:
                Logger() = default;

                //! The type of m_queue
                using QueueType = std::priority_queue<Message, std::vector<Message>, std::greater<Message>>;

                //! A sorted list of messages to be outputted
                QueueType m_queue;

                //! A mutex used for locking m_queue while it is in use
                std::mutex m_mutex;

                //! The info lines
                std::queue<Message> m_info_lines;
        };

        class Writer {
            public:
                class Proxy {
                    public:
                        ~Proxy();

                        Proxy& operator<<(const OutputMod&);

                        template<typename T>
                        Proxy& operator<<(const T& data) {
                            m_stream << data;
                            return *this;
                        }

                        Message composeMessage() const;

                    private:
                        Proxy(Writer&);
                        Proxy(Proxy&&) = default;

                        std::stringstream m_stream;
                        Writer& m_writer;

                        bool m_write_prefix;
                        bool m_allow_empty;

                        friend Writer;
                };

                Writer(const Destination&);
                ~Writer();

                template<typename T>
                Proxy operator<<(const T& value) {
                    return std::move(Proxy(*this) << value);
                }

                void write(const Message&);

                void tryFlush();

                void flush();

                template<typename ToDuration>
                ToDuration timeSinceLastFlush() {
                    return std::chrono::duration_cast<ToDuration>(std::chrono::system_clock::now() - m_last_flush);
                }

                const Destination& getDestination() const;

            protected:
                void resetTimeSinceLastFlush();

            private:
                std::vector<Message> m_buffer;

                std::chrono::time_point<std::chrono::system_clock> m_last_flush;

                Destination m_destination;
        };

        Writer& getInfoLine();
        Writer& getStdout();
        Writer& getDebug();
        Writer& getError();
        Writer& getWarning();
    }

    template<typename... Args>
    void setInfoLine(const Args&... args) {
        // if(MapNormalizer::prog_opts.quiet) return;

        // s_log_builder.str(std::string{});
        // (s_log_builder << ... << args);
        // setInfoLineImpl(s_log_builder.str());

        (Log::getInfoLine() << ... << args);
    }

    template<bool WritePrefix = true, typename... Args>
    void writeError(const Args&... args) {
        // s_log_builder.str(std::string{});
        // (s_log_builder << ... << args);
        // writeErrorImpl(s_log_builder.str(), WritePrefix);

        auto proxy = Log::getError() << (!WritePrefix ? Log::Mod::WRITE_NO_PREFIX :
                                                        Log::Mod::NONE);

        (proxy << ... << args);
    }

    template<bool WritePrefix = true, typename... Args>
    void writeWarning(const Args&... args) {
        // s_log_builder.str(std::string{});
        // (s_log_builder << ... << args);
        // writeWarningImpl(s_log_builder.str(), WritePrefix);

        auto proxy = Log::getWarning() << (!WritePrefix ? Log::Mod::WRITE_NO_PREFIX :
                                                          Log::Mod::NONE);
        (proxy << ... << args);
    }

    template<bool WritePrefix = true, typename... Args>
    void writeStdout(const Args&... args) {
        if(prog_opts.quiet) return;

        // s_log_builder.str(std::string{});
        // (s_log_builder << ... << args);
        // writeStdoutImpl(s_log_builder.str(), WritePrefix);

        auto proxy = Log::getStdout() << (!WritePrefix ? Log::Mod::WRITE_NO_PREFIX :
                                                         Log::Mod::NONE);
        (proxy << ... << args);
    }

    template<bool WritePrefix = true, typename... Args>
    void writeDebug(const Args&... args) {
        if(!prog_opts.verbose) return;

        // s_log_builder.str(std::string{});
        // (s_log_builder << ... << args);
        // writeDebugImpl(s_log_builder.str(), WritePrefix);

        auto proxy = Log::getDebug() << (!WritePrefix ? Log::Mod::WRITE_NO_PREFIX :
                                                        Log::Mod::NONE);

        (proxy << ... << args);
    }
}

#endif

