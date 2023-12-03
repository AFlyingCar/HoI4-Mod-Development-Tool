#ifndef PROJECT_HIERARCHY_PROVINCENODE_H
# define PROJECT_HIERARCHY_PROVINCENODE_H

# include <vector>

# include "Types.h"

# include "GroupNode.h"

namespace HMDT::Project::Hierarchy {
    /**
     * @brief Specialized GroupNode that represents a Province
     */
    class ProvinceNode: public GroupNode {
        public:
            using GroupNode::GroupNode;

            virtual ~ProvinceNode() = default;

            virtual Type getType() const noexcept override;

            MaybeVoid setID(const IPropertyNode::ValueLookup<ProvinceID>&,
                            const INodeVisitor&) noexcept;
            MaybeVoid setColor(const IPropertyNode::ValueLookup<const Color>&,
                               const INodeVisitor&) noexcept;
            MaybeVoid setProvinceType(const IPropertyNode::ValueLookup<ProvinceType>&,
                                      const INodeVisitor&) noexcept;
            MaybeVoid setCoastal(const IPropertyNode::ValueLookup<bool>&,
                                 const INodeVisitor&) noexcept;
            MaybeVoid setTerrain(const IPropertyNode::ValueLookup<TerrainID>&,
                                 const INodeVisitor&) noexcept;
            MaybeVoid setContinent(const IPropertyNode::ValueLookup<Continent>&,
                                   const INodeVisitor&) noexcept;
            MaybeVoid setState(const IPropertyNode::ValueLookup<StateID>&,
                               const INodeVisitor&) noexcept;
            MaybeVoid setAdjacentProvinces(const std::set<ProvinceID>&,
                                           const INodeVisitor&) noexcept;

            Maybe<std::shared_ptr<const IPropertyNode>> getIDProperty() const noexcept;
            Maybe<std::shared_ptr<const IPropertyNode>> getColorProperty() const noexcept;
            Maybe<std::shared_ptr<const IPropertyNode>> getProvinceType() const noexcept;
            Maybe<std::shared_ptr<const IPropertyNode>> getCoastalProperty() const noexcept;
            Maybe<std::shared_ptr<const IPropertyNode>> getTerrainProperty() const noexcept;
            Maybe<std::shared_ptr<const IPropertyNode>> getContinentProperty() const noexcept;
            Maybe<std::shared_ptr<const ILinkNode>> getStateProperty() const noexcept;
            Maybe<std::shared_ptr<const IGroupNode>> getAdjacentProvincesProperty() const noexcept;
    };
}

#endif

