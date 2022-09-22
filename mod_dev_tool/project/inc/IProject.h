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

# include "Terrain.h"

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
        //! Callback type for prompting the user with some question
        using PromptCallback = std::function<Maybe<uint32_t>(const std::string&,
                                                             const std::vector<std::string>&)>;

        virtual ~IProject() = default;

        virtual MaybeVoid save(const std::filesystem::path&) = 0;
        virtual MaybeVoid load(const std::filesystem::path&) = 0;

        virtual MaybeVoid export_(const std::filesystem::path&) const noexcept = 0;

        virtual IRootProject& getRootParent() = 0;

        void setPromptCallback(const PromptCallback&);
        void resetPromptCallback();

        protected:
            Maybe<uint32_t> prompt(const std::string&,
                                   const std::vector<std::string>&);

        private:
            static PromptCallback DEFAULT_PROMPT_CALLBACK;

            //! A generic callback which can be used to ask the user a question.
            PromptCallback m_prompt_callback = DEFAULT_PROMPT_CALLBACK;
    };

////////////////////////////////////////////////////////////////////////////////
// Forward declarations
    struct IRootMapProject;
    struct IRootHistoryProject;

////////////////////////////////////////////////////////////////////////////////
// Map Projects

    /**
     * @brief The base project class used by all Map-related projects
     */
    struct IMapProject: public IProject {
        virtual ~IMapProject() = default;

        virtual std::shared_ptr<MapData> getMapData() = 0;
        virtual const std::shared_ptr<MapData> getMapData() const = 0;

        virtual void import(const ShapeFinder&, std::shared_ptr<MapData>) = 0;

        virtual bool validateData() = 0;

        virtual IRootMapProject& getRootMapParent() = 0;
    };

    /**
     * @brief The interface for the ProvinceProject
     */
    struct IProvinceProject: public IMapProject {
        using ProvinceDataPtr = std::shared_ptr<unsigned char[]>;

        bool isValidProvinceLabel(uint32_t) const;

        const Province& getProvinceForLabel(uint32_t) const;
        Province& getProvinceForLabel(uint32_t);

        virtual ProvinceDataPtr getPreviewData(ProvinceID) = 0;
        virtual ProvinceDataPtr getPreviewData(const Province*) = 0;

        virtual ProvinceList& getProvinces() = 0;
        virtual const ProvinceList& getProvinces() const = 0;
    };

    /**
     * @brief The interface for StateProject
     */
    struct IStateProject: public IMapProject {
        using StateMap = std::map<uint32_t, State>;

        virtual ~IStateProject() = default;

        bool isValidStateID(StateID) const;

        MaybeRef<const State> getStateForID(StateID) const;
        MaybeRef<State> getStateForID(StateID);

        virtual const StateMap& getStates() const = 0;

        virtual StateID addNewState(const std::vector<uint32_t>&) = 0;
        virtual void removeState(StateID) = 0;

        protected:
            virtual StateMap& getStateMap() = 0;
    };

    /**
     * @brief The interface for the HeightMapProject
     */
    struct IHeightMapProject: public IMapProject {
        virtual ~IHeightMapProject() = default;

        virtual MaybeVoid loadFile(const std::filesystem::path&) noexcept = 0;
    };

    /**
     * @brief The interface for the ContinentProject
     */
    struct IContinentProject: public IMapProject {
        using ContinentSet = std::set<std::string>;

        virtual ~IContinentProject() = default;

        virtual const ContinentSet& getContinentList() const = 0;

        void addNewContinent(const std::string&);
        void removeContinent(const std::string&);
        bool doesContinentExist(const std::string&) const;

        protected:
            virtual ContinentSet& getContinents() = 0;
    };

////////////////////////////////////////////////////////////////////////////////
// History Projects
    struct IHistoryProject: public IProject {
        virtual ~IHistoryProject() = default;

        virtual IRootHistoryProject& getRootHistoryParent() noexcept = 0;
        virtual const IRootHistoryProject& getRootHistoryParent() const noexcept = 0;

        virtual bool validateData() = 0;
    };

////////////////////////////////////////////////////////////////////////////////
// Root Projects (Level 2)

    /**
     * @brief The interface for the root of all map-based projects
     */
    struct IRootMapProject: public IMapProject {
        virtual ~IRootMapProject() = default;

        virtual std::shared_ptr<MapData> getMapData() = 0;
        virtual const std::shared_ptr<MapData> getMapData() const = 0;

        virtual void moveProvinceToState(uint32_t, StateID) = 0;
        virtual void moveProvinceToState(Province&, StateID) = 0;
        virtual void removeProvinceFromState(Province&, bool = true) = 0;

        virtual void calculateCoastalProvinces(bool = false) = 0;

        // TODO: This should be its own sub-project
        virtual const std::vector<Terrain>& getTerrains() const = 0;

        ////////////////////////////////////////////////////////////////////////

        virtual IProvinceProject& getProvinceProject() noexcept = 0;
        virtual const IProvinceProject& getProvinceProject() const noexcept = 0;

        virtual IStateProject& getStateProject() noexcept = 0;
        virtual const IStateProject& getStateProject() const noexcept = 0;

        virtual IHeightMapProject& getHeightMapProject() noexcept = 0;
        virtual const IHeightMapProject& getHeightMapProject() const noexcept = 0;

        virtual IContinentProject& getContinentProject() noexcept = 0;
        virtual const IContinentProject& getContinentProject() const noexcept = 0;
    };

    struct IRootHistoryProject: public IHistoryProject {
        virtual ~IRootHistoryProject() = default;

        virtual IStateProject& getStateProject() noexcept = 0;
        virtual const IStateProject& getStateProject() const noexcept = 0;
    };

////////////////////////////////////////////////////////////////////////////////
// Root Project (Level 1)

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
        virtual std::filesystem::path getExportRoot() const = 0;

        virtual IRootProject& getRootParent() override final;

        virtual IRootMapProject& getMapProject() noexcept = 0;
        virtual const IRootMapProject& getMapProject() const noexcept = 0;
    };
}

#endif

