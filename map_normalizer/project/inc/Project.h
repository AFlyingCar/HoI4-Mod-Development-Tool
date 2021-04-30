#ifndef PROJECT_H
# define PROJECT_H

# include <string>
# include <vector>
# include <filesystem>

# include "Version.h"

# include "IProject.h"
# include "MapProject.h"

namespace MapNormalizer::Project {
    /**
     * @brief Defines a HoI4 project
     */
    class HoI4Project: public IProject {
        public:
            HoI4Project();

            HoI4Project(const HoI4Project&);
            HoI4Project(HoI4Project&&);

            HoI4Project(const std::filesystem::path&);

            virtual ~HoI4Project() = default;

            HoI4Project& operator=(const HoI4Project&);

            const std::filesystem::path& getPath() const;
            const std::string& getName() const;
            const Version& getToolVersion() const;
            const Version& getHoI4Version() const;
            const std::vector<std::string>& getTags() const;
            const std::vector<std::filesystem::path>& getOverrides() const;

            MapProject& getMapProject();

            bool load();
            bool save();

            void setPath(const std::filesystem::path&);
            void setName(const std::string&);

            void setPathAndName(const std::filesystem::path&);

            void importFile(const std::filesystem::path&);

        protected:
            virtual bool save(const std::filesystem::path&) override;
            virtual bool load(const std::filesystem::path&) override;

        private:
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
    };

    using Project = HoI4Project;
}

#endif

