#ifndef PROJECT_HIERARCHY_PROJECTNODE_H
# define PROJECT_HIERARCHY_PROJECTNODE_H

# include "GroupNode.h"

namespace HMDT::Project::Hierarchy {
#if 0
    class ProjectNode: public IGroupNode {
        public:
            ProjectNode(const std::string&);
            virtual ~ProjectNode() = default;

            virtual const Children& getChildren() const noexcept override;

            virtual const std::string& getName() const noexcept override;
            virtual Type getType() const noexcept override;

            MaybeVoid addChild(ChildNode) noexcept;

        protected:
            virtual Children getChildren() noexcept override;

        private:
            std::string m_name;
            Children m_children;
    };
#else
    class ProjectNode: public GroupNode {
        public:
            using GroupNode::GroupNode;

            virtual Type getType() const noexcept override {
                return Type::PROJECT;
            }
    };
#endif
}

#endif

