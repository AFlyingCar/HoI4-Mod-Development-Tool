
#include "Logger.h"

#include <chrono>
#include <future>

std::vector<MapNormalizer::Log::Logger::OutputFunction>
    MapNormalizer::Log::Logger::output_funcs{};

MapNormalizer::Log::Logger::~Logger() {
    destroyWorkerThread();
}

auto MapNormalizer::Log::Logger::getInstance() -> Logger& {
    static Logger logger;

    return logger;
}

auto MapNormalizer::Log::Logger::now() -> Timestamp {
    return std::chrono::system_clock::now();
}

std::string MapNormalizer::Log::Logger::getTimestampAsString(const Timestamp& timestamp,
                                                             const std::string& timestamp_format)
{
    auto timestamp_as_time_t = std::chrono::system_clock::to_time_t(timestamp);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timestamp_as_time_t), timestamp_format.c_str());
    return ss.str();
}

auto MapNormalizer::Log::Logger::getTimestampFromString(const std::string& time_str,
                                                         const std::string& timestamp_format)
    -> Timestamp
{
    std::istringstream ss{time_str};

    std::tm t = { };

    ss >> std::get_time(&t, timestamp_format.c_str());

    return std::chrono::system_clock::from_time_t(std::mktime(&t));
}

void MapNormalizer::Log::Logger::logMessage(const Message& message) {
    // TODO: Is there a way we can send data via signals rather than locking a mutex?
    m_messages_mutex.lock();

    m_messages.push_back(message);

    m_messages_mutex.unlock();
}

void MapNormalizer::Log::Logger::update() {
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
    m_messages.push_back(buildMessage(Message::Level::STDOUT, MN_LOG_SOURCE(),
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

MapNormalizer::Log::Logger::Logger(): m_quit(false),
                                      m_messages(),
                                      m_messages_mutex(),
                                      m_worker_thread(&Logger::update, this)
{ }

void MapNormalizer::Log::Logger::registerOutputFunction(const OutputFunction& output_func)
{
    output_funcs.push_back(output_func);
}

bool MapNormalizer::Log::Logger::started() const {
    return m_worker_thread.joinable();
}

void MapNormalizer::Log::Logger::destroyWorkerThread() {
    if(m_worker_thread.joinable()) {
        m_quit = true;

        m_worker_thread.join();
    }
}

void MapNormalizer::Log::Logger::reset() {
    destroyWorkerThread();

    output_funcs.clear();
    m_quit = false;
    m_messages.clear();
    m_worker_thread = std::thread(&Logger::update, this);
}

