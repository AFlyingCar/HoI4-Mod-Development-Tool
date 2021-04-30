#ifndef IPROJECT_H
# define IPROJECT_H

#include <filesystem>

namespace MapNormalizer::Project {
    /**
     * @brief The interface for a project
     */
    struct IProject {
        virtual ~IProject() = default;

        virtual bool save(const std::filesystem::path&) = 0;
        virtual bool load(const std::filesystem::path&) = 0;
    };
}

#endif

