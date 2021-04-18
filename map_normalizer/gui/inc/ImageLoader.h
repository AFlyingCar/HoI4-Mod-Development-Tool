#ifndef IMAGE_LOADER_H
# define IMAGE_LOADER_H

# include <functional>
# include <map>

# include "MyGUI_OpenGLImageLoader.h"

namespace MapNormalizer::GUI {
    class ImageLoader: public MyGUI::OpenGLImageLoader {
        public:
            ImageLoader();
            virtual ~ImageLoader();

            virtual void* loadImage(int&, int&, MyGUI::PixelFormat&,
                                    const std::string&) override;
            virtual void saveImage(int, int, MyGUI::PixelFormat, void*,
                                   const std::string&) override;

        protected:
            void* loadBitMap(int&, int&, MyGUI::PixelFormat&,
                             const std::string&);
            void saveBitMap(int, int, MyGUI::PixelFormat, void*, const std::string&);
    };
}

#endif

