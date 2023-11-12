/**
 * @file INode.h 
 * @brief Defines the interfaces for a project hierarchy structure.
 */

#ifndef PROJECT_HIERARCHY_INODE_H
# define PROJECT_HIERARCHY_INODE_H

# include <any>
# include <map>
# include <string>
# include <memory>
# include <typeindex>

# include "Maybe.h"
# include "StatusCodes.h"

# include "Logger.h"

namespace HMDT::Project::Hierarchy {
    // This namespace exists for public usage of the Type class
    namespace Node {
        /**
         * @brief All possible node types
         */
        enum class Type {
# include "NodeTypes.def"
        };
    }

    class INode;

    //! Helper alias for an INode pointer
    using INodePtr = std::shared_ptr<INode>;

    /**
     * @brief Helper alias for a node's visitor function
     *
     * @tparam T The expected return type
     */
    template<typename T = void>
    using NodeVisitor = std::function<Maybe<T>(INodePtr)>;

    //! A specialized helper alias for INode visitor
    using INodeVisitor = NodeVisitor<void>;

    /**
     * @brief Base class for all nodes
     */
    class INode: public std::enable_shared_from_this<INode> {
        protected:
            //! Helper alias
            using Type = Node::Type;

        public:
            INode() = default;
            INode(const INode&) = delete;

            virtual ~INode() = default;

            virtual Type getType() const noexcept = 0;
            virtual const std::string& getName() const noexcept = 0;

            virtual MaybeVoid visit(INodeVisitor) noexcept;
    };

    /**
     * @brief Defines a node which is used to refer to another
     */
    class ILinkNode: public INode {
        public:
            //! Helper const alias for the linked node
            using ConstLinkedNode = std::shared_ptr<const INode>;

            //! Helper alias for the linked node
            using LinkedNode = std::shared_ptr<INode>;

            ILinkNode() = default;
            ILinkNode(const ILinkNode&) = delete;

            virtual ConstLinkedNode getLinkedNode() const noexcept = 0;
            virtual LinkedNode getLinkedNode() noexcept = 0;

            virtual bool isLinkValid() const noexcept = 0;

            virtual bool resolveLink(LinkedNode) noexcept = 0;

            virtual MaybeVoid resolve(INodePtr) noexcept = 0;
    };

    /**
     * @brief Defines a node which contains multiple other nodes
     */
    class IGroupNode: public INode {
        public:
            //! Helper alias to refer to the child node
            using ChildNode = std::shared_ptr<INode>;

            //! Helper const alias to refer to the child node
            using ConstChildNode = std::shared_ptr<const INode>;

            //! Helper alias for the children of this group
            using Children = std::unordered_map<std::string, ChildNode>;

            IGroupNode() = default;
            IGroupNode(const IGroupNode&) = delete;

            virtual ~IGroupNode() = default;

            virtual MaybeVoid visit(INodeVisitor) noexcept override;

            virtual const Children& getChildren() const noexcept = 0;
            virtual Children getChildren() noexcept = 0;

            Maybe<ConstChildNode> operator[](const std::string&) const noexcept;
            Maybe<ChildNode> operator[](const std::string&) noexcept;

            Maybe<ConstChildNode> getChild(const std::string&) const noexcept;
            Maybe<ChildNode> getChild(const std::string&) noexcept;
    };

    /**
     * @brief Defines a node which holds a single property
     */
    class IPropertyNode: public INode {
        public:
            IPropertyNode() = default;
            IPropertyNode(const IPropertyNode&) = delete;

            /**
             * @brief Gets the value referred to by this property
             *
             * @tparam T The type to get the value as
             *
             * @return A Maybe containing the value held by this property, or an
             *         error code if a failure occurs
             */
            template<typename T>
            MaybeRef<const T> getValue() const noexcept {
                auto result = getAnyValue();
                RETURN_IF_ERROR(result);

                result = result.andThen([](const std::any& v) -> MaybeVoid {
                    try {
                        return std::any_cast<T>(v);
                    } catch(const std::bad_any_cast& e) {
                        WRITE_ERROR("Invalid type requested for value.");
                        RETURN_ERROR(STATUS_INVALID_TYPE);
                    }
                });
                RETURN_IF_ERROR(result);

                return STATUS_SUCCESS;
            }

            /**
             * @brief Gets the value referred to by this property
             *
             * @tparam T The type to get the value as
             *
             * @return A Maybe containing the value held by this property, or an
             *         error code if a failure occurs
             */
            template<typename T>
            Maybe<T> getValue() noexcept {
                auto result = getAnyValue();
                RETURN_IF_ERROR(result);

                result = result.andThen([](const std::any& v) -> MaybeVoid {
                    try {
                        return std::any_cast<T>(v);
                    } catch(const std::bad_any_cast& e) {
                        WRITE_ERROR("Invalid type requested for value.");
                        RETURN_ERROR(STATUS_INVALID_TYPE);
                    }
                });
                RETURN_IF_ERROR(result);

                return STATUS_SUCCESS;
            }

            /**
             * @brief Checks if this property's value matches the given value
             *
             * @tparam T The type of the right hand side value
             * @param other The value to compare against
             *
             * @return True if the value held by this property matches 'other',
             *         false otherwise.
             */
            template<typename T>
            bool operator==(const T& other) const noexcept {
                auto any_value = getAnyValue();

                if(!IS_FAILURE(any_value)) {
                    try {
                        return std::any_cast<T>(*any_value) == other;
                    } catch(const std::bad_any_cast& e) {
                        WRITE_ERROR("Invalid type requested for value.");
                    }
                }

                return false;
            }


            /**
             * @brief Checks if this property's value doesn't match the given value
             *
             * @tparam T The type of the right hand side value
             * @param other The value to compare against
             *
             * @return True if the value held by this property doesn't match 'other',
             *         false otherwise.
             */
            template<typename T>
            bool operator!=(const T& other) const noexcept {
                return !(operator==(other));
            }

            virtual Maybe<std::any> getAnyValue() const noexcept = 0;
            virtual Maybe<std::any> getAnyValue() noexcept = 0;
            virtual Maybe<std::type_index> getTypeInfo() const noexcept = 0;

            virtual MaybeVoid setValue(const std::any&) noexcept = 0;

            virtual bool canSetValue() const noexcept = 0;

            /**
             * @brief Checks if this property holds a value
             *
             * @return True if this property holds a value, false otherwise
             */
            virtual bool hasValue() const noexcept {
                return getAnyValue().has_value();
            }
    };

    /**
     * @brief Key for looking up nodes in the tree
     */
    class Key {
        public:
            Key(const std::vector<std::string>&);
            Key(std::initializer_list<std::string>);

            Maybe<INodePtr> lookup(INodePtr) const noexcept;

        private:
            //! The parts of this key used for looking up a node
            std::vector<std::string> m_parts;
    };
}

namespace std {
    string to_string(const HMDT::Project::Hierarchy::Node::Type&);
    string to_string(const HMDT::Project::Hierarchy::INode&);
}

#endif

