/**
 * @file MapRenderingViewBase.h
 *
 * @brief A base class for rendering just the map by itself.
 *
 * @details While this can be used just by itself, it is meant to be inherited
 *          from by more specific *RenderingView classes, which do extra things
 *          on top of the basic map.
 */

#ifndef MAPRENDERINGVIEWBASE_H
# define MAPRENDERINGVIEWBASE_H

# include <array>
# include <memory>

# include <glm/vec4.hpp>

# include "Program.h"
# include "Texture.h"
# include "IRenderingView.h"

namespace HMDT::GUI::GL {
    /**
     * @brief Base class for rendering the map.
     */
    class MapRenderingViewBase: public IRenderingView {
        public:
            MapRenderingViewBase();
            virtual ~MapRenderingViewBase();

            virtual void init() override;
            virtual void beginRender() override;
            virtual void render() override;
            virtual void endRender() override;

            virtual ProgramList getPrograms() override;

        protected:
            Program& getMapProgram();

            virtual void setupUniforms() = 0;

            virtual const std::string& getVertexShaderSource() const = 0;
            virtual const std::string& getFragmentShaderSource() const = 0;

            std::array<glm::vec4, 6> getMapVertices();

            Texture& getSelectionTexture();

            void drawMapVAO();

        private:
            //! The map VAO
            uint32_t m_vao = -1;

            //! The map VBO
            uint32_t m_vbo = -1;

            //! The basic Program for rendering the map
            Program m_program;

            //! The texture to draw on top of a selected province
            Texture m_selection_texture;
    };
}

#endif
