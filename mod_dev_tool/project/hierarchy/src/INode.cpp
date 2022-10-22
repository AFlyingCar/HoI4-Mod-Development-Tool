
#include "INode.h"

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


