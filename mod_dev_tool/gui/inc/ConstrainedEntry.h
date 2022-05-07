#ifndef CONSTRAINED_ENTRY_H
# define CONSTRAINED_ENTRY_H

# include "gtkmm/entry.h"

namespace HMDT::GUI {
    /**
     * @brief Special Entry which can constrain input to a limited set of
     *        characters.
     */
    class ConstrainedEntry: public Gtk::Entry {
        public:
            using Gtk::Entry::Entry;

            void setAllowedChars(const std::string&);

            virtual void on_insert_text(const Glib::ustring&, int*) override;

        private:
            //! The only characters which are allowed to be inputted
            std::string m_allowed_chars;
    };
}

#endif

