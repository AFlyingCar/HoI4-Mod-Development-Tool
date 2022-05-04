
#include "ActionTests.h"

#include "gtest/gtest.h"

#include "IAction.h"
#include "ActionManager.h"

#include "SetPropertyAction.h"

namespace HMDT::UnitTests {
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

////////////////////////////////////////////////////////////////////////////////
    TEST_F(ActionTests, SetPropertyActionTests) {
        struct TestStructure {
            int a;
            char b;
            float c;
        };

        TestStructure s1{ 5, 'a', 3.1415f };

        // Verify that the action succeeds when we do it
        ASSERT_TRUE(Action::ActionManager::getInstance().doAction(
                    NewSetPropertyAction(&s1, a, 6)));

        // Verify contents of s1
        ASSERT_EQ(s1.a, 6);
        ASSERT_EQ(s1.b, 'a');
        ASSERT_EQ(s1.c, 3.1415f);

        // Verify that the action succeeds when we undo it
        ASSERT_TRUE(Action::ActionManager::getInstance().undoAction());

        // Verify contents of s1
        ASSERT_EQ(s1.a, 5);
        ASSERT_EQ(s1.b, 'a');
        ASSERT_EQ(s1.c, 3.1415f);

        // Verify that the action succeeds when we do multiple mods in a row
        ASSERT_TRUE(Action::ActionManager::getInstance().doAction(
                    NewSetPropertyAction(&s1, a, 6)));
        ASSERT_TRUE(Action::ActionManager::getInstance().doAction(
                    NewSetPropertyAction(&s1, a, 7)));
        ASSERT_TRUE(Action::ActionManager::getInstance().doAction(
                    NewSetPropertyAction(&s1, a, 1024)));
        ASSERT_TRUE(Action::ActionManager::getInstance().doAction(
                    NewSetPropertyAction(&s1, c, 2.718281f)));

        // Verify contents of s1
        ASSERT_EQ(s1.a, 1024);
        ASSERT_EQ(s1.b, 'a');
        ASSERT_EQ(s1.c, 2.718281f);

        // Verify that the action succeeds when we undo only a few mods
        ASSERT_TRUE(Action::ActionManager::getInstance().undoAction());
        ASSERT_TRUE(Action::ActionManager::getInstance().undoAction());

        // Verify contents of s1
        ASSERT_EQ(s1.a, 7);
        ASSERT_EQ(s1.b, 'a');
        ASSERT_EQ(s1.c, 3.1415f);
    }
}

