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
# include "Version.h"

# include "Terrain.h"

# include "INode.h"

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
        /**
         * @brief Defines the different prompt types that might be given to the
         *        PromptCallback
         */
        enum class PromptType {
            INFO,
            WARN,
            ERROR,
            ALERT,
            QUESTION,
        };

        //! Callback type for prompting the user with some question
        using PromptCallback = std::function<Maybe<uint32_t>(const std::string&,
                                                             const std::vector<std::string>&,
                                                             const PromptType&)>;
        IProject();
        virtual ~IProject() = default;

        virtual MaybeVoid save(const std::filesystem::path&) = 0;
        virtual MaybeVoid load(const std::filesystem::path&) = 0;

        virtual MaybeVoid export_(const std::filesystem::path&) const noexcept = 0;

        virtual IRootProject& getRootParent() = 0;
        virtual const IRootProject& getRootParent() const = 0;

        virtual bool validateData() = 0;

        virtual Maybe<std::shared_ptr<Hierarchy::INode>> visit(const std::function<MaybeVoid(Hierarchy::INode&)>&) const noexcept = 0;

        void setPromptCallback(const PromptCallback&);
        void resetPromptCallback();

        protected:
            Maybe<uint32_t> prompt(const std::string&,
                                   const std::vector<std::string>&,
                                   const PromptType& = PromptType::INFO) const;

            const PromptCallback& getPromptCallback() const noexcept;

        private:
            Maybe<uint32_t> defaultPromptCallback(const std::string&,
                                                  const std::vector<std::string>&,
                                                  const PromptType&);

            //! A generic callback which can be used to ask the user a question.
            PromptCallback m_prompt_callback;
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

        virtual IRootMapProject& getRootMapParent() = 0;
        virtual const IRootMapProject& getRootMapParent() const = 0;
    };

    /**
     * @brief The interface for the ProvinceProject
     */
    struct IProvinceProject: public IMapProject {
        using ProvinceDataPtr = std::shared_ptr<unsigned char[]>;

        bool isValidProvinceLabel(uint32_t) const;
        bool isValidProvinceID(ProvinceID) const;

        const Province& getProvinceForID(ProvinceID) const;
        Province& getProvinceForID(ProvinceID);

        const Province& getProvinceForLabel(uint32_t) const;
        Province& getProvinceForLabel(uint32_t);

        Maybe<std::string> genProvinceChildTree(ProvinceID) const noexcept;

        virtual ProvinceDataPtr getPreviewData(ProvinceID) = 0;
        virtual ProvinceDataPtr getPreviewData(const Province*) = 0;

        virtual ProvinceList& getProvinces() = 0;
        virtual const ProvinceList& getProvinces() const = 0;

        virtual const std::unordered_map<uint32_t, UUID>& getOldIDToUUIDMap() const noexcept = 0;

        virtual uint32_t getIDForProvinceID(const ProvinceID&) const noexcept = 0;

        virtual MaybeRef<const Province> getRootProvinceParent(const ProvinceID&) const noexcept;
        virtual MaybeRef<Province> getRootProvinceParent(const ProvinceID&) noexcept;

        virtual MaybeVoid mergeProvinces(const ProvinceID&, const ProvinceID&) noexcept;
        virtual MaybeVoid unmergeProvince(const ProvinceID&) noexcept;

        virtual std::set<ProvinceID> getMergedProvinces(const ProvinceID&) const noexcept;
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

    struct IRiversProject: public IMapProject {
        virtual ~IRiversProject() = default;

        virtual MaybeVoid loadFile(const std::filesystem::path&) noexcept = 0;
        virtual MaybeVoid writeTemplate(const std::filesystem::path&) const noexcept = 0;
    };

////////////////////////////////////////////////////////////////////////////////
// History Projects
    struct IHistoryProject: public IProject {
        virtual ~IHistoryProject() = default;

        virtual IRootHistoryProject& getRootHistoryParent() noexcept = 0;
        virtual const IRootHistoryProject& getRootHistoryParent() const noexcept = 0;

        virtual bool validateData() = 0;
    };

    /**
     * @brief The interface for StateProject
     */
    struct IStateProject: public IHistoryProject {
        using StateMap = std::map<uint32_t, State>;

        virtual ~IStateProject() = default;

        bool isValidStateID(StateID) const;

        MaybeRef<const State> getStateForID(StateID) const;
        MaybeRef<State> getStateForID(StateID);

        virtual const StateMap& getStates() const = 0;

        virtual StateID addNewState(const std::vector<ProvinceID>&) = 0;
        virtual MaybeVoid removeState(StateID) noexcept = 0;

        virtual State& getStateForIterator(StateMap::const_iterator) = 0;
        virtual const State& getStateForIterator(StateMap::const_iterator) const = 0;

        virtual void updateStateIDMatrix() = 0;

        virtual MaybeVoid addProvinceToState(StateID, ProvinceID) = 0;
        virtual MaybeVoid removeProvinceFromState(StateID, ProvinceID) = 0;

        protected:
            virtual StateMap& getStateMap() = 0;
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

        virtual void moveProvinceToState(ProvinceID, StateID) = 0;
        virtual void moveProvinceToState(Province&, StateID) = 0;
        virtual void removeProvinceFromState(Province&, bool = true) = 0;

        virtual void calculateCoastalProvinces(bool = false) = 0;

        // TODO: This should be its own sub-project
        virtual const std::vector<Terrain>& getTerrains() const = 0;

        ////////////////////////////////////////////////////////////////////////

        virtual IProvinceProject& getProvinceProject() noexcept = 0;
        virtual const IProvinceProject& getProvinceProject() const noexcept = 0;

        virtual IHeightMapProject& getHeightMapProject() noexcept = 0;
        virtual const IHeightMapProject& getHeightMapProject() const noexcept = 0;

        virtual IContinentProject& getContinentProject() noexcept = 0;
        virtual const IContinentProject& getContinentProject() const noexcept = 0;

        virtual IRiversProject& getRiversProject() noexcept = 0;
        virtual const IRiversProject& getRiversProject() const noexcept = 0;
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
        virtual std::filesystem::path getHistoryRoot() const = 0;
        virtual std::filesystem::path getDebugRoot() const = 0;
        virtual std::filesystem::path getExportRoot() const = 0;

        virtual IRootProject& getRootParent() override final;
        virtual const IRootProject& getRootParent() const override final;

        virtual IRootMapProject& getMapProject() noexcept = 0;
        virtual const IRootMapProject& getMapProject() const noexcept = 0;

        virtual IRootHistoryProject& getHistoryProject() noexcept = 0;
        virtual const IRootHistoryProject& getHistoryProject() const noexcept = 0;

        virtual const Version& getToolVersion() const = 0;
        virtual const Version& getHoI4Version() const = 0;
    };
}

#endif

