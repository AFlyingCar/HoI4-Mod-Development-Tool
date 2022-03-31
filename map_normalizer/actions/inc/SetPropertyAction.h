
#ifndef SETPROPERTYACTION_H
# define SETPROPERTYACTION_H

# include "IAction.h"

namespace MapNormalizer::Action {
    /**
     * @brief Sets a single property in a structure
     *
     * @tparam S The structure type
     * @tparam T The type of value that is to get set
     */
    template<typename S, typename T>
    class SetPropertyAction: public Action::IAction {
        public:
            SetPropertyAction(S* structure,
                              T S::* field,
                              const T& new_value):
                m_structure(structure),
                m_field(field),
                m_old_value(structure->*m_field),
                m_new_value(new_value)
            {
            }

            template<typename V,
                     typename = std::enable_if_t<std::is_convertible_v<V, T>>>
            SetPropertyAction(S* structure, T S::* field, const V& new_value):
                SetPropertyAction(structure, field, static_cast<const T&>(new_value))
            { }

            virtual bool doAction(const Action::IAction::Callback& callback = _) override
            {
                if(!callback(0)) return false;

                auto& curr_val = m_structure->*m_field;
                if(curr_val != m_old_value) {
                    // WRITE_WARN()
                }
                (m_structure->*m_field) = m_new_value;

                // Set back if we are told of a failure
                if(!callback(1)) {
                    (m_structure->*m_field) = m_old_value;
                    return false;
                }

                return true;
            }

            virtual bool undoAction(const Action::IAction::Callback& callback = _) override
            {
                if(!callback(0)) return false;

                auto& curr_val = m_structure->*m_field;
                if(curr_val != m_new_value) {
                    // WRITE_WARN()
                }
                (m_structure->*m_field) = m_old_value;

                // Set back if we are told of a failure
                if(!callback(1)) {
                    (m_structure->*m_field) = m_new_value;
                    return false;
                }

                return true;
            }

        private:
            //! The structure that is getting modified
            S* m_structure;

            //! The field of the structure that is getting modified
            T S::* m_field;

            //! The old value of the field
            T m_old_value;

            //! The new value of the field
            T m_new_value;
    };

# define NewSetPropertyAction(OBJECT, NAME, VALUE) \
    new MapNormalizer::Action::SetPropertyAction< \
        MapNormalizer::RemoveAllPointers_t<decltype(OBJECT)>, \
                                           decltype(MapNormalizer::RemoveAllPointers_t<decltype(OBJECT)>::NAME)> \
            (OBJECT, &MapNormalizer::RemoveAllPointers_t<decltype(OBJECT)>::NAME, VALUE)
}

#endif

