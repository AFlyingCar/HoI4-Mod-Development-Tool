
#include "Logger.h"

#include <chrono>
#include <future>
#include <iostream>

//! The vector of all output functions
std::vector<HMDT::Log::Logger::OutputFunction> HMDT::Log::Logger::output_funcs{};

/**
 * Destroys the logger, and shuts down the logging worker thread
 */
HMDT::Log::Logger::~Logger() {
    destroyWorkerThread();
}

/**
 * @brief Gets the logger instance
 */
auto HMDT::Log::Logger::getInstance() -> Logger& {
    static Logger logger;

    return logger;
}

/**
 * @brief Gets the system time
 *
 * @return The system time
 */
auto HMDT::Log::Logger::now() -> Timestamp {
    return std::chrono::system_clock::now();
}

/**
 * @brief Converts a given Timestamp into a string
 *
 * @param timestamp The timestamp
 * @param timestamp_format The format to convert the Timestamp into
 *
 * @return A string representation of timestamp in timestamp_format format.
 */
std::string HMDT::Log::Logger::getTimestampAsString(const Timestamp& timestamp,
                                                    const std::string& timestamp_format)
{
    auto timestamp_as_time_t = std::chrono::system_clock::to_time_t(timestamp);
    std::stringstream ss;

    auto tm = std::localtime(&timestamp_as_time_t);
    // Make sure we enforce daylight savings on both the serialized and deserialized sides
    tm->tm_isdst = 1;

    ss << std::put_time(tm, timestamp_format.c_str());
    return ss.str();
}

/**
 * @brief Converts a string representation of a timestamp into a Timestamp
 *
 * @param time_str The string representation of a timestamp
 * @param timestamp_format The format of the string
 *
 * @return A Timestamp object
 */
auto HMDT::Log::Logger::getTimestampFromString(const std::string& time_str,
                                               const std::string& timestamp_format)
    -> Timestamp
{
    // https://en.cppreference.com/w/cpp/io/manip/get_time
    std::istringstream ss{time_str};

    std::tm t = { };

    ss >> std::get_time(&t, timestamp_format.c_str());

    if(!time_str.empty() && ss.fail()) {
        static std::string last_failed_timestamp;
        static std::string last_failed_format;

        if(last_failed_timestamp != time_str ||
           last_failed_format != timestamp_format)
        {
            // TODO: This is a bit spammy. We should instead render text to the
            //  gui or something. But for now this will have to do.
            std::cerr << "Failed to parse time '" << time_str << "' using format '" << timestamp_format << "'" << std::endl;
        }

        last_failed_timestamp = time_str;
        last_failed_format = timestamp_format;
    }

    // Make sure we enforce daylight savings on the serialized and deserialized sides
    t.tm_isdst = 1;

    return std::chrono::system_clock::from_time_t(std::mktime(&t));
}

void HMDT::Log::Logger::logMessage(const Message& message) {
    // TODO: Is there a way we can send data via signals rather than locking a mutex?
    m_messages_mutex.lock();

    m_messages.push_back(message);

    m_messages_mutex.unlock();
}

/**
 * @brief Updates the logger
 */
void HMDT::Log::Logger::update() {
    using namespace std::chrono_literals;

    std::vector<Message> tempMessages;
    std::vector<std::future<bool>> results;

    // How long the last update took
    std::chrono::seconds update_time{0};

    while(!m_quit) {
        // Each update should take UPDATE_SLEEP_TIME at least, so if we didn't
        //  need that much time on the last loop, go ahead and sleep the
        //  remaining time off so that the rest of the codebase can have time to
        //  output messages into the queue before we lock it again
        if(update_time < UPDATE_SLEEP_TIME)
            std::this_thread::sleep_for(UPDATE_SLEEP_TIME - update_time);

        auto curr_time = now();
        {
            m_messages_mutex.lock();

            tempMessages.insert(tempMessages.begin(), m_messages.begin(), m_messages.end());
            m_messages.clear();

            m_messages_mutex.unlock();
        }

        for(auto&& message : tempMessages) {
            results.reserve(output_funcs.size());

            for(auto&& output_func : output_funcs) {
                results.push_back(std::async(std::launch::async, output_func, message));
            }

            bool result = std::all_of(results.begin(), results.end(),
                                      [](auto&& fut) {
                                            return fut.get();
                                      });
            if(!result) {
                std::fprintf(stderr, "One or more output functions failed! Result=%d", result);
            }

            results.clear();
        }

        tempMessages.clear();

        // Calculate how long this last update call took
        update_time = std::chrono::duration_cast<std::chrono::seconds>(now() - curr_time);
    }

    // Push a new output message to log what we are doing
    // We aren't using logMessage or any of the macros because we don't want to
    //  worry about locking the mutex
    m_messages.push_back(buildMessage(Message::Level::INFO, MN_LOG_SOURCE(),
                                      now(), "Outputting all remaining messages."
    ));

    // Do not bother locking the mutexes, since m_quit will only be true if the
    //  whole program is shutting down, so no new messages should be getting
    //  pushed into the queue. We want to just clear out any remaining messages
    //  in the queue and then terminate the thread
    if(!m_messages.empty()) {
        for(auto&& message : m_messages) {
            uint32_t i = 0;
            for(auto&& output_func : output_funcs) {
                // Do not bother async-ing these, just output them and be done with
                //  it
                if(!output_func(message)) {
                    std::fprintf(stderr, "Output function #%d failed!", i);
                }
                ++i;
            }
        }
    }
}

HMDT::Log::Logger::Logger(): m_quit(false), m_messages(), m_messages_mutex(),
                             m_worker_thread(&Logger::update, this)
{ }

/**
 * @brief Registers a single output function
 *
 * @param output_func The function which defines how to output a message
 */
void HMDT::Log::Logger::registerOutputFunction(const OutputFunction& output_func)
{
    output_funcs.push_back(output_func);
}

/**
 * @brief Registers a single output function with user data
 *
 * @param output_func The funcction which defines how to output a message
 * @param user_data The user data to bind to this function
 */
void HMDT::Log::Logger::registerOutputFunction(const OutputFunctionWithUD& output_func,
                                               UserData user_data)
{
    if(user_data == nullptr) {
        WRITE_WARN("Given UserData function is null! This means that likely the"
                   " output function will not have access to the eventual "
                   "pointer due to the way shared_ptr works.");
    }

    output_funcs.push_back(std::bind(output_func, std::placeholders::_1, user_data));
}

/**
 * @brief Gets if the logger has been started
 *
 * @return true if the logging worker thread has been started, false otherwise
 */
bool HMDT::Log::Logger::started() const {
    return m_worker_thread.joinable();
}

/**
 * @brief Shuts down the logging worker thread
 */
void HMDT::Log::Logger::destroyWorkerThread() {
    // Only try to shut down the worker thread if it is actually running
    if(m_worker_thread.joinable()) {
        m_quit = true;

        m_worker_thread.join();
    }
}

// No documentation on this one, nobody should be calling it outside of unit
//  tests
void HMDT::Log::Logger::reset() {
    destroyWorkerThread();

    output_funcs.clear();
    m_quit = false;
    m_messages.clear();
    m_worker_thread = std::thread(&Logger::update, this);
}

