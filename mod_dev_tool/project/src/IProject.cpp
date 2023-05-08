
#include "IProject.h"

#include "StatusCodes.h"

HMDT::Project::IProject::IProject() {
    resetPromptCallback();
}

/**
 * @brief Sets the prompt callback
 *
 * @param callback The callback to set
 */
void HMDT::Project::IProject::setPromptCallback(const PromptCallback& callback)
{
    m_prompt_callback = callback;
}

/**
 * @brief Resets the prompt callback
 */
void HMDT::Project::IProject::resetPromptCallback() {
    m_prompt_callback = [this](auto&&... args) {
        return defaultPromptCallback(args...);
    };
}

auto HMDT::Project::IProject::getPromptCallback() const noexcept
    -> const PromptCallback&
{
    return m_prompt_callback;
}

/**
 * @brief Prompts the user with a question for some response.
 * @details This is meant for cases where load/save/export/import can continue
 *          based on some input from the user. If no callback has been
 *          registered then STATUS_CALLBACK_NOT_REGISTERED is returned.
 *
 * @param prompt The prompt to ask the user
 * @param opts The options to present the user with
 *
 * @return Maybe an index into opts, referencing which option was chosen. If no
 *         callback has been registered then STATUS_CALLBACK_NOT_REGISTERED is
 *         returned instead.
 */
auto HMDT::Project::IProject::prompt(const std::string& prompt,
                                     const std::vector<std::string>& opts,
                                     const PromptType& type) const
    -> Maybe<uint32_t>
{
    return m_prompt_callback(prompt, opts, type);
}

auto HMDT::Project::IProject::defaultPromptCallback(const std::string& prompt,
                                                    const std::vector<std::string>& opts,
                                                    const PromptType& type)
    -> Maybe<uint32_t>
{
    if(&getRootParent() != this) {
        return getRootParent().getPromptCallback()(prompt, opts, type);
    }

    RETURN_ERROR(STATUS_CALLBACK_NOT_REGISTERED);
}

////////////////////////////////////////////////////////////////////////////////

HMDT::Project::IRootProject& HMDT::Project::IRootProject::getRootParent() {
    return *this;
}

const HMDT::Project::IRootProject& HMDT::Project::IRootProject::getRootParent() const
{
    return *this;
}

bool HMDT::Project::IProvinceProject::isValidProvinceLabel(uint32_t label) const
{
    return getProvinces().count(HashOnlyUUID(label)) != 0;
}

bool HMDT::Project::IProvinceProject::isValidProvinceID(ProvinceID label) const
{
    return getProvinces().count(label) != 0;
}

auto HMDT::Project::IProvinceProject::getProvinceForID(ProvinceID id) const
    -> const Province&
{
    return getProvinces().at(id);
}

auto HMDT::Project::IProvinceProject::getProvinceForID(ProvinceID id)
    -> Province&
{
    return getProvinces().at(id);
}

auto HMDT::Project::IProvinceProject::getProvinceForLabel(uint32_t label) const
    -> const Province&
{
    return getProvinces().at(HashOnlyUUID(label));
}

auto HMDT::Project::IProvinceProject::getProvinceForLabel(uint32_t label)
    -> Province&
{
    return getProvinces().at(HashOnlyUUID(label));
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

