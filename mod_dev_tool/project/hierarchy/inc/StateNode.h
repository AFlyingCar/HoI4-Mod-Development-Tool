#ifndef PROJECT_HIERARCHY_STATENODE_H
# define PROJECT_HIERARCHY_STATENODE_H

# include <vector>

# include "Types.h"

# include "GroupNode.h"

namespace HMDT::Project::Hierarchy {
    class StateNode: public GroupNode {
        public:
            using GroupNode::GroupNode;

            static constexpr const char* const ID = "ID";
            static constexpr const char* const MANPOWER = "Manpower";
            static constexpr const char* const CATEGORY = "Category";
            static constexpr const char* const BUILDINGS_MAX_LEVEL_FACTOR = "BuildingsMaxLevelFactor";
            static constexpr const char* const IMPASSABLE = "Impassable";
            static constexpr const char* const PROVINCES = "Provinces";

            virtual ~StateNode() = default;

            virtual Type getType() const noexcept override;

            MaybeVoid setID(uint32_t&) noexcept;
            MaybeVoid setManpower(size_t&) noexcept;
            MaybeVoid setCategory(std::string&) noexcept;
            MaybeVoid setBuildingsMaxLevelFactor(float&) noexcept;
            MaybeVoid setImpassable(bool&) noexcept;
            MaybeVoid setProvinces(const std::vector<ProvinceID>&) noexcept;

            Maybe<std::shared_ptr<const IPropertyNode>> getIDProperty() const noexcept;
            Maybe<std::shared_ptr<const IPropertyNode>> getManpowerProperty() const noexcept;
            Maybe<std::shared_ptr<const IPropertyNode>> getCategoryProperty() const noexcept;
            Maybe<std::shared_ptr<const IPropertyNode>> getBuildingsMaxLevelFactorProperty() const noexcept;
            Maybe<std::shared_ptr<const IPropertyNode>> getImpassableProperty() const noexcept;
            Maybe<std::shared_ptr<const IGroupNode>> getProvincesProperty() const noexcept;
    };
}

#endif

