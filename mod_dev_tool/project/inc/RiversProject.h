#ifndef RIVERS_PROJECT_H
# define RIVERS_PROJECT_H

# include "BitMap.h"

# include "IProject.h"

namespace HMDT::Project {
    /**
     * @brief Defines a province project for HoI4
     */
    class RiversProject: public IRiversProject {
        public:
            RiversProject(IRootMapProject&);

            virtual ~RiversProject() = default;

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

            MonadOptionalRef<const BitMap2> getBitMap() const;

        protected:
            MaybeVoid generateTemplate(std::unique_ptr<unsigned char[]>&) const noexcept;
            static ColorTable generateColorTable() noexcept;

        private:
            //! The parent project
            IRootMapProject& m_parent_project;

            std::shared_ptr<BitMap2> m_rivers_bmp;
    };
}

#endif

