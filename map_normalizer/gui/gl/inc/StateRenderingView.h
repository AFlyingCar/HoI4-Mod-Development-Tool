/**
 * @file StateRenderingView.h
 *
 * @file Defines the StateRenderingView class
 */

#ifndef STATERENDERINGVIEW_H
# define STATERENDERINGVIEW_H

# include <optional>

# include "MapRenderingViewBase.h"

# include "IMapDrawingArea.h" // SelectionInfo

namespace MapNormalizer::GUI::GL {
    class StateRenderingView: public MapRenderingViewBase {
        public:
            StateRenderingView() = default;

            virtual void init() override;
            virtual void beginRender() override;
            virtual void render() override;

            virtual void onMapDataChanged(std::shared_ptr<const MapData>) override;
            virtual void onSelectionChanged(std::optional<IMapDrawingAreaBase::SelectionInfo>) override;

        protected:
            virtual void setupUniforms() override;

            virtual const std::string& getVertexShaderSource() const override;
            virtual const std::string& getFragmentShaderSource() const override;

            Texture& getStateIDMatrixTexture();

        private:
            std::shared_ptr<const MapData> m_map_data;

            //! The state ID texture
            Texture m_state_id_texture;

            //! The texture to draw on top of a selected province
            // Texture m_selection_texture;
    };
}

#endif

