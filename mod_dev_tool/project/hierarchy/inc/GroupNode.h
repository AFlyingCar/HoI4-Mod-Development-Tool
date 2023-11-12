#ifndef PROJECT_HIERARCHY_GROUPNODE_H
# define PROJECT_HIERARCHY_GROUPNODE_H

# include "INode.h"

namespace HMDT::Project::Hierarchy {
    /**
     * @brief Represents a group of nodes where the children in the group do not
     *        dynamically update
     */
    class GroupNode: public IGroupNode {
        public:
            GroupNode(const std::string&);
            virtual ~GroupNode() = default;

            virtual const Children& getChildren() const noexcept override;
            virtual Children getChildren() noexcept override;

            virtual const std::string& getName() const noexcept override;
            virtual Type getType() const noexcept override;

            MaybeVoid addChild(const std::string&, ChildNode) noexcept;
            MaybeVoid addChild(ChildNode) noexcept;

            MaybeVoid setChild(const std::string&, ChildNode) noexcept;
            MaybeVoid removeChild(const std::string&) noexcept;

        private:
            //! The name of this group
            std::string m_name;

            //! The children in this group
            Children m_children;
    };
}

#endif

