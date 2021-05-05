#ifndef DRIVER_H
# define DRIVER_H

# include <optional>
# include <memory>

# include "glibmm/refptr.h"

# include "HoI4Project.h"

# include "Types.h"

namespace MapNormalizer::GUI {
    class MapNormalizerApplication;

    /**
     * @brief The main driver of the GUI-based frontend
     *
     * @details The purpose of this class is to hold both the GUI application,
     *          as well as all global state information about the rest of the
     *          application.
     */
    class Driver {
        public:
            using HProject = Project::HoI4Project;
            using UniqueProject = std::unique_ptr<HProject>;

            static Driver& getInstance();

            Driver(const Driver&) = delete;
            Driver(Driver&&) = delete;

            ~Driver();

            bool initialize();

            void run();

            OptionalReference<HProject> getProject();
            OptionalReference<const HProject> getProject() const;

            void setProject(UniqueProject&&);
            void setProject();

        protected:
            Driver();

        private:
            Glib::RefPtr<MapNormalizerApplication> m_app;

            UniqueProject m_project;
    };
}

#endif

