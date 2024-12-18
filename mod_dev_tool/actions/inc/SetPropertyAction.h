
#ifndef SETPROPERTYACTION_H
# define SETPROPERTYACTION_H

# include <string>

# include "PreprocessorUtils.h"
# include "Util.h"

# include "Logger.h"

# include "IAction.h"

namespace HMDT::Action {
    /**
     * @brief Sets a single property in a structure
     *
     * @tparam S The structure type
     * @tparam T The type of value that is to get set
     */
    template<typename S, typename T>
    class SetPropertyAction: public Action::IAction {
        public:
            using OnValueChangedCallback = std::function<void(const T&, const T&)>;

            SetPropertyAction(S* structure,
                              T S::* field,
                              const T& new_value,
                              std::string_view field_name = ""):
                m_structure(structure),
                m_field(field),
                m_old_value(structure->*m_field),
                m_new_value(new_value),
                m_field_name(field_name),
                m_on_value_changed_callback([](auto&&...) { })
            {
            }

            template<typename V,
                     typename = std::enable_if_t<std::is_convertible_v<V, T>>>
            SetPropertyAction(S* structure, T S::* field, const V& new_value,
                              std::string_view field_name = ""):
                SetPropertyAction(structure, field,
                                  static_cast<const T&>(new_value),
                                  field_name)
            { }

            virtual bool doAction(const Action::IAction::Callback& callback = _) override
            {
                if(!callback(0)) return false;

                auto& curr_val = m_structure->*m_field;
                if(curr_val != m_old_value) {
                    WRITE_ERROR("Current value does not match the old value!");
                    return false;
                }
                WRITE_DEBUG("Setting field '", m_field_name.data(), "'.");
                (m_structure->*m_field) = m_new_value;

                // Set back if we are told of a failure
                if(!callback(1)) {
                    (m_structure->*m_field) = m_old_value;
                    return false;
                }

                m_on_value_changed_callback(m_old_value, m_new_value);

                return true;
            }

            virtual bool undoAction(const Action::IAction::Callback& callback = _) override
            {
                if(!callback(0)) return false;

                auto& curr_val = m_structure->*m_field;
                if(curr_val != m_new_value) {
                    WRITE_ERROR("Current value does not match the old value!");
                    return false;
                }
                WRITE_DEBUG("UnSetting field '", m_field_name.data(), "'.");
                (m_structure->*m_field) = m_old_value;

                // Set back if we are told of a failure
                if(!callback(1)) {
                    (m_structure->*m_field) = m_new_value;
                    return false;
                }

                m_on_value_changed_callback(m_old_value, m_new_value);

                return true;
            }

            SetPropertyAction<S, T>& onValueChanged(const OnValueChangedCallback& callback) noexcept
            {
                m_on_value_changed_callback = callback;
                return *this;
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

            //! A string representation of the field that is being set (for debug)
            std::string_view m_field_name;

            //! Callback called when the value is changed
            OnValueChangedCallback m_on_value_changed_callback;
    };

# define NewSetPropertyAction(OBJECT, NAME, VALUE) \
    (new HMDT::Action::SetPropertyAction< \
        HMDT::RemoveAllPointers_t<decltype(OBJECT)>, \
                                  decltype(HMDT::RemoveAllPointers_t<decltype(OBJECT)>::NAME)> \
            (OBJECT, &HMDT::RemoveAllPointers_t<decltype(OBJECT)>::NAME, VALUE, STR(NAME)))
}

#endif

