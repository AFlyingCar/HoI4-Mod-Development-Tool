#ifndef MAINWINDOWFILETREEPART_H
# define MAINWINDOWFILETREEPART_H

# include <memory>

# include "gtkmm/paned.h"
# include "gtkmm/frame.h"
# include "gtkmm/treemodel.h"
# include "gtkmm/treeview.h"

# include "BaseMainWindow.h"
# include "SelectionManager.h"
# include "Driver.h"

# include "INode.h"

namespace HMDT::GUI {
    /**
     * @brief Defines the UI for the FileTree part of the UI
     */
    class MainWindowFileTreePart: public virtual BaseMainWindow {
        public:
            //! Helper alias for a callback to be used when a node is clicked on
            using NodeClickCallback = std::function<void(Project::Hierarchy::INodePtr /* node */,
                                                         GdkEventType /* type */,
                                                         uint32_t /* button */)>;

            //! The max width of a name for the tree in characters
            static constexpr std::int32_t MAX_TREE_NAME_WIDTH = 32;

            MainWindowFileTreePart() = default;

            /**
             * @brief Data structure that will be sent to the SelectionManager
             *        when selecting a node in the file tree.
             */
            struct OnSelectNodeData {
                bool skip_select_in_tree = false;
                std::optional<Project::Hierarchy::Key> select_in_tree_override = std::nullopt;
            };

            void setOnNodeClickCallback(const NodeClickCallback&) noexcept;
            void setOnNodeDoubleClickCallback(const NodeClickCallback&) noexcept;

        protected:
            /**
             * @brief Defines the model for representing the project hierarchy
             *        tree.
             */
            class HierarchyModel: public Gtk::TreeModel, Glib::Object
            {
                public:
                    using ParentMap = std::unordered_map<Project::Hierarchy::INode*,
                                                         Project::Hierarchy::INodePtr>;
                    using OrderedChildrenMap = std::unordered_map<Project::Hierarchy::INode*,
                                                                  std::vector<Project::Hierarchy::INodePtr>>;

                    /**
                     * @brief Helper type for mapping all nodes with their index
                     * @details Note that these indices are RELATIVE to their
                     *          position as children. This means that two or
                     *          more nodes can share the same index.
                     */
                    using NodeIndexMap = std::unordered_map<Project::Hierarchy::INode*,
                                                            std::size_t>;

                    /**
                     * @brief The columns that we expect to be able to display
                     */
                    enum class Columns: int {
                        NAME = 0,
                        TYPE,
                        VALUE,
                        TOOLTIP,

                        // Invalid column
                        MAX
                    };

                    HierarchyModel(Project::Hierarchy::INodePtr,
                                   const ParentMap&,
                                   const OrderedChildrenMap&,
                                   const NodeIndexMap&);

                    virtual ~HierarchyModel() = default;

                    std::shared_ptr<Project::Hierarchy::INode> getHierarchy() noexcept;

                    Gtk::TreeModelFlags get_flags_vfunc() const override;
                    int get_n_columns_vfunc() const override;
                    GType get_column_type_vfunc(int index) const override;
                    void get_value_vfunc(const iterator& iter, int column, Glib::ValueBase& value) const override;
                    bool iter_next_vfunc(const iterator& iter, iterator& iter_next) const override;
                    bool iter_children_vfunc(const iterator& parent, iterator& iter) const override;
                    bool iter_has_child_vfunc(const iterator& iter) const override;
                    int iter_n_children_vfunc(const iterator& iter) const override;
                    int iter_n_root_children_vfunc() const override;
                    bool iter_nth_child_vfunc(const iterator& parent, int n, iterator& iter) const override;
                    bool iter_nth_root_child_vfunc(int n, iterator& iter) const override;
                    bool iter_parent_vfunc(const iterator& child, iterator& iter) const override;
                    Path get_path_vfunc(const iterator& iter) const override;
                    bool get_iter_vfunc(const Path& path, iterator& iter) const override;

                    bool isValid(const iterator&) const noexcept;

                    Maybe<Project::Hierarchy::Key> getKeyForNode(Project::Hierarchy::INodePtr) const noexcept;

                protected:
                    Maybe<std::string> valueAsString(const Project::Hierarchy::IPropertyNode&) const noexcept;

                    MaybeVoid getValueFromNode(Project::Hierarchy::INodePtr,
                                               int,
                                               Glib::ValueBase&) const noexcept;

                    std::string getTooltipForNode(Project::Hierarchy::INodePtr,
                                                  bool = true) const noexcept;

                    std::int32_t getNumChildrenForNode(Project::Hierarchy::INodePtr) const noexcept;
                    Maybe<Project::Hierarchy::INodePtr>
                        getNthChildForNode(Project::Hierarchy::INodePtr,
                                           std::int32_t,
                                           bool = true) const noexcept;

                private:
                    //! The root of the current project hierarchy
                    std::shared_ptr<Project::Hierarchy::INode> m_project_hierarchy;

                    //! A map of nodes -> parent because GTK apparently expects the tree to be bi-directional
                    ParentMap m_parent_map;

                    //! Ordered map of node children, since GtkTreeView requires ordered lookup
                    OrderedChildrenMap m_ordered_children_map;

                    //! Helper map to get a node's index
                    NodeIndexMap m_node_index_map;

                    //! A stamp used to determine if a given iterator is valid
                    std::int32_t m_stamp;

                    //! The next stamp to use for this model
                    static std::int32_t next_stamp;
            };

            Gtk::Frame* buildFileTree(Gtk::Paned*);

            std::shared_ptr<Project::Hierarchy::INode> getHierarchy() noexcept;

            MaybeVoid onProjectOpened();

            MaybeVoid handleNodeValueSelection(Project::Hierarchy::INode*,
                                               std::vector<std::pair<ProvinceID, OnSelectNodeData>>&,
                                               std::vector<std::pair<StateID, OnSelectNodeData>>&,
                                               OnSelectNodeData&);

            void updateFileTree(const Project::Hierarchy::Key&) noexcept;

            void selectNode(const std::vector<Project::Hierarchy::Key>&,
                            const SelectionManager::Action&) noexcept;

            Maybe<Project::Hierarchy::Key> getKeyForNode(Project::Hierarchy::INodePtr) const noexcept;

            static Driver::Pixbuf getTypeIcon(const Project::Hierarchy::Node::Type&) noexcept;

        private:
            //! The model used by the Tree for this 
            Glib::RefPtr<HierarchyModel> m_model;

            //! We want the view to be scrollable
            Gtk::ScrolledWindow m_swindow;

            //! The tree view
            Gtk::TreeView* m_tree_view;

            //! Flag to prevent infinitely recursing
            bool m_in_select_node = false;

            //! Callback invoked when a node is clicked
            NodeClickCallback m_on_click = [](auto&&...) { };

            //! Callback invoked when a node is double clicked
            NodeClickCallback m_on_double_click = [](auto&&...) { };
    };
}

#endif

