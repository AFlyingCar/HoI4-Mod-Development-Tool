#ifndef MAPDRAWINGAREAGL_H
# define MAPDRAWINGAREAGL_H

# include <memory>
# include <vector>
# include <map>

# include <gtkmm/glarea.h>
# include <glm/vec4.hpp>

# include "IMapDrawingArea.h"
# include "Program.h"
# include "Texture.h"

namespace MapNormalizer::GUI::GL {
    class MapDrawingArea: public IMapDrawingArea<Gtk::GLArea> {
        public:
            MapDrawingArea();
            virtual ~MapDrawingArea();

        protected:
            virtual void on_realize() override;
            virtual bool on_render(const Glib::RefPtr<Gdk::GLContext>&) override;
            virtual void on_unrealize() override;

            virtual void init() override;
            virtual void onZoom() override;
            virtual void onViewingModeChange(ViewingMode) override;
            virtual void onSetImage(const BitMap*) override;
            virtual void onSetGraphicsData(const unsigned char*) override;

            void renderProvinceView();
            void renderStatesView();

            const std::vector<std::shared_ptr<Texture>>& getTexturesFor(ViewingMode);

            std::array<glm::vec4, 6> getMapVertices();
        private:
            Program* m_current_program;

            Program m_provinceview_program;
            // All other programs...

            std::map<ViewingMode, std::vector<std::shared_ptr<Texture>>> m_textures;

            uint32_t m_map_vao;
            uint32_t m_map_vbo;
    };
}

#endif

