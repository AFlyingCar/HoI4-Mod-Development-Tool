#ifndef PROGRESSBAR_DIALOG_H
# define PROGRESSBAR_DIALOG_H

# include "gtkmm/dialog.h"
# include "gtkmm/progressbar.h"
# include "gtkmm/label.h"

namespace MapNormalizer::GUI {
    /**
     * @brief A simple dialog box which shows a progress bar
     */
    class ProgressBarDialog: public Gtk::Dialog {
        public:
            ProgressBarDialog(Gtk::Window&, const std::string&, const std::string& = "", bool = false);

            void setShowText(bool = true);

            void setText(const std::string&);
            void setFraction(double);

        private:
            //! The label for the progress bar
            Gtk::Label m_label;

            //! The progress bar widget
            Gtk::ProgressBar m_progress_bar;
    };
}

#endif

