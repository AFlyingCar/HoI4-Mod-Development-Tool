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
            using ValueGetter = std::function<MaybeRef<T>()>;
            using ValueSetter = std::function<MaybeVoid(const T&)>;

            /**
             * @brief Builds this property
             *
             * @param name The name of the property
             * @param value A reference to the value this property represents
             */
            PropertyNode(const std::string& name,
                         const ValueGetter& getter,
                         const ValueSetter& setter):
                m_name(name),
                m_getter(getter),
                m_setter(setter),
                m_type_index(typeid(T))
            { }

            virtual ~PropertyNode() = default;

            /**
             * @brief Gets the name of this node
             *
             * @return The name of this node
             */
            virtual const std::string& getName() const noexcept override {
                return m_name;
            }

            /**
             * @brief Gets the type of PropertyNode
             *
             * @return Node::Type::PROPERTY
             */
            virtual Node::Type getType() const noexcept override {
                return Node::Type::PROPERTY;
            }

            /**
             * @brief Gets the value held by this property
             * @details The std::any returned will always be a
             *          reference_wrapper<T>
             *
             * @return The value held by this property
             */
            virtual Maybe<std::any> getAnyValue() const noexcept override {
                auto result = m_getter();
                RETURN_IF_ERROR(result);

                return refAsConstRef(*result);
            }

            /**
             * @brief Gets the value held by this property
             * @details Note that the Any returned here will _not_ contain a
             *          reference_wrapper<T>, but instead will only hold a T
             *
             * @return The value held by this property
             */
            virtual Maybe<std::any> getAnyValue() noexcept override {
                auto result = m_getter();
                RETURN_IF_ERROR(result);

                return result->get();
            }

            /**
             * @brief Gets the type info of the type held by this property
             *
             * @return The type info of the type held by this property
             */
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
                        auto result = m_setter(std::any_cast<T>(value));
                        RETURN_IF_ERROR(result);
                    } catch(const std::bad_any_cast& e) {
                        WRITE_ERROR("Invalid type requested for value.");
                        RETURN_ERROR(STATUS_INVALID_TYPE);
                    }
                }

                return STATUS_SUCCESS;
            }

            /**
             * @brief Checks if the value of this property can be set
             *
             * @return True
             */
            virtual bool canSetValue() const noexcept override {
                return true;
            }

            /**
             * @brief Checks if this property holds a value
             *
             * @return True
             */
            virtual bool hasValue() const noexcept override {
                return true;
            }

        protected:
            //! The name of this property
            std::string m_name;

            //! Function to get the raw value referred to by this node
            ValueGetter m_getter;

            //! Function to set the raw value referred to by this node
            ValueSetter m_setter;

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
            ConstPropertyNode(const std::string& name,
                              const typename PropertyNode<const T>::ValueGetter& getter):
                PropertyNode<const T>::PropertyNode(
                        name,
                        getter,
                        [](auto&&...) {
                            RETURN_ERROR(STATUS_NOT_IMPLEMENTED);
                        })
            { }

            /**
             * @brief Builds this property
             *
             * @param name The name of this property
             */
            ConstPropertyNode(const std::string& name):
                PropertyNode<const T>(name,
                        [this]() -> MaybeRef<const T> { return std::ref(this->getName()); },
                        [](auto&&...) {
                            RETURN_ERROR(STATUS_NOT_IMPLEMENTED);
                        })
            { }

            /**
             * @brief Gets the type of ConstPropertyNode
             *
             * @return Node::Type::CONST_PROPERTY
             */
            virtual Node::Type getType() const noexcept override {
                return Node::Type::CONST_PROPERTY;
            }

            /**
             * @brief Sets the value that this node is representing
             *
             * @return STATUS_NOT_IMPLEMENTED
             */
            virtual MaybeVoid setValue(const std::any& value) noexcept override
            {
                RETURN_ERROR(STATUS_NOT_IMPLEMENTED);
            }

            /**
             * @brief Checks if the value of this property can be set
             *
             * @return False
             */
            virtual bool canSetValue() const noexcept override {
                return false;
            }
    };
}

#endif

