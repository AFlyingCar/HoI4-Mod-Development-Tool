
#include "IProject.h"

#include <queue>

#include "StatusCodes.h"
#include "Constants.h"

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

auto HMDT::Project::IProvinceProject::getRootProvinceParent(const ProvinceID& id) const noexcept
    -> MaybeRef<const Province>
{
    for(ProvinceID root_id = id; root_id != INVALID_PROVINCE;) {
        if(!isValidProvinceID(root_id)) {
            RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
        }

        const Province& province = getProvinceForID(root_id);

        // When we find an invalid province parent ID, then we are at the root
        if(province.id == INVALID_PROVINCE) {
            return province;
        }

        root_id = province.parent_id;
    }

    // We should never reach here. To do so implies that root_id somehow became
    //   INVALID_PROVINCE and we ended up _not_ returning in the middle of it.
    RETURN_ERROR(STATUS_UNEXPECTED);
}

auto HMDT::Project::IProvinceProject::getRootProvinceParent(const ProvinceID& id) noexcept
    -> MaybeRef<Province>
{
    for(ProvinceID root_id = id; root_id != INVALID_PROVINCE;) {
        if(!isValidProvinceID(root_id)) {
            RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
        }

        Province& province = getProvinceForID(root_id);

        // When we find an invalid province parent ID, then we are at the root
        if(province.parent_id == INVALID_PROVINCE) {
            return province;
        }

        root_id = province.parent_id;
    }

    // We should never reach here. To do so implies that root_id somehow became
    //   INVALID_PROVINCE and we ended up _not_ returning in the middle of it.
    RETURN_ERROR(STATUS_UNEXPECTED);
}

/**
 * @brief Merges two provinces together so that they can be treated as one
 *        single unit.
 *
 * @param id1 One of the IDs to merge.
 * @param id2 One of the IDs to merge.
 *
 * @return STATUS_SUCCESS upon success, or a failure code otherwise.
 */
auto HMDT::Project::IProvinceProject::mergeProvinces(const ProvinceID& id1,
                                                     const ProvinceID& id2) noexcept
    -> MaybeVoid
{
    if(!isValidProvinceID(id1) || !isValidProvinceID(id2)) {
        WRITE_ERROR("One of the following IDs is invalid: ", id1, " or ", id2);
        RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
    }

    // First see if we can just add either as a parent
    if(Province& prov1 = getProvinceForID(id1);
       prov1.parent_id == INVALID_PROVINCE)
    {
        // But make sure that we only set the parent to the other's own parent
        auto maybe_root = getRootProvinceParent(id2);
        RETURN_IF_ERROR(maybe_root);

        prov1.parent_id = maybe_root->get().id;
        maybe_root->get().children.insert(prov1.id);
        return STATUS_SUCCESS;
    }

    if(Province& prov2 = getProvinceForID(id2);
       prov2.parent_id == INVALID_PROVINCE)
    {
        // But make sure that we only set the parent to the other's own parent
        auto maybe_root = getRootProvinceParent(id1);
        RETURN_IF_ERROR(maybe_root);

        prov2.parent_id = maybe_root->get().id;
        maybe_root->get().children.insert(prov2.id);
        return STATUS_SUCCESS;
    }

    // Since we cannot, find the root of either of the two provinces and set
    //   its parent to the root parent of the other
    auto maybe_root1 = getRootProvinceParent(id1);
    RETURN_IF_ERROR(maybe_root1);

    auto maybe_root2 = getRootProvinceParent(id2);
    RETURN_IF_ERROR(maybe_root2);

    maybe_root1->get().parent_id = maybe_root2->get().id;
    maybe_root2->get().children.insert(maybe_root1->get().id);

    return STATUS_SUCCESS;
}

/**
 * @brief Gets all provinces that have been merged together with the provided
 *        province.
 * @details Performs a breadth first search
 *
 * @return Will return a set of all connected/merged province ids, or an empty
 *         list if the province id is invalid. If the ID is valid, the list will
 *         contain at least one element.
 */
auto HMDT::Project::IProvinceProject::getMergedProvinces(const ProvinceID& id) const noexcept
    -> std::set<ProvinceID>
{
    WRITE_DEBUG("Getting all provinces merged with ", id);

    std::set<ProvinceID> connected_provinces;

    std::queue<ProvinceID> to_search;
    to_search.push(id);

    while(!to_search.empty()) {
        const auto& next_id = to_search.front();
        to_search.pop();

        WRITE_DEBUG("Check ", next_id);

        if(!isValidProvinceID(next_id)) {
            WRITE_ERROR("Invalid province id ", next_id);

            continue;
        }

        // Check if we've already visisted this id. do this to prevent loops
        if(connected_provinces.count(next_id) == 0) {
            const auto& next_province = getProvinceForID(next_id);

            // Mark that we've visited this id
            connected_provinces.insert(next_id);

            // Add parent first if we haven't visited it yet
            if(const auto& parent = next_province.parent_id;
                    connected_provinces.count(parent) == 0)
            {
                // Make sure that we don't push invalid provinces into the list
                //   if this province doesn't have a parent.
                if(parent != INVALID_PROVINCE) {
                    WRITE_DEBUG("Add parent ", parent);
                    to_search.push(parent);
                }
            }

            // Add any children if we haven't visited them yet.
            for(auto&& child_id : next_province.children) {
                if(connected_provinces.count(child_id) == 0) {
                    // Get the child as well and perform a quick check on its
                    //   parent to warn about any discrepencies
                    if(const auto& child = getProvinceForID(child_id);
                            child.parent_id != next_id)
                    {
                        WRITE_WARN("Child province ", child_id, " is listed as "
                                   "a child of ", next_id, ", but its own parent "
                                   "is ", child.parent_id);
                    }

                    WRITE_DEBUG("Add child ", child_id);
                    to_search.push(child_id);
                }
            }
        }
    }

    return connected_provinces;
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

