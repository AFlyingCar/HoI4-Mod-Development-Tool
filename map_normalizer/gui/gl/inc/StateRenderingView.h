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

            void updateStateIDTexture();

        private:
            std::shared_ptr<const MapData> m_map_data;

            //! The state ID texture
            Texture m_state_id_texture;

            //! A tag for the last state ID matrix value, used to know if it needs to be refreshed
            uint32_t m_last_state_id_matrix_updated_tag;
    };
}

#endif

