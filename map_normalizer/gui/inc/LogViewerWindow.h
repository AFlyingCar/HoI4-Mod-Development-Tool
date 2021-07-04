#ifndef LOG_VIEWER_WINDOW_H
# define LOG_VIEWER_WINDOW_H

# include <deque>

# include "gtkmm/box.h"
# include "gtkmm/window.h"
# include "gtkmm/liststore.h"
# include "gtkmm/treeview.h"
# include "gtkmm/scrolledwindow.h"

# include "Types.h"

# include "Message.h"

namespace MapNormalizer::GUI {
    class LogViewerWindow: public Gtk::Window {
        public:
            struct ViewableLog {
                Log::Message::Level level;
                Log::Timestamp timestamp;
                Log::Source source;

                std::string formatted_message;
            };

            constexpr static size_t VIEWER_BUFFER_SIZE = 1024;

            LogViewerWindow();

            static void pushMessage(const Log::Message&,
                                    OptionalReference<LogViewerWindow>);

        protected:
            void initWidgets();

            void initTreeView();

            void updateTreeModelWith(const Log::Message&);
            void populateTreeModel();

            static std::string formatMessage(const Log::Message::PieceList&);

        private:
            static std::deque<Log::Message> viewable_messages;

            struct LogRowColumns: public Gtk::TreeModel::ColumnRecord {
                LogRowColumns();

                Gtk::TreeModelColumn<Glib::ustring> m_level;
                Gtk::TreeModelColumn<Glib::ustring> m_timestamp;
                // Log::Source source;

                Gtk::TreeModelColumn<Glib::ustring> m_message;
            } m_columns;

            //! The container for all sub widgets
            Gtk::Box m_box;

            //! We want the view to be scrollable
            Gtk::ScrolledWindow m_swindow;

            //! The view used to see the list store
            Gtk::TreeView m_log_view;

            //! The log listing
            Glib::RefPtr<Gtk::ListStore> m_log_list;
    };
}

#endif

