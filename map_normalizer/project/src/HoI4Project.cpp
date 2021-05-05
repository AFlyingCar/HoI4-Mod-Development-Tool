
#include "HoI4Project.h"

#include <fstream>
#include <iomanip>
#include <cstring>
#include <cerrno>

#include "nlohmann/json.hpp"

#include "Logger.h"
#include "Constants.h"

MapNormalizer::Project::HoI4Project::HoI4Project():
    m_path(),
    m_root(),
    m_name(),
    m_tool_version(),
    m_hoi4_version(),
    m_tags(),
    m_overrides(),
    m_map_project(*this)
{ }

MapNormalizer::Project::HoI4Project::HoI4Project(const std::filesystem::path& path):
    m_path(path),
    m_root(path.parent_path()),
    m_name(),
    m_tool_version(),
    m_hoi4_version(),
    m_tags(),
    m_overrides(),
    m_map_project(*this)
{
}

MapNormalizer::Project::HoI4Project::HoI4Project(HoI4Project&& other):
    m_path(std::move(other.m_path)),
    m_root(std::move(other.m_root)),
    m_name(std::move(other.m_name)),
    m_tool_version(std::move(other.m_tool_version)),
    m_hoi4_version(std::move(other.m_hoi4_version)),
    m_tags(std::move(other.m_tags)),
    m_overrides(std::move(other.m_overrides)),
    m_map_project(*this)
{ }

const std::filesystem::path& MapNormalizer::Project::HoI4Project::getPath() const {
    return m_path;
}

std::filesystem::path MapNormalizer::Project::HoI4Project::getRoot() const {
    return m_root;
}

std::filesystem::path MapNormalizer::Project::HoI4Project::getMetaRoot() const {
    return getRoot() / PROJ_META_FOLDER;
}

std::filesystem::path MapNormalizer::Project::HoI4Project::getInputsRoot() const
{
    return getMetaRoot() / "inputs";
}

std::filesystem::path MapNormalizer::Project::HoI4Project::getMapRoot() const {
    return getMetaRoot() / "map";
}

const std::string& MapNormalizer::Project::HoI4Project::getName() const {
    return m_name;
}

auto MapNormalizer::Project::HoI4Project::getToolVersion() const -> const Version& {
    return m_tool_version;
}

auto MapNormalizer::Project::HoI4Project::getHoI4Version() const -> const Version& {
    return m_hoi4_version;
}

const std::vector<std::string>& MapNormalizer::Project::HoI4Project::getTags() const
{
    return m_tags;
}

const std::vector<std::filesystem::path>& MapNormalizer::Project::HoI4Project::getOverrides() const
{
    return m_overrides;
}

auto MapNormalizer::Project::HoI4Project::getMapProject() -> MapProject& {
    return m_map_project;
}

/**
 * @brief Loads a json file referenced by 'path'
 * @details Format of the project file should be as follows:
 * @code
 *     {
 *         "name": "NameOfProject",
 *         "tool_version": "VersionOfTool",
 *         "hoi4_version": "VersionOfHoI4",
 *         "tags": [ "tag1", "tag2", ... ],
 *         "overrides": [ "relative/path/to/override1", "relative/path/to/override2", ... ]
 *     }
 * @endcode
 *
 * @param path The path to load the project file from
 *
 * @return True if the project file could be loaded correctly, false otherwise.
 */
bool MapNormalizer::Project::HoI4Project::load(const std::filesystem::path& path) {
    using json = nlohmann::json;

    if(std::ifstream in(path); in) {
        json proj;

        in >> proj;

        // This map is for error-checking to make sure we got each property we
        //  are expecting
        std::unordered_map<std::string, bool> complete = {
            { "name", true },
            { "tool_version", true },
            { "hoi4_version", true },
            { "tags", true },
            { "overrides", true }
        };

        if(proj.contains("name")) {
            m_name = proj["name"];
        } else {
            complete["name"] = false;
        }

        if(proj.contains("tool_version")) {
            m_tool_version = Version(proj["tool_version"]);
        } else {
            complete["tool_version"] = false;
        }

        if(proj.contains("hoi4_version")) {
            m_hoi4_version = Version(proj["hoi4_version"]);
        } else {
            complete["hoi4_version"] = false;
        }

        if(proj.contains("tags") && proj["tags"].is_array()) {
            proj["tags"].get_to(m_tags);
        } else {
            complete["tags"] = false;
        }

        if(proj.contains("overrides") && proj["overrides"].is_array()) {
            proj["overrides"].get_to(m_overrides);
        } else {
            complete["overrides"] = false;
        }

        // Make sure we got each property that we expect
        if(std::any_of(complete.begin(), complete.end(),
                        [](auto& pair) { return !pair.second; }))
        {
            writeWarning("Project file ", path, " is missing the following expected elements: ");

            std::stringstream ss;
            bool first = true;
            for(auto&& [element, has] : complete) {
                if(!has) {
                    ss << (first ? "" : ",") << element;
                }
            }
            writeWarning<false>("\t", ss.str());
        }
    } else {
        return false;
    }

    // If the data isn't there, we can still finish successfully loading, but we
    //  will not have any of the actual project's data
    // So just log a warning that the data might be missing and move on
    if(!std::filesystem::exists(getMetaRoot())) {
        writeWarning("Project meta directory ", getMetaRoot(),
                     " is missing. This folder contains the actual project "
                     "data, so it missing could imply a loss of data. Please "
                     "verify this path.");
        return true;
    }

    // Load in sub-projects
    return m_map_project.load(getMapRoot());
}

/**
 * @brief Saves a project to the file specified by path.
 *
 * @param path
 *
 * @return True if the project was successfully saved, false otherwise.
 */
bool MapNormalizer::Project::HoI4Project::save(const std::filesystem::path& path)
{
    return save(path, true);
}

/**
 * @brief Saves a project to the file specified by path.
 * @details Format of the project file should be as follows:
 * @code
 *     {
 *         "name": "NameOfProject",
 *         "tool_version": "VersionOfTool",
 *         "hoi4_version": "VersionOfHoI4",
 *         "tags": [ "tag1", "tag2", ... ],
 *         "overrides": [ "relative/path/to/override1", "relative/path/to/override2", ... ]
 *     }
 * @endcode
 *
 * @param path
 * @param do_save_subprojects Should subprojects get saved recursively as well?
 *
 * @return 
 */
bool MapNormalizer::Project::HoI4Project::save(const std::filesystem::path& path,
                                               bool do_save_subprojects)
{
    using json = nlohmann::json;

    if(std::ofstream out(path); out) {
        json proj;

        proj["name"] = m_name;
        proj["tool_version"] = m_tool_version.str();
        proj["hoi4_version"] = m_hoi4_version.str();
        proj["tags"] = m_tags;
        proj["overrides"] = m_overrides;

        out << std::setw(4) << proj << std::endl;
    } else {
        writeError("Failed to write file to ", path, ". Reason: ", std::strerror(errno));
        return false;
    }

    // Make the directory that the sub-projects will get saved to
    if(!std::filesystem::exists(getMetaRoot())) {
        std::filesystem::create_directory(getMetaRoot());
    }

    if(!do_save_subprojects) {
        return true;
    }

    // Save sub-projects
    return m_map_project.save(getMapRoot());
}

bool MapNormalizer::Project::HoI4Project::load() {
    return load(m_path);
}

bool MapNormalizer::Project::HoI4Project::save(bool do_save_subprojects) {
    return save(m_path, do_save_subprojects);
}

/**
 * @brief Sets m_path and m_root
 *
 * @param path
 */
void MapNormalizer::Project::HoI4Project::setPath(const std::filesystem::path& path)
{
    m_path = path;
    m_root = path.parent_path();
}

void MapNormalizer::Project::HoI4Project::setName(const std::string& name) {
    m_name = name;
}

/**
 * @brief Parses out a project name and path
 *
 * @param full_path
 */
void MapNormalizer::Project::HoI4Project::setPathAndName(const std::filesystem::path& full_path) {
    if(full_path.has_filename()) {
        setName(full_path.filename().replace_extension());
    }

    auto path = full_path;
    if(!path.has_extension()) {
        path.replace_extension(PROJ_EXTENSION);
    }

    setPath(path);
}

