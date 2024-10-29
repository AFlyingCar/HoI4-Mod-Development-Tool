#ifndef DRIVER_H
# define DRIVER_H

# include <optional>
# include <memory>

# include "glibmm/refptr.h"
# include "giomm/resource.h"
# include "gdkmm/pixbuf.h"
# include "giomm/inputstream.h"

# include "MultiKeyMap.h"

# include "HoI4Project.h"

# include "Types.h"
# include "Maybe.h"

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
            using Resource = Glib::RefPtr<Gio::Resource>;
            using Pixbuf = Glib::RefPtr<Gdk::Pixbuf>;

            /**
             * @brief Structure defining a pack of resources
             */
            struct ResourcePack {
                //! The Gio resource object
                Resource resource;

                //! The prefix that resources are stored under
                std::string prefix;
            };

            using HProject = Project::HoI4Project;
            using UniqueProject = std::unique_ptr<HProject>;

            using IconMap = std::unordered_map<std::string,
                                               Pixbuf>;
            using ResourceMap = std::unordered_map<std::string, ResourcePack>;
            using ResourcePixbufCache = mkm::MultiKeyMap<std::string, int, int,
                                                         bool, Pixbuf>;

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

            Maybe<ResourcePack> getResourcePack(const std::string&) const noexcept;
            ResourceMap getResources() const noexcept;

            Pixbuf getIcon(const std::string&, std::int32_t, std::int32_t) noexcept;

            Maybe<Glib::RefPtr<Gio::InputStream>> getResourceStream(const std::string&,
                                                                    const std::string&) const noexcept;
            Maybe<Pixbuf> getResourcePixbuf(const std::string&,
                                            const std::string&,
                                            int = -1, int = -1,
                                            bool = true) noexcept;

            Pixbuf getFailurePixbuf() const noexcept;

        protected:
            Driver();

            MaybeVoid registerResourceFile(const std::string&,
                                           const std::filesystem::path&,
                                           const std::string&) noexcept;

        private:
            Glib::RefPtr<Application> m_app;
            ResourceMap m_resources;

            UniqueProject m_project;

            //! Cache of pixbufs from a resource. Done to prevent constantly re-creating pixbufs
            ResourcePixbufCache m_resource_pixbuf_cache;
    };
}

#endif

