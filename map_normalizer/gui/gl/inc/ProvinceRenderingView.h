#ifndef PROVINCERENDERINGVIEW_H
# define PROVINCERENDERINGVIEW_H

# include <optional>

# include "MapRenderingViewBase.h"

# include "IMapDrawingArea.h" // SelectionInfo

namespace MapNormalizer::GUI::GL {
    class ProvinceRenderingView: public MapRenderingViewBase {
        public:
            ProvinceRenderingView() = default;

            virtual void init() override;
            virtual void render() override;

            virtual ProgramList getPrograms() override;

            virtual void onMapDataChanged(std::shared_ptr<const MapData>) override;
            virtual void onSelectionChanged(std::optional<IMapDrawingAreaBase::SelectionInfo>) override;

        private:
            Program m_outline_shader;
            Program m_selection_shader;

            Texture m_outline_texture;
            Texture m_selection_area_texture;
            Texture m_selection_texture;
    };
}

#endif

