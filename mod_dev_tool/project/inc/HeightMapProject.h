#ifndef HEIGHTMAP_PROJECT_H
# define HEIGHTMAP_PROJECT_H

# include "BitMap.h"

# include "IProject.h"

namespace HMDT::Project {
    /**
     * @brief Defines a province project for HoI4
     */
    class HeightMapProject: public IHeightMapProject {
        public:
            HeightMapProject(IRootMapProject&);

            virtual ~HeightMapProject() = default;

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

            virtual MaybeVoid loadFile(const std::filesystem::path&) noexcept override;

            virtual Maybe<std::shared_ptr<Hierarchy::INode>> visit(const std::function<MaybeVoid(std::shared_ptr<Hierarchy::INode>)>&) const noexcept override;

            MonadOptionalRef<const BitMap2> getBitMap() const;

        private:
            //! The parent project
            IRootMapProject& m_parent_project;

            std::shared_ptr<BitMap2> m_heightmap_bmp;
    };
}

#endif

