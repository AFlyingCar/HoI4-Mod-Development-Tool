
#include "ConstrainedEntry.h"

#include "Logger.h"

/**
 * @brief Sets the allowed set of characters. If the string is empty, then all
 *        characters are allowed.
 *
 * @param allowed_chars The list of allowed characters.
 */
void MapNormalizer::GUI::ConstrainedEntry::setAllowedChars(const std::string& allowed_chars)
{
    m_allowed_chars = allowed_chars;
}

void MapNormalizer::GUI::ConstrainedEntry::on_insert_text(const Glib::ustring& text,
                                                          int* position)
{
    // If m_allowed_chars is empty, then we allow all text
    if(m_allowed_chars.empty() || text.find_first_not_of(m_allowed_chars) == Glib::ustring::npos)
    {
        Gtk::Entry::on_insert_text(text, position);
    }
}

