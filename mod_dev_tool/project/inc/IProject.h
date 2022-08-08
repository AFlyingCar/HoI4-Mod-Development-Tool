#ifndef IPROJECT_H
# define IPROJECT_H

# include <filesystem>
# include <system_error>
# include <memory>
# include <set>
# include <map>
# include <string>

# include "fifo_map.hpp"

# include "Maybe.h"
# include "Types.h"

// Forward declarations
namespace HMDT {
    class MapData;
    class ShapeFinder;
}

namespace HMDT::Project {
    struct IRootProject;

    /**
     * @brief The interface for a project
     */
    struct IProject {
        virtual ~IProject() = default;

        virtual MaybeVoid save(const std::filesystem::path&) = 0;
        virtual MaybeVoid load(const std::filesystem::path&) = 0;

        virtual IRootProject& getRootParent() = 0;
    };

    /**
     * @brief Interface for the root of any project hierarchy
     */
    struct IRootProject: public IProject {
        virtual const std::filesystem::path& getPath() const = 0;
        virtual std::filesystem::path getRoot() const = 0;

        virtual std::filesystem::path getMetaRoot() const = 0;
        virtual std::filesystem::path getInputsRoot() const = 0;
        virtual std::filesystem::path getMapRoot() const = 0;
        virtual std::filesystem::path getDebugRoot() const = 0;

        virtual IRootProject& getRootParent() override final;
    };

    struct IMapProject: public IProject {
        virtual ~IMapProject() = default;

        virtual std::shared_ptr<MapData> getMapData() = 0;
        virtual const std::shared_ptr<MapData> getMapData() const = 0;

        virtual void import(const ShapeFinder&, std::shared_ptr<MapData>) = 0;

        virtual bool validateData() = 0;

        virtual IMapProject& getRootMapParent() = 0;
    };

    struct IProvinceProject: public IProject {
        using ProvinceDataPtr = std::shared_ptr<unsigned char[]>;

        bool isValidProvinceLabel(uint32_t) const;

        const Province& getProvinceForLabel(uint32_t) const;
        Province& getProvinceForLabel(uint32_t);

        virtual ProvinceDataPtr getPreviewData(ProvinceID) = 0;
        virtual ProvinceDataPtr getPreviewData(const Province*) = 0;

        virtual ProvinceList& getProvinces() = 0;
        virtual const ProvinceList& getProvinces() const = 0;
    };

    struct IStateProject: public IProject {
        using StateMap = std::map<uint32_t, State>;

        virtual ~IStateProject() = default;

        bool isValidStateID(StateID) const;

        MaybeRef<const State> getStateForID(StateID) const;
        MaybeRef<State> getStateForID(StateID);

        virtual const StateMap& getStates() const = 0;

        protected:
            virtual StateMap& getStateMap() = 0;
    };

    struct IContinentProject: public IProject {
        using ContinentSet = std::set<std::string>;

        virtual ~IContinentProject() = default;

        virtual const ContinentSet& getContinentList() const = 0;

        void addNewContinent(const std::string&);
        void removeContinent(const std::string&);
        bool doesContinentExist(const std::string&) const;

        protected:
            virtual ContinentSet& getContinents() = 0;
    };
}

#endif

