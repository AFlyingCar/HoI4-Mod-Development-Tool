
#include "HistoryProject.h"

#include "StatusCodes.h"

#include "ProjectNode.h"

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

auto HMDT::Project::HistoryProject::visit(const std::function<MaybeVoid(Hierarchy::INode&)>& visitor) const noexcept
    -> Maybe<std::shared_ptr<Hierarchy::INode>>
{
    auto history_project_node = std::make_shared<Hierarchy::ProjectNode>("History");

    auto result = visitor(*history_project_node);
    RETURN_IF_ERROR(result);

    result = getStateProject().visit(visitor)
        .andThen([&history_project_node](auto state_project_node) -> MaybeVoid {
            auto result = history_project_node->addChild(state_project_node);
            RETURN_IF_ERROR(result);

            return STATUS_SUCCESS;
        });
    RETURN_IF_ERROR(result);

    return history_project_node;
}

