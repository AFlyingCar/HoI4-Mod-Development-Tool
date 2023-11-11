
#include "INode.h"
#include "Util.h"

auto HMDT::Project::Hierarchy::INode::visit(INodeVisitor visitor) noexcept
    -> MaybeVoid
{
    auto result = visitor(shared_from_this());
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

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

auto HMDT::Project::Hierarchy::IGroupNode::operator[](const std::string& name) const noexcept
    -> Maybe<ConstChildNode>
{
    if(getChildren().count(name) != 0) {
        return getChildren().at(name);
    } else {
        RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
    }
}

auto HMDT::Project::Hierarchy::IGroupNode::operator[](const std::string& name) noexcept
    -> Maybe<ChildNode>
{
    if(getChildren().count(name) != 0) {
        return getChildren().at(name);
    } else {
        RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
    }
}

auto HMDT::Project::Hierarchy::IGroupNode::getChild(const std::string& name) const noexcept
    -> Maybe<ConstChildNode>
{
    return (*this)[name];
}

auto HMDT::Project::Hierarchy::IGroupNode::getChild(const std::string& name) noexcept
    -> Maybe<ChildNode>
{
    return (*this)[name];
}

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
}

std::string std::to_string(const HMDT::Project::Hierarchy::INode& node) {
    return node.getName() + " (" + std::to_string(&node) + ") [" + std::to_string(node.getType()) + "]";
}

