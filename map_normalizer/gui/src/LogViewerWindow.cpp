
#include "LogViewerWindow.h"

std::deque<MapNormalizer::Log::Message>
    MapNormalizer::GUI::LogViewerWindow::viewable_messages;

MapNormalizer::GUI::LogViewerWindow::LogRowColumns::LogRowColumns() {
    add(m_level);
    add(m_timestamp);
    // add(m_source);
    add(m_message);
}

MapNormalizer::GUI::LogViewerWindow::LogViewerWindow():
    m_box(Gtk::ORIENTATION_VERTICAL)
{
    set_title("Log Viewer");
    set_default_size(640, 420); // TODO

    initWidgets();
}

void MapNormalizer::GUI::LogViewerWindow::pushMessage(const Log::Message& msg,
                                                      OptionalReference<LogViewerWindow> lvw)
{
    // TODO: do we need a mutex here?

    if(viewable_messages.size() == VIEWER_BUFFER_SIZE) {
        viewable_messages.pop_front();
    }

    viewable_messages.push_back(msg);

    if(lvw) {
        lvw->get().updateTreeModelWith(msg);
    }
}

void MapNormalizer::GUI::LogViewerWindow::initWidgets() {
    add(m_box);

    // Add the TreeView 
    m_swindow.add(m_log_view);

    m_swindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    // m_swindow.set_expand();

    // Add all widgets to the box
    m_box.pack_start(m_swindow);

    // Create the Tree Model
    m_log_list = Gtk::ListStore::create(m_columns);
    m_log_view.set_model(m_log_list);

    populateTreeModel();

    // Add all of the columns to the view
    m_log_view.append_column("Level", m_columns.m_level);
    m_log_view.append_column("Timestamp", m_columns.m_timestamp);
    // m_log_view.append_column("Source", m_columns.m_source);

    // TODO: We may want to make this a special type of cell instead, so we can control formatting on it
    m_log_view.append_column("Message", m_columns.m_message);

    // TODO: Do we want to allow the columns to be re-orderable?

    show_all_children();
}

void MapNormalizer::GUI::LogViewerWindow::updateTreeModelWith(const Log::Message& message)
{
    auto row = *(m_log_list->append());
    row[m_columns.m_level] = std::to_string(message.getDebugLevel());

    // TODO: Do we want to allow the user to enter their own time display format?
    row[m_columns.m_timestamp] = message.getTimestampAsString();
    // row[m_columns.m_source] = message.getSource();
    row[m_columns.m_message] = formatMessage(message.getPieces());
}

void MapNormalizer::GUI::LogViewerWindow::populateTreeModel() {
    for(auto&& message : viewable_messages) {
        updateTreeModelWith(message);
    }
}

std::string MapNormalizer::GUI::LogViewerWindow::formatMessage(const Log::Message::PieceList& pieces)
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

