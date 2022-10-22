#ifndef PROJECT_HIERARCHY_GROUPNODE_H
# define PROJECT_HIERARCHY_GROUPNODE_H

# include "INode.h"

namespace HMDT::Project::Hierarchy {
    /**
     * @brief Represents a dynamic group of generic Nodes
     */
    class DynamicGroupNode: public IGroupNode {
        public:
            //! A function which will return all children for this group
            using ChildVisitor = std::function<Children(const DynamicGroupNode&)>;

            DynamicGroupNode(const std::string&, ChildVisitor);
            virtual ~DynamicGroupNode() = default;

            virtual const Children& getChildren() const noexcept override;

            virtual const std::string& getName() const noexcept override;
            virtual Type getType() const noexcept override;

        protected:
            virtual Children getChildren() noexcept override;

        private:
            //! The name of this group
            std::string m_name;

            //! The function which returns all children in this group
            ChildVisitor m_child_visitor;
    };

    /**
     * @brief Represents a group of nodes where the children in the group do not
     *        dynamically update
     */
    class GroupNode: public IGroupNode {
        public:
            GroupNode(const std::string&);
            virtual ~GroupNode() = default;

            virtual const Children& getChildren() const noexcept override;

            virtual const std::string& getName() const noexcept override;
            virtual Type getType() const noexcept override;

            MaybeVoid addChild(const std::string&, ChildNode) noexcept;
            MaybeVoid addChild(ChildNode) noexcept;

        protected:
            virtual Children getChildren() noexcept override;

        private:
            //! The name of this group
            std::string m_name;

            //! The children in this group
            Children m_children;
    };
}

#endif

