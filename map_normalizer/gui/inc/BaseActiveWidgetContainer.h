#ifndef BASEACTIVEWIDGETCONTAINER_H
# define BASEACTIVEWIDGETCONTAINER_H

# include <variant>

# include "WidgetContainer.h"

namespace MapNormalizer::GUI {
    template<typename... ChildTypes>
    class BaseActiveWidgetContainer: public virtual WidgetContainer  {
        public:
            using ActiveChildVariant = std::variant<std::monostate, ChildTypes*...>;

            /**
             * @brief Adds a widget to this window and marks that it is now the
             *        active widget to be added to.
             */
            template<typename W, typename... Args>
            W* addActiveWidget(Args&&... args) {
                return std::get<W*>(setActiveChild(addWidget<W>(std::forward<Args>(args)...)));
            }

            const ActiveChildVariant& getActiveChild() const {
                return m_active_child;
            }

        protected:
            template<typename W>
            ActiveChildVariant& setActiveChild(W* active_child) {
                return (m_active_child = active_child);
            }

            ActiveChildVariant setActiveChild() {
                return (m_active_child = std::monostate{});
            }

            ActiveChildVariant& getActiveChild() {
                return m_active_child;
            }

        private:
            //! The currently active widget to be added to
            ActiveChildVariant m_active_child;
    };
}

#endif

