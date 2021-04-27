#ifndef DRIVER_H
# define DRIVER_H

# include "glibmm/refptr.h"

namespace MapNormalizer::GUI {
    class MapNormalizerApplication;

    /**
     * @brief The main driver of the GUI-based frontend
     *
     * @details The purpose of this class is to hold both the GUI application,
     *          as well as all global state information about the rest of the
     *          application.
     */
    class Driver {
        public:
            static Driver& getInstance();

            Driver(const Driver&) = delete;
            Driver(Driver&&) = delete;

            ~Driver();

            bool initialize();

            void run();

        protected:
            Driver();

        private:
            Glib::RefPtr<MapNormalizerApplication> m_app;
    };
}

#endif

