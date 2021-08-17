#ifndef MAPDRAWINGAREAGL_H
# define MAPDRAWINGAREAGL_H

# include <memory>
# include <vector>
# include <map>

# include <gtkmm/glarea.h>
# include <glm/vec4.hpp>
# include <glm/mat4x4.hpp>

# include "IMapDrawingArea.h"
# include "Program.h"
# include "Texture.h"

namespace MapNormalizer::GUI::GL {
    class MapDrawingArea: public IMapDrawingArea<Gtk::GLArea> {
        public:
            MapDrawingArea();
            virtual ~MapDrawingArea();

            virtual void queueDraw() override;

        protected:
            virtual bool on_render(const Glib::RefPtr<Gdk::GLContext>&) override;
            virtual void on_unrealize() override;
            virtual bool on_button_press_event(GdkEventButton*) override;

            virtual void init() override;
            virtual void onZoom() override;
            virtual void onViewingModeChange(ViewingMode) override;
            virtual void onSetData(const BitMap*, const unsigned char*) override;
            virtual void onShow() override;

            void setupAllUniforms();

            void setupProvinceViewUniforms();
            void setupStatesViewUniforms();

            const std::vector<std::shared_ptr<Texture>>& getTexturesFor(ViewingMode);

            glm::mat4 getProjection();
            glm::mat4 getTransformation();

            std::array<glm::vec4, 6> getMapVertices();
        private:
            Program* m_current_program;

            Program m_provinceview_program;
            // All other programs...

            std::map<ViewingMode, std::vector<std::shared_ptr<Texture>>> m_textures;

            uint32_t m_map_vao;
            uint32_t m_map_vbo;

            bool m_initialized;
    };
}

#endif

