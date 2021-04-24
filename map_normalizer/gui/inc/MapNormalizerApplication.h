#ifndef MAPNORMALIZER_APPLICATION_H
# define MAPNORMALIZER_APPLICATION_H

# include <map>
# include <string>
# include <memory>
# include <optional>
# include <functional>

# include "gtkmm.h"

# include "Window.h"

namespace MapNormalizer::GUI {
    class MapNormalizerApplication: public Gtk::Application {
        public:
            MapNormalizerApplication();
            virtual ~MapNormalizerApplication() = default;

        protected:
            void on_activate() override;
            void on_startup() override;

        private:
            std::unique_ptr<Window> m_window;
    };
}

#endif

