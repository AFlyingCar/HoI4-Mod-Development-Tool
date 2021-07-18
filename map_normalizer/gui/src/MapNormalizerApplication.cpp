
#include "MapNormalizerApplication.h"

#include "gtkmm.h"

#include "Constants.h"
#include "Logger.h"

#include "MainWindow.h"
#include "LogViewerWindow.h"

MapNormalizer::GUI::MapNormalizerApplication::MapNormalizerApplication():
    Gtk::Application(APPLICATION_ID),
    m_settings(Gtk::Settings::get_default()),
    m_is_exiting(new bool(false))
{
    std::shared_ptr<bool> is_exiting = m_is_exiting;

    // Register a function that can send logs to a special log viewing window
    Log::Logger::registerOutputFunction([this, is_exiting](const Log::Message& message)
    {
        if(is_exiting && *is_exiting) return true;

        LogViewerWindow::pushMessage(message,
                                     m_window == nullptr ? std::nullopt
                                                         : m_window->getLogViewerWindow());

        return true;
    });

    Glib::set_application_name(APPLICATION_NAME);
}

MapNormalizer::GUI::MapNormalizerApplication::~MapNormalizerApplication() {
    *m_is_exiting = true;
}

void MapNormalizer::GUI::MapNormalizerApplication::on_activate() {
    Gtk::Application::on_activate();

    m_window.reset(new MainWindow{*this});
    if(!m_window->initialize()) {
        WRITE_ERROR("Failed to initialize main window!");
        return;
    }

    add_window(*m_window);

    m_window->show();
}

void MapNormalizer::GUI::MapNormalizerApplication::on_startup() {
    Gtk::Application::on_startup();

    createMenu("Root", {});
    createMenu("Root", "File", {
        { "_New Project", "win.new", {} },
        { "_Open Project", "win.open", {} }, // TODO: We should probably have a subsubmenu for this: different input types (province-map, heightmap, etc...)
        { "_Save Project", "win.save", {} },
        { "_Close Project", "win.close", {} },
        { "_Quit", "win.quit", {} }
    });

    createMenu("Root", "Edit", {
        { "_Undo", "win.undo", {} },
        { "_Redo", "win.redo", {} }
    });

    createMenu("Root", "View", {
        { "_Properties", "win.properties", {} },
        { "_Log Window", "win.log_window" {} }
        { "_Switch Renderers", "win.switch_renderers", {
            { "_Use OpenGL", "win.switch_renderers.usegl" },
            { "_Use Cairo", "win.switch_renderers.usecairo" },
        } }
    });

    createMenu("Root", "Project", {
            { "_Import Province Map", "win.import_provincemap", {} }
    });

    // Debug dump the menus
    for(auto&& [menu_name, menu] : m_menus) {
        WRITE_DEBUG(menu_name, " -> {");
        for(auto&& menu_item_name : menu.menu_item_names) {
            auto opt_menu_item = getMenuItemInfo(menu_item_name);
            if(opt_menu_item) {
                WRITE_DEBUG("  ", menu_item_name, " = {", opt_menu_item->get().label, ", ", opt_menu_item->get().detailed_action, "}");
            } else {
                WRITE_ERROR("Menu '", menu_name, "' refers to non-existant menu item '", menu_item_name);
            }
        }
        WRITE_DEBUG("}");
    }
}

auto MapNormalizer::GUI::MapNormalizerApplication::createMenu(
        const std::string& menu_name,
        std::initializer_list<MenuItemInfo> menu_items)
    -> MenuWrapper
{
    return createMenu("", menu_name, menu_items);
}

auto MapNormalizer::GUI::MapNormalizerApplication::createMenu(
        const std::string& parent_menu_full_name,
        const std::string& menu_name,
        const std::vector<MenuItemInfo>& menu_items)
    -> MenuWrapper
{
    auto menu = Gio::Menu::create();

    // Build the full name of this menu
    std::string full_name = menu_name;
    if(!parent_menu_full_name.empty()) {
        full_name = parent_menu_full_name + "." + full_name;
    }

    // Set this up now, so that any submenus that exist can access it as soon as
    //  possible
    MenuWrapper& this_menu_wrapper = m_menus[full_name] = MenuWrapper {
        menu, { }
    };
    std::vector<std::string>& menu_item_names = this_menu_wrapper.menu_item_names;

    // Add all menu items to the menu
    for(auto&& item : menu_items) {
        auto menu_item = addMenuItem(full_name, item);

        // If the menuitem is a submenu
        // The submenu is named the same as the menu item
        if(!item.submenu_items.empty()) {
            // Convert the list of SubMenuItemInfos into MenuItemInfos
            std::vector<MenuItemInfo> submenu_items;
            submenu_items.reserve(submenu_items.size());
            std::transform(item.submenu_items.begin(), item.submenu_items.end(),
                           std::back_inserter(submenu_items),
                           [](auto&& submenu_item) {
                               return MenuItemInfo{ submenu_item.label,
                                                    submenu_item.detailed_action,
                                                    { }
                                                  };
                           });

            // Recursively create the menu
            createMenu(full_name, item.label, submenu_items);
        } else {
            menu->append_item(menu_item.menu_item);
        }

        menu_item_names.push_back(menu_item.full_name);
    }

    // Find which parent menu to add this to, or set this menu to the menu bar
    if(parent_menu_full_name.empty()) {
        set_menubar(menu);
    } else {
        if(m_menus.count(parent_menu_full_name)) {
            m_menus.at(parent_menu_full_name).menu->append_submenu(menu_name, menu);
        } else {
            WRITE_ERROR("Failed to find parent menu at ", parent_menu_full_name);
        }
    }

    return this_menu_wrapper;
}

auto MapNormalizer::GUI::MapNormalizerApplication::getMenuItem(const std::string& full_name)
    -> std::optional<std::reference_wrapper<MenuItemWrapper>>
{
    if(m_menu_items.count(full_name)) {
        return m_menu_items.at(full_name);
    }

    return std::nullopt;
}

auto MapNormalizer::GUI::MapNormalizerApplication::getMenuItemInfo(const std::string& full_name)
    -> std::optional<std::reference_wrapper<MenuItemInfo>>
{
    if(m_menu_items.count(full_name)) {
        return m_menu_items.at(full_name).info;
    }

    return std::nullopt;
}

auto MapNormalizer::GUI::MapNormalizerApplication::addMenuItem(const MenuItemInfo& menu_item_info)
    -> MenuItemWrapper&
{
    return addMenuItem("", menu_item_info);
}

auto MapNormalizer::GUI::MapNormalizerApplication::addMenuItem(const std::string& parent_name,
                                                               const MenuItemInfo& menu_item_info)
    -> MenuItemWrapper&
{
    auto menu_item = Gio::MenuItem::create(menu_item_info.label,
                                           menu_item_info.detailed_action);

    std::string full_name = menu_item_info.label;
    if(!parent_name.empty()) {
        MenuItemInfo default_parent_name{parent_name, "", {}};
        full_name = getMenuItemInfo(parent_name).value_or(std::ref(default_parent_name)).get().label + "." + full_name;
    }

    return m_menu_items[full_name] = MenuItemWrapper {
                                         menu_item_info,
                                         menu_item,
                                         full_name
                                     };
}

