#ifndef DRIVER_H
# define DRIVER_H

# include <memory>

# include "MyGUI.h"
# include "MyGUI_OpenGLPlatform.h"

# include "ImageLoader.h"

namespace MapNormalizer::GUI {
    class Driver {
        public:
            Driver();
            ~Driver();

            void initialize();

        private:
            std::unique_ptr<MyGUI::OpenGLPlatform> m_platform;
            std::unique_ptr<MyGUI::Gui> m_gui;

            ImageLoader m_image_loader;
    };
}

#endif

