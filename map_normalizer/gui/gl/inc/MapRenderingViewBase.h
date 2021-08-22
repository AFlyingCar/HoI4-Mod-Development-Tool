#ifndef MAPRENDERINGVIEWBASE_H
# define MAPRENDERINGVIEWBASE_H

# include <array>
# include <memory>

# include <glm/vec4.hpp>

# include "Program.h"
# include "Texture.h"
# include "IRenderingView.h"

namespace MapNormalizer::GUI::GL {
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

            virtual void onMapDataChanged(std::shared_ptr<const MapData>) override;

        protected:
            Program& getMapProgram();
            Texture& getMapTexture();

            virtual void setupUniforms();

            std::array<glm::vec4, 6> getMapVertices();

            void drawMapVAO();

        private:
            uint32_t m_vao = -1;
            uint32_t m_vbo = -1;

            Program m_program;
            Texture m_texture;
    };
}

#endif
