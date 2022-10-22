
#include "ProvinceNode.h"

#include <memory>

#include "PropertyNode.h"
#include "LinkNode.h"
#include "StateNode.h"

auto HMDT::Project::Hierarchy::ProvinceNode::getType() const noexcept -> Type {
    return Type::PROVINCE;
}

auto HMDT::Project::Hierarchy::ProvinceNode::setID(ProvinceID& id) noexcept
    -> MaybeVoid
{
    auto id_node = std::make_shared<PropertyNode<uint32_t>>(ID, id);

    auto result = addChild(id_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

auto HMDT::Project::Hierarchy::ProvinceNode::setColor(const Color& color) noexcept
    -> MaybeVoid
{
    auto color_node = std::make_shared<ConstPropertyNode<Color>>(COLOR, color);

    auto result = addChild(color_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

auto HMDT::Project::Hierarchy::ProvinceNode::setProvinceType(ProvinceType& province_type) noexcept
    -> MaybeVoid
{
    auto prov_type_node = std::make_shared<PropertyNode<ProvinceType>>(TYPE, province_type);

    auto result = addChild(prov_type_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

auto HMDT::Project::Hierarchy::ProvinceNode::setCoastal(bool& coastal) noexcept
    -> MaybeVoid
{
    auto coastal_node = std::make_shared<PropertyNode<bool>>(COASTAL, coastal);

    auto result = addChild(coastal_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

auto HMDT::Project::Hierarchy::ProvinceNode::setTerrain(TerrainID& terrain_id) noexcept
    -> MaybeVoid
{
    auto terrain_node = std::make_shared<PropertyNode<TerrainID>>(TERRAIN,
                                                                  terrain_id);

    auto result = addChild(terrain_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

auto HMDT::Project::Hierarchy::ProvinceNode::setContinent(Continent& continent) noexcept
    -> MaybeVoid
{
    auto continent_node = std::make_shared<PropertyNode<Continent>>(CONTINENT,
                                                                    continent);

    auto result = addChild(continent_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

auto HMDT::Project::Hierarchy::ProvinceNode::setState(State& state) noexcept
    -> MaybeVoid
{
    auto state_id = state.id;

    auto province_link_node = std::make_shared<LinkNode>(
        state.name,
        [state_id](ILinkNode::LinkedNode node) -> bool {
            if(node->getType() == Node::Type::STATE) {
                auto id_node = std::dynamic_pointer_cast<StateNode>(node)->getIDProperty();
                if(IS_FAILURE(id_node)) return false;

                return (**id_node == state_id);
            }

            return false;
        });

    return STATUS_SUCCESS;
}

auto HMDT::Project::Hierarchy::ProvinceNode::setAdjacentProvinces(const std::set<ProvinceID>& adj_provinces) noexcept
    -> MaybeVoid
{
    return STATUS_NOT_IMPLEMENTED;
}

auto HMDT::Project::Hierarchy::ProvinceNode::getIDProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>>
{
    return (*this)[ID].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> Maybe<std::shared_ptr<const IPropertyNode>>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

auto HMDT::Project::Hierarchy::ProvinceNode::getColorProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>>
{
    return (*this)[COLOR].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> std::shared_ptr<const IPropertyNode>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

auto HMDT::Project::Hierarchy::ProvinceNode::getProvinceType() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>>
{
    return (*this)[TYPE].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> std::shared_ptr<const IPropertyNode>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

auto HMDT::Project::Hierarchy::ProvinceNode::getCoastalProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>>
{
    return (*this)[COASTAL].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> std::shared_ptr<const IPropertyNode>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

auto HMDT::Project::Hierarchy::ProvinceNode::getTerrainProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>>
{
    return (*this)[TERRAIN].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> std::shared_ptr<const IPropertyNode>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

auto HMDT::Project::Hierarchy::ProvinceNode::getContinentProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>>
{
    return (*this)[CONTINENT].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> std::shared_ptr<const IPropertyNode>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

auto HMDT::Project::Hierarchy::ProvinceNode::getStateProperty() const noexcept
    -> Maybe<std::shared_ptr<const ILinkNode>>
{
    return (*this)[STATE].andThen<std::shared_ptr<const ILinkNode>>([](auto node)
        -> std::shared_ptr<const ILinkNode>
        {
            return std::dynamic_pointer_cast<const ILinkNode>(node);
        });
}

auto HMDT::Project::Hierarchy::ProvinceNode::getAdjacentProvincesProperty() const noexcept
    -> Maybe<std::shared_ptr<const IGroupNode>>
{
    return (*this)[ADJACENT_PROVINCES].andThen<std::shared_ptr<const IGroupNode>>([](auto node)
        -> std::shared_ptr<const IGroupNode>
        {
            return std::dynamic_pointer_cast<const IGroupNode>(node);
        });
}

