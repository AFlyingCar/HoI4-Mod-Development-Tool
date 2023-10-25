
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
        if(province.parent_id == INVALID_PROVINCE) {
            return province;
        }

        if(province.parent_id == root_id) {
            WRITE_ERROR("Province ", province.id,
                        " is marked as its own parent! This should never be possible to happen.");
            RETURN_ERROR(STATUS_UNEXPECTED);
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

        if(province.parent_id == root_id) {
            WRITE_ERROR("Province ", province.id,
                        " is marked as its own parent! This should never be possible to happen.");
            RETURN_ERROR(STATUS_UNEXPECTED);
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
    WRITE_DEBUG("Merging ", id1, " and ", id2);

    if(id1 == id2) {
        WRITE_ERROR("Cannot merge a province with itself.");
        RETURN_ERROR(STATUS_UNEXPECTED);
    }

    if(!isValidProvinceID(id1) || !isValidProvinceID(id2)) {
        WRITE_ERROR("One of the following IDs is invalid: ", id1, " or ", id2);
        RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
    }

    // Find the root of either of the two provinces and set
    //   its parent to the root parent of the other
    auto maybe_root1 = getRootProvinceParent(id1);
    RETURN_IF_ERROR(maybe_root1);

    auto maybe_root2 = getRootProvinceParent(id2);
    RETURN_IF_ERROR(maybe_root2);

    if(maybe_root1->get().id == maybe_root2->get().id) {
        WRITE_ERROR("Both provinces are already merged.");
        RETURN_ERROR(STATUS_UNEXPECTED);
    }

    WRITE_DEBUG("Setting root1 (", maybe_root1->get().id, ") parent=",
                maybe_root2->get().id);

    maybe_root1->get().parent_id = maybe_root2->get().id;
    maybe_root2->get().children.insert(maybe_root1->get().id);

    WRITE_DEBUG("New child tree after merging:\n",
                genProvinceChildTree(maybe_root2->get().id).orElse(""));

    return STATUS_SUCCESS;
}

/**
 * @brief Removes the specified province from its parent, "un-merging" them
 *
 * @param id The ID of the province to remove from its parent.
 *
 * @return STATUS_SUCCESS upon success, or a failure code otherwise.
 */
auto HMDT::Project::IProvinceProject::unmergeProvince(const ProvinceID& id) noexcept
    -> MaybeVoid
{
    if(!isValidProvinceID(id)) {
        WRITE_ERROR("The province ID is invalid: ", id);
        RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
    }

    // Get the province
    Province& province = getProvinceForID(id);

    WRITE_DEBUG("Un-merging province ", id, " whose parent is ",
                province.parent_id, " and which has ", province.children.size(),
                " children.");

    // If parent id is set to nil, instead try to remove from top->down
    if(province.parent_id == INVALID_PROVINCE) {
        if(province.children.size() == 0) {
            WRITE_WARN("Attempted to unmerge a province that doesn't have a "
                       "parent and has no children. Doing nothing instead.");
            return STATUS_SUCCESS;
        }

        // Go through every child and first make sure they all exist as we only
        //   want this operation to do anything if it will succeed (failure
        //   should be a no-op)
        RefVector<Province> child_provinces;
        child_provinces.reserve(province.children.size());
        for(auto&& child_id : province.children) {
            if(!isValidProvinceID(child_id)) {
                WRITE_ERROR("This province has an invalid child id: ", child_id);
                RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
            }

            child_provinces.push_back(getProvinceForID(child_id));
        }

        // Pick a new parent from our children
        auto new_parent_id = child_provinces.front().get().id;
        Province& new_parent = getProvinceForID(new_parent_id);

        for(auto& child : child_provinces) {
            // Make sure we don't mark a child as its own parent
            if(child.get().id != new_parent_id) {
                child.get().parent_id = new_parent_id;
                new_parent.children.insert(child.get().id);
            } else {
                // For the new parent, make it have a nil parent id
                child.get().parent_id = INVALID_PROVINCE;
            }
        }

        // Remove all of our children
        province.children.clear();

        return STATUS_SUCCESS;
    }

    // If the parent id is not nil, but still isn't a valid ID, then return an
    //   error
    if(!isValidProvinceID(province.parent_id)) {
        WRITE_ERROR("This province does not have a valid parent id: ", province.parent_id);
        RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
    }

    WRITE_DEBUG("\n", genProvinceChildTree(province.parent_id).orElse(""));

    // Try and find our root parent so we can remove ourselves from their list
    //   of children
    auto maybe_root = getRootProvinceParent(province.parent_id);
    RETURN_IF_ERROR(maybe_root);

    auto& root_children = maybe_root->get().children;

    // Get all of our children and make sure that they know who the new parent is
    // Go through every child and first make sure they all exist as we only
    //   want this operation to do anything if it will succeed (failure
    //   should be a no-op)
    RefVector<Province> child_provinces;
    child_provinces.reserve(province.children.size());
    for(auto&& child_id : province.children) {
        if(!isValidProvinceID(child_id)) {
            WRITE_ERROR("This province has an invalid child id: ", child_id);
            RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
        }

        child_provinces.push_back(getProvinceForID(child_id));
    }

    // Make sure the province is in its parent's children list before attempting to
    //   remove, and issue a warning if we cannot find it.
    if(auto c = root_children.count(id); c == 0) {
        // Log the root's children for debugging, since we really shouldn't hit
        //   this branch unless something has gone wrong.
        std::stringstream ss;
        for(auto&& c : root_children)
            ss << c << ',';

        WRITE_WARN("Child ID ", id, " not found in its root parent's children's list of"
                   " children: {", ss.str(), '}');
    } else {
        try {
            root_children.erase(id);
        } catch(...) {
            WRITE_ERROR("std::set threw unexpected exception.");
            RETURN_ERROR(STATUS_UNEXPECTED);
        }
    }

    // Once we have all of our children, then update their parent to be our
    //   parent
    WRITE_DEBUG("Modifying ", child_provinces.size(),
                " children's parents to be ", province.parent_id);
    for(auto& child : child_provinces) {
        child.get().parent_id = province.parent_id;

        // Make sure to insert the child into it's new parent's list of children
        root_children.insert(child.get().id);
    }

    // Remove the province's parent id to unlink it fully from the parent.
    province.parent_id = INVALID_PROVINCE;

    // Make sure that after all of this we end up with no children.
    province.children.clear();

    return STATUS_SUCCESS;
}

auto HMDT::Project::IProvinceProject::genProvinceChildTree(ProvinceID id) const noexcept
    -> Maybe<std::string>
{
    constexpr std::size_t MAX_RECURSE_DEPTH = 32;

    // Try and find our root parent so we can remove ourselves from their list
    //   of children
    auto maybe_root = getRootProvinceParent(id);
    RETURN_IF_ERROR(maybe_root);

    id = maybe_root->get().id;

    std::stringstream ss;

    // root should never have a parent, but print it out anyway for debugging
    ss << maybe_root->get().id
       << "  <-  "
       << maybe_root->get().parent_id
       << std::endl;

    std::function<MaybeVoid(const Province&, const std::string&, std::size_t, bool)> builder;
    builder = [this, &ss, &builder](const Province& p,
                                    const std::string& prefix,
                                    std::size_t depth,
                                    bool is_last)
        -> MaybeVoid
    {
        if(depth > MAX_RECURSE_DEPTH) {
            RETURN_ERROR(STATUS_RECURSION_TOO_DEEP);
        }

        ss << prefix;

        auto c = 0; // counter for how many children we've consumed
        for(auto&& child_id : p.children) {
            if(!isValidProvinceID(child_id)) {
                RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
            }

            const auto& child = getProvinceForID(child_id);

            // print the strand only if the child has children of its own
            bool print_strand = true;// child.children.size() > 0;

            auto new_prefix = prefix + (print_strand ? "│   " : "    ");

            if(is_last) {
                ss << new_prefix << "└── ";
            } else {
                ss << new_prefix << "├── ";
            }

            ss << child_id << "  <-  " << child.parent_id << std::endl;
            auto result = builder(child, new_prefix, depth + 1, (c == (p.children.size() - 1)));
            RETURN_IF_ERROR(result);

            ++c;
        }

        return STATUS_SUCCESS;
    };

    auto result = builder(maybe_root->get(), "", 0, false);
    RETURN_IF_ERROR(result);

    return ss.str();
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

