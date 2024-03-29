
#include "ProvinceNode.h"

#include <memory>

#include "PropertyNode.h"
#include "ProjectNode.h"
#include "LinkNode.h"
#include "StateNode.h"
#include "NodeKeyNames.h"

/**
 * @brief Gets the type of ProvinceNode
 *
 * @return Node::Type::PROVINCE
 */
auto HMDT::Project::Hierarchy::ProvinceNode::getType() const noexcept -> Type {
    return Type::PROVINCE;
}


/**
 * @brief Sets the ID property of this node
 *
 * @param id The value to set
 * @param visitor The visitor callback
 *
 * @return A status code
 */
auto HMDT::Project::Hierarchy::ProvinceNode::setID(const IPropertyNode::ValueLookup<ProvinceID>& lookup,
                                                   const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto id_node = std::make_shared<PropertyNode<ProvinceID>>(ProvinceKeys::ID,
            lookup,
            [lookup](const ProvinceID& id) -> MaybeVoid {
                auto result = lookup();
                RETURN_IF_ERROR(result);
                result->get() = id;
                return STATUS_SUCCESS;
            });
    visitor(id_node);

    auto result = addChild(id_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

/**
 * @brief Sets the color property of this node
 *
 * @param color The value to set
 * @param visitor The vistor callback
 *
 * @return A status code
 */
auto HMDT::Project::Hierarchy::ProvinceNode::setColor(const IPropertyNode::ValueLookup<const Color>& lookup,
                                                      const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto color_node = std::make_shared<ConstPropertyNode<Color>>(ProvinceKeys::COLOR, 
            lookup);
    visitor(color_node);

    auto result = addChild(color_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

/**
 * @brief Sets the ProvinceType property of this node
 *
 * @param province_type The value to set
 * @param visitor The vistor callback
 *
 * @return A status code
 */
auto HMDT::Project::Hierarchy::ProvinceNode::setProvinceType(const IPropertyNode::ValueLookup<ProvinceType>& lookup,
                                                             const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto prov_type_node = std::make_shared<PropertyNode<ProvinceType>>(ProvinceKeys::TYPE,
            lookup,
            [lookup](const auto& province_type) -> MaybeVoid {
                auto result = lookup();
                RETURN_IF_ERROR(result);
                result->get() = province_type;
                return STATUS_SUCCESS;
            });
    visitor(prov_type_node);

    auto result = addChild(prov_type_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

/**
 * @brief Sets the coastal property of this node
 *
 * @param coastal The value to set
 * @param visitor The vistor callback
 *
 * @return A status code
 */
auto HMDT::Project::Hierarchy::ProvinceNode::setCoastal(const IPropertyNode::ValueLookup<bool>& lookup,
                                                        const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto coastal_node = std::make_shared<PropertyNode<bool>>(ProvinceKeys::COASTAL,
            lookup,
            [lookup](const auto& coastal) -> MaybeVoid {
                auto result = lookup();
                RETURN_IF_ERROR(result);
                result->get() = coastal;
                return STATUS_SUCCESS;
            });
    visitor(coastal_node);

    auto result = addChild(coastal_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

/**
 * @brief Sets the terrain_id property of this node
 *
 * @param terrain_id The value to set
 * @param visitor The vistor callback
 *
 * @return A status code
 */
auto HMDT::Project::Hierarchy::ProvinceNode::setTerrain(const IPropertyNode::ValueLookup<TerrainID>& lookup,
                                                        const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto terrain_node = std::make_shared<PropertyNode<TerrainID>>(ProvinceKeys::TERRAIN,
            lookup,
            [lookup](const auto& terrain_id) -> MaybeVoid {
                auto result = lookup();
                RETURN_IF_ERROR(result);
                result->get() = terrain_id;
                return STATUS_SUCCESS;
            });
    visitor(terrain_node);

    auto result = addChild(terrain_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

/**
 * @brief Sets the continent property of this node
 *
 * @param continent The value to set
 * @param visitor The vistor callback
 *
 * @return A status code
 */
auto HMDT::Project::Hierarchy::ProvinceNode::setContinent(const IPropertyNode::ValueLookup<Continent>& lookup,
                                                          const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto continent_node = std::make_shared<PropertyNode<Continent>>(ProvinceKeys::CONTINENT,
            lookup,
            [lookup](const auto& continent) -> MaybeVoid {
                auto result = lookup();
                RETURN_IF_ERROR(result);
                result->get() = continent;
                return STATUS_SUCCESS;
            });
    visitor(continent_node);

    auto result = addChild(continent_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

/**
 * @brief Sets the state_id property of this node
 *
 * @param state_id The value to set
 * @param visitor The vistor callback
 *
 * @return A status code
 */
auto HMDT::Project::Hierarchy::ProvinceNode::setState(const IPropertyNode::ValueLookup<StateID>& lookup,
                                                      const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    // TODO: This can all change, so the link nodes will need to be dynamic
    //   enough to "re-discover" their new link.
    // This whole function is disabled for now though, so leave it like this to
    //   make sure it compiles
    auto result = lookup();
    RETURN_IF_ERROR(result);
    auto state_id = *result;

    auto state_link_node = std::make_shared<LinkNode>(
        std::to_string(state_id),
        [state_id](ILinkNode::LinkedNode node) -> bool {
            if(node->getType() == Node::Type::STATE) {
                auto id_node = std::dynamic_pointer_cast<StateNode>(node)->getIDProperty();
                if(IS_FAILURE(id_node)) return false;

                return (**id_node == state_id);
            }

            return false;
        },
        [state_id](INodePtr root) -> Maybe<ILinkNode::LinkedNode> {
            // Get Root[Project]->History[Project]->States[Project]->States[Group]->ID

            // TODO: Make magic strings here constants
            Key key{
                ProjectKeys::HISTORY,
                ProjectKeys::STATES,
                GroupKeys::STATES,
                std::to_string(state_id)
            };
            auto node = key.lookup(root);
            RETURN_IF_ERROR(node);

            RETURN_ERROR_IF((*node)->getType() != Node::Type::STATE,
                            STATUS_INVALID_TYPE);

            return node;
        }
        );
    visitor(state_link_node);

    result = addChild(state_link_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

/**
 * @brief Sets the adjacent provinces property of this node
 *
 * @param adj_provinces The value to set
 * @param visitor The vistor callback
 *
 * @return A status code
 */
auto HMDT::Project::Hierarchy::ProvinceNode::setAdjacentProvinces(const std::set<ProvinceID>& adj_provinces,
                                                                  const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    return STATUS_NOT_IMPLEMENTED;
}

/**
 * @brief Gets the id property
 *
 * @return A Maybe containing the id property, or an error code if it's not found
 */
auto HMDT::Project::Hierarchy::ProvinceNode::getIDProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>>
{
    return (*this)[ProvinceKeys::ID].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> Maybe<std::shared_ptr<const IPropertyNode>>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

/**
 * @brief Gets the color property
 *
 * @return A Maybe containing the color property, or an error code if it's not found
 */
auto HMDT::Project::Hierarchy::ProvinceNode::getColorProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>>
{
    return (*this)[ProvinceKeys::COLOR].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> std::shared_ptr<const IPropertyNode>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

/**
 * @brief Gets the province type property
 *
 * @return A Maybe containing the province type property, or an error code if it's not found
 */
auto HMDT::Project::Hierarchy::ProvinceNode::getProvinceType() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>>
{
    return (*this)[ProvinceKeys::TYPE].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> std::shared_ptr<const IPropertyNode>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

/**
 * @brief Gets the coastal property
 *
 * @return A Maybe containing the coastal property, or an error code if it's not found
 */
auto HMDT::Project::Hierarchy::ProvinceNode::getCoastalProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>>
{
    return (*this)[ProvinceKeys::COASTAL].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> std::shared_ptr<const IPropertyNode>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

/**
 * @brief Gets the terrain property
 *
 * @return A Maybe containing the terrain property, or an error code if it's not found
 */
auto HMDT::Project::Hierarchy::ProvinceNode::getTerrainProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>>
{
    return (*this)[ProvinceKeys::TERRAIN].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> std::shared_ptr<const IPropertyNode>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

/**
 * @brief Gets the continent property
 *
 * @return A Maybe containing the continent property, or an error code if it's not found
 */
auto HMDT::Project::Hierarchy::ProvinceNode::getContinentProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>>
{
    return (*this)[ProvinceKeys::CONTINENT].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> std::shared_ptr<const IPropertyNode>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

/**
 * @brief Gets the state property
 *
 * @return A Maybe containing the state property, or an error code if it's not found
 */
auto HMDT::Project::Hierarchy::ProvinceNode::getStateProperty() const noexcept
    -> Maybe<std::shared_ptr<const ILinkNode>>
{
    return (*this)[ProvinceKeys::STATE].andThen<std::shared_ptr<const ILinkNode>>([](auto node)
        -> std::shared_ptr<const ILinkNode>
        {
            return std::dynamic_pointer_cast<const ILinkNode>(node);
        });
}

/**
 * @brief Gets the adjacent provinces property
 *
 * @return A Maybe containing the adjacent provinces property, or an error code if it's not found
 */
auto HMDT::Project::Hierarchy::ProvinceNode::getAdjacentProvincesProperty() const noexcept
    -> Maybe<std::shared_ptr<const IGroupNode>>
{
    return (*this)[ProvinceKeys::ADJACENT_PROVINCES].andThen<std::shared_ptr<const IGroupNode>>([](auto node)
        -> std::shared_ptr<const IGroupNode>
        {
            return std::dynamic_pointer_cast<const IGroupNode>(node);
        });
}

