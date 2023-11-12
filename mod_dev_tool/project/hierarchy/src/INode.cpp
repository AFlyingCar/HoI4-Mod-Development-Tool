
#include "INode.h"
#include "Util.h"

/**
 * @brief Visits this INode object
 *
 * @param visitor The visitor callback
 *
 * @return A status code
 */
auto HMDT::Project::Hierarchy::INode::visit(INodeVisitor visitor) noexcept
    -> MaybeVoid
{
    auto result = visitor(shared_from_this());
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

/**
 * @brief Visits this IGroupNode, and then visits every child node
 *
 * @param visitor The visitor callback
 *
 * @return A status code
 */
auto HMDT::Project::Hierarchy::IGroupNode::visit(INodeVisitor visitor) noexcept
    -> MaybeVoid
{
    auto result = INode::visit(visitor);
    RETURN_IF_ERROR(result);

    const Children& children = getChildren();
    for(auto&& [_, child] : children) {
        result = child->visit(visitor);
        RETURN_IF_ERROR(result);
    }

    return STATUS_SUCCESS;
}

/**
 * @brief Looks up a node held by this group
 *
 * @param name The name to use for lookup
 *
 * @return A Maybe containing the node to lookup, or VALUE_NOT_FOUND if it
 *         doesn't exist
 */
auto HMDT::Project::Hierarchy::IGroupNode::operator[](const std::string& name) const noexcept
    -> Maybe<ConstChildNode>
{
    if(getChildren().count(name) != 0) {
        return getChildren().at(name);
    } else {
        RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
    }
}

/**
 * @brief Looks up a node held by this group
 *
 * @param name The name to use for lookup
 *
 * @return A Maybe containing the node to lookup, or VALUE_NOT_FOUND if it
 *         doesn't exist
 */
auto HMDT::Project::Hierarchy::IGroupNode::operator[](const std::string& name) noexcept
    -> Maybe<ChildNode>
{
    if(getChildren().count(name) != 0) {
        return getChildren().at(name);
    } else {
        RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
    }
}

/**
 * @brief Looks up a node held by this group
 *
 * @param name The name to use for lookup
 *
 * @return A Maybe containing the node to lookup, or VALUE_NOT_FOUND if it
 *         doesn't exist
 */
auto HMDT::Project::Hierarchy::IGroupNode::getChild(const std::string& name) const noexcept
    -> Maybe<ConstChildNode>
{
    return (*this)[name];
}

/**
 * @brief Looks up a node held by this group
 *
 * @param name The name to use for lookup
 *
 * @return A Maybe containing the node to lookup, or VALUE_NOT_FOUND if it
 *         doesn't exist
 */
auto HMDT::Project::Hierarchy::IGroupNode::getChild(const std::string& name) noexcept
    -> Maybe<ChildNode>
{
    return (*this)[name];
}

/**
 * @brief Builds a new Key
 *
 * @param parts The parts of the key
 */
HMDT::Project::Hierarchy::Key::Key(const std::vector<std::string>& parts):
    m_parts(parts)
{ }

/**
 * @brief Builds a new Key
 *
 * @param parts The parts of the key
 */
HMDT::Project::Hierarchy::Key::Key(std::initializer_list<std::string> parts):
    m_parts(parts)
{ }

/**
 * @brief Looks up a node using this key starting from the given root.
 *
 * @param root The root to start searching from
 *
 * @return The node that matches this key.
 */
auto HMDT::Project::Hierarchy::Key::lookup(INodePtr root) const noexcept
    -> Maybe<INodePtr>
{
    for(auto&& part : m_parts) {
        RETURN_ERROR_IF(root->getType() != Node::Type::PROJECT &&
                        root->getType() != Node::Type::GROUP &&
                        root->getType() != Node::Type::STATE &&
                        root->getType() != Node::Type::PROVINCE,
                        STATUS_INVALID_TYPE);
        auto result = std::dynamic_pointer_cast<IGroupNode>(root)->getChild(part);
        RETURN_IF_ERROR(result);
        root = *result;
    }

    return root;
}

/**
 * @brief Converts Type enum to string
 *
 * @param type The type to convert
 *
 * @return A string representation of 'type'
 */
std::string std::to_string(const HMDT::Project::Hierarchy::Node::Type& type) {
    switch(type) {
        case HMDT::Project::Hierarchy::Node::Type::GROUP:
            return "Group";
        case HMDT::Project::Hierarchy::Node::Type::PROJECT:
            return "Project";
        case HMDT::Project::Hierarchy::Node::Type::PROPERTY:
            return "Property";
        case HMDT::Project::Hierarchy::Node::Type::CONST_PROPERTY:
            return "ConstProperty";
        case HMDT::Project::Hierarchy::Node::Type::PROVINCE:
            return "Province";
        case HMDT::Project::Hierarchy::Node::Type::STATE:
            return "State";
        case HMDT::Project::Hierarchy::Node::Type::LINK:
            return "Link";
    }

    UNREACHABLE();
}

/**
 * @brief Converts an INode to a string
 * @details Will generate a string representation in the format of
 *          <tt>NAME (ADDRESS) [NODETYPE]</tt>
 *
 * @param node The node to convert
 *
 * @return A string representation of this INode
 */
std::string std::to_string(const HMDT::Project::Hierarchy::INode& node) {
    return node.getName() + " (" + std::to_string(&node) + ") [" + std::to_string(node.getType()) + "]";
}

