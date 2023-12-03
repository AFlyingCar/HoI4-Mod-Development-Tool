#ifndef PROJECT_HIERARCHY_PROJECTNODE_H
# define PROJECT_HIERARCHY_PROJECTNODE_H

# include "GroupNode.h"

namespace HMDT::Project::Hierarchy {
    /**
     * @brief Specialized GroupNode that represents a Project
     */
    class ProjectNode: public GroupNode {
        public:
            using GroupNode::GroupNode;

            /**
             * @brief Gets the type of ProjectNode
             *
             * @return Node::Type::PROJECT
             */
            virtual Type getType() const noexcept override {
                return Type::PROJECT;
            }
    };
}

#endif

