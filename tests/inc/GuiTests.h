
#ifndef GUI_TESTS_H
# define GUI_TESTS_H

# include "gtest/gtest.h"

# include "glibmm/refptr.h"
# include "giomm/resource.h"
# include "gtkmm.h"

namespace HMDT::UnitTests {
    class GuiTests: public ::testing::Test {
        protected:
            void SetUp() override;
            void TearDown() override;

            Glib::RefPtr<Gio::Resource> m_resources;
            Glib::RefPtr<Gtk::Application> m_app;
    };
}

#endif

