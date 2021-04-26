#ifndef PROGRESSBAR_DIALOG_H
# define PROGRESSBAR_DIALOG_H

# include "gtkmm/dialog.h"
# include "gtkmm/progressbar.h"
# include "gtkmm/label.h"

namespace MapNormalizer::GUI {
    class ProgressBarDialog: public Gtk::Dialog {
        public:
            ProgressBarDialog(Gtk::Window&, const std::string&, const std::string& = "", bool = false);

            void setShowText(bool = true);

            void setText(const std::string&);
            void setFraction(double);

        private:
            Gtk::Label m_label;
            Gtk::ProgressBar m_progress_bar;
    };
}

#endif

