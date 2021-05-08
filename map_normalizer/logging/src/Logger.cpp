#include "Logger.h"

#include <iostream>
#include <vector>
#include <thread>
#include <stdio.h>

#include <unistd.h>

#include "Options.h"

static std::string info_line;

/**
 * @brief Checks if we can display ANSI escape codes through stdout.
 * @details Specifically, checks if the STDOUT file is a TTY.
 *
 * @return True if we can display ANSI through stdout, false otherwise.
 */
static bool isOutAnsiEnabled() {
    static bool ansi_enabled = isatty(fileno(stdout));

    return ansi_enabled;
}

/**
 * @brief Checks if we can display ANSI escape codes through stderr.
 * @details Specifically, checks if the STDERR file is a TTY.
 *
 * @return True if we can display ANSI through stderr, false otherwise.
 */
static bool isErrAnsiEnabled() {
    static bool ansi_enabled = isatty(fileno(stderr));

    return ansi_enabled;
}

/**
 * @brief Outputs the current information line.
 * @details Will do nothing if program output is set to QUIET
 */
static void writeInfoLine() {
    if(MapNormalizer::prog_opts.quiet) return;

    if(!info_line.empty()) {
        std::cout << (isOutAnsiEnabled() ? "\33[32m" : "") << "==> " << info_line
                  << (isOutAnsiEnabled() ? "\33[0m" : "")
                  << (!isOutAnsiEnabled() ? "\n" : "");
    }
}

/**
 * @brief Initializes the logging system
 */
void MapNormalizer::init() {
    // https://stackoverflow.com/a/61854612
    std::ios_base::sync_with_stdio(false);
}

/**
 * @brief Deletes the last information line from the console, so that we can display the next one.
 * @details Will do nothing if program output is set to QUIET or if ANSI escape
 *          codes are not enabled for STDOUT.
 */
void MapNormalizer::deleteInfoLine() {
    if(prog_opts.quiet) return;

    if(!info_line.empty() && isOutAnsiEnabled()) {
        std::cout << "\33[1000D"  // Go to start of line
                     "\33[0K"     // Clear the line
                     "\33[1000D"; // Go to start of line
    }
}

/**
 * @brief Sets and displays the next information line.
 *
 * @param line The next textual string to use for the information line.
 */
void MapNormalizer::setInfoLineImpl(std::string&& line) {
    if(MapNormalizer::prog_opts.quiet) return;

    info_line = std::move(line);

    if(!info_line.empty() && isOutAnsiEnabled()) {
        std::cout << "\33[1000D" // Go to start of line
                     "\33[0K"    // Clear the line
                     "\33[1000D" // Go to start of line
                     "\33[32m==> " << info_line; // Write the line
    } else {
        std::cout << info_line << "\n";
    }
}

/**
 * @brief Writes a warning.
 * @details Outputs to stderr. If requested, prefix is "[WRN] ~ ".
 *
 * @param message The message to write.
 * @param write_prefix Whether a prefix should be attached to this message.
 */
void MapNormalizer::writeWarningImpl(const std::string& message,
                                     bool write_prefix)
{
    deleteInfoLine();
    std::cerr << (isErrAnsiEnabled() ? "\33[33m" : "")
              << (write_prefix ? "[WRN] ~ " : "") << message
              << (isErrAnsiEnabled() ? "\33[0m" : "") << std::endl;
    writeInfoLine();
}

/**
 * @brief Writes an error.
 * @details Outputs to stderr. If requested, prefix is "[ERR] ~ ".
 *
 * @param message The message to write.
 * @param write_prefix Whether a prefix should be attached to this message.
 */
void MapNormalizer::writeErrorImpl(const std::string& message,
                                   bool write_prefix)
{
    deleteInfoLine();
    std::cerr << (isErrAnsiEnabled() ? "\33[31m" : "")
              << (write_prefix ? "[ERR] ~ " : "") << message
              << (isErrAnsiEnabled() ? "\33[0m" : "") << std::endl;
    writeInfoLine();
}

/**
 * @brief Writes a output message.
 * @details Outputs to stdout. If requested, prefix is "[OUT] ~ ".
 *
 * @param message The message to write.
 * @param write_prefix Whether a prefix should be attached to this message.
 */
void MapNormalizer::writeStdoutImpl(const std::string& message,
                                    bool write_prefix)
{
    if(prog_opts.quiet) return;

    deleteInfoLine();
    std::cout << (isOutAnsiEnabled() ? "\33[37m" : "")
              << (write_prefix ? "[OUT] ~ " : "") << message
              << (isOutAnsiEnabled() ? "\33[0m" : "") << std::endl;
    writeInfoLine();
}

/**
 * @brief Writes a debug message.
 * @details Outputs to stdout. If requested, prefix is "[DBG] ~ ".
 *
 * @param message The message to write.
 * @param write_prefix Whether a prefix should be attached to this message.
 */
void MapNormalizer::writeDebugImpl(const std::string& message,
                                   bool write_prefix)
{
    // Do nothing if we don't have verbose output.
    if(!prog_opts.verbose) return;

    deleteInfoLine();
    std::cout << (isOutAnsiEnabled() ? "\33[34m" : "")
              << (write_prefix ? "[DBG] ~ " : "") << message
              << (isOutAnsiEnabled() ? "\33[0m" : "") << std::endl;
    writeInfoLine();
}

////////////////////////////////////////////////////////////////////////////////

MapNormalizer::Log::Message::Message(const std::string& message,
                                     const Destination& destination,
                                     bool write_prefix,
                                     bool allow_empty):
    m_timestamp(std::chrono::system_clock::now()),
    m_message(message),
    m_destination(destination),
    m_write_prefix(write_prefix),
    m_allow_empty(allow_empty)
{ }

MapNormalizer::Log::Message::Message(const Message& other):
    m_timestamp(other.m_timestamp),
    m_message(other.m_message),
    m_destination(other.m_destination),
    m_write_prefix(other.m_write_prefix),
    m_allow_empty(other.m_allow_empty)
{ }

bool MapNormalizer::Log::Message::operator<(const Message& other) const {
    return m_timestamp < other.m_timestamp;
}

bool MapNormalizer::Log::Message::operator>(const Message& other) const {
    return m_timestamp > other.m_timestamp;
}

auto MapNormalizer::Log::Message::getDestination() const -> Destination {
    return m_destination;
}

const std::string& MapNormalizer::Log::Message::getMessage() const {
    return m_message;
}

bool MapNormalizer::Log::Message::doWritePrefix() const {
    return m_write_prefix;
}

bool MapNormalizer::Log::Message::doAllowEmpty() const {
    return m_allow_empty;
}

/**
 * @brief Gets the Logger instance
 *
 * @return The Logger instance
 */
MapNormalizer::Log::Logger& MapNormalizer::Log::Logger::getInstance() {
    static Logger logger;

    return logger;
}

/**
 * @brief Will process logged messages and occassionally output them
 *
 * @param quit A boolean that informs this thread when it is time to quit
 */
void MapNormalizer::Log::Logger::work(bool& quit) {
    using namespace std::chrono_literals;

    while(!quit) {
        QueueType queue;

        // Swap our empty temporary and the held queue, so that we can begin
        //  processing the messages while other threads begin pushing more
        //  into the next queue
        {
            std::lock_guard l(m_mutex);
            queue.swap(m_queue);
        }

        // If there are no messages in the queue, let them build up before we
        //  try gathering the next batch
        if(queue.empty()) {
            // TODO: We should have the ability to immediately awaken if we need to
            std::this_thread::sleep_for(0.5s);
        } else {
            // Output every message in the queue
            while(!queue.empty()) {
                outputMessage(queue.top());
                queue.pop();
            }
        }
    }

    // Dump any remaining messages before exiting this function
    QueueType queue;

    {
        // Though, we still do the lock just in case
        std::lock_guard l(m_mutex);
        queue.swap(m_queue);
    }

    // Output every message in the queue
    while(!queue.empty()) {
        outputMessage(queue.top());
        queue.pop();
    }
}

/**
 * @brief Will output a given messagge
 *
 * @param message The message to output
 */
void MapNormalizer::Log::Logger::outputMessage(const Message& message) {
    // Skip empty messages if they are not marked with ALLOW_EMPTY
    if(message.getMessage().empty() && !message.doAllowEmpty()) {
        return;
    }

    // TODO: We should have a way for these messages to go to multiple places
    switch(message.getDestination()) {
        case Destination::STDOUT:
            writeStdoutImpl(message.getMessage(), message.doWritePrefix());
            break;
        case Destination::DEBUG:
            writeDebugImpl(message.getMessage(), message.doWritePrefix());
            break;
        case Destination::ERROR:
            writeErrorImpl(message.getMessage(), message.doWritePrefix());
            break;
        case Destination::WARNING:
            writeWarningImpl(message.getMessage(), message.doWritePrefix());
            break;
        case Destination::INFOLINE:
            // TODO: How do we want to deal with info lines?
            break;
    }
}

/**
 * @brief Tries to push a buffer of messages into the message queue.
 * @details Will only perform the push if the mutex lock is able to be acquired
 *
 * @param buffer The messages to push into the queue
 *
 * @return True if the buffer was successfully pushed, false otherwise
 */
bool MapNormalizer::Log::Logger::tryPushIntoQueue(const std::vector<Message>& buffer)
{
    if(!m_mutex.try_lock()) {
        return false;
    }

    for(auto&& message : buffer) {
        m_queue.push(message);
    }

    m_mutex.unlock();

    return true;
}

/**
 * @brief Pushes a buffer of messages into the message queue.
 *
 * @param buffer The messages to push into the queue
 */
void MapNormalizer::Log::Logger::pushIntoQueue(const std::vector<Message>& buffer)
{
    m_mutex.lock();

    for(auto&& message : buffer) {
        m_queue.push(message);
    }

    m_mutex.unlock();
}

/**
 * @brief Constructs a Proxy
 *
 * @param writer The writer this proxy belongs to
 */
MapNormalizer::Log::Writer::Proxy::Proxy(Writer& writer):
    m_writer(writer),
    m_write_prefix(true),
    m_allow_empty(false)
{ }

/**
 * @brief Will write The held stream to m_writer as a new message
 */
MapNormalizer::Log::Writer::Proxy::~Proxy() {
    m_writer.write(composeMessage());
}

auto MapNormalizer::Log::Writer::Proxy::composeMessage() const -> Message {
    return Message(m_stream.str(), m_writer.getDestination(), m_write_prefix, m_allow_empty);
}

/**
 * @brief Modifies the output for this message
 *
 * @param mod The modification to apply to the output
 */
auto MapNormalizer::Log::Writer::Proxy::operator<<(const OutputMod& mod)
    -> Proxy&
{
    switch(mod.type) {
        case OutputModType::WRITE_NO_PREFIX:
            m_write_prefix = false;
            break;
        case OutputModType::ALLOW_EMPTY:
            m_allow_empty = true;
            break;
        case OutputModType::NONE:
            break;
    }

    return *this;
}

/**
 * @brief Constructs a Writer
 *
 * @param destination Where this Writer sends output to
 */
MapNormalizer::Log::Writer::Writer(const Destination& destination):
    m_buffer(),
    m_last_flush(std::chrono::system_clock::now()),
    m_destination(destination)
{
}

/**
 * @brief Destroys this Writer, and flushes any remaining messages it may have
 */
MapNormalizer::Log::Writer::~Writer() {
    flush();
}

/**
 * @brief Puts a single message into this Writer's buffer. May try to flush as
 *        well.
 *
 * @param message The message to output
 */
void MapNormalizer::Log::Writer::write(const Message& message) {
    m_buffer.push_back(message);

    tryFlush();
}

/**
 * @brief Will attempt to flush the buffer of messages.
 *
 * @par The buffer will try to flush data if: The buffer has more than
 *      MAX_BUFFER_LENGTH_BEFORE_TRY_FLUSH  messages or if it has been
 *      longer than MAX_TIME_BEFORE_TRY_FLUSH since the last flush.
 */
void MapNormalizer::Log::Writer::tryFlush() {
    using namespace std::chrono_literals; // TODO: Put this in Constants.h
#if 0
#define MAX_BUFFER_LENGTH_BEFORE_TRY_FLUSH 1 // TODO
#define MAX_TIME_BEFORE_TRY_FLUSH 5s

    if(m_buffer.size() > MAX_BUFFER_LENGTH_BEFORE_TRY_FLUSH ||
       timeSinceLastFlush<std::chrono::seconds>() > MAX_TIME_BEFORE_TRY_FLUSH)
#endif
    {
        if(Logger::getInstance().tryPushIntoQueue(m_buffer)) {
            m_buffer.clear();
            resetTimeSinceLastFlush();
        }
    }
}

/**
 * @brief Will flush the buffer of messages.
 *
 * @details Note that unlike tryFlush, this method will wait until a lock is
 *          able to be acquired for the Logger.
 */
void MapNormalizer::Log::Writer::flush() {
    Logger::getInstance().pushIntoQueue(m_buffer);
    m_buffer.clear();
    resetTimeSinceLastFlush();
}

auto MapNormalizer::Log::Writer::getDestination() const -> const Destination& {
    return m_destination;
}

/**
 * @brief Will reset the time since the last flush to the current time.
 */
void MapNormalizer::Log::Writer::resetTimeSinceLastFlush() {
    m_last_flush = std::chrono::system_clock::now();
}

/**
 * @brief Gets a Writer for stdout for the current thread.
 *
 * @return A writer for stdout
 */
MapNormalizer::Log::Writer& MapNormalizer::Log::getInfoLine() {
    static thread_local Writer infoline_writer(Destination::INFOLINE);

    return infoline_writer;
}

/**
 * @brief Gets a Writer for stdout for the current thread.
 *
 * @return A writer for stdout
 */
MapNormalizer::Log::Writer& MapNormalizer::Log::getStdout() {
    static thread_local Writer stdout_writer(Destination::STDOUT);

    return stdout_writer;
}

/**
 * @brief Gets a Writer for debug for the current thread.
 *
 * @return A writer for debug
 */
MapNormalizer::Log::Writer& MapNormalizer::Log::getDebug() {
    static thread_local Writer debug_writer(Destination::DEBUG);

    return debug_writer;
}

/**
 * @brief Gets a Writer for error for the current thread.
 *
 * @return A writer for error
 */
MapNormalizer::Log::Writer& MapNormalizer::Log::getError() {
    static thread_local Writer error_writer(Destination::ERROR);

    return error_writer;
}

/**
 * @brief Gets a Writer for warning for the current thread.
 *
 * @return A writer for warning
 */
MapNormalizer::Log::Writer& MapNormalizer::Log::getWarning() {
    static thread_local Writer warning_writer(Destination::WARNING);

    return warning_writer;
}

