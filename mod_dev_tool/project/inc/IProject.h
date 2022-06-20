#ifndef IPROJECT_H
# define IPROJECT_H

# include <filesystem>
# include <system_error>
# include <memory>

# include "Maybe.h"

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
        virtual ~IProject() = default;

        virtual MaybeVoid save(const std::filesystem::path&) = 0;
        virtual MaybeVoid load(const std::filesystem::path&) = 0;
    };

    struct IMapProject: public IProject {
        virtual ~IMapProject() = default;

        virtual std::shared_ptr<MapData> getMapData() = 0;
        virtual const std::shared_ptr<MapData> getMapData() const = 0;

        virtual void import(const ShapeFinder&, std::shared_ptr<MapData>) = 0;

        virtual bool validateData() = 0;
    };
}

#endif

