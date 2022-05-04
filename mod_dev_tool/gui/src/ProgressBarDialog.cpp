
#include "ProgressBarDialog.h"

HMDT::GUI::ProgressBarDialog::ProgressBarDialog(Gtk::Window& parent,
                                                         const std::string& title,
                                                         const std::string& message,
                                                         bool modal):
    Gtk::Dialog(title, parent, modal),
    m_label(message),
    m_progress_bar()
{
    // Add the label and progress bar widgets
    reinterpret_cast<Gtk::Bin*>(get_child())->add(m_label);
    reinterpret_cast<Gtk::Bin*>(get_child())->add(m_progress_bar);
}

void HMDT::GUI::ProgressBarDialog::setShowText(bool show_text) {
    m_progress_bar.set_show_text(show_text);
}

void HMDT::GUI::ProgressBarDialog::setText(const std::string& text) {
    m_progress_bar.set_text(text);
}

void HMDT::GUI::ProgressBarDialog::setFraction(double fraction) {
    m_progress_bar.set_fraction(fraction);
}

