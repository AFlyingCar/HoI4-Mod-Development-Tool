
#include "IProject.h"

#include "StatusCodes.h"

////////////////////////////////////////////////////////////////////////////////

HMDT::Project::IRootProject& HMDT::Project::IRootProject::getRootParent() {
    return *this;
}

bool HMDT::Project::IProvinceProject::isValidProvinceLabel(uint32_t label) const
{
    return (label - 1) < getProvinces().size();
}

auto HMDT::Project::IProvinceProject::getProvinceForLabel(uint32_t label) const
    -> const Province&
{
    return getProvinces().at(label - 1);
}

auto HMDT::Project::IProvinceProject::getProvinceForLabel(uint32_t label)
    -> Province&
{
    return getProvinces().at(label - 1);
}

////////////////////////////////////////////////////////////////////////////////

bool HMDT::Project::IStateProject::isValidStateID(StateID state_id) const {
    return getStates().count(state_id) != 0;
}

auto HMDT::Project::IStateProject::getStateForID(StateID state_id) const
    -> MaybeRef<const State>
{
    if(isValidStateID(state_id)) {
        return std::ref(getStates().at(state_id));
    }
    return STATUS_STATE_DOES_NOT_EXIST;
}

auto HMDT::Project::IStateProject::getStateForID(StateID state_id)
    -> MaybeRef<State>
{
    if(isValidStateID(state_id)) {
        return std::ref(getStateMap().at(state_id));
    }
    return STATUS_STATE_DOES_NOT_EXIST;
}

////////////////////////////////////////////////////////////////////////////////

void HMDT::Project::IContinentProject::addNewContinent(const std::string& continent)
{
    getContinents().insert(continent);
}

void HMDT::Project::IContinentProject::removeContinent(const std::string& continent)
{
    getContinents().erase(continent);
}

bool HMDT::Project::IContinentProject::doesContinentExist(const std::string& continent) const
{
    return getContinentList().count(continent) != 0;
}

