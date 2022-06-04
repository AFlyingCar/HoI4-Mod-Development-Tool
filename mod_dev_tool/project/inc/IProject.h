#ifndef IPROJECT_H
# define IPROJECT_H

# include <filesystem>
# include <system_error>
# include <memory>

// Forward declarations
namespace HMDT {
    class MapData;
    class ShapeFinder;
}

namespace HMDT::Project {
    /**
     * @brief The interface for a project
     */
    struct IProject {
        inline static std::error_code last_error;

        virtual ~IProject() = default;

        virtual bool save(const std::filesystem::path&,
                          std::error_code& = last_error) = 0;
        virtual bool load(const std::filesystem::path&,
                          std::error_code& = last_error) = 0;
    };

    struct IMapProject: public IProject {
        virtual ~IMapProject() = default;

        virtual std::shared_ptr<MapData> getMapData() = 0;
        virtual const std::shared_ptr<MapData> getMapData() const = 0;

        virtual void import(const ShapeFinder&, std::shared_ptr<MapData>) = 0;
    };
}

#endif

