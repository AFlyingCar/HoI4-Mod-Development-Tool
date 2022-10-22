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

# include "Maybe.h"
# include "StatusCodes.h"

# include "Logger.h"

namespace HMDT::Project::Hierarchy {
    // This namespace exists for public usage of the Type class
    namespace Node {
        enum class Type {
# include "NodeTypes.def"
        };
    }

    class INode {
        protected:
            using Type = Node::Type;

        public:
            virtual ~INode() = default;

            virtual Type getType() const noexcept = 0;
            virtual const std::string& getName() const noexcept = 0;
    };

    class ILinkNode: public INode {
        public:
            using ConstLinkedNode = std::shared_ptr<const INode>;
            using LinkedNode = std::shared_ptr<INode>;

            virtual ConstLinkedNode getLinkedNode() const noexcept = 0;
            virtual LinkedNode getLinkedNode() noexcept = 0;

            virtual bool isLinkValid() const noexcept = 0;

            virtual bool resolveLink(LinkedNode) noexcept = 0;
    };

    class IGroupNode: public INode {
        public:
            using ChildNode = std::shared_ptr<INode>;
            using ConstChildNode = std::shared_ptr<const INode>;
            using Children = std::unordered_map<std::string, ChildNode>;

            virtual ~IGroupNode() = default;

            virtual const Children& getChildren() const noexcept = 0;

            Maybe<ConstChildNode> operator[](const std::string&) const noexcept;
            Maybe<ChildNode> operator[](const std::string&) noexcept;

        protected:
            virtual Children getChildren() noexcept = 0;
    };

    class IPropertyNode: public INode {
        public:
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

            template<typename T>
            bool operator!=(const T& other) const noexcept {
                return !(operator==(other));
            }

            virtual Maybe<std::any> getAnyValue() const noexcept = 0;
            virtual Maybe<std::any> getAnyValue() noexcept = 0;

            virtual MaybeVoid setValue(const std::any&) noexcept = 0;

            virtual bool canSetValue() const noexcept = 0;
    };
}

#endif

