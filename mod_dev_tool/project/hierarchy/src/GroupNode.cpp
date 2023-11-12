
#include "GroupNode.h"

/**
 * @brief Builds a new group node
 *
 * @param name The name of this node
 */
HMDT::Project::Hierarchy::GroupNode::GroupNode(const std::string& name):
    m_name(name)
{ }

/**
 * @brief Gets the collection of nodes held in this group
 *
 * @return The collection of nodes held in this group
 */
auto HMDT::Project::Hierarchy::GroupNode::getChildren() const noexcept
    -> const Children&
{
    return m_children;
}

/**
 * @brief Gets the name of this node
 *
 * @return The name of this group
 */
auto HMDT::Project::Hierarchy::GroupNode::getName() const noexcept
    -> const std::string&
{
    return m_name;
}

/**
 * @brief Gets the type of GroupNode
 *
 * @return Node::Type::GROUP
 */
auto HMDT::Project::Hierarchy::GroupNode::getType() const noexcept -> Type {
    return Type::GROUP;
}

/**
 * @brief Gets the collection of nodes held in this group
 *
 * @return The collection of nodes held in this group
 */
auto HMDT::Project::Hierarchy::GroupNode::getChildren() noexcept -> Children
{
    return m_children;
}

/**
 * @brief Adds a child to this Group.
 *
 * @param node The child node
 *
 * @return STATUS_SUCCESS on success, STATUS_KEY_EXISTS if a node with the same
 *         name as 'node' already exists, and STATUS_PARAM_CANNOT_BE_NULL if
 *         node is null.
 */
auto HMDT::Project::Hierarchy::GroupNode::addChild(ChildNode node) noexcept
    -> MaybeVoid
{
    if(node == nullptr) {
        RETURN_ERROR(STATUS_PARAM_CANNOT_BE_NULL);
    }

    return addChild(node->getName(), node);
}

/**
 * @brief Adds a child to this Group.
 *
 * @param name The name of the new child node
 * @param node The child node
 *
 * @return STATUS_SUCCESS on success, STATUS_KEY_EXISTS if a node already exists
 *         under the given name, and STATUS_PARAM_CANNOT_BE_NULL if node is
 *         null.
 */
auto HMDT::Project::Hierarchy::GroupNode::addChild(const std::string& name,
                                                        ChildNode node) noexcept
    -> MaybeVoid
{
    if(node == nullptr) {
        RETURN_ERROR(STATUS_PARAM_CANNOT_BE_NULL);
    }

    if(m_children.count(name) == 0) {
        m_children[name] = node;
        return STATUS_SUCCESS;
    } else {
        RETURN_ERROR(STATUS_KEY_EXISTS);
    }
}

/**
 * @brief Sets a child node stored in this group
 *
 * @param name The name of the new child node
 * @param node The new child node
 *
 * @return STATUS_SUCCESS on success, STATUS_KEY_NOT_FOUND if a node does not
 *         already exist under the given name, and STATUS_PARAM_CANNOT_BE_NULL
 *         if node is null.
 */
auto HMDT::Project::Hierarchy::GroupNode::setChild(const std::string& name,
                                                   ChildNode node) noexcept
    -> MaybeVoid
{
    auto result = removeChild(name);
    RETURN_IF_ERROR(result);

    result = addChild(name, node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

/**
 * @brief Removes a child node stored in this group
 *
 * @param name The name of the child node
 *
 * @return STATUS_SUCCESS on success, STATUS_KEY_NOT_FOUND if a node does not
 *         already exist under the given name.
 */
auto HMDT::Project::Hierarchy::GroupNode::removeChild(const std::string& name) noexcept
    -> MaybeVoid
{
    if(m_children.count(name) == 0) {
        RETURN_ERROR(STATUS_KEY_NOT_FOUND);
    }

    m_children.erase(name);

    return STATUS_SUCCESS;
}

