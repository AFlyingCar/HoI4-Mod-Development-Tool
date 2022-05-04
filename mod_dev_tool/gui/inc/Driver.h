#ifndef DRIVER_H
# define DRIVER_H

# include <optional>
# include <memory>

# include "glibmm/refptr.h"
# include "giomm/resource.h"

# include "HoI4Project.h"

# include "Types.h"

namespace HMDT::GUI {
    class Application;

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

            const Glib::RefPtr<Gio::Resource> getResources() const;

        protected:
            Driver();

        private:
            Glib::RefPtr<Application> m_app;
            Glib::RefPtr<Gio::Resource> m_resources;

            UniqueProject m_project;
    };
}

#endif

