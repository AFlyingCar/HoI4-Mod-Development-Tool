
#include "ActionTests.h"

#include "gtest/gtest.h"

#include "IAction.h"
#include "ActionManager.h"

namespace MapNormalizer::UnitTests {
    void ActionTests::SetUp() { }

    void ActionTests::TearDown() {
        // Make sure we clear the history at the end of each test
        Action::ActionManager::getInstance().clearHistory();
    }

    class TestAction: public Action::IAction {
        public:
            TestAction(int data): m_data(data) {
            }

            bool doAction(const Callback& callback = Action::IAction::_)
            {
                return callback(0) &&
                       callback(1) &&
                       callback(2) &&
                       callback(3);
            }

            bool undoAction(const Callback& callback = Action::IAction::_)
            {
                return callback(0) &&
                       callback(1) &&
                       callback(2) &&
                       callback(3);
            }

            int getData() const { return m_data; }

        private:
            int m_data;
    };

    class NonUndoableAction: public TestAction {
        public:
            using TestAction::TestAction;

            virtual bool canBeUndone() const { return false; }
    };

    TEST_F(ActionTests, DoActionTest) {
        uint32_t expected_state = 0;

        ASSERT_TRUE(Action::ActionManager::getInstance().doAction(new TestAction(5),
                    [&expected_state](uint32_t state) -> bool {
                        EXPECT_EQ(state, expected_state);

                        auto result = state == expected_state;
                        ++expected_state;
                        return result;
                    }));
    }

    TEST_F(ActionTests, UndoRedoActionTest) {
        uint32_t expected_state = 0;

        ASSERT_TRUE(Action::ActionManager::getInstance().doAction(new TestAction(5),
                    [&expected_state](uint32_t state) -> bool {
                        EXPECT_EQ(state, expected_state);

                        auto result = state == expected_state;
                        ++expected_state;
                        return result;
                    }));
        ASSERT_TRUE(Action::ActionManager::getInstance().canUndo());
        ASSERT_FALSE(Action::ActionManager::getInstance().canRedo());

        expected_state = 0;
        ASSERT_TRUE(Action::ActionManager::getInstance().undoAction(
                    [&expected_state](uint32_t state) -> bool {
                        EXPECT_EQ(state, expected_state);

                        auto result = state == expected_state;
                        ++expected_state;
                        return result;
                    }));
        ASSERT_FALSE(Action::ActionManager::getInstance().canUndo());
        ASSERT_TRUE(Action::ActionManager::getInstance().canRedo());

        expected_state = 0;
        ASSERT_TRUE(Action::ActionManager::getInstance().redoAction(
                    [&expected_state](uint32_t state) -> bool {
                        EXPECT_EQ(state, expected_state);

                        auto result = state == expected_state;
                        ++expected_state;
                        return result;
                    }));
        ASSERT_TRUE(Action::ActionManager::getInstance().canUndo());
        ASSERT_FALSE(Action::ActionManager::getInstance().canRedo());
    }

    TEST_F(ActionTests, NonUndoableTest) {
        uint32_t expected_state = 0;

        ASSERT_TRUE(Action::ActionManager::getInstance().doAction(new NonUndoableAction(5),
                    [&expected_state](uint32_t state) -> bool {
                        EXPECT_EQ(state, expected_state);

                        auto result = state == expected_state;
                        ++expected_state;
                        return result;
                    }));
        ASSERT_FALSE(Action::ActionManager::getInstance().canUndo());
    }

    TEST_F(ActionTests, clearHistoryTest) {
        ASSERT_TRUE(Action::ActionManager::getInstance().doAction(new TestAction(5)));
        ASSERT_TRUE(Action::ActionManager::getInstance().doAction(new TestAction(5)));
        ASSERT_TRUE(Action::ActionManager::getInstance().doAction(new TestAction(5)));
        ASSERT_TRUE(Action::ActionManager::getInstance().doAction(new TestAction(5)));
        ASSERT_TRUE(Action::ActionManager::getInstance().doAction(new TestAction(5)));

        Action::ActionManager::getInstance().clearHistory();

        ASSERT_FALSE(Action::ActionManager::getInstance().canUndo());
    }
}

