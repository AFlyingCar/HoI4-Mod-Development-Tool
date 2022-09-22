#ifndef PROJECT_H
# define PROJECT_H

# include <string>
# include <vector>
# include <filesystem>

# include "Version.h"

# include "IProject.h"
# include "MapProject.h"

namespace HMDT::Project {
    /**
     * @brief Defines a HoI4 project
     */
    class HoI4Project: public IRootProject {
        public:
            HoI4Project();

            HoI4Project(HoI4Project&&);

            HoI4Project(const std::filesystem::path&);

            virtual ~HoI4Project() = default;

            virtual const std::filesystem::path& getPath() const override;
            virtual std::filesystem::path getRoot() const override;

            virtual std::filesystem::path getMetaRoot() const override;
            virtual std::filesystem::path getInputsRoot() const override;
            virtual std::filesystem::path getMapRoot() const override;
            virtual std::filesystem::path getDebugRoot() const override;
            virtual std::filesystem::path getExportRoot() const override;

            std::filesystem::path getDefaultExportRoot() const;

            void setExportRoot(const std::filesystem::path&);

            const std::string& getName() const;
            const Version& getToolVersion() const;
            const Version& getHoI4Version() const;
            const std::vector<std::string>& getTags() const;
            const std::vector<std::filesystem::path>& getOverrides() const;

            virtual IRootMapProject& getMapProject() noexcept override;
            virtual const IRootMapProject& getMapProject() const noexcept override;

            MaybeVoid load();
            MaybeVoid save(bool = true);
            MaybeVoid export_() const noexcept;

            void setPath(const std::filesystem::path&);
            void setName(const std::string&);

            void setPathAndName(const std::filesystem::path&);

            void importFile(const std::filesystem::path&);

            void setToolVersion(const Version&);
            void setHoI4Version(const Version&);

        protected:
            MaybeVoid save(const std::filesystem::path&, bool);

            virtual MaybeVoid save(const std::filesystem::path&) override;
            virtual MaybeVoid load(const std::filesystem::path&) override;

            virtual MaybeVoid export_(const std::filesystem::path&) const noexcept override;

        private:
            //! The path to the project file (The .hoi4proj file)
            std::filesystem::path m_path;

            //! The root of the project (where the project file goes)
            std::filesystem::path m_root;

            //! The name of the project
            std::string m_name;

            //! The tool version this project was built with
            Version m_tool_version;

            //! The HoI4 version this project is targetting
            Version m_hoi4_version;

            //! The tags for the project
            std::vector<std::string> m_tags;

            //! All vanilla files which should be overridden as empty
            std::vector<std::filesystem::path> m_overrides;

            //! All maps for this project
            MapProject m_map_project;

            //! The path to export into.
            std::filesystem::path m_export_root;
    };

    using Project = HoI4Project;
}

#endif

