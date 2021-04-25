
#ifndef MENUITEMWRAPPER_H
# define MENUITEMWRAPPER_H

# include <vector>
# include <string>

# include "gtkmm.h"

namespace MapNormalizer::GUI {
    struct MenuItemInfo {
        Glib::ustring label;
        Glib::ustring detailed_action;
    };

    struct MenuItemWrapper {
        MenuItemInfo info;

        Glib::RefPtr<Gio::MenuItem> menu_item;

        std::string full_name;
    };

    struct MenuWrapper {
        Glib::RefPtr<Gio::Menu> menu;
        std::vector<std::string> menu_item_names;
    };
}

#endif

