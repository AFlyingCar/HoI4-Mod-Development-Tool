#ifndef MAPNORMALIZER_APPLICATION_H
# define MAPNORMALIZER_APPLICATION_H

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

namespace MapNormalizer::Log {
    class Message;
}

namespace MapNormalizer::GUI {
    class MapNormalizerApplication: public Gtk::Application {
        public:
            MapNormalizerApplication();
            virtual ~MapNormalizerApplication();

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
                                   std::initializer_list<MenuItemInfo>);

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

