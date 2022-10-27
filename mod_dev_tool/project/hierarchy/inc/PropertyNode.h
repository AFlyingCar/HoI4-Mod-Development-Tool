#ifndef PROJECT_HIERARCHY_PROPERTYNODE_H
# define PROJECT_HIERARCHY_PROPERTYNODE_H

# include "INode.h"

namespace HMDT::Project::Hierarchy {
    /**
     * @brief A node which represents a single property/value
     *
     * @tparam T The type of property
     */
    template<typename T>
    class PropertyNode: public IPropertyNode {
        public:
            /**
             * @brief Builds this property
             *
             * @param name The name of the property
             * @param value A reference to the value this property represents
             */
            PropertyNode(const std::string& name, T& value):
                m_name(name),
                m_value(value),
                m_type_index(typeid(T))
            { }

            virtual ~PropertyNode() = default;

            virtual const std::string& getName() const noexcept override {
                return m_name;
            }

            virtual Node::Type getType() const noexcept override {
                return Node::Type::PROPERTY;
            }

            virtual Maybe<std::any> getAnyValue() const noexcept override {
                return m_value;
            }

            virtual Maybe<std::any> getAnyValue() noexcept override {
                return m_value;
            }

            virtual Maybe<std::type_index> getTypeInfo() const noexcept override
            {
                return m_type_index;
            }

            /**
             * @brief Sets the value that this node is representing
             *
             * @return STATUS_SUCCESS on success, STATUS_INVALID_TYPE if 'value'
             *         does not hold a T
             */
            virtual MaybeVoid setValue(const std::any& value) noexcept override
            {
                if constexpr(!std::is_const_v<T>) {
                    try {
                        m_value = std::any_cast<T>(value);
                    } catch(const std::bad_any_cast& e) {
                        WRITE_ERROR("Invalid type requested for value.");
                        RETURN_ERROR(STATUS_INVALID_TYPE);
                    }
                }

                return STATUS_SUCCESS;
            }

            virtual bool canSetValue() const noexcept override {
                return true;
            }

            virtual bool hasValue() const noexcept override {
                return true;
            }

        protected:
            //! The name of this property
            std::string m_name;

            //! A reference to the value this node represents
            T& m_value;

            //! Information about the type stored in this node
            std::type_index m_type_index;
    };

    /**
     * @brief A representation of a const property. Cannot be set, only read.
     *
     * @tparam T The type of property
     */
    template<typename T>
    class ConstPropertyNode: public PropertyNode<const T> {
        public:
            /**
             * @brief Builds this property
             *
             * @param name The name of the property
             * @param value A reference to the value this property represents
             */
            ConstPropertyNode(const std::string& name, const T& value):
                PropertyNode<const T>::PropertyNode(name, value)
            { }

            ConstPropertyNode(const std::string& name):
                PropertyNode<const T>(name, name)
            { }

            virtual Node::Type getType() const noexcept override {
                return Node::Type::CONST_PROPERTY;
            }

            virtual MaybeVoid setValue(const std::any& value) noexcept override
            {
                return STATUS_SUCCESS;
            }

            virtual bool canSetValue() const noexcept override {
                return false;
            }
    };
}

#endif

