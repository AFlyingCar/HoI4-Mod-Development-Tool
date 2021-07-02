
#include "gtest/gtest.h"

#include <sstream>

#include "NewLogger.h"
#include "Message.h"
#include "Format.h"

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

    // Just in case it has not yet been reset
    Logger::getInstance().reset();

    std::vector<Message> expected_messages {
        Message(Message::Level::STDOUT, { }, Logger::now()),
        Message(Message::Level::ERROR, { "message" }, Logger::now() + 1s),
    };

    std::vector<Message> received_messages;
    received_messages.reserve(expected_messages.size());

    Logger::registerOutputFunction([&received_messages](const Message& m) {
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
}

TEST(LogTests, LogMacroTests) {
    using MapNormalizer::Log::Logger;
    using MapNormalizer::Log::Message;
    using MapNormalizer::Log::Source;

    using namespace std::chrono_literals;
    using namespace std::string_literals;

    Logger::getInstance().reset();

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

    Logger::registerOutputFunction([&received_messages](const Message& m) -> bool
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
}

