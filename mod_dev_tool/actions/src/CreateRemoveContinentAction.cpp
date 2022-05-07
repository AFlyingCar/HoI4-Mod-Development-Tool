
#include "CreateRemoveContinentAction.h"

#include "Logger.h"

HMDT::Action::CreateRemoveContinentAction::CreateRemoveContinentAction(
        Project::MapProject& map_project,
        const std::string& continent_name,
        Type type):
    m_map_project(map_project),
    m_continent_name(continent_name),
    m_type(type)
{ }

bool HMDT::Action::CreateRemoveContinentAction::doAction(const Callback& callback)
{
    if(!callback(0)) return false;

    switch(m_type) {
        case Type::CREATE:
            if(!create()) return false;
            break;
        case Type::REMOVE:
            if(!remove()) return false;
            break;
    }

    if(!callback(1)) return false;

    return true;
}

bool HMDT::Action::CreateRemoveContinentAction::undoAction(const Callback& callback)
{

    if(!callback(0)) return false;

    switch(m_type) {
        case Type::CREATE:
            if(!remove()) return false;
            break;
        case Type::REMOVE:
            if(!create()) return false;
            break;
    }

    if(!callback(1)) return false;

    return true;
}

bool HMDT::Action::CreateRemoveContinentAction::create() {
    if(m_map_project.doesContinentExist(m_continent_name)) {
        WRITE_ERROR("Continent ", m_continent_name, " does not exist.");
        return false;
    }

    WRITE_DEBUG("Creating continent ", m_continent_name);
    m_map_project.addNewContinent(m_continent_name);

    return true;
}

bool HMDT::Action::CreateRemoveContinentAction::remove() {
    if(!m_map_project.doesContinentExist(m_continent_name)) {
        WRITE_ERROR("Continent ", m_continent_name, " does not exist.");
        return false;
    }

    WRITE_DEBUG("Removing continent ", m_continent_name);
    m_map_project.removeContinent(m_continent_name);

    return true;
}

