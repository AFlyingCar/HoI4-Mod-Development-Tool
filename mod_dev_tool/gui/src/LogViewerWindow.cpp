
#include "LogViewerWindow.h"

#include <chrono>

#include "Logger.h"

std::deque<HMDT::Log::Message> HMDT::GUI::LogViewerWindow::viewable_messages;
std::mutex HMDT::GUI::LogViewerWindow::next_message_mutex;
std::queue<HMDT::Log::Message> HMDT::GUI::LogViewerWindow::next_messages;

HMDT::GUI::LogViewerWindow::LogRowColumns::LogRowColumns() {
    add(m_level);
    add(m_timestamp);
    add(m_module);
    add(m_filename_line);
    add(m_function);
    add(m_message);
}

HMDT::GUI::LogViewerWindow::LogViewerWindow():
    m_filtering_box(Gtk::ORIENTATION_HORIZONTAL),
    m_box(Gtk::ORIENTATION_VERTICAL),
    m_info_enabled("Info"),
    m_debug_enabled("Debug"),
    m_error_enabled("Error"),
    m_warn_enabled("Warning"),
    m_from_label("From:"),
    m_until_label("Until:"),
    m_module_search_label("Module search:"),
    m_filename_search_label("Filename search:"),
    m_message_search_label("Text search:"),
    m_filter_reset("Reset Filters"),
    m_dispatcher(),
    m_dispatcher_ptr(nullptr)
{
    set_title("Log Viewer");
    set_default_size(1124, 640);

    initWidgets();

    m_dispatcher.connect([this]() {
        next_message_mutex.lock();
        auto msg = next_messages.front();
        next_messages.pop();
        next_message_mutex.unlock();

        updateTreeModelWith(msg);
    });

    // We are done constructing m_dispatcher, so make it available to be used
    //  now
    m_dispatcher_ptr = &m_dispatcher;
}

void HMDT::GUI::LogViewerWindow::pushMessage(const Log::Message& msg,
                                             OptionalReference<LogViewerWindow> lvw)
{
    if(viewable_messages.size() == VIEWER_BUFFER_SIZE) {
        viewable_messages.pop_front();
        ++removed_messages;
    }

    viewable_messages.push_back(msg);

    if(lvw) {
        if(auto* dispatcher = lvw->get().getDispatcher(); dispatcher) {
            next_message_mutex.lock();
            next_messages.push(msg);
            next_message_mutex.unlock();

            dispatcher->emit();
        }
    }
}

void HMDT::GUI::LogViewerWindow::initWidgets() {
    add(m_box);

    // Add the TreeView 
    m_swindow.add(m_log_view);

    m_swindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    // m_swindow.set_expand();

    // Add all widgets to the box
    m_box.pack_start(m_filtering_box, Gtk::PACK_SHRINK);
    m_filtering_box.pack_start(m_filtering_grid, Gtk::PACK_SHRINK);
    m_box.pack_start(m_swindow);

    // Build filter tools
    {
        // Level search field
        {
            m_info_enabled.set_active(true);
            m_debug_enabled.set_active(true);
            m_error_enabled.set_active(true);
            m_warn_enabled.set_active(true);

            auto update_func = [this]() { updateFilter(); };

            m_info_enabled.signal_toggled().connect(update_func);
            m_debug_enabled.signal_toggled().connect(update_func);
            m_error_enabled.signal_toggled().connect(update_func);
            m_warn_enabled.signal_toggled().connect(update_func);
        }

        // Time search fields
        {
            m_from_time_select.append("");
            m_from_time_select.append("Now");
            m_from_time_select.append("-1min");
            m_from_time_select.append("-5min");
            m_from_time_select.signal_changed().connect([this]() {
                auto current = m_from_time_select.get_active_row_number();
                auto field_time = m_from_time_search.get_text();

                auto now = Log::Logger::now();

                switch(current) {
                    case 1:
                        break;
                    case 2:
                        if(!field_time.empty()) {
                            now = Log::Logger::getTimestampFromString(field_time);
                        }
                        now -= std::chrono::minutes(1);
                        break;
                    case 3:
                        if(!field_time.empty()) {
                            now = Log::Logger::getTimestampFromString(field_time);
                        }
                        now -= std::chrono::minutes(5);
                        break;
                    case 0:
                    default:
                        return;
                }

                m_from_time_search.set_text(Log::Logger::getTimestampAsString(now));

                m_from_time_select.set_active(0);
            });

            m_until_time_select.append("");
            m_until_time_select.append("Now");
            m_until_time_select.append("+1min");
            m_until_time_select.append("+5min");
            m_until_time_select.signal_changed().connect([this]() {
                auto current = m_until_time_select.get_active_row_number();
                auto field_time = m_until_time_search.get_text();

                auto now = Log::Logger::now();

                switch(current) {
                    case 1:
                        break;
                    case 2:
                        if(!field_time.empty()) {
                            now = Log::Logger::getTimestampFromString(field_time);
                        }
                        now += std::chrono::minutes(1);
                        break;
                    case 3:
                        if(!field_time.empty()) {
                            now = Log::Logger::getTimestampFromString(field_time);
                        }
                        now += std::chrono::minutes(5);
                        break;
                    case 0:
                    default:
                        return;
                }

                m_until_time_search.set_text(Log::Logger::getTimestampAsString(now));

                m_until_time_select.set_active(0);
            });

            m_from_time_search.signal_changed().connect([this]() {
                updateFilter();
            });
            m_until_time_search.signal_changed().connect([this]() {
                updateFilter();
            });
        }

        // Module Entry field
        {
            m_module_search.signal_changed().connect([this]() {
                updateFilter();
            });
        }

        // Filename Entry field
        {
            m_filename_search.signal_changed().connect([this]() {
                updateFilter();
            });
        }

        // Message Entry field
        {
            m_message_search.signal_changed().connect([this]() {
                updateFilter();
            });
        }

        // Filter Reset button
        {
            m_filter_reset.signal_clicked().connect([this]() {
                resetFilters();
                updateFilter();
            });
        }
    }

    // Add the elements to the gui
    {
        size_t x = 0;

        // The level search area
        {
            // m_filtering_grid.attach(m_level_select, 0, 1, 1, 1);
            m_filtering_grid.attach(m_info_enabled, x, 1, 1, 1);
            m_filtering_grid.attach(m_debug_enabled, x, 2, 1, 1);

            // We want them to be in a square
            ++x;
            m_filtering_grid.attach(m_error_enabled, x, 1, 1, 1);
            m_filtering_grid.attach(m_warn_enabled, x, 2, 1, 1);
        }
        ++x;

        // The time search area
        {
            m_filtering_grid.attach(m_from_label, x, 1, 1, 1);
            m_filtering_grid.attach(m_until_label, x, 2, 1, 1);
            ++x;

            m_filtering_grid.attach(m_from_time_search, x, 1, 2, 1);
            m_filtering_grid.attach(m_until_time_search, x, 2, 2, 1);
            ++x;++x; // Increment twice because the search field is 2 cells wide

            m_filtering_grid.attach(m_from_time_select, x, 1, 1, 1);
            m_filtering_grid.attach(m_until_time_select, x, 2, 1, 1);
        }
        ++x;

        // The module search area
        {
            m_filtering_grid.attach(m_module_search_label, x, 1, 2, 1);
            m_filtering_grid.attach(m_module_search, x, 2, 2, 1);
            ++x; // One extra addition since the search area is 2 cells wide
        }
        ++x;

        // The filename search area
        {
            m_filtering_grid.attach(m_filename_search_label, x, 1, 2, 1);
            m_filtering_grid.attach(m_filename_search, x, 2, 2, 1);
            ++x; // One extra addition since the search area is 2 cells wide
        }
        ++x;

        // The message search area
        {
            m_filtering_grid.attach(m_message_search_label, x, 1, 2, 1);
            m_filtering_grid.attach(m_message_search, x, 2, 2, 1); // X, Y, W, H
            ++x; // One extra addition since the search area is 2 cells wide
        }
        ++x;

        {
            m_filter_reset.set_vexpand(false);
            m_filter_reset.set_valign(Gtk::ALIGN_CENTER);
            m_filtering_box.pack_start(m_filter_reset, Gtk::PACK_SHRINK, 5);
        }
    }
    // m_filtering_grid.set_grid_lines(Gtk::TREE_VIEW_GRID_LINES_HORIZONTAL);
    m_filtering_grid.set_column_spacing(5);

    // Create the Tree Model
    m_log_list = Gtk::ListStore::create(m_columns);
    m_log_filter_model = Gtk::TreeModelFilter::create(m_log_list);

    m_log_filter_model->set_visible_func([this](const Gtk::TreeModel::const_iterator& iter)
        -> bool
    {
        return shouldBeVisible(*iter);
    });

    m_log_view.set_model(m_log_filter_model);

    populateTreeModel();

    // Add all of the columns to the view
    m_log_view.append_column("Level", m_columns.m_level);
    m_log_view.append_column("Timestamp", m_columns.m_timestamp);

    m_log_view.append_column("Module", m_columns.m_module);
    m_log_view.append_column("Filename:Line", m_columns.m_filename_line);
    m_log_view.append_column("Function", m_columns.m_function);

    // TODO: We may want to make this a special type of cell instead, so we can control formatting on it
    m_log_view.append_column("Message", m_columns.m_message);

    // Set properties of the columns
    for(Gtk::TreeViewColumn* column : m_log_view.get_columns()) {
        column->set_resizable(true);
        column->set_reorderable(true);
    }

    show_all_children();
}

void HMDT::GUI::LogViewerWindow::updateTreeModelWith(const Log::Message& message)
{
    auto row = *(m_log_list->append());
    row[m_columns.m_level] = std::to_string(message.getDebugLevel());

    row[m_columns.m_timestamp] = message.getTimestampAsString();

    auto&& source = message.getSource();
    row[m_columns.m_module] = source.getModulePath().filename().generic_string();
    row[m_columns.m_filename_line] = source.getFileName().lexically_relative(MN_PROJECT_ROOT).generic_string() + ":" + std::to_string(source.getLineNumber());
    row[m_columns.m_function] = source.getFunctionName();

    row[m_columns.m_message] = formatMessage(message.getPieces());
}

void HMDT::GUI::LogViewerWindow::populateTreeModel() {
    for(auto&& message : viewable_messages) {
        updateTreeModelWith(message);
    }
}

std::string HMDT::GUI::LogViewerWindow::formatMessage(const Log::Message::PieceList& pieces)
{
    // TODO: Support some sort of formatting?
    std::stringstream ss;
    for(auto&& piece : pieces) {
        if(std::holds_alternative<std::string>(piece)) {
            ss << std::get<std::string>(piece);
        }
    }

    return ss.str();
}

/**
 * @brief Gets a string containing every logging level currently enabled
 *
 * @return "level1;level2;level3;..."
 */
std::string HMDT::GUI::LogViewerWindow::getEnabledLevels() const {
    std::string enabled_levels;

    if(m_info_enabled.get_active()) {
        enabled_levels += "INFO;";
    }

    if(m_debug_enabled.get_active()) {
        enabled_levels += "DEBUG;";
    }

    if(m_error_enabled.get_active()) {
        enabled_levels += "ERROR;";
    }

    if(m_warn_enabled.get_active()) {
        enabled_levels += "WARN;";
    }

    return enabled_levels;
}

bool HMDT::GUI::LogViewerWindow::shouldBeVisible(const Gtk::TreeRow& row) const
{
    const Glib::ustring& row_level = row[m_columns.m_level];
    const Glib::ustring& row_timestamp_str = row[m_columns.m_timestamp];
    const Glib::ustring& row_module_str = row[m_columns.m_module];
    const Glib::ustring& row_filename_line_str = row[m_columns.m_filename_line];
    const Glib::ustring& row_message = row[m_columns.m_message];

    // Check each individual filter in turn
    if(Glib::ustring level_filter = getEnabledLevels();
       level_filter.find(row_level) == std::string::npos)
    {
        return false;
    }

    auto row_timestamp = Log::Logger::getTimestampFromString(row_timestamp_str);

    // If a from timestamp is provided, only display rows newer than that time
    if(auto from_timestamp_str = m_from_time_search.get_text();
       !from_timestamp_str.empty())
    {
        auto from_timestamp = Log::Logger::getTimestampFromString(from_timestamp_str);

        if(row_timestamp < from_timestamp) {
            return false;
        }
    }

    // If a until timestamp is provided, only display rows older than that time
    if(auto until_timestamp_str = m_until_time_search.get_text();
       !until_timestamp_str.empty())
    {
        auto until_timestamp = Log::Logger::getTimestampFromString(until_timestamp_str);

        if(row_timestamp > until_timestamp) {
            return false;
        }
    }

    // Modules
    if(const auto& module_filter = m_module_search.get_text();
       !module_filter.empty() && row_module_str.find(module_filter) == std::string::npos)
    {
        return false;
    }

    // Filename
    if(const auto& filename_filter = m_filename_search.get_text();
       !filename_filter.empty())
    {
        // Find the filename and line number components for the text
        auto delim_text = row_filename_line_str.find(':');
        auto filename_text = row_filename_line_str.substr(0, delim_text);

        // Check the filename
        if(filename_text.find(filename_filter) == std::string::npos) {
            return false;
        }
    }

    // Message Text
    if(const auto& text_filter = m_message_search.get_text();
       !text_filter.empty() && row_message.find(text_filter) == std::string::npos)
    {
        return false;
    }

    return true;
}

void HMDT::GUI::LogViewerWindow::updateFilter() {
    m_log_filter_model->refilter();
}

void HMDT::GUI::LogViewerWindow::resetFilters() {
    m_info_enabled.set_active(true);
    m_debug_enabled.set_active(true);
    m_error_enabled.set_active(true);
    m_warn_enabled.set_active(true);

    m_from_time_search.set_text("");
    m_until_time_search.set_text("");

    m_message_search.set_text("");
}

Glib::Dispatcher* HMDT::GUI::LogViewerWindow::getDispatcher() {
    return m_dispatcher_ptr;
}
