
#include "INode.h"
#include "Util.h"

/**
 * @brief Builds a new INode iterator
 *
 * @param node The starting root node to iterate from
 */
template<typename NodePtr>
HMDT::Project::Hierarchy::INode::IteratorImpl<NodePtr>::IteratorImpl(NodePtr node):
    m_nodes()
{
    if(node != nullptr) {
        m_nodes.push(node);
    }
}

/**
 * @brief Increments the iterator by one
 *
 * @return This iterator
 */
template<typename NodePtr>
auto HMDT::Project::Hierarchy::INode::IteratorImpl<NodePtr>::operator++() -> IteratorImpl& {
    // Do not attempt to increment if we are at the end
    if(m_nodes.empty()) {
        return *this;
    }

    // Advance by 1
    advance();

    return *this;
}

/**
 * @brief Dereferences this iterator
 *
 * @return The current node
 */
template<typename NodePtr>
auto HMDT::Project::Hierarchy::INode::IteratorImpl<NodePtr>::operator*() const noexcept
    -> NodePtr
{
    return this->getNodeStack().top();
}

/**
 * @brief Dereferences this iterator
 *
 * @return The current node
 */
template<typename NodePtr>
auto HMDT::Project::Hierarchy::INode::IteratorImpl<NodePtr>::operator->() const noexcept
    -> NodePtr
{
    return this->getNodeStack().top();
}

/**
 * @brief Tests if this iterator is at the end.
 *
 * @return True if there are no nodes left, false otherwise.
 */
template<typename NodePtr>
bool HMDT::Project::Hierarchy::INode::IteratorImpl<NodePtr>::isEnd() const noexcept {
    return m_nodes.empty();
}

/**
 * @brief Advances the iterator by one node.
 * @details Performs one single step of a breadth-first
 *          search of the node tree. Will assume that
 *          m_nodes is not empty.
 */
template<typename NodePtr>
void HMDT::Project::Hierarchy::INode::IteratorImpl<NodePtr>::advance() noexcept {
    // _MKM_DEBUG_OUTPUT << "advance()" << std::endl;
    auto node = m_nodes.top();
    m_nodes.pop();

    // If the node is some sort of group node, then add all of its children to
    //   m_nodes. Use dynamic_pointer_cast instead of getType() as we want to do
    //   this for any subclass of IGroupNode, not just for GroupNodes.
    if(auto gnode = std::dynamic_pointer_cast<const IGroupNode>(node);
            gnode != nullptr)
    {
        auto&& children = gnode->getChildren();
        for(auto&& [_, child] : children) {
            m_nodes.push(child);
        }
    }
}

// Explicitly instantiate const and non-const iterators
template class HMDT::Project::Hierarchy::INode::IteratorImpl<HMDT::Project::Hierarchy::INodePtr>;
template class HMDT::Project::Hierarchy::INode::IteratorImpl<HMDT::Project::Hierarchy::ConstINodePtr>;

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
 * @brief Gets a const iterator to the start of the hierarchy
 */
auto HMDT::Project::Hierarchy::INode::begin() const noexcept -> ConstIterator {
    return ConstIterator(shared_from_this());
}

/**
 * @brief Gets a non-const iterator to the start of the hierarchy
 */
auto HMDT::Project::Hierarchy::INode::begin() noexcept -> Iterator {
    return Iterator(shared_from_this());
}

/**
 * @brief Gets a const iterator to the end of the hierarchy
 */
auto HMDT::Project::Hierarchy::INode::end() const noexcept -> ConstIterator {
    return ConstIterator();
}

/**
 * @brief Gets a non-const iterator to the end of the hierarchy
 */
auto HMDT::Project::Hierarchy::INode::end() noexcept -> Iterator {
    return Iterator();
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
        WRITE_ERROR("Could not find ", name, " in ", std::to_string(*this));
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

const std::vector<std::string>& HMDT::Project::Hierarchy::Key::getParts() const noexcept
{
    return m_parts;
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
 * @param include_address Whether to include the memory address of node
 *
 * @return A string representation of this INode
 */
std::string std::to_string(const HMDT::Project::Hierarchy::INode& node,
                           bool include_address)
{
    std::string s = node.getName();
    if(include_address) {
        s += " (" + std::to_string(&node) + ")";
    }

    return s + " [" + std::to_string(node.getType()) + "]";
}

std::string std::to_string(const HMDT::Project::Hierarchy::Key& key) {
    std::stringstream ss;
    for(auto&& p : key.getParts()) ss << p << ':';
    return ss.str();
}

