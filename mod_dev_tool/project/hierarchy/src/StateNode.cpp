
#include "StateNode.h"

#include <memory>

#include "PropertyNode.h"
#include "ProvinceNode.h"
#include "ProjectNode.h"
#include "LinkNode.h"

auto HMDT::Project::Hierarchy::StateNode::getType() const noexcept -> Type {
    return Node::Type::STATE;
}

auto HMDT::Project::Hierarchy::StateNode::setID(uint32_t& id,
                                                const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto id_node = std::make_shared<PropertyNode<uint32_t>>(ID, id);
    visitor(id_node);

    auto result = addChild(id_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

auto HMDT::Project::Hierarchy::StateNode::setManpower(size_t& manpower,
                                                      const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto manpower_node = std::make_shared<PropertyNode<size_t>>(MANPOWER,
                                                                manpower);
    visitor(manpower_node);

    auto result = addChild(manpower_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

auto HMDT::Project::Hierarchy::StateNode::setCategory(std::string& category,
                                                      const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto category_node = std::make_shared<PropertyNode<std::string>>(CATEGORY,
                                                                     category);
    visitor(category_node);

    auto result = addChild(category_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

auto HMDT::Project::Hierarchy::StateNode::setBuildingsMaxLevelFactor(float& buildings_max_level_factor,
                                                                     const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto bmlf_node = std::make_shared<PropertyNode<float>>(BUILDINGS_MAX_LEVEL_FACTOR,
                                                           buildings_max_level_factor);
    visitor(bmlf_node);

    auto result = addChild(bmlf_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

auto HMDT::Project::Hierarchy::StateNode::setImpassable(bool& impassable,
                                                        const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto impassable_node = std::make_shared<PropertyNode<bool>>(IMPASSABLE,
                                                                impassable);
    visitor(impassable_node);

    auto result = addChild(impassable_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

auto HMDT::Project::Hierarchy::StateNode::setProvinces(const std::vector<ProvinceID>& provinces,
                                                       const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    MaybeVoid result = STATUS_SUCCESS;

    auto provinces_group_node = std::make_shared<GroupNode>(PROVINCES);
    visitor(provinces_group_node);

    for(auto&& province_id : provinces) {
        // Create a link node which will resolve on ProvinceNodes where
        //   ID == province_id
        auto province_link_node = std::make_shared<LinkNode>(
            std::to_string(province_id),
            [province_id](ILinkNode::LinkedNode node) -> bool {
                if(node->getType() == Node::Type::PROVINCE) {
                    auto id_node = std::dynamic_pointer_cast<ProvinceNode>(node)->getIDProperty();
                    if(IS_FAILURE(id_node)) {
                        WRITE_ERROR_CODE(id_node.error());
                        return false;
                    }

                    return (**id_node == province_id);
                }

                return false;
            },
            [province_id](INodePtr root) -> Maybe<ILinkNode::LinkedNode> {
                // Get Root[Project]->Map[Project]->Provinces[Project]->Provinces[Group]->ID

                // TODO: Make magic strings here constants
                Key key{ "Map", "Provinces", "Provinces", std::to_string(province_id) };
                auto node = key.lookup(root);
                RETURN_IF_ERROR(node);

                RETURN_ERROR_IF((*node)->getType() != Node::Type::PROVINCE,
                                STATUS_INVALID_TYPE);


                return node;
            });
        visitor(province_link_node);

        result = provinces_group_node->addChild(province_link_node);
        RETURN_IF_ERROR(result);
    }

    result = addChild(provinces_group_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

auto HMDT::Project::Hierarchy::StateNode::getIDProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>>
{
    return (*this)[ID].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> Maybe<std::shared_ptr<const IPropertyNode>>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

auto HMDT::Project::Hierarchy::StateNode::getManpowerProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>> 
{
    return (*this)[MANPOWER].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> Maybe<std::shared_ptr<const IPropertyNode>>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

auto HMDT::Project::Hierarchy::StateNode::getCategoryProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>> 
{
    return (*this)[CATEGORY].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> Maybe<std::shared_ptr<const IPropertyNode>>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

auto HMDT::Project::Hierarchy::StateNode::getBuildingsMaxLevelFactorProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>> 
{
    return (*this)[BUILDINGS_MAX_LEVEL_FACTOR].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> Maybe<std::shared_ptr<const IPropertyNode>>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

auto HMDT::Project::Hierarchy::StateNode::getImpassableProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>> 
{
    return (*this)[IMPASSABLE].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> Maybe<std::shared_ptr<const IPropertyNode>>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

auto HMDT::Project::Hierarchy::StateNode::getProvincesProperty() const noexcept
    -> Maybe<std::shared_ptr<const IGroupNode>>
{
    return (*this)[PROVINCES].andThen<std::shared_ptr<const IGroupNode>>([](auto node)
        -> Maybe<std::shared_ptr<const IGroupNode>>
        {
            return std::dynamic_pointer_cast<const IGroupNode>(node);
        });
}

