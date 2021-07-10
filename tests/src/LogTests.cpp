
#include "gtest/gtest.h"

#include <sstream>

#include "NewLogger.h"
#include "Message.h"
#include "Format.h"
#include "ConsoleOutputFunctions.h"

#include "TestOverrides.h"
#include "TestUtils.h"

TEST(LogTests, OutputFunctionRegistrationTest) {
    using MapNormalizer::Log::Logger;
    using MapNormalizer::Log::Message;

    Logger::registerOutputFunction([](const Message&) { return true; });
}

TEST(LogTests, SimpleFormatTests) {
    using MapNormalizer::Log::Format;

    Format f1{ Format::CLEAR, { 0 } };
    Format f2{ Format::CLEAR, { 0 } };

    ASSERT_EQ(f1, f2);
}

TEST(LogTests, SimpleLogWriteTest) {
    using MapNormalizer::Log::Logger;
    using MapNormalizer::Log::Message;
    using MapNormalizer::Log::Source;

    using namespace std::chrono_literals;

    // Make sure that the logger is reset, and that we reset it when we are done
    Logger::getInstance().reset();
    RUN_AT_SCOPE_END([]() { Logger::getInstance().reset(); });

    std::vector<Message> expected_messages {
        Message(Message::Level::STDOUT, { }, Logger::now()),
        Message(Message::Level::ERROR, { "message" }, Logger::now() + 1s),
    };

    std::vector<Message> received_messages;
    received_messages.reserve(expected_messages.size());

    Logger::registerOutputFunction([&received_messages](const Message& m)
    {
        received_messages.push_back(m);
        return true;
    });

    for(auto&& message : expected_messages) {
        Logger::getInstance().logMessage(message);
    }

    ASSERT_TRUE(Logger::getInstance().started());

    // Sleep for a bit while we wait for the logger to finish its tasks
    // Normally we don't actually need to worry about _when_ the logs get
    //  processed, but we do for the unit test
    std::this_thread::sleep_for(2.5s);

    ASSERT_EQ(received_messages.size(), expected_messages.size());

    auto messages_checker = [&received_messages, &expected_messages]() -> bool {
        for(uint32_t i = 0; i < received_messages.size(); ++i) {
            const auto& recv_message = received_messages.at(i);
            const auto& expc_message = expected_messages.at(i);
            if(recv_message != expc_message) {
                auto recv_time_t = std::chrono::system_clock::to_time_t(recv_message.getTimestamp());
                auto expc_time_t = std::chrono::system_clock::to_time_t(expc_message.getTimestamp());

                TEST_CERR << std::put_time(std::localtime(&recv_time_t), "%Y-%m-%d %X")
                          << "!="
                          << std::put_time(std::localtime(&expc_time_t), "%Y-%m-%d %X")
                          << std::endl;
                TEST_CERR << (int)recv_message.getDebugLevel() << "!=" << (int)expc_message.getDebugLevel() << std::endl;
                // TEST_CERR << recv_message.getPieces() << "!=" << expc_message.getPieces() << std::endl;

                return false;
            }
        }

        return true;
    };

    ASSERT_TRUE(messages_checker());

    Logger::getInstance().reset();
}

TEST(LogTests, LogMacroTests) {
    using MapNormalizer::Log::Logger;
    using MapNormalizer::Log::Message;
    using MapNormalizer::Log::Source;

    using namespace std::chrono_literals;
    using namespace std::string_literals;

    // Make sure that the logger is reset, and that we reset it when we are done
    Logger::getInstance().reset();
    RUN_AT_SCOPE_END([]() { Logger::getInstance().reset(); });

    auto doubleToPreciseString = [](long double d) -> std::string {
        std::stringstream ss;

        ss << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
           << std::fixed << d;
        return ss.str();
    };

    std::vector<std::string> expected_messages {
        "message",                                                     // "message"
        "helloworld!",                                                 // "hello", "world!"
        "truefalse",                                                   // true, false
        "123456",                                                      // 123456
        doubleToPreciseString(3.14156789),                             // 3.14156789
        "a",                                                           // 'a'
        "foobar56"s + doubleToPreciseString(3.15167) + "trueairplanea" // "foobar", 5, 6, 3.15167, true, "airplane", 'a'
    };

    std::vector<std::string> received_messages;
    received_messages.reserve(expected_messages.size());

    Logger::registerOutputFunction([&received_messages](const Message& m)
        -> bool
    {
        EXPECT_EQ(m.getDebugLevel(), Message::Level::STDOUT);

        std::stringstream ss;
        for(auto&& piece : m.getPieces()) {
            if(std::holds_alternative<std::string>(piece)) {
                ss << std::get<std::string>(piece);
            }
        }
        received_messages.push_back(ss.str());

        return true;
    });

    WRITE_STDOUT("message");
    WRITE_STDOUT("hello", "world!");
    WRITE_STDOUT(true, false);
    WRITE_STDOUT(123456);
    WRITE_STDOUT(3.14156789);
    WRITE_STDOUT('a');
    WRITE_STDOUT("foobar", 5, 6, 3.15167, true, "airplane", 'a');

    ASSERT_TRUE(Logger::getInstance().started());

    // Sleep for a bit while we wait for the logger to finish its tasks
    // Normally we don't actually need to worry about _when_ the logs get
    //  processed, but we do for the unit test
    std::this_thread::sleep_for(2.5s);

    ASSERT_EQ(received_messages, expected_messages);

    Logger::getInstance().reset();
}

TEST(LogTests, ANSIOutputTests) {
    using MapNormalizer::Log::Logger;
    using MapNormalizer::Log::Message;
    using MapNormalizer::Log::Source;

    using namespace std::chrono_literals;
    using namespace std::string_literals;

    // Make sure that the logger is reset, and that we reset it when we are done
    Logger::getInstance().reset();
    RUN_AT_SCOPE_END([]() { Logger::getInstance().reset(); });

#define FMT_STDOUT_PREFIX "\33[38;5;15m[OUT] ~ "
#define FMT_DEBUG_PREFIX "\33[38;5;9m[DBG] ~ "
#define FMT_ERROR_PREFIX "\33[38;5;12m[ERR] ~ "
#define FMT_WARN_PREFIX "\33[38;5;11m[WRN] ~ "

#define FMT_SUFFIX "\33[0m\n"

    std::string expected_stdout =
        // "hello world", 1500, 'a'
        FMT_STDOUT_PREFIX "hello world1500a" FMT_SUFFIX
        // "airplane", FORMAT(FBOLD), 'a', FORMAT(FRESET), "foobar"
        FMT_STDOUT_PREFIX "airplane\33[1ma\33[0m\33[38;5;15mfoobar" FMT_SUFFIX
        // "white", FORMAT(FCOLOR(204)), "salmon", FORMAT(FRESET), "white again"
        FMT_STDOUT_PREFIX "white\33[38;5;204msalmon\33[0m\33[38;5;15mwhite again" FMT_SUFFIX
        ;
    // TODO: We should also test every individual formatting operation

    std::stringstream stdout_ss;
    // TODO: We should also do some minimal testing for the other 3 levels.

    Logger::registerOutputFunction([&stdout_ss](const Message& m)
    {
        return MapNormalizer::Log::outputToStream(m, true, true,
            [&stdout_ss](uint8_t debug_level) -> std::ostream& {
                switch(static_cast<Message::Level>(debug_level)) {
                    case Message::Level::STDOUT:
                        return stdout_ss;
#if 0
                    case Message::Level::DEBUG:
                        return debug_ss;
                    case Message::Level::ERROR:
                        return error_ss;
                    case Message::Level::WARN:
                        return warn_ss;
#else
                    default:
                        return stdout_ss;
#endif
                }
            });
    });

    // Write all of the messages
    WRITE_STDOUT("hello world", 1500, 'a');
    WRITE_STDOUT("airplane", FORMAT(FBOLD), 'a', FORMAT(FRESET), "foobar");
    WRITE_STDOUT("white", FORMAT(FCOLOR(204)), "salmon", FORMAT(FRESET), "white again");

    ASSERT_TRUE(Logger::getInstance().started());

    // Sleep for a bit while we wait for the logger to finish its tasks
    // Normally we don't actually need to worry about _when_ the logs get
    //  processed, but we do for the unit test
    std::this_thread::sleep_for(2.5s);

    std::cout << stdout_ss.str();

    ASSERT_EQ(stdout_ss.str(), expected_stdout);

    Logger::getInstance().reset();
}

