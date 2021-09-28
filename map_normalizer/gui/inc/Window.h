#ifndef WINDOW_H
# define WINDOW_H

# include <vector>
# include <string>
# include <type_traits>

# include <gtkmm/applicationwindow.h>
# include <gtkmm/widget.h>
# include <gtkmm/box.h>

# include "WidgetContainer.h"

namespace MapNormalizer::GUI {
    class Window: public Gtk::ApplicationWindow, public WidgetContainer {
        public:
            Window(const std::string&, Gtk::Application&);
            virtual ~Window();

            virtual bool initialize() final;

        protected:
            virtual bool initializeActions() = 0;
            virtual bool initializeWidgets() = 0;
            virtual bool initializeFinal() = 0;

            virtual Gtk::Orientation getDisplayOrientation() const;

            virtual void addWidgetToParent(Gtk::Widget&) override;

            Gtk::Box* getBox();

            Gtk::Application* getApplication();

            /**
             * @brief Helper function to get an action of a specific type
             *
             * @tparam T The type of action to get.
             * @param action_name The name of the action to get
             *
             * @return The action, or nullptr if no action was found of that
             *         name or type
             */
            template<typename T = Gio::SimpleAction,
                     typename = std::enable_if_t<std::is_base_of_v<Gio::Action, T>, T>>
            T* getAction(const Glib::ustring& action_name) {
                auto action = lookup_action(action_name);

                if(action.get() == nullptr)
                    return nullptr;

                return dynamic_cast<T*>(&(*action.get()));
            }

        private:
            std::string m_window_name;

            Gtk::Box* m_box;

            Gtk::Application& m_application;
    };
}

#endif

