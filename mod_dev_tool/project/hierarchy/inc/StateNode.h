#ifndef PROJECT_HIERARCHY_STATENODE_H
# define PROJECT_HIERARCHY_STATENODE_H

# include <vector>

# include "Types.h"

# include "GroupNode.h"

namespace HMDT::Project::Hierarchy {
    /**
     * @brief Specialized GroupNode that represents a State
     */
    class StateNode: public GroupNode {
        public:
            using GroupNode::GroupNode;

            virtual ~StateNode() = default;

            virtual Type getType() const noexcept override;

            MaybeVoid setID(const IPropertyNode::ValueLookup<StateID>&,
                            const INodeVisitor&) noexcept;
            MaybeVoid setManpower(const IPropertyNode::ValueLookup<size_t>&,
                                  const INodeVisitor&) noexcept;
            MaybeVoid setCategory(const IPropertyNode::ValueLookup<std::string>&,
                                  const INodeVisitor&) noexcept;
            MaybeVoid setBuildingsMaxLevelFactor(const IPropertyNode::ValueLookup<float>&,
                                                 const INodeVisitor&) noexcept;
            MaybeVoid setImpassable(const IPropertyNode::ValueLookup<bool>&,
                                    const INodeVisitor&) noexcept;
            MaybeVoid setProvinces(const std::vector<ProvinceID>&,
                                   const INodeVisitor&) noexcept;

            Maybe<std::shared_ptr<const IPropertyNode>> getIDProperty() const noexcept;
            Maybe<std::shared_ptr<const IPropertyNode>> getManpowerProperty() const noexcept;
            Maybe<std::shared_ptr<const IPropertyNode>> getCategoryProperty() const noexcept;
            Maybe<std::shared_ptr<const IPropertyNode>> getBuildingsMaxLevelFactorProperty() const noexcept;
            Maybe<std::shared_ptr<const IPropertyNode>> getImpassableProperty() const noexcept;
            Maybe<std::shared_ptr<const IGroupNode>> getProvincesProperty() const noexcept;
    };
}

#endif

