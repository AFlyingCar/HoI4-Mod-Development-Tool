#ifndef APPLICATION_H
# define APPLICATION_H

# include <map>
# include <queue>
# include <string>
# include <memory>
# include <optional>
# include <functional>
# include <initializer_list>

# include "gtkmm.h"

# include "MainWindow.h"
# include "MenuItemWrapper.h"

namespace HMDT::Log {
    class Message;
}

namespace HMDT::GUI {
    class Application: public Gtk::Application {
        public:
            Application();
            virtual ~Application();

        protected:
            void on_activate() override;
            void on_startup() override;

            std::optional<std::reference_wrapper<MenuItemWrapper>> getMenuItem(const std::string&);
            std::optional<std::reference_wrapper<MenuItemInfo>> getMenuItemInfo(const std::string&);

            MenuItemWrapper& addMenuItem(const MenuItemInfo&);
            MenuItemWrapper& addMenuItem(const std::string&, const MenuItemInfo&);


            MenuWrapper createMenu(const std::string&,
                                   std::initializer_list<MenuItemInfo>);

            MenuWrapper createMenu(const std::string&, const std::string&,
                                   const std::vector<MenuItemInfo>&);

            Glib::RefPtr<Gtk::Settings> getSettings();

        private:
            std::unique_ptr<MainWindow> m_window;

            std::map<std::string, MenuItemWrapper> m_menu_items;
            std::map<std::string, MenuWrapper> m_menus;

            Glib::RefPtr<Gtk::Settings> m_settings;

            std::shared_ptr<bool> m_is_exiting;
    };
}

#endif

