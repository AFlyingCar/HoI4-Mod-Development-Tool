#ifndef IPROJECT_H
# define IPROJECT_H

#include <filesystem>
#include <system_error>

namespace MapNormalizer::Project {
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
}

#endif

