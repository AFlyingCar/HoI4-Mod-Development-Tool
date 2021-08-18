#ifndef PROVINCERENDERINGVIEW_H
# define PROVINCERENDERINGVIEW_H

# include "MapRenderingViewBase.h"

namespace MapNormalizer::GUI::GL {
    class ProvinceRenderingView: public MapRenderingViewBase {
        public:
            ProvinceRenderingView() = default;

            virtual void init() override;
            virtual void render() override;

        private:
            Program m_outline_shader;
            Program m_selection_shader;

            Texture m_outline_texture;
    };
}

#endif

