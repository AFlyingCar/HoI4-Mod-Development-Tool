
#include "LinkNode.h"

/**
 * @brief Builds a LinkNode
 *
 * @param name The name of this node
 * @param resolution_check A function which checks if a given node can be used
 *                         to resolve this link
 * @param link_search A function which searches from a given root for a node
 *                    that should resolve this link
 */
HMDT::Project::Hierarchy::LinkNode::LinkNode(const std::string& name,
                                             ResolutionCheck resolution_check,
                                             LinkSearch link_search):
    m_name(name),
    m_resolution_check(resolution_check),
    m_link_search(link_search),
    m_cached_link(nullptr)
{ }

/**
 * @brief Gets the type of LinkNode
 *
 * @return Node::Type::LINK
 */
auto HMDT::Project::Hierarchy::LinkNode::getType() const noexcept -> Type {
    return Node::Type::LINK;
}

/**
 * @brief Gets the name of this node
 *
 * @return This node's name
 */
const std::string& HMDT::Project::Hierarchy::LinkNode::getName() const noexcept
{
    return m_name;
}

/**
 * @brief Gets the Node that this node links to
 * @details This function will return null if the link has not yet been resolved
 *
 * @return The linked node
 */
auto HMDT::Project::Hierarchy::LinkNode::getLinkedNode() const noexcept
    -> ConstLinkedNode
{
    return m_cached_link;
}

/**
 * @brief Gets the Node that this node links to
 * @details This function will return null if the link has not yet been resolved
 *
 * @return The linked node
 */
auto HMDT::Project::Hierarchy::LinkNode::getLinkedNode() noexcept -> LinkedNode
{
    return m_cached_link;
}

/**
 * @brief Gets if the link node has been resolved
 *
 * @return True if the cached link is not null, false otherwise
 */
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

/**
 * @brief Attempts to resolve the link
 *
 * @param root The root of the hiearchy to search for the link to resole with
 *
 * @return STATUS_SUCCESS if resolution succeeded, or a failure code otherwise.
 */
auto HMDT::Project::Hierarchy::LinkNode::resolve(INodePtr root) noexcept
    -> MaybeVoid
{
    // Do a search starting from root for the value that should resolve this link
    MaybeVoid result = m_link_search(root)
        .andThen([this](auto search_result) -> MaybeVoid {
            // If a value was found, attempt to resolve the link
            if(!resolveLink(search_result)) {
                WRITE_ERROR("Failed to resolve link with search result of ",
                        std::to_string(*search_result));
                // TODO: We may want to return a better error code in this case?
                RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
            }

            return STATUS_SUCCESS;
        });
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

