
#include "GroupNode.h"

HMDT::Project::Hierarchy::GroupNode::GroupNode(const std::string& name):
    m_name(name)
{ }

auto HMDT::Project::Hierarchy::GroupNode::getChildren() const noexcept
    -> const Children&
{
    return m_children;
}

auto HMDT::Project::Hierarchy::GroupNode::getName() const noexcept
    -> const std::string&
{
    return m_name;
}

auto HMDT::Project::Hierarchy::GroupNode::getType() const noexcept -> Type {
    return Type::GROUP;
}

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

