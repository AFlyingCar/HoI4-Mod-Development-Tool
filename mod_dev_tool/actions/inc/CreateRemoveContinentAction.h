#ifndef CREATEREMOVECONTINENTACTION_H
# define CREATEREMOVECONTINENTACTION_H

# include <string>

# include "MapProject.h"

# include "IAction.h"

namespace HMDT::Action {
    class CreateRemoveContinentAction: public Action::IAction {
        public:
            enum class Type {
                CREATE,
                REMOVE
            };

            CreateRemoveContinentAction(Project::MapProject&,
                                        const std::string&,
                                        Type);

            virtual bool doAction(const Callback& = _) override;
            virtual bool undoAction(const Callback& = _) override;

        protected:
            bool create();
            bool remove();

        private:
            // TODO: This is potentially dangerous to hold onto, we should
            //   instead try to find an elegant solution for getting the current
            //   map project, as it is potentially possible for the map project
            //   to go out of scope before this action is destroyed
            Project::MapProject& m_map_project;
            std::string m_continent_name;
            Type m_type;
    };
}

#endif

