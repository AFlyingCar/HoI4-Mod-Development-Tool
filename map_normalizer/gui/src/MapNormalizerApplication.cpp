
#include "MapNormalizerApplication.h"

#include "gtkmm.h"

#include "Constants.h"
#include "Logger.h"

#include "MainWindow.h"

MapNormalizer::GUI::MapNormalizerApplication::MapNormalizerApplication():
    Gtk::Application(APPLICATION_ID)
{
    Glib::set_application_name(APPLICATION_NAME);
}

void MapNormalizer::GUI::MapNormalizerApplication::on_activate() {
    Gtk::Application::on_activate();

    m_window.reset(new MainWindow{*this});
    if(!m_window->initialize()) {
        writeError("Failed to initialize main window!");
        return;
    }

    add_window(*m_window);

    m_window->show();
}

void MapNormalizer::GUI::MapNormalizerApplication::on_startup() {
    Gtk::Application::on_startup();

    //Create the per-Window menu:
    auto win_menu = Gio::Menu::create();

    auto submenu_file = Gio::Menu::create();
    submenu_file->append("_New", "win.new");
    submenu_file->append("_Open", "win.open"); // TODO: We should probably have a subsubmenu for this: different input types (province-map, heightmap, etc...)
    submenu_file->append("_Close", "win.close");
    win_menu->append_submenu("File", submenu_file);

    set_menubar(win_menu);

}

