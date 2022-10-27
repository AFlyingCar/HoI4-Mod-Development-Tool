#ifndef HISTORYPROJECT_H
# define HISTORYPROJECT_H

# include <filesystem>

# include "IProject.h"
# include "StateProject.h"

namespace HMDT::Project {
    class HistoryProject: public IRootHistoryProject {
        public:
            HistoryProject(IProject&);
            virtual ~HistoryProject();

            virtual MaybeVoid save(const std::filesystem::path&) override;
            virtual MaybeVoid load(const std::filesystem::path&) override;
            virtual MaybeVoid export_(const std::filesystem::path&) const noexcept override;

            virtual IRootProject& getRootParent() override;
            virtual const IRootProject& getRootParent() const override;

            virtual bool validateData() override;

            virtual IRootHistoryProject& getRootHistoryParent() noexcept override;
            virtual const IRootHistoryProject& getRootHistoryParent() const noexcept override;

            virtual StateProject& getStateProject() noexcept override;
            virtual const StateProject& getStateProject() const noexcept override;

            virtual Maybe<std::shared_ptr<Hierarchy::INode>> visit(const std::function<MaybeVoid(std::shared_ptr<Hierarchy::INode>)>&) const noexcept override;

        private:
            //! The State project
            StateProject m_state_project;

            //! The parent project that this HistoryProject belongs to
            IProject& m_parent_project;
    };
}

#endif

