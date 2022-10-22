#ifndef PROJECT_HIERARCHY_LinkNODE_H
# define PROJECT_HIERARCHY_LinkNODE_H

# include "INode.h"

namespace HMDT::Project::Hierarchy {
    class LinkNode: public ILinkNode {
        public:
            using ResolutionCheck = std::function<bool(LinkedNode)>;

            LinkNode(const std::string&, ResolutionCheck);

            virtual Type getType() const noexcept override;
            virtual const std::string& getName() const noexcept override;

            virtual ConstLinkedNode getLinkedNode() const noexcept override;
            virtual LinkedNode getLinkedNode() noexcept override;

            virtual bool isLinkValid() const noexcept override;

            virtual bool resolveLink(LinkedNode) noexcept override;

        private:
            //! The name of this link
            std::string m_name;

            //! A function which checks if a given node will resolve the link
            ResolutionCheck m_resolution_check;

            //! A cached pointer to the node this link points to
            LinkedNode m_cached_link;
    };
}

#endif

