#ifndef DRIVER_H
# define DRIVER_H

# include <memory>

# include "MapNormalizerApplication.h"
# include "Window.h"

namespace MapNormalizer::GUI {
    enum class State {
        NORMAL
    };

    class Driver {
        public:
            Driver();
            ~Driver();

            bool initialize();

            void run();

            State getState();
            void setState(State);

        private:
            Glib::RefPtr<MapNormalizerApplication> m_app;

            State m_state;
    };
}

#endif

