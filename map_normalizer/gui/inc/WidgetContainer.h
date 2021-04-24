#ifndef WIDGET_CONTAINER_H
# define WIDGET_CONTAINER_H

# include <vector>

# include "gtkmm/widget.h"

namespace MapNormalizer::GUI {
    class WidgetContainer {
        public:
            WidgetContainer() = default;
            virtual ~WidgetContainer();

        protected:
            template<typename W, typename... Args>
            W* addWidget(Args&&... args) {
                // TODO: Mark that W is managed by the parent container instead?
                m_widgets.push_back(new W{std::forward<Args>(args)...});

                addWidgetToParent(*m_widgets.back());

                return static_cast<W*>(m_widgets.back());
            }

            virtual void addWidgetToParent(Gtk::Widget&) = 0;

        private:
            std::vector<Gtk::Widget*> m_widgets;
    };
}

#endif

