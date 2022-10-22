#ifndef PROJECT_HIERARCHY_PROVINCENODE_H
# define PROJECT_HIERARCHY_PROVINCENODE_H

# include <vector>

# include "Types.h"

# include "GroupNode.h"

namespace HMDT::Project::Hierarchy {
    class ProvinceNode: public GroupNode {
        public:
            using GroupNode::GroupNode;

            static constexpr const char* const ID = "ID";
            static constexpr const char* const COLOR = "Color";
            static constexpr const char* const TYPE = "Type";
            static constexpr const char* const COASTAL = "Coastal";
            static constexpr const char* const TERRAIN = "Terrain";
            static constexpr const char* const CONTINENT = "Continent";
            static constexpr const char* const STATE = "State";
            static constexpr const char* const ADJACENT_PROVINCES = "AdjacentProvinces";

            virtual ~ProvinceNode() = default;

            virtual Type getType() const noexcept override;

            MaybeVoid setID(ProvinceID&) noexcept;
            MaybeVoid setColor(const Color&) noexcept;
            MaybeVoid setProvinceType(ProvinceType&) noexcept;
            MaybeVoid setCoastal(bool&) noexcept;
            MaybeVoid setTerrain(TerrainID&) noexcept;
            MaybeVoid setContinent(Continent&) noexcept;
            MaybeVoid setState(State&) noexcept;
            MaybeVoid setAdjacentProvinces(const std::set<ProvinceID>&) noexcept;

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

