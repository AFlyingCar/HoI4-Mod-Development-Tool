#ifndef IRENDERINGVIEW_H
# define IRENDERINGVIEW_H

# include "BitMap.h"

namespace MapNormalizer::GUI::GL {
    class Program;

    /**
     * @brief Base interface for implementing a rendering view
     */
    class IRenderingView {
        public:
            IRenderingView() = default;
            virtual ~IRenderingView() = default;
            IRenderingView(IRenderingView&&) = default;

            IRenderingView(const IRenderingView&) = delete;
            IRenderingView& operator=(const IRenderingView&) = delete;

            virtual void init() = 0;

            virtual void onMapDataChanged(const BitMap*,
                                          const unsigned char*) = 0;

            virtual void beginRender() = 0;
            virtual void render() = 0;
            virtual void endRender() = 0;

            virtual Program& getProgram() = 0;
    };
}

#endif

