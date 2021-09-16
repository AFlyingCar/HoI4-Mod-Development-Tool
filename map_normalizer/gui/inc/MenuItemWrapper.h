
#ifndef MENUITEMWRAPPER_H
# define MENUITEMWRAPPER_H

# include <vector>
# include <string>

# include "gtkmm.h"

namespace MapNormalizer::GUI {
    /**
     * @brief Information about a sub-menu item. Because we probably won't need
     *        to worry about it, we will not support completely recursive
     *        sub-menus.
     */
    struct SubMenuItemInfo {
        Glib::ustring label;
        Glib::ustring detailed_action;
    };

    struct MenuItemInfo {
        Glib::ustring label;
        Glib::ustring detailed_action;

        std::vector<SubMenuItemInfo> submenu_items;
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

