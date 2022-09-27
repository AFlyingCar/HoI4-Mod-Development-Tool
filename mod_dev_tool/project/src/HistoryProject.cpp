
#include "HistoryProject.h"

#include "StatusCodes.h"

HMDT::Project::HistoryProject::HistoryProject(IProject& parent_project):
    m_state_project(*this),
    m_parent_project(parent_project)
{
}

HMDT::Project::HistoryProject::~HistoryProject() {
}

auto HMDT::Project::HistoryProject::getStateProject() noexcept -> StateProject&
{
    return m_state_project;
}

auto HMDT::Project::HistoryProject::getStateProject() const noexcept
    -> const StateProject&
{
    return m_state_project;
}

HMDT::Project::IRootProject& HMDT::Project::HistoryProject::getRootParent() {
    return m_parent_project.getRootParent();
}

const HMDT::Project::IRootProject& HMDT::Project::HistoryProject::getRootParent() const
{
    return m_parent_project.getRootParent();
}

auto HMDT::Project::HistoryProject::getRootHistoryParent() noexcept
    -> IRootHistoryProject&
{
    return *this;
}

auto HMDT::Project::HistoryProject::getRootHistoryParent() const noexcept
    -> const IRootHistoryProject&
{
    return *this;
}

HMDT::MaybeVoid HMDT::Project::HistoryProject::save(const std::filesystem::path& path)
{
    WRITE_DEBUG("Saving all history projects to ", path);

    if(!std::filesystem::exists(path)) {
        WRITE_DEBUG("Creating directory ", path);
        std::filesystem::create_directory(path);
    }

    auto states_result = getStateProject().save(path);
    RETURN_IF_ERROR(states_result);

    return STATUS_SUCCESS;
}

HMDT::MaybeVoid HMDT::Project::HistoryProject::load(const std::filesystem::path& path)
{
    WRITE_DEBUG("Loading all history projects from ", path);

    if(auto result = getStateProject().load(path);
            result.error() != std::errc::no_such_file_or_directory)
    {
        RETURN_IF_ERROR(result);
    }

    return STATUS_SUCCESS;
}

HMDT::MaybeVoid HMDT::Project::HistoryProject::export_(const std::filesystem::path& root) const noexcept
{
    auto result = getStateProject().export_(root / "states");
    RETURN_IF_ERROR(result);

    return STATUS_SUCCESS;
}

bool HMDT::Project::HistoryProject::validateData() {
    // TODO
    return true;
}

