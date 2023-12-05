#ifndef HMDT_LOGGER_H
# define HMDT_LOGGER_H

# include <chrono>
# include <deque>
# include <mutex>
# include <functional>
# include <thread>
# include <sstream>
# include <iomanip>
# include <condition_variable>

# include "Source.h"
# include "Message.h"
# include "Format.h"

namespace HMDT::Log {
    class Logger final {
        public:
            /**
             * @brief Type of output function
             */
            using OutputFunction = std::function<bool(const Message&)>;

            using UserData = std::shared_ptr<void>;
            using OutputFunctionWithUD = std::function<bool(const Message&, UserData)>;

            //! Amount of time to sleep between each update loop
            constexpr static std::chrono::seconds UPDATE_SLEEP_TIME{ 1 };

            ~Logger();

            static Logger& getInstance();

            static Timestamp now();

            static void registerOutputFunction(const OutputFunction&);
            static void registerOutputFunction(const OutputFunctionWithUD&,
                                               UserData);

            static std::string getTimestampAsString(const Timestamp&,
                                                    const std::string& = "%Y-%m-%d %X");

            static Timestamp getTimestampFromString(const std::string&,
                                                    const std::string& = "%Y-%m-%d %X");

            bool started() const;

            /**
             * @brief Writes a message to INFO
             *
             * @tparam Args All argument types to the message
             * @param source Where the message was created at
             * @param args All arguments to build the message
             */
            template<typename... Args>
            void writeInfo(const Source& source, Args&&... args) {
                logMessage(buildMessage(Message::Level::INFO, source, now(),
                                        std::forward<Args>(args)...));
            }

            /**
             * @brief Writes a message to ERROR
             *
             * @tparam Args All argument types to the message
             * @param source Where the message was created at
             * @param args All arguments to build the message
             */
            template<typename... Args>
            void writeError(const Source& source, Args&&... args) {
                logMessage(buildMessage(Message::Level::ERROR, source, now(),
                                        std::forward<Args>(args)...));
            }

            /**
             * @brief Writes a message to WARN
             *
             * @tparam Args All argument types to the message
             * @param source Where the message was created at
             * @param args All arguments to build the message
             */
            template<typename... Args>
            void writeWarn(const Source& source, Args&&... args) {
                logMessage(buildMessage(Message::Level::WARN, source, now(),
                                        std::forward<Args>(args)...));
            }

            /**
             * @brief Writes a message to DEBUG
             *
             * @tparam Args All argument types to the message
             * @param source Where the message was created at
             * @param args All arguments to build the message
             */
            template<typename... Args>
            void writeDebug(const Source& source, Args&&... args) {
                logMessage(buildMessage(Message::Level::DEBUG, source, now(),
                                        std::forward<Args>(args)...));
            }

            void logMessage(const Message&);

            // NOTE: DO NOT CALL THIS FUNCTION NORMALLY!
            //  It should only ever be used by the unit testing framework for
            //  testing
            void reset();

            void waitForLogger() const noexcept;

        private:
            /**
             * @brief Builds a Message object
             *
             * @tparam Args All argument types for the message
             * @param level The debug level of the message
             * @param source Where the message was created from
             * @param timestamp When the message was created
             * @param args All arguments to build the message
             *
             * @return A new message object
             */
            template<typename... Args>
            Message buildMessage(const Message::Level& level,
                                 const Source& source,
                                 const Timestamp& timestamp,
                                 Args&&... args)
            {
                return Message{
                    level,
                    std::vector<Message::Piece>{
                        buildPiece(args)...
                    },
                    timestamp,
                    source
                };
            }

            template<typename T>
            Message::Piece buildPiece(const T& arg) {
                if constexpr(std::is_same_v<T, Format>) {
                    return arg;
                } else {
                    std::stringstream ss;
                    // Perform all formattingg first
                    ss << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
                       << std::fixed << std::boolalpha
                       << arg // Output the argument
                       << std::noboolalpha; // Reset all formatting options
                    return ss.str();
                }
            }

            void destroyWorkerThread();

            void update();

            Logger();

            //! Whether the logging thread should quit
            bool m_quit;

            //! The queue of messages
            std::deque<Message> m_messages;

            //! The mutex for accessing m_messages
            std::mutex m_messages_mutex;

            //! The thread where update() gets called from
            std::thread m_worker_thread;

            /**
             * @brief Condition variable used to signal to other threads that
             *        Logger has finished processing a batch of messages.
             */
            mutable std::condition_variable m_wait_cv;

            //! Mutex for locking the condition
            mutable std::mutex m_wait_mutex;

            //! The list of output functions
            static std::vector<OutputFunction> output_funcs;
    };
}

# define WRITE_INFO(...) \
    HMDT::Log::Logger::getInstance().writeInfo(HMDT_LOG_SOURCE(), __VA_ARGS__)
# define WRITE_DEBUG(...) \
    HMDT::Log::Logger::getInstance().writeDebug(HMDT_LOG_SOURCE(), __VA_ARGS__)
# define WRITE_ERROR(...) \
    HMDT::Log::Logger::getInstance().writeError(HMDT_LOG_SOURCE(), __VA_ARGS__)
# define WRITE_WARN(...) \
    HMDT::Log::Logger::getInstance().writeWarn(HMDT_LOG_SOURCE(), __VA_ARGS__)

#endif

