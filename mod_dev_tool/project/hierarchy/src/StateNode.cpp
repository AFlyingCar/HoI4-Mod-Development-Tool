
#include "StateNode.h"

#include <memory>

#include "PropertyNode.h"
#include "ProvinceNode.h"
#include "ProjectNode.h"
#include "LinkNode.h"
#include "NodeKeyNames.h"

/**
 * @brief Gets the type of StateNode
 *
 * @return Node::Type::STATE
 */
auto HMDT::Project::Hierarchy::StateNode::getType() const noexcept -> Type {
    return Node::Type::STATE;
}

/**
 * @brief Sets the ID property of this node
 *
 * @param id The StateID to set
 * @param visitor The visitor callback
 *
 * @return A status code
 */
auto HMDT::Project::Hierarchy::StateNode::setID(StateID& id,
                                                const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto id_node = std::make_shared<PropertyNode<StateID>>(StateKeys::ID, id);
    visitor(id_node);

    auto result = addChild(id_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

/**
 * @brief Sets the Manpower property of this node
 *
 * @param manpower The manpower to set
 * @param visitor The visitor callback
 *
 * @return A status code
 */
auto HMDT::Project::Hierarchy::StateNode::setManpower(size_t& manpower,
                                                      const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto manpower_node = std::make_shared<PropertyNode<size_t>>(StateKeys::MANPOWER,
                                                                manpower);
    visitor(manpower_node);

    auto result = addChild(manpower_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

/**
 * @brief Sets the Category property of this node
 *
 * @param category The category to set
 * @param visitor The visitor callback
 *
 * @return A status code
 */
auto HMDT::Project::Hierarchy::StateNode::setCategory(std::string& category,
                                                      const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto category_node = std::make_shared<PropertyNode<std::string>>(StateKeys::CATEGORY,
                                                                     category);
    visitor(category_node);

    auto result = addChild(category_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

/**
 * @brief Sets the BuildingsMaxLevelFactor property of this node
 *
 * @param buildings_max_level_factor The category to set
 * @param visitor The visitor callback
 *
 * @return A status code
 */
auto HMDT::Project::Hierarchy::StateNode::setBuildingsMaxLevelFactor(float& buildings_max_level_factor,
                                                                     const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto bmlf_node = std::make_shared<PropertyNode<float>>(StateKeys::BUILDINGS_MAX_LEVEL_FACTOR,
                                                           buildings_max_level_factor);
    visitor(bmlf_node);

    auto result = addChild(bmlf_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

/**
 * @brief Sets the impassable property of this node
 *
 * @param impassable The category to set
 * @param visitor The visitor callback
 *
 * @return A status code
 */
auto HMDT::Project::Hierarchy::StateNode::setImpassable(bool& impassable,
                                                        const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    auto impassable_node = std::make_shared<PropertyNode<bool>>(StateKeys::IMPASSABLE,
                                                                impassable);
    visitor(impassable_node);

    auto result = addChild(impassable_node);
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

/**
 * @brief Sets the provinces property of this node
 * @details Builds a GroupNode to hold LinkNodes which point at the relevant
 *          province node
 *
 * @param provinces The category to set
 * @param visitor The visitor callback
 *
 * @return A status code
 */
auto HMDT::Project::Hierarchy::StateNode::setProvinces(const std::vector<ProvinceID>& provinces,
                                                       const INodeVisitor& visitor) noexcept
    -> MaybeVoid
{
    MaybeVoid result = STATUS_SUCCESS;

    auto provinces_group_node = std::make_shared<GroupNode>(StateKeys::PROVINCES);
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
                Key key{
                    ProjectKeys::MAP,
                    ProjectKeys::PROVINCES,
                    GroupKeys::PROVINCES,
                    std::to_string(province_id)
                };
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

/**
 * @brief Gets the ID property
 *
 * @return A Maybe containing the ID property, or an error code if it's not found
 */
auto HMDT::Project::Hierarchy::StateNode::getIDProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>>
{
    return (*this)[StateKeys::ID].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> Maybe<std::shared_ptr<const IPropertyNode>>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

/**
 * @brief Gets the manpower property
 *
 * @return A Maybe containing the manpower property, or an error code if it's not found
 */
auto HMDT::Project::Hierarchy::StateNode::getManpowerProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>> 
{
    return (*this)[StateKeys::MANPOWER].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> Maybe<std::shared_ptr<const IPropertyNode>>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

/**
 * @brief Gets the category property
 *
 * @return A Maybe containing the category property, or an error code if it's not found
 */
auto HMDT::Project::Hierarchy::StateNode::getCategoryProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>> 
{
    return (*this)[StateKeys::CATEGORY].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> Maybe<std::shared_ptr<const IPropertyNode>>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

/**
 * @brief Gets the BuildingsMaxLevelFactor property
 *
 * @return A Maybe containing the BuildingsMaxLevelFactor property, or an error code if it's not found
 */
auto HMDT::Project::Hierarchy::StateNode::getBuildingsMaxLevelFactorProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>> 
{
    return (*this)[StateKeys::BUILDINGS_MAX_LEVEL_FACTOR].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> Maybe<std::shared_ptr<const IPropertyNode>>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

/**
 * @brief Gets the impassable property
 *
 * @return A Maybe containing the impassable property, or an error code if it's not found
 */
auto HMDT::Project::Hierarchy::StateNode::getImpassableProperty() const noexcept
    -> Maybe<std::shared_ptr<const IPropertyNode>> 
{
    return (*this)[StateKeys::IMPASSABLE].andThen<std::shared_ptr<const IPropertyNode>>([](auto node)
        -> Maybe<std::shared_ptr<const IPropertyNode>>
        {
            return std::dynamic_pointer_cast<const IPropertyNode>(node);
        });
}

/**
 * @brief Gets the provinces property
 *
 * @return A Maybe containing the provinces property, or an error code if it's not found
 */
auto HMDT::Project::Hierarchy::StateNode::getProvincesProperty() const noexcept
    -> Maybe<std::shared_ptr<const IGroupNode>>
{
    return (*this)[StateKeys::PROVINCES].andThen<std::shared_ptr<const IGroupNode>>([](auto node)
        -> Maybe<std::shared_ptr<const IGroupNode>>
        {
            return std::dynamic_pointer_cast<const IGroupNode>(node);
        });
}

