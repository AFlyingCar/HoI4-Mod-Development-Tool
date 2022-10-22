
#include "HoI4Project.h"

#include <fstream>
#include <iomanip>
#include <cstring>
#include <cerrno>

#include "nlohmann/json.hpp"

#include "Logger.h"
#include "Constants.h"
#include "StatusCodes.h"

#include "GroupNode.h"
#include "ProjectNode.h"
#include "PropertyNode.h"

HMDT::Project::HoI4Project::HoI4Project():
    m_path(),
    m_root(),
    m_name(),
    m_tool_version(TOOL_VERSION),
    m_hoi4_version(),
    m_tags(),
    m_overrides(),
    m_map_project(*this),
    m_history_project(*this)
{ }

HMDT::Project::HoI4Project::HoI4Project(const std::filesystem::path& path):
    m_path(path),
    m_root(path.parent_path()),
    m_name(),
    m_tool_version(TOOL_VERSION),
    m_hoi4_version(),
    m_tags(),
    m_overrides(),
    m_map_project(*this),
    m_history_project(*this)
{
}

HMDT::Project::HoI4Project::HoI4Project(HoI4Project&& other):
    m_path(std::move(other.m_path)),
    m_root(std::move(other.m_root)),
    m_name(std::move(other.m_name)),
    m_tool_version(std::move(other.m_tool_version)),
    m_hoi4_version(std::move(other.m_hoi4_version)),
    m_tags(std::move(other.m_tags)),
    m_overrides(std::move(other.m_overrides)),
    m_map_project(*this),
    m_history_project(*this)
{ }

const std::filesystem::path& HMDT::Project::HoI4Project::getPath() const {
    return m_path;
}

std::filesystem::path HMDT::Project::HoI4Project::getRoot() const {
    return m_root;
}

std::filesystem::path HMDT::Project::HoI4Project::getMetaRoot() const {
    return getRoot() / PROJ_META_FOLDER;
}

std::filesystem::path HMDT::Project::HoI4Project::getInputsRoot() const
{
    return getMetaRoot() / "inputs";
}

std::filesystem::path HMDT::Project::HoI4Project::getMapRoot() const {
    return getMetaRoot() / "map";
}

std::filesystem::path HMDT::Project::HoI4Project::getHistoryRoot() const {
    return getMetaRoot() / "history";
}

std::filesystem::path HMDT::Project::HoI4Project::getDebugRoot() const {
    return getMetaRoot() / "debug";
}

std::filesystem::path HMDT::Project::HoI4Project::getExportRoot() const {
    return m_export_root;
}

std::filesystem::path HMDT::Project::HoI4Project::getDefaultExportRoot() const
{
    return getMetaRoot() / "out";
}

void HMDT::Project::HoI4Project::setExportRoot(const std::filesystem::path& root)
{
    m_export_root = root;
}

const std::string& HMDT::Project::HoI4Project::getName() const {
    return m_name;
}

auto HMDT::Project::HoI4Project::getToolVersion() const -> const Version& {
    return m_tool_version;
}

auto HMDT::Project::HoI4Project::getHoI4Version() const -> const Version& {
    return m_hoi4_version;
}

const std::vector<std::string>& HMDT::Project::HoI4Project::getTags() const
{
    return m_tags;
}

const std::vector<std::filesystem::path>& HMDT::Project::HoI4Project::getOverrides() const
{
    return m_overrides;
}

auto HMDT::Project::HoI4Project::getMapProject() noexcept -> IRootMapProject& {
    return m_map_project;
}

auto HMDT::Project::HoI4Project::getMapProject() const noexcept
    -> const IRootMapProject&
{
    return m_map_project;
}

auto HMDT::Project::HoI4Project::getHistoryProject() noexcept
    -> IRootHistoryProject&
{
    return m_history_project;
}

auto HMDT::Project::HoI4Project::getHistoryProject() const noexcept
    -> const IRootHistoryProject&
{
    return m_history_project;
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
auto HMDT::Project::HoI4Project::load(const std::filesystem::path& path)
    -> MaybeVoid
{
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
            std::vector<std::string> overrides;
            proj["overrides"].get_to(overrides);

            std::transform(overrides.begin(), overrides.end(),
                           std::back_inserter(m_overrides),
                           [](const std::string& path) {
                               return std::filesystem::path(path);
                           });
        } else {
            complete["overrides"] = false;
        }

        // Make sure we got each property that we expect
        if(std::any_of(complete.begin(), complete.end(),
                        [](auto& pair) { return !pair.second; }))
        {
            WRITE_WARN("Project file ", path, " is missing the following expected elements: ");

            std::stringstream ss;
            bool first = true;
            for(auto&& [element, has] : complete) {
                if(!has) {
                    ss << (first ? "" : ",") << element;
                }
            }
            WRITE_WARN("\t", ss.str());
        }
    } else {
        RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
    }

    // If the data isn't there, we can still finish successfully loading, but we
    //  will not have any of the actual project's data
    // So just log a warning that the data might be missing and move on
    if(!std::filesystem::exists(getMetaRoot())) {
        WRITE_WARN("Project meta directory ", getMetaRoot(),
                   " is missing. This folder contains the actual project "
                   "data, so it missing could imply a loss of data. Please "
                   "verify this path.");
        return STATUS_SUCCESS;
    }

    // Load in sub-projects
    auto result = m_map_project.load(getMapRoot());
    RETURN_IF_ERROR(result);

    result = m_history_project.load(getHistoryRoot());
    RETURN_IF_ERROR(result);

    ////////////////////////////////////////////////////////////////////////////

    RETURN_ERROR_IF(!validateData(), STATUS_PROJECT_VALIDATION_FAILED);

    return STATUS_SUCCESS;
}

/**
 * @brief Saves a project to the file specified by path.
 *
 * @param path
 *
 * @return True if the project was successfully saved, false otherwise.
 */
auto HMDT::Project::HoI4Project::save(const std::filesystem::path& path)
    -> MaybeVoid
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
auto HMDT::Project::HoI4Project::save(const std::filesystem::path& path,
                                      bool do_save_subprojects)
    -> MaybeVoid
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
        WRITE_ERROR("Failed to write file to ", path, ". Reason: ", std::strerror(errno));
        RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
    }

    // Make the directory that the sub-projects will get saved to
    if(!std::filesystem::exists(getMetaRoot())) {
        std::filesystem::create_directory(getMetaRoot());
    }

    if(!do_save_subprojects) {
        return STATUS_SUCCESS;
    }

    // Save sub-projects
    auto result = m_map_project.save(getMapRoot());
    RETURN_IF_ERROR(result);

    result = m_history_project.save(getHistoryRoot());
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

auto HMDT::Project::HoI4Project::export_(const std::filesystem::path& root) const noexcept
    -> MaybeVoid
{
    std::error_code fs_ec;

    WRITE_DEBUG("Exporting to ", root);

    // First create the root export path if it doesn't exist
    if(!std::filesystem::exists(root, fs_ec)) {
        RETURN_ERROR_IF(fs_ec.value() != 0 &&
                        fs_ec != std::errc::no_such_file_or_directory,
                        fs_ec);

        auto result = std::filesystem::create_directory(root, fs_ec);

        RETURN_ERROR_IF(!result, fs_ec);
    }

    MaybeVoid result;

    // Export descriptor.mod
    if(std::ofstream descriptor(root / "descriptor.mod"); descriptor) {
        descriptor << "version=\"" << m_hoi4_version << "\"" << std::endl;
        descriptor << "tags={" << std::endl;
        for(auto&& tag : m_tags) {
            descriptor << "\t\"" << tag << '"' << std::endl;
        }
        descriptor << "}" << std::endl;
        // TODO: Do we want to force all thumbnails to be the same filename?
        descriptor << "picture=\"thumbnail.png\"" << std::endl;
        // TODO: How can we handle if dependencies exist?
        // descriptor << "dependencies={" << std::endl;
        //   list each dependency
        // descriptor << "}" << std::endl;
        descriptor << "name=\"" << m_name << '"' << std::endl;
        // TODO: Is this supposed to be identical to version?
        descriptor << "supported_version=\"" << m_hoi4_version << "\"" << std::endl;

    } else {
        WRITE_ERROR("Failed to open file ", root);
        RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
    }

    result = m_map_project.export_(root / "map");
    RETURN_IF_ERROR(result);

    result = m_history_project.export_(root / "history");
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

HMDT::MaybeVoid HMDT::Project::HoI4Project::load() {
    return load(m_path);
}

HMDT::MaybeVoid HMDT::Project::HoI4Project::save(bool do_save_subprojects) {
    return save(m_path, do_save_subprojects);
}

HMDT::MaybeVoid HMDT::Project::HoI4Project::export_() const noexcept {
    return export_(getExportRoot());
}

/**
 * @brief Sets m_path and m_root
 *
 * @param path
 */
void HMDT::Project::HoI4Project::setPath(const std::filesystem::path& path) {
    m_path = path;
    m_root = path.parent_path();
}

void HMDT::Project::HoI4Project::setName(const std::string& name) {
    m_name = name;
}

/**
 * @brief Parses out a project name and path
 *
 * @param full_path
 */
void HMDT::Project::HoI4Project::setPathAndName(const std::filesystem::path& full_path)
{
    if(full_path.has_filename()) {
        setName(full_path.filename().replace_extension().generic_string());
    }

    auto path = full_path;
    if(!path.has_extension()) {
        path.replace_extension(PROJ_EXTENSION);
    }

    setPath(path);
}

void HMDT::Project::HoI4Project::setToolVersion(const Version& version) {
    m_tool_version = version;
}

void HMDT::Project::HoI4Project::setHoI4Version(const Version& version) {
    m_hoi4_version = version;
}

bool HMDT::Project::HoI4Project::validateData() {
    return m_map_project.validateData() &&
           m_history_project.validateData();
}

auto HMDT::Project::HoI4Project::visit(const std::function<MaybeVoid(Hierarchy::INode&)>& visitor) const noexcept
    -> Maybe<std::shared_ptr<Hierarchy::INode>>
{
    auto hoi4_project_node = std::make_shared<Hierarchy::ProjectNode>("Root");

    auto result = visitor(*hoi4_project_node);
    RETURN_IF_ERROR(result);

    auto name_node = std::make_shared<Hierarchy::PropertyNode<std::string>>("Name", const_cast<std::string&>(m_name));
    result = visitor(*name_node);
    RETURN_IF_ERROR(result);
    hoi4_project_node->addChild(name_node);

    auto hoi4_version_node = std::make_shared<Hierarchy::PropertyNode<Version>>("HoI4 Version", const_cast<Version&>(m_hoi4_version));
    result = visitor(*hoi4_version_node);
    RETURN_IF_ERROR(result);
    hoi4_project_node->addChild(hoi4_version_node);

    auto tags_node = std::make_shared<Hierarchy::GroupNode>("Tags");
    result = visitor(*tags_node);
    RETURN_IF_ERROR(result);

    for(auto&& tag : m_tags) {
        auto tag_node = std::make_shared<Hierarchy::ConstPropertyNode<std::string>>(tag);

        result = visitor(*tag_node);
        RETURN_IF_ERROR(result);

        result = tags_node->addChild(tag_node);
        RETURN_IF_ERROR(result);
    }

    hoi4_project_node->addChild(tags_node);

    ////////////////////////////////////////////////////////////////////////////

    result = getMapProject().visit(visitor)
        .andThen([&hoi4_project_node](auto map_project_node) -> MaybeVoid {
            auto result = hoi4_project_node->addChild(map_project_node);
            RETURN_IF_ERROR(result);

            return STATUS_SUCCESS;
        });
    RETURN_IF_ERROR(result);

    result = getHistoryProject().visit(visitor)
        .andThen([&hoi4_project_node](auto history_project_node) -> MaybeVoid {
            auto result = hoi4_project_node->addChild(history_project_node);
            RETURN_IF_ERROR(result);

            return STATUS_SUCCESS;
        });
    RETURN_IF_ERROR(result);

    return hoi4_project_node;
}

