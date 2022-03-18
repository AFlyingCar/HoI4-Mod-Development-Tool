/**
 * @file ProvinceRenderingView.h
 *
 * @file Defines the ProvinceRenderingView class
 */

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
            virtual void beginRender() override;
            virtual void render() override;

            virtual ProgramList getPrograms() override;

            virtual void onMapDataChanged(std::shared_ptr<const MapData>) override;
            virtual void onSelectionChanged(std::optional<IMapDrawingAreaBase::SelectionInfo>) override;

        protected:
            Texture& getMapTexture();
            Texture& getLabelTexture();

            virtual void setupUniforms() override;
            virtual const std::string& getVertexShaderSource() const override;
            virtual const std::string& getFragmentShaderSource() const override;

        private:
            //! The shader for rendering the map outlines
            Program m_outline_shader;

            //! The shader for rendering the selection of a single province
            Program m_selection_shader;

            //! The Texture of the map
            Texture m_texture;

            //! The texture of the label matrix
            Texture m_label_texture;

            //! The outline texture
            Texture m_outline_texture;

            //! The texture to draw on top of a selected province
            Texture m_selection_texture;
    };
}

#endif

