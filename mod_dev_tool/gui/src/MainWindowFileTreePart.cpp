#include "MainWindowFileTreePart.h"

#include "gtkmm/icontheme.h"

#include "StatusCodes.h"
#include "Maybe.h"
#include "Util.h"
#include "Options.h"

#include "Driver.h"

#include "LinkNode.h"
#include "ProvinceNode.h"
#include "StateNode.h"

//! Helper boolean for optionally enabling logs that tend to spam the console
static bool _spam_logs = false;

std::int32_t HMDT::GUI::MainWindowFileTreePart::HierarchyModel::next_stamp = 0;

/**
 * @brief Constructs a new HierarchyModel
 *
 * @param node The root node of the hierarchy
 * @param parent_map A mapping of all nodes to their parent
 * @param ordered_children_map A map of all nodes with their children in a
 *                             defined order
 * @param node_index_map A map of all nodes with their index
 */
HMDT::GUI::MainWindowFileTreePart::HierarchyModel::HierarchyModel(Project::Hierarchy::INodePtr node,
                                                                  const ParentMap& parent_map,
                                                                  const OrderedChildrenMap& ordered_children_map,
                                                                  const NodeIndexMap& node_index_map):
    Glib::ObjectBase(typeid(HierarchyModel)), // Register a custom GType
    Glib::Object(), // The custom GType is actually registered here
    m_project_hierarchy(node),
    m_parent_map(parent_map),
    m_ordered_children_map(ordered_children_map),
    m_node_index_map(node_index_map),
    m_stamp(++next_stamp)
{ }

/**
 * @brief Gets the project hierarchy
 *
 * @return The project hierarchy
 */
auto HMDT::GUI::MainWindowFileTreePart::HierarchyModel::getHierarchy() noexcept
    -> std::shared_ptr<Project::Hierarchy::INode>
{
    return m_project_hierarchy;
}

/**
 * @brief Gets the flags for this tree model
 *
 * @return TreeModelFlags(0)
 */
Gtk::TreeModelFlags HMDT::GUI::MainWindowFileTreePart::HierarchyModel::get_flags_vfunc() const
{
    return Gtk::TreeModelFlags(0);
}

/**
 * @brief Gets the number of columns
 *
 * @return Columns::MAX
 */
int HMDT::GUI::MainWindowFileTreePart::HierarchyModel::get_n_columns_vfunc() const {
    return static_cast<int>(Columns::MAX);
}

/**
 * @brief Gets the type of a given column
 *
 * @param index The index of the column to get the type of
 *
 * @return The type for the given column, or G_TYPE_INVALID if the index is not
 *         valid.
 */
GType HMDT::GUI::MainWindowFileTreePart::HierarchyModel::get_column_type_vfunc(int index) const
{
    switch(static_cast<Columns>(index)) {
        case Columns::NAME:
        case Columns::VALUE:
        case Columns::TOOLTIP:
            return Glib::Value<Glib::ustring>::value_type();
        case Columns::TYPE:
            return Glib::Value<Glib::RefPtr<Gdk::Pixbuf>>::value_type();
        default:
            WRITE_ERROR("Invalid column index ", index);
            return G_TYPE_INVALID;
    }
}

/**
 * @brief Gets the value for the iterator in the given column
 *
 * @param iter The iterator to specify the node
 * @param column The column to get a value for
 * @param value The object to write the value into
 */
void HMDT::GUI::MainWindowFileTreePart::HierarchyModel::get_value_vfunc(const iterator& iter,
                                                                        int column,
                                                                        Glib::ValueBase& value) const
{
    // WRITE_DEBUG("get_value_vfunc(", std::to_string(*static_cast<Project::Hierarchy::INode*>(iter.gobj()->user_data)), ", ", column, ")");

    if(!isValid(iter)) {
        WRITE_WARN("Invalid iterator given, cannot get value.");
        return;
    }

    // Get the current node on the iterator
    auto* node = static_cast<Project::Hierarchy::INode*>(iter.gobj()->user_data);

    getValueFromNode(node->shared_from_this(), column, value);
}

/**
 * @brief Gets a value for a given node
 *
 * @param node The node to get the value from
 * @param column The column to get the value for
 * @param value The object to write the value to
 *
 * @return STATUS_SUCCESS if successful, a failure code otherwise.
 */
auto HMDT::GUI::MainWindowFileTreePart::HierarchyModel::getValueFromNode(Project::Hierarchy::INodePtr node,
                                                                         int column,
                                                                         Glib::ValueBase& value) const noexcept
    -> MaybeVoid
{
    if(node == nullptr) {
        WRITE_WARN("Iter has a null node. Cannot get a value from it.");
        RETURN_ERROR(STATUS_PARAM_CANNOT_BE_NULL);
    }

    switch(static_cast<Columns>(column)) {
        case Columns::NAME:
        {
            // set the value to the name
            Glib::Value<Glib::ustring> v;
            v.init(v.value_type());
            v.set(node->getName());

            value.init(v.gobj());
            break;
        }
        case Columns::TYPE:
        {
            Glib::RefPtr<Gdk::Pixbuf> icon = MainWindowFileTreePart::getTypeIcon(node->getType());

            Glib::Value<Glib::RefPtr<Gdk::Pixbuf>> v;
            v.init(v.value_type());
            v.set(icon);

            value.init(v.gobj());
            break;
        }
        case Columns::VALUE:
        {
            using namespace std::string_literals;

            // set the value to the name
            Glib::Value<Glib::ustring> v;
            v.init(v.value_type());

            switch(node->getType()) {
                case Project::Hierarchy::Node::Type::GROUP:
                case Project::Hierarchy::Node::Type::PROJECT:
                case Project::Hierarchy::Node::Type::PROVINCE:
                case Project::Hierarchy::Node::Type::STATE:
                    // For group-like nodes, don't display any value
                    break;
                case Project::Hierarchy::Node::Type::LINK:
                    // Link nodes are a special case, they should essentially
                    //  just _be_ the thing they are pointing at
                    if(auto lnode = std::dynamic_pointer_cast<Project::Hierarchy::ILinkNode>(node);
                            lnode != nullptr)
                    {
                        auto result = getValueFromNode(lnode->getLinkedNode(),
                                                       column, value);
                        RETURN_IF_ERROR(result);

                        // Return here to prevent Gtk errors of initializing
                        //   'value' multiple times.
                        return STATUS_SUCCESS;
                    } else {
                        WRITE_WARN("Node ", std::to_string(*node, true),
                                   " is marked as a link node, but we failed to"
                                   " cast it to an ILinkNode object.");
                    }
                    break;
                case Project::Hierarchy::Node::Type::PROPERTY:
                case Project::Hierarchy::Node::Type::CONST_PROPERTY:
                    if(auto pnode = std::dynamic_pointer_cast<Project::Hierarchy::IPropertyNode>(node);
                            pnode != nullptr)
                    {
                        v.set(valueAsString(*pnode).value_or("<Failed to get TypeInfo>"));
                    } else {
                        WRITE_WARN("Node ", std::to_string(*node, true),
                                   " is marked as a property node, but we failed"
                                   " to cast it to an IPropertyNode object.");
                    }
            }

            value.init(v.gobj());
            break;
        }
        case Columns::TOOLTIP:
        {
            using namespace std::string_literals;

            // set the value to the name
            Glib::Value<Glib::ustring> v;
            v.init(v.value_type());

            std::string tooltip = getTooltipForNode(node);
            v.set(tooltip);

            value.init(v.gobj());
            break;
        }
        default:
            WRITE_ERROR("Invalid column index ", column);
            RETURN_ERROR(STATUS_INVALID_VALUE);
    }

    return STATUS_SUCCESS;
}

/**
 * @brief Gets the tooltip for the given node
 * @details Tooltips will typically be in the format of
 *          "<tt>$NAME ($TYPE) [$MEMADDR] = $VALUE</tt>", however
 *          @c $NAME, @c $MEMADDR, and <tt>$VALUE</tt> may or may not show up,
 *          depending on the following factors:
 *          - @c $NAME: Will be skipped if @c include_name is @c false
 *          - @c $MEMADDR: Will be skipped if the program is not in debug mode
 *          - @c <tt>$VALUE</tt>: Will only be displayed for property, const
 *                                property, and link nodes (if the link node
 *                                points at a property/const property)
 *
 * @param node The node to get the tooltip for
 * @param include_name Whether the name of the node should be included in the
 *                     tooltip
 *
 * @return The tooltip
 */
std::string HMDT::GUI::MainWindowFileTreePart::HierarchyModel::getTooltipForNode(
        Project::Hierarchy::INodePtr node,
        bool include_name) const noexcept
{
    std::string tooltip = "";
    if(include_name) {
        tooltip += node->getName();
    }

    tooltip += " (" + std::to_string(node->getType()) + ")";

    if(prog_opts.debug) {
        tooltip += " [" + std::to_string((void*)node.get()) + "]";
    }

    auto type = node->getType();
    if(type == Project::Hierarchy::Node::Type::PROPERTY ||
       type == Project::Hierarchy::Node::Type::CONST_PROPERTY)
    {
        if(auto pnode = std::dynamic_pointer_cast<Project::Hierarchy::IPropertyNode>(node);
                pnode != nullptr)
        {
            tooltip += " = " + valueAsString(*pnode).value_or("<Failed to get TypeInfo>");
        } else {
            WRITE_WARN("Node ", std::to_string(*node, true), " is marked"
                       " as a property node, but we failed to cast it to"
                       " an IPropertyNode object.");
        }
    } else if(type == Project::Hierarchy::Node::Type::LINK) {
        if(auto lnode = std::dynamic_pointer_cast<Project::Hierarchy::ILinkNode>(node);
                lnode != nullptr)
        {
            // Format:
            //   NAME (TYPE) [ADDRESS] => (TYPE) [ADDRESS] = VALUE
            tooltip += " =>" + getTooltipForNode(lnode->getLinkedNode(), false);
        } else {
            WRITE_WARN("Node ", std::to_string(*node, true), " is marked"
                       " as a link node, but we failed to cast it to"
                       " an ILinkNode object.");
        }
    }

    return tooltip;
}

/**
 * @brief Gets the next iterator from 'iter'.
 * @details Stores an INode in the iterator's user_data (the current node being
 *          looked at)
 *
 * @param iter The previous iterator
 * @param iter_next The next iterator
 *
 * @return True on success, false otherwise
 */
bool HMDT::GUI::MainWindowFileTreePart::HierarchyModel::iter_next_vfunc(const iterator& iter,
                                                                        iterator& iter_next) const
{
    if(!isValid(iter)) {
        return false;
    }

    // Get the current node on the iterator
    auto* node = static_cast<Project::Hierarchy::INode*>(iter.gobj()->user_data);

    // WRITE_DEBUG("iter_next_vfunc(", std::to_string(*node), ")");

    if(node == nullptr) {
        WRITE_WARN("Cannot iterate to the next from an iterator pointing at a "
                   "null node!");
        return false;
    }

    // Get the parent of this node (one should always exist, though the parent
    //   will be null for the root node)
    if(m_parent_map.count(node) == 0) {
        WRITE_ERROR("Could not find parent of ", std::to_string(*node), " in parent map.");
        return false;
    }
    auto parent = m_parent_map.at(node);

    // The next node for the iterator
    std::shared_ptr<Project::Hierarchy::INode> next_node = nullptr;

    if(parent != nullptr) {
        // First, what is the current index for the node
        if(m_node_index_map.count(node) == 0) {
            WRITE_ERROR("Node ", std::to_string(*node), " was not found in the "
                        "node index map.");
            return false;
        }
        auto cur_idx = m_node_index_map.at(node);

        // Is there a next index?
        if(m_ordered_children_map.count(parent.get()) == 0) {
            WRITE_ERROR("Parent ", std::to_string(*parent), " was not found in "
                        "the ordered children map.");
            return false;
        }

        auto&& children = m_ordered_children_map.at(parent.get());
        if(cur_idx + 1 >= children.size()) {
            // Reached the end of iteration for this parent
            return false;
        } else {
            next_node = children[cur_idx + 1];
        }
    } else {
        // WRITE_DEBUG("Reached end of iteration (node is root).");
        return false;
    }

    // The iterator will have the same parent as this one
    // WRITE_DEBUG("Next node is ", std::to_string(*next_node));
    iter_next.gobj()->user_data = next_node.get();

    iter_next.set_stamp(m_stamp);

    return iter_next.gobj()->user_data != nullptr;
}

/**
 * @brief Gets the first child of the parent node
 *
 * @param parent The parent iterator
 * @param iter The child iterator
 *
 * @return True on success, false otherwise or if there are no children
 */
bool HMDT::GUI::MainWindowFileTreePart::HierarchyModel::iter_children_vfunc(const iterator& parent,
                                                                            iterator& iter) const
{
    // WRITE_DEBUG("iter_children_vfunc(", std::to_string(*static_cast<Project::Hierarchy::INode*>(parent.gobj()->user_data)), ')');
    return iter_nth_child_vfunc(parent, 0, iter);
}

/**
 * @brief Checks if the iterator has children
 *
 * @param iter The iterator to check
 *
 * @return True if the number of children is greater than 0, false otherwise.
 */
bool HMDT::GUI::MainWindowFileTreePart::HierarchyModel::iter_has_child_vfunc(const iterator& iter) const
{
    // WRITE_DEBUG("iter_has_child_vfunc(", std::to_string(*static_cast<Project::Hierarchy::INode*>(iter.gobj()->user_data)), ')');
    return iter_n_children_vfunc(iter) > 0;
}

/**
 * @brief Gets the number of children the iterator has.
 * @details This will only return non-zero for group nodes (or sub-types), and
 *          for link nodes that happen to point at a group node (or sub-type).
 *
 * @param iter The iterator the check
 *
 * @return The number of children for the iterator
 */
int HMDT::GUI::MainWindowFileTreePart::HierarchyModel::iter_n_children_vfunc(const iterator& iter) const
{
    // WRITE_DEBUG("iter_n_children_vfunc(", std::to_string(*static_cast<Project::Hierarchy::INode*>(iter.gobj()->user_data)), ")");

    if(!isValid(iter)) {
        return 0;
    }

    // Get the current node on the iterator
    auto* node = static_cast<Project::Hierarchy::INode*>(iter.gobj()->user_data);

    if(node == nullptr) {
        WRITE_WARN("Cannot get number of children from an iterator pointing at a null node!");
        return 0;
    }

    // TODO: Add code so this also works for link nodes

    // Do a separate dynamic cast here so we can actually do the casting (TODO: Does this even work?)
    auto* gnode = dynamic_cast<Project::Hierarchy::IGroupNode*>(node);

    // Make sure the cast was successful. If it wasn't, then we do not have a
    //   group node (and thus, no children)
    if(gnode == nullptr) {
        return 0;
    }

    // WRITE_DEBUG("node ", std::to_string(*gnode), " has ", gnode->getChildren().size(), " children.");
    return gnode->getChildren().size();
}

/**
 * @brief Gets the number of children the root node has
 *
 * @return The number of children of the root node
 */
int HMDT::GUI::MainWindowFileTreePart::HierarchyModel::iter_n_root_children_vfunc() const
{
    // WRITE_DEBUG("iter_n_root_children_vfunc()");
    if(auto gnode = std::dynamic_pointer_cast<const Project::Hierarchy::IGroupNode>(m_project_hierarchy);
            gnode != nullptr)
    {
        return gnode->getChildren().size();
    } else {
        // This should never happen as the root should always be some form of
        //   group node, but just in case lets return 0 here anyway
        return 0;
    }
}

/**
 * @brief Gets the nth child of the parent node
 *
 * @param parent The parent iterator
 * @param n The child index to get
 * @param iter The child iterator
 *
 * @return True on success, false otherwise or if there are no children
 */
bool HMDT::GUI::MainWindowFileTreePart::HierarchyModel::iter_nth_child_vfunc(
        const iterator& parent,
        int n,
        iterator& iter) const
{
    if(!isValid(parent)) {
        return false;
    }

    // Get the current node on the iterator
    auto* node = static_cast<Project::Hierarchy::INode*>(parent.gobj()->user_data);

    // WRITE_DEBUG("iter_nth_child_vfunc(",
    //         (node == nullptr ? std::string("<null>") : std::to_string(*node)),
    //         ", ", n, ")");

    // If the parent has a null node but it's still valid, then this is a
    //   "virtual" parent above root, so just return root
    if(node == nullptr && n == 0) {
        // WRITE_DEBUG("Virtual parent detected, assuming root.");
        iter.gobj()->user_data = static_cast<void*>(m_project_hierarchy.get());
        iter.set_stamp(m_stamp);
        return true;
    } else if(node == nullptr) {
        // Fail here if n != 0, since that means we're trying to get a child on
        //   a non-virtual parent
        WRITE_WARN("Cannot get child from an iterator pointing at a null node!");
        return false;
    }
    // WRITE_DEBUG("Get child of node ", std::to_string(*node));

    // Make sure the node is known to have children
    if(m_ordered_children_map.count(node) == 0) {
        return false;
    }

    // Make sure the requested child is valid
    if(auto num_children = m_ordered_children_map.at(node).size();
            n < 0 || n >= num_children)
    {
        WRITE_ERROR("Requested child ", n, " is out of range. Node ",
                    std::to_string(*node), " has ", num_children, " children.");
        return false;
    }

    iter.gobj()->user_data = static_cast<void*>(m_ordered_children_map.at(node)[n].get());

    iter.set_stamp(m_stamp);

    return true;
}

bool HMDT::GUI::MainWindowFileTreePart::HierarchyModel::iter_nth_root_child_vfunc(int n, iterator& iter) const
{
    if(n != 0) {
        WRITE_WARN("Trying to get child id ", n, ", but we are assuming a virtual root with only 1 value!");
        return false;
    }

    // WRITE_DEBUG("iter_nth_root_child_vfunc(", n, ')');
    // Set up a faux parent iterator to get the nth child on
    iterator parent;
    parent.gobj()->user_data = nullptr; // static_cast<void*>(m_project_hierarchy.get());
    parent.set_stamp(m_stamp);

    return iter_nth_child_vfunc(parent, n, iter);
}

/**
 * @brief Gets an iterator to the parent of a given child
 *
 * @param child The child iterator
 * @param iter The iterator to write the parent to
 *
 * @return True on success, false otherwise
 */
bool HMDT::GUI::MainWindowFileTreePart::HierarchyModel::iter_parent_vfunc(const iterator& child,
                                                                          iterator& iter) const
{
    // WRITE_DEBUG("iter_parent_vfunc(", std::to_string(*static_cast<Project::Hierarchy::INode*>(child.gobj()->user_data)), ")");

    if(!isValid(child)) {
        return false;
    }

    auto* child_node = static_cast<Project::Hierarchy::INode*>(child.gobj()->user_data);

    if(m_parent_map.count(child_node) == 0) {
        WRITE_ERROR("Could not find child node ", std::to_string(*child_node),
                    " in the parent map! This should never happen.");
        return false;
    }
    auto* parent = m_parent_map.at(child_node).get();

    // WRITE_DEBUG("Initializing iter with parent=", std::to_string(*parent),
    //             ", from node=", std::to_string(*child_node));
    iter.gobj()->user_data = parent;
    iter.set_stamp(m_stamp);

    return true;
}

/**
 * @brief Gets a path object to the given iterator
 *
 * @param iter The iterator
 *
 * @return A path object
 */
Gtk::TreeModel::Path
    HMDT::GUI::MainWindowFileTreePart::HierarchyModel::get_path_vfunc(const iterator& iter) const
{
    if(_spam_logs) WRITE_DEBUG("get_path_vfunc(...)");

    Gtk::TreeModel::Path path;

    if(!isValid(iter)) {
        return path;
    }

    auto* node = static_cast<Project::Hierarchy::INode*>(iter.gobj()->user_data);

    // Walk back up to the start of the tree to rebuild the path
    while(node != nullptr) {
        auto index = -1;

        if(m_parent_map.count(node) == 0) {
            WRITE_ERROR("Cannot find parent of ", std::to_string(*node),
                        " in the parent_map. This should never happen.");
            return Gtk::TreeModel::Path{};
        }

        auto parent = m_parent_map.at(node);
        if(parent == nullptr) {
            if(node != m_project_hierarchy.get()) {
                WRITE_ERROR("Somehow managed to reach a node without a parent "
                            "that is not root! node=", std::to_string(*node),
                            ", path=", path.to_string());
                return Gtk::TreeModel::Path{};
            } else {
                index = 0;
            }
        } else {
            // Lookup the index for the current node
            if(m_node_index_map.count(node) == 0) {
                WRITE_ERROR("Failed to find node ", std::to_string(*node),
                            " in node index map.");
                return Gtk::TreeModel::Path{};
            }

            index = m_node_index_map.at(node);

            if(_spam_logs) {
                WRITE_DEBUG("Index of ", std::to_string(*node), " (parent = ",
                            std::to_string(*parent), ") is ", index);
            }
        }

        // Add the index to the front
        path.push_front(index);
        node = parent.get();
    }

    if(_spam_logs) WRITE_DEBUG("Returning path=", path.to_string());
    return path;
}

/**
 * @brief Gets an iterator for the given path object
 *
 * @param path The path object
 * @param iter The object to write the iterator to
 *
 * @return True on success, false otherwise
 */
bool HMDT::GUI::MainWindowFileTreePart::HierarchyModel::get_iter_vfunc(const Path& path,
                                                                       iterator& iter) const
{
    if(_spam_logs) WRITE_DEBUG("get_iter_vfunc(", path.to_string(), ')');

    if(path.empty()) {
        WRITE_WARN("Got empty path!");
    }

    // we need to walk through the tree directly in order to make sure that we
    //   can start from the correct node, as 'iterator' is a more flat
    //   representation and thus is too easy to walk past where 'path' is
    //   pointing to
    auto node = m_project_hierarchy;

    bool is_first = true;
    for(auto&& part : path) {
        // Skip the first part of the path
        if(is_first) {
            is_first = false;
            continue;
        }

        if(m_ordered_children_map.count(node.get()) == 0) {
            WRITE_ERROR("Asked to find a child node in node ",
                        std::to_string(*node), ", however it is not in the "
                        "ordered children map.");
            return false;
        }

        auto&& children = m_ordered_children_map.at(node.get());

        // WRITE_DEBUG("Get child ", part, " of child ", std::to_string(*node));
        if(part < 0 || part >= children.size()) {
            WRITE_ERROR("Asked to get child ", part, " of node ",
                        std::to_string(*node), ", however we only know of that "
                        "node having ", children.size(),
                        " children.");
            return false;
        }

        node = children[part];
        if(_spam_logs) WRITE_DEBUG("Next node=", std::to_string(*node));
    }

    if(node == nullptr) {
        WRITE_ERROR("Got a null node after parsing the path ", path.to_string());
        return false;
    }

    // TODO: REMOVE THIS TO REDUCE SPAM!!!
    if(_spam_logs) 
        WRITE_DEBUG("Initializing iterator with node=",
                    (node == nullptr ?
                        std::string("<null>") :
                        std::to_string(*node)),
                    " for path ", path.to_string()
        );

    iter.gobj()->user_data = static_cast<void*>(node.get());
    iter.set_stamp(m_stamp);

    return iter.gobj()->user_data != nullptr;
}

/**
 * @brief Checks if a given iterator is valid
 *
 * @param iter The iterator to check
 *
 * @return True if the iterator's stamp matches this model's stamp
 */
bool HMDT::GUI::MainWindowFileTreePart::HierarchyModel::isValid(const iterator& iter) const noexcept
{
    return iter.get_stamp() == m_stamp;
}

/**
 * @brief Gets a property node's value as a string
 *
 * @param node The property node
 *
 * @return The property node's value as a string, or an error code on failure
 */
auto HMDT::GUI::MainWindowFileTreePart::HierarchyModel::valueAsString(const Project::Hierarchy::IPropertyNode& node) const noexcept
    -> Maybe<std::string>
{
    auto result = node.getTypeInfo();
    RETURN_IF_ERROR(result);

    // Define a bunch of helper macros here
#define IS_TYPE(T) (result.value() == typeid(T))
#define RETURN_T_TO_STR(T) do { \
    auto r = node.getValue<T>(); \
    RETURN_IF_ERROR(r); \
    return std::to_string(*r); \
} while(0)
#define RETURN_BOOL() do { \
    auto r = node.getValue<bool>(); \
    RETURN_IF_ERROR(r); \
    return *r ? "true" : "false"; \
} while(0)
#define RETURN_COLOR() do { \
    auto r = node.getValue<Color>(); \
    RETURN_IF_ERROR(r); \
    std::stringstream ss; \
    ss << "0x" << *r; \
    return ss.str(); \
} while (0)
#define RETURN_PROVTYPE() do { \
    auto r = node.getValue<ProvinceType>(); \
    RETURN_IF_ERROR(r); \
    std::stringstream ss; \
    ss << *r; \
    return ss.str(); \
} while (0)

    if(IS_TYPE(bool)) RETURN_BOOL();
    else if(IS_TYPE(Color)) RETURN_COLOR();
    else if(IS_TYPE(ProvinceType)) RETURN_PROVTYPE();
    else if(IS_TYPE(uint32_t)) RETURN_T_TO_STR(uint32_t);
    else if(IS_TYPE(int32_t)) RETURN_T_TO_STR(int32_t);
    else if(IS_TYPE(int64_t)) RETURN_T_TO_STR(int64_t);
    else if(IS_TYPE(uint64_t)) RETURN_T_TO_STR(uint64_t);
    else if(IS_TYPE(int16_t)) RETURN_T_TO_STR(int16_t);
    else if(IS_TYPE(uint16_t)) RETURN_T_TO_STR(uint16_t);
    else if(IS_TYPE(int8_t)) RETURN_T_TO_STR(int8_t);
    else if(IS_TYPE(uint8_t)) RETURN_T_TO_STR(uint8_t);
    else if(IS_TYPE(float)) RETURN_T_TO_STR(float);
    else if(IS_TYPE(double)) RETURN_T_TO_STR(double);
    else if(IS_TYPE(UUID)) RETURN_T_TO_STR(UUID);
    else if(IS_TYPE(Version)) {
        auto r = node.getValue<Version>();
        RETURN_IF_ERROR(r);
        return std::string("\"") + r->get().str() + "\"";
    }
    else if(IS_TYPE(std::string)) {
        auto r = node.getValue<std::string>();
        RETURN_IF_ERROR(r);
        return std::string("\"") + r->get() + "\"";
    }
    else return "<data>";

#undef IS_TYPE
#undef RETURN_T_TO_STR
#undef RETURN_BOOL
#undef RETURN_COLOR
#undef RETURN_PROVTYPE
}

/**
 * @brief Gets the icon for the given type
 *
 * @param type The type to get the icon for
 *
 * @return The icon associated with the given type
 */
auto HMDT::GUI::MainWindowFileTreePart::getTypeIcon(const Project::Hierarchy::Node::Type& type) noexcept
    -> Driver::Pixbuf
{
    Maybe<Driver::Pixbuf> result;

    switch(type) {
        case Project::Hierarchy::Node::Type::LINK:
            result = Driver::getInstance().getResourcePixbuf(
                                    HMDT_GLIB_IONICONS_RESOURCES,
                                    "link.svg",
                                    16 /* width */, 16 /* height */,
                                    true /* preserve_aspect_ratio */);
            if(IS_FAILURE(result)) {
                WRITE_ERROR("Failed to load hierarchy icon for ",
                            std::to_string(type));
                return Driver::getInstance().getFailurePixbuf();
            } else {
                return *result;
            }
        case Project::Hierarchy::Node::Type::GROUP:
            result = Driver::getInstance().getResourcePixbuf(
                                    HMDT_GLIB_IONICONS_RESOURCES,
                                    "folder.svg",
                                    16 /* width */, 16 /* height */,
                                    true /* preserve_aspect_ratio */);
            if(IS_FAILURE(result)) {
                WRITE_ERROR("Failed to load hierarchy icon for ",
                            std::to_string(type));
                return Driver::getInstance().getFailurePixbuf();
            } else {
                return *result;
            }
        case Project::Hierarchy::Node::Type::PROJECT:
            result = Driver::getInstance().getResourcePixbuf(
                                    HMDT_GLIB_IONICONS_RESOURCES,
                                    "briefcase.svg",
                                    16 /* width */, 16 /* height */,
                                    true /* preserve_aspect_ratio */);
            if(IS_FAILURE(result)) {
                WRITE_ERROR("Failed to load hierarchy icon for ",
                            std::to_string(type));
                return Driver::getInstance().getFailurePixbuf();
            } else {
                return *result;
            }
        case Project::Hierarchy::Node::Type::PROPERTY:
        case Project::Hierarchy::Node::Type::CONST_PROPERTY:
            // TODO: CONST_PROPERTY should honestly use a different icon (document+lock maybe?)
            result = Driver::getInstance().getResourcePixbuf(
                                    HMDT_GLIB_IONICONS_RESOURCES,
                                    "document.svg",
                                    16 /* width */, 16 /* height */,
                                    true /* preserve_aspect_ratio */);
            if(IS_FAILURE(result)) {
                WRITE_ERROR("Failed to load hierarchy icon for ",
                            std::to_string(type));
                return Driver::getInstance().getFailurePixbuf();
            } else {
                return *result;
            }
        case Project::Hierarchy::Node::Type::STATE:
        case Project::Hierarchy::Node::Type::PROVINCE:
            // TODO: We should probably have specific icons for States, Province, etc...
            //   but for now just use cube to show it's an "Object"
            result = Driver::getInstance().getResourcePixbuf(
                                    HMDT_GLIB_IONICONS_RESOURCES,
                                    "cube.svg",
                                    16 /* width */, 16 /* height */,
                                    true /* preserve_aspect_ratio */);
            if(IS_FAILURE(result)) {
                WRITE_ERROR("Failed to load hierarchy icon for ",
                            std::to_string(type));
                return Driver::getInstance().getFailurePixbuf();
            } else {
                return *result;
            }
        default:
            WRITE_WARN("Unrecognized type ", std::to_string(type));
            result = Driver::getInstance().getResourcePixbuf(
                                    HMDT_GLIB_IONICONS_RESOURCES,
                                    "help.svg",
                                    16 /* width */, 16 /* height */,
                                    true /* preserve_aspect_ratio */);
            if(IS_FAILURE(result)) {
                WRITE_ERROR("Failed to load hierarchy icon for ",
                            std::to_string(type));
                return Driver::getInstance().getFailurePixbuf();
            } else {
                return *result;
            }
    }
}

//////////////////////////////

/**
 * @brief Builds the file tree UI.
 *
 * @param pane The pane to add the file tree into
 *
 * @return The frame the UI is built into
 */
Gtk::Frame* HMDT::GUI::MainWindowFileTreePart::buildFileTree(Gtk::Paned* pane) {
    WRITE_INFO("Building FileTree parts.");

    Gtk::Frame* file_tree_frame = new Gtk::Frame();
    setActiveChild(file_tree_frame);

    m_swindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    setActiveChild(&m_swindow);

    pane->pack1(*file_tree_frame, false, false);

    m_tree_view = addWidget<Gtk::TreeView>();

    Gtk::TreeViewColumn* column = Gtk::manage(new Gtk::TreeViewColumn(""));
    Gtk::CellRendererText* text_renderer = Gtk::manage(new Gtk::CellRendererText);

    // Names can be very long, so ellipsize them and we'll add it as part of the
    //   tooltip
    text_renderer->property_ellipsize() = Pango::ELLIPSIZE_END;
    text_renderer->property_max_width_chars() = MAX_TREE_NAME_WIDTH;

    // TODO: This should be an icon, not text
    Gtk::CellRendererPixbuf* type_renderer = Gtk::manage(new Gtk::CellRendererPixbuf);

    Gtk::CellRendererText* value_renderer = Gtk::manage(new Gtk::CellRendererText);

    value_renderer->property_ellipsize() = Pango::ELLIPSIZE_END;
    value_renderer->property_max_width_chars() = MAX_TREE_NAME_WIDTH;

    column->pack_start(*type_renderer, false); // Show the type first
    column->pack_start(*text_renderer, false);
    column->pack_start(*value_renderer, false);
    column->add_attribute(*text_renderer, "text", static_cast<int>(HierarchyModel::Columns::NAME));
    column->add_attribute(*type_renderer, "pixbuf", static_cast<int>(HierarchyModel::Columns::TYPE));
    column->add_attribute(*value_renderer, "text", static_cast<int>(HierarchyModel::Columns::VALUE));
    column->set_expand(false);
    column->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);

    m_tree_view->append_column(*column);

    m_tree_view->set_tooltip_column(static_cast<int>(HierarchyModel::Columns::TOOLTIP));
    m_tree_view->get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);

    // TODO: We should really change this to instead just listen for a mouse click
    //   rather than expecting Gtk to tell us if the tree has changed
    auto update_selections_callback = [this](bool clear_previous) {
        auto selected_rows = m_tree_view->get_selection()->get_selected_rows();

        std::vector<ProvinceID> selected_provs;
        std::vector<StateID> selected_states;

        WRITE_DEBUG("Tree view signals: Selection changed");
        for(auto&& path : selected_rows) {
            if(m_tree_view->get_selection()->is_selected(path)) {
                Gtk::TreeModel::iterator iter;
                auto result = m_model->get_iter_vfunc(path, iter);
                if(!result) {
                    WRITE_WARN("Failed to get iterator for path ",
                               path.to_string());
                    continue;
                }

                auto* node = static_cast<Project::Hierarchy::INode*>(iter.gobj()->user_data);

                // If it was a Province, then select it
                if(auto pnode = dynamic_cast<Project::Hierarchy::ProvinceNode*>(node);
                        pnode != nullptr)
                {
                    auto maybe_id = pnode->getIDProperty();
                    if(IS_FAILURE(maybe_id)) {
                        WRITE_ERROR("Failed to get province ID property: ",
                                    maybe_id.error());
                        continue;
                    }

                    auto maybe_id2 = (*maybe_id)->getValue<ProvinceID>();
                    if(IS_FAILURE(maybe_id2)) {
                        WRITE_ERROR("Failed to get ID from property: ",
                                    maybe_id2.error());
                        continue;
                    }
                    ProvinceID id = *maybe_id2;

                    // TODO: Select province with SelectionManager
                    //   Is there a way we can do this without a lot of large
                    //   boiler plate code?
                    selected_provs.push_back(id);
                } else if(auto snode = dynamic_cast<Project::Hierarchy::StateNode*>(node);
                          snode != nullptr)
                {
                    auto maybe_id = snode->getIDProperty();
                    if(IS_FAILURE(maybe_id)) {
                        WRITE_ERROR("Failed to get province ID property: ",
                                    maybe_id.error());
                        continue;
                    }

                    auto maybe_id2 = (*maybe_id)->getValue<StateID>();
                    if(IS_FAILURE(maybe_id2)) {
                        WRITE_ERROR("Failed to get ID from property: ",
                                    maybe_id2.error());
                        continue;
                    }
                    StateID id = *maybe_id2;

                    selected_states.push_back(id);
                }
            }
        }

        // WRITE_DEBUG("There are ", selected_provs.size(), " selected provinces.");

        if(!selected_provs.empty()) {
            // Only clear the province selection if a new selection is made in
            //   the view
            if(clear_previous) {
                SelectionManager::getInstance().clearProvinceSelection();
            }

            for(auto&& prov_id : selected_provs) {
                SelectionManager::getInstance().addProvinceSelection(prov_id);
            }
        }

        if(!selected_states.empty()) {
            // Only clear the state selection if a new selection is made in
            //   the view
            if(clear_previous) {
                SelectionManager::getInstance().clearStateSelection();
            }


            for(auto&& state_id : selected_states) {
                SelectionManager::getInstance().addStateSelection(state_id);
            }
        }

        return true;
    };

    m_tree_view->get_selection()->signal_changed().connect([update_selections_callback]() {
        update_selections_callback(true /* clear_previous */);
    });

    // m_tree_view->append_column("Name", Gtk::TreeModelColumn<Glib::ustring>());

    file_tree_frame->add(m_swindow);
    file_tree_frame->show_all();

    return file_tree_frame;
}

/**
 * @brief Gets the project hierarchy
 *
 * @return The project hierarchy
 */
auto HMDT::GUI::MainWindowFileTreePart::getHierarchy() noexcept
    -> std::shared_ptr<Project::Hierarchy::INode>
{
    return m_model->getHierarchy();
}

/**
 * @brief Callback invoked when a project is opened
 *
 * @return STATUS_SUCCESS on success, or a status code on failure
 */
auto HMDT::GUI::MainWindowFileTreePart::onProjectOpened() -> MaybeVoid {
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        auto maybe_hierarchy = opt_project->get().visit([](auto&&...)
            -> MaybeVoid
        {
            return STATUS_SUCCESS;
        });
        RETURN_IF_ERROR(maybe_hierarchy);

        auto root_node = *maybe_hierarchy;

        HierarchyModel::ParentMap parent_map;
        HierarchyModel::OrderedChildrenMap ordered_children_map;
        HierarchyModel::NodeIndexMap node_index_map;

        // The root has no parent
        parent_map[root_node.get()] = nullptr;
        node_index_map[root_node.get()] = 0;

        // Resolve links and build parent map
        auto result = root_node->visit([&root_node,
                                        &parent_map,
                                        &ordered_children_map,
                                        &node_index_map](auto node)
            -> MaybeVoid
        {
            if(node->getType() == Project::Hierarchy::Node::Type::LINK) {
                WRITE_DEBUG("Resolve link node ", node->getName());
                auto link_node = std::dynamic_pointer_cast<Project::Hierarchy::LinkNode>(node);
                auto result = link_node->resolve(root_node);
                RETURN_IF_ERROR(result);

                if(!link_node->isLinkValid()) {
                    WRITE_ERROR("Link resolution succeeded, but the link node is still invalid.");
                    RETURN_ERROR(STATUS_UNEXPECTED);
                }
            } else if(auto gnode = std::dynamic_pointer_cast<Project::Hierarchy::IGroupNode>(node);
                      gnode != nullptr)
            {
                auto&& children = gnode->getChildren();

                WRITE_DEBUG("Found group node ",
                            std::to_string((Project::Hierarchy::INode&)*gnode),
                            ", adding all ", children.size(),
                            " children to the maps.");
                for(auto&& [_, child] : children) {
                    parent_map[child.get()] = gnode;
                    ordered_children_map[node.get()].push_back(child);
                    node_index_map[child.get()] = ordered_children_map[node.get()].size() - 1;
                }
            }

            return STATUS_SUCCESS;
        });
        RETURN_IF_ERROR(result);

        // We have to make a new object every time a project is opened in order
        //   to force TreeView to be refreshed
        m_model = Glib::RefPtr<HierarchyModel>(new HierarchyModel(root_node,
                                                                  parent_map,
                                                                  ordered_children_map,
                                                                  node_index_map));

        // Make sure that we refresh the model with the new data
        m_tree_view->set_model(m_model);
    }

    return STATUS_SUCCESS;
}

/**
 * @brief Updates a specific part of the file tree
 *
 * @param key The key specifying which node in the file tree to update
 */
void HMDT::GUI::MainWindowFileTreePart::updateFileTree(const Project::Hierarchy::Key& key) noexcept
{
    WRITE_DEBUG("Asked to update file tree for key=", std::to_string(key));

    auto maybe_node = key.lookup(m_model->getHierarchy());
    if(IS_FAILURE(maybe_node)) {
        WRITE_ERROR("Failed to lookup node for key ", std::to_string(key));
        return;
    }
    auto expect_node = *maybe_node;

    // Go over the entire model and search for the node that was changed
    m_model->foreach_iter([this, expect_node](const Gtk::TreeModel::iterator& iter)
        -> bool
    {
        if(!m_model->isValid(iter)) {
            WRITE_WARN("Invalid iterator given, cannot update.");
            return false;
        }

        // Get the current node on the iterator
        auto* node = static_cast<Project::Hierarchy::INode*>(iter.gobj()->user_data);

        if(node == expect_node.get()) {
            WRITE_DEBUG("Update node ", std::to_string(*node));

            m_model->row_changed(m_model->get_path(iter), iter);

            return true;
        }

        return false;
    });
}

/**
 * @brief Callback to invoke when a selection is made outside of the file tree
 *
 * @param keys Keys for every thing that was selected
 * @param action The specific selection action that was taken
 */
void HMDT::GUI::MainWindowFileTreePart::selectNode(const std::vector<Project::Hierarchy::Key>& keys,
                                                   const SelectionManager::Action& action) noexcept
{
    if(m_in_select_node) {
        // Early return to make sure we don't infintely recurse
        return;
    }
    m_in_select_node = true;
    RUN_AT_SCOPE_END([this]() { m_in_select_node = false; });

    // WRITE_DEBUG("selectNode(", std::to_string(key), ", ", (int)action, ')');

    // If we are going to set the selection, then make sure we unselect
    //   everything first anyway, but don't return from here
    if(action == SelectionManager::Action::CLEAR ||
       action == SelectionManager::Action::SET)
    {
        WRITE_DEBUG("Clearing tree view selection.");
        m_tree_view->get_selection()->unselect_all();

        // Only return from here if we are supposed to clear all selections
        if(action == SelectionManager::Action::CLEAR) return;
    }

    for(auto&& key : keys) {
        auto maybe_node = key.lookup(m_model->getHierarchy());
        if(IS_FAILURE(maybe_node)) {
            WRITE_ERROR("Failed to lookup node for key ", std::to_string(key));
            return;
        }
        auto expect_node = *maybe_node;

        // Go over the entire model and search for the node that was changed
        m_model->foreach_iter([this, expect_node, &action](const Gtk::TreeModel::iterator& iter)
            -> bool
        {
            if(!m_model->isValid(iter)) {
                WRITE_WARN("Invalid iterator given, cannot update.");
                return false;
            }

            // Get the current node on the iterator
            auto* node = static_cast<Project::Hierarchy::INode*>(iter.gobj()->user_data);

            if(node == expect_node.get()) {
                auto path = m_model->get_path(iter);

                // If we are to select the given path, then add it, otherwise if
                //   we are supposed to remove it then "unselect" it and do not
                //   scroll
                if(action == SelectionManager::Action::ADD ||
                   action == SelectionManager::Action::SET)
                {
                    // Collapse the row first so that we show the children as
                    //   well and don't end up with the main row at the bottom
                    //   of the window
                    m_tree_view->collapse_row(path);
                    m_tree_view->expand_to_path(path);

                    m_tree_view->grab_focus();
                    m_tree_view->get_selection()->select(path);
                    m_tree_view->scroll_to_row(path);
                } else {
                    m_tree_view->get_selection()->unselect(path);
                }

                return true;
            }

            return false;
        });
    }
}

