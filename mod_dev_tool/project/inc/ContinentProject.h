#ifndef CONTINENT_PROJECT_H
# define CONTINENT_PROJECT_H

# include "IProject.h"

namespace HMDT::Project {
    /**
     * @brief Defines a province project for HoI4
     */
    class ContinentProject: public IContinentProject {
        public:
            ContinentProject(IRootMapProject&);

            virtual ~ContinentProject() = default;

            virtual const ContinentSet& getContinentList() const override;

            virtual MaybeVoid save(const std::filesystem::path&) override;
            virtual MaybeVoid load(const std::filesystem::path&) override;
            virtual MaybeVoid export_(const std::filesystem::path&) const noexcept override;

            virtual IRootProject& getRootParent() override;
            virtual const IRootProject& getRootParent() const override;

            virtual std::shared_ptr<MapData> getMapData() override;
            virtual const std::shared_ptr<MapData> getMapData() const override;

            virtual void import(const ShapeFinder&, std::shared_ptr<MapData>) override;

            virtual bool validateData() override;

            virtual IRootMapProject& getRootMapParent() override;
            virtual const IRootMapProject& getRootMapParent() const override;

            virtual Maybe<std::shared_ptr<Hierarchy::INode>> visit(const std::function<MaybeVoid(Hierarchy::INode&)>&) const noexcept override;

        private:
            virtual ContinentSet& getContinents() override;

            //! The parent project
            IRootMapProject& m_parent_project;

            //! All continents defined for this project
            std::set<std::string> m_continents;

            // We friend this so that it can access getContinents()
            friend class MapProject;
    };
}

#endif

