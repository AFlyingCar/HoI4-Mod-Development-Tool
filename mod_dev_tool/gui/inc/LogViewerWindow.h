#ifndef LOG_VIEWER_WINDOW_H
# define LOG_VIEWER_WINDOW_H

# include <deque>
# include <queue>

# include "gtkmm/grid.h"
# include "gtkmm/box.h"
# include "gtkmm/window.h"
# include "gtkmm/liststore.h"
# include "gtkmm/treeview.h"
# include "gtkmm/treemodelfilter.h"
# include "gtkmm/scrolledwindow.h"
# include "gtkmm/checkbutton.h"
# include "gtkmm/comboboxtext.h"

# include "glibmm/dispatcher.h"

# include "Types.h"

# include "Message.h"

namespace HMDT::GUI {
    /**
     * @brief A window for viewing log messages
     */
    class LogViewerWindow: public Gtk::Window {
        public:
            struct ViewableLog {
                Log::Message::Level level;
                Log::Timestamp timestamp;
                Log::Source source;

                std::string formatted_message;
            };

            /**
             * @brief The maximum number of messages we can display in the buffer.
             * @details This number was chosen because the (at time of writing)
             *          size of ViewableLog is 56 bytes, and this means the
             *          maximum amount of logs allowed comes out to a little
             *          less than 1 MB (917504 bytes)
             */
            constexpr static size_t VIEWER_BUFFER_SIZE = 16384;

            LogViewerWindow();

            static void pushMessage(const Log::Message&,
                                    OptionalReference<LogViewerWindow>);

        protected:
            void initWidgets();

            void initTreeView();

            void updateTreeModelWith(const Log::Message&);
            void populateTreeModel();

            static std::string formatMessage(const Log::Message::PieceList&);

            void updateFilter();
            void resetFilters();

            std::string getEnabledLevels() const;

            bool shouldBeVisible(const Gtk::TreeRow&) const;

            Glib::Dispatcher* getDispatcher();

        private:
            /**
             * @brief The number of messages that have been removed from the
             *        buffer. This is used to fix LogRowColumns::index, which
             *        will not have the right value once messages start getting
             *        removed from the buffer
             */
            static inline size_t removed_messages = 0;
            static std::deque<Log::Message> viewable_messages;

            static std::mutex next_message_mutex;
            static std::queue<Log::Message> next_messages;

            struct LogRowColumns: public Gtk::TreeModel::ColumnRecord {
                LogRowColumns();

                // The individual cells for the row
                Gtk::TreeModelColumn<Glib::ustring> m_level;
                Gtk::TreeModelColumn<Glib::ustring> m_timestamp;

                Gtk::TreeModelColumn<Glib::ustring> m_module;
                Gtk::TreeModelColumn<Glib::ustring> m_filename_line;
                Gtk::TreeModelColumn<Glib::ustring> m_function;

                Gtk::TreeModelColumn<Glib::ustring> m_message;
            } m_columns;

            Gtk::Box m_filtering_box;

            //! The container for all filtering tools
            Gtk::Grid m_filtering_grid;

            //! The container for all sub widgets
            Gtk::Box m_box;

            //////////////////////////////////////////////////////

            //! A special column view for m_level
            Gtk::TreeView::Column m_level;

            //! A custom text cell renderer for certain columns
            Gtk::CellRendererText m_cell_renderer_text;

            //! We want the view to be scrollable
            Gtk::ScrolledWindow m_swindow;

            //! The view used to see the list store
            Gtk::TreeView m_log_view;

            //! The log listing
            Glib::RefPtr<Gtk::ListStore> m_log_list;

            //! A wrapper around m_log_list to handle filtering
            Glib::RefPtr<Gtk::TreeModelFilter> m_log_filter_model;

            //////////////////////////////////////////////////////

            Gtk::CheckButton m_info_enabled;
            Gtk::CheckButton m_debug_enabled;
            Gtk::CheckButton m_error_enabled;
            Gtk::CheckButton m_warn_enabled;

            Gtk::Label m_from_label;
            Gtk::Entry m_from_time_search;
            Gtk::Label m_until_label;
            Gtk::Entry m_until_time_search;

            Gtk::ComboBoxText m_from_time_select;
            Gtk::ComboBoxText m_until_time_select;

            // Entry field for searching for modules
            Gtk::Label m_module_search_label;
            Gtk::Entry m_module_search;

            // Doubles as both filename and line number search
            Gtk::Label m_filename_search_label;
            Gtk::Entry m_filename_search;

            Gtk::Label m_message_search_label;
            //! The entry field for searching by string
            Gtk::Entry m_message_search;

            //! Used to reset filtering options
            Gtk::Button m_filter_reset;

            /**
             * @brief Check-button for determining if cells should be colorized
             *        if applicable.
             */
            Gtk::CheckButton m_cell_colorize_enabled;

            //////////////////////////////////////////////////////

            Glib::Dispatcher m_dispatcher;

            /**
             * @brief Pointer to m_dispatcher to protect against race conditions
             *        caused by the logging thread attempting to invoke the 
             *        dispatcher before it has been fully constructed.
             */
            Glib::Dispatcher* m_dispatcher_ptr;
    };
}

#endif

