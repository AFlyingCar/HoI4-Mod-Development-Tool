
#include "ProvinceNode.h"

#include <memory>

#include "PropertyNode.h"
#include "ProjectNode.h"
#include "LinkNode.h"
#include "StateNode.h"

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
auto HMDT::Project::Hierarchy::ProvinceNode::setID(ProvinceID& id,
                                                   const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto id_node = std::make_shared<PropertyNode<ProvinceID>>(ID, id);
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
auto HMDT::Project::Hierarchy::ProvinceNode::setColor(const Color& color,
                                                      const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto color_node = std::make_shared<ConstPropertyNode<Color>>(COLOR, color);
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
auto HMDT::Project::Hierarchy::ProvinceNode::setProvinceType(ProvinceType& province_type,
                                                             const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto prov_type_node = std::make_shared<PropertyNode<ProvinceType>>(TYPE, province_type);
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
auto HMDT::Project::Hierarchy::ProvinceNode::setCoastal(bool& coastal,
                                                        const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto coastal_node = std::make_shared<PropertyNode<bool>>(COASTAL, coastal);
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
auto HMDT::Project::Hierarchy::ProvinceNode::setTerrain(TerrainID& terrain_id,
                                                        const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto terrain_node = std::make_shared<PropertyNode<TerrainID>>(TERRAIN,
                                                                  terrain_id);
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
auto HMDT::Project::Hierarchy::ProvinceNode::setContinent(Continent& continent,
                                                          const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto continent_node = std::make_shared<PropertyNode<Continent>>(CONTINENT,
                                                                    continent);
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
auto HMDT::Project::Hierarchy::ProvinceNode::setState(const StateID& state_id,
                                                      const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
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
            Key key{ "History", "States", "States", std::to_string(state_id) };
            auto node = key.lookup(root);
            RETURN_IF_ERROR(node);

            RETURN_ERROR_IF((*node)->getType() != Node::Type::STATE,
                            STATUS_INVALID_TYPE);

            return node;
        }
        );
    visitor(state_link_node);

    auto result = addChild(state_link_node);
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
    return (*this)[ID].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
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
    return (*this)[COLOR].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
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
    return (*this)[TYPE].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
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
    return (*this)[COASTAL].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
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
    return (*this)[TERRAIN].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
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
    return (*this)[CONTINENT].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
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
    return (*this)[STATE].andThen<std::shared_ptr<const ILinkNode>>([](auto node)
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
    return (*this)[ADJACENT_PROVINCES].andThen<std::shared_ptr<const IGroupNode>>([](auto node)
        -> std::shared_ptr<const IGroupNode>
        {
            return std::dynamic_pointer_cast<const IGroupNode>(node);
        });
}

