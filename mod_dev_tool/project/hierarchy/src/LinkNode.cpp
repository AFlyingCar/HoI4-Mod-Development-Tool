
#include "LinkNode.h"

HMDT::Project::Hierarchy::LinkNode::LinkNode(const std::string& name,
                                             ResolutionCheck resolution_check):
    m_name(name),
    m_resolution_check(resolution_check),
    m_cached_link(nullptr)
{ }

auto HMDT::Project::Hierarchy::LinkNode::getType() const noexcept -> Type {
    return Node::Type::LINK;
}

const std::string& HMDT::Project::Hierarchy::LinkNode::getName() const noexcept
{
    return m_name;
}

auto HMDT::Project::Hierarchy::LinkNode::getLinkedNode() const noexcept
    -> ConstLinkedNode
{
    return m_cached_link;
}

auto HMDT::Project::Hierarchy::LinkNode::getLinkedNode() noexcept -> LinkedNode
{
    return m_cached_link;
}

bool HMDT::Project::Hierarchy::LinkNode::isLinkValid() const noexcept {
    return m_cached_link != nullptr;
}

/**
 * @brief Tries to resolve the link with the given node
 *
 * @param node The node to try to resolve with
 *
 * @return True if the link was resolved, false otherwise.
 */
bool HMDT::Project::Hierarchy::LinkNode::resolveLink(LinkedNode node) noexcept {
    if(m_resolution_check(node)) {
        m_cached_link = node;
        return true;
    }

    return false;
}

