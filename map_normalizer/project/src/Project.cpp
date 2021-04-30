
#include "Project.h"

#include <fstream>
#include <iomanip>
#include <cstring>
#include <cerrno>

#include "nlohmann/json.hpp"

#include "Logger.h"
#include "Constants.h"

MapNormalizer::Project::HoI4Project::HoI4Project():
    m_root(),
    m_name(),
    m_tool_version(),
    m_hoi4_version(),
    m_tags(),
    m_overrides()
{ }

MapNormalizer::Project::HoI4Project::HoI4Project(const std::filesystem::path& path):
    m_root(path),
    m_name(),
    m_tool_version(),
    m_hoi4_version(),
    m_tags(),
    m_overrides()
{
    load();
}

MapNormalizer::Project::HoI4Project::HoI4Project(const HoI4Project& other):
    m_root(other.m_root),
    m_name(other.m_name),
    m_tool_version(other.m_tool_version),
    m_hoi4_version(other.m_hoi4_version),
    m_tags(other.m_tags),
    m_overrides(other.m_overrides)
{ }

MapNormalizer::Project::HoI4Project::HoI4Project(HoI4Project&& other):
    m_root(std::move(other.m_root)),
    m_name(std::move(other.m_name)),
    m_tool_version(std::move(other.m_tool_version)),
    m_hoi4_version(std::move(other.m_hoi4_version)),
    m_tags(std::move(other.m_tags)),
    m_overrides(std::move(other.m_overrides))
{ }

auto MapNormalizer::Project::HoI4Project::operator=(const HoI4Project& other)
    -> HoI4Project&
{
    m_root = other.m_root;
    m_name = other.m_name;
    m_tool_version = other.m_tool_version;
    m_hoi4_version = other.m_hoi4_version;
    m_tags = other.m_tags;
    m_overrides = other.m_overrides;

    return *this;
}

const std::filesystem::path& MapNormalizer::Project::HoI4Project::getPath() const {
    return m_root;
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

bool MapNormalizer::Project::HoI4Project::load(const std::filesystem::path& path) {
    using json = nlohmann::json;

    if(std::ifstream in(path); in) {
        json proj;

        in >> proj;

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
    auto projmeta_path = path.parent_path() / PROJ_META_FOLDER;
    if(!std::filesystem::exists(projmeta_path)) {
        writeWarning("Project meta directory ", projmeta_path,
                     " is missing. This folder contains the actual project "
                     "data, so it missing could imply a loss of data. Please "
                     "verify this path.");
        return true;
    }

    // Load in sub-projects
    return m_map_project.load(projmeta_path / "map");
}

bool MapNormalizer::Project::HoI4Project::save(const std::filesystem::path& path) {
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

    auto projmeta_path = path.parent_path() / PROJ_META_FOLDER;

    // Make the directory that the sub-projects will get saved to
    std::filesystem::create_directory(projmeta_path);

    // Save sub-projects
    return m_map_project.save(projmeta_path / "map");
}

bool MapNormalizer::Project::HoI4Project::load() {
    return load(m_root);
}

bool MapNormalizer::Project::HoI4Project::save() {
    return save(m_root);
}

void MapNormalizer::Project::HoI4Project::setPath(const std::filesystem::path& path)
{
    m_root = path;
}

void MapNormalizer::Project::HoI4Project::setName(const std::string& name) {
    m_name = name;
}

void MapNormalizer::Project::HoI4Project::setPathAndName(const std::filesystem::path& full_path) {
    if(full_path.has_filename()) {
        setName(full_path.filename().replace_extension());
    }

    auto path = full_path;
    if(!path.has_extension()) {
        path.replace_extension(".hoi4proj");
    }

    setPath(path);
}

