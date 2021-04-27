#ifndef WIDGET_CONTAINER_H
# define WIDGET_CONTAINER_H

# include "gtkmm/widget.h"

namespace MapNormalizer::GUI {
    /**
     * @brief A generic container for widgets, provides a simplified interface
     *        for adding widgets to a parent container
     */
    class WidgetContainer {
        public:
            WidgetContainer() = default;
            virtual ~WidgetContainer() = default;

        protected:
            /**
             * @brief Adds a new widget of type W to the parent widget
             * @par Note that new W objects are created as "managed", meaning
             *      that their lifetime is controlled by the parent they get
             *      added to.
             *
             * @tparam W The type of widget to add
             * @tparam Args The type of every argument to create a new W
             * @param args Every argument to get forwarded into a call to `new W{}`
             *
             * @return the newly created W.
             */
            template<typename W, typename... Args>
            W* addWidget(Args&&... args) {
                W* widget = manage(new W{std::forward<Args>(args)...});

                addWidgetToParent(*widget);

                return widget;
            }

            virtual void addWidgetToParent(Gtk::Widget&) = 0;
    };
}

#endif

