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
# include "IRenderingView.h"

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
            virtual void onViewingModeChange(ViewingMode) override { };
            virtual void onSetData(std::shared_ptr<const MapData>) override;
            virtual void onShow() override;

            std::shared_ptr<IRenderingView> getCurrentRenderingView();

            void setupAllUniforms();

            glm::mat4 getProjection();
            glm::mat4 getTransformation();

        private:
            std::map<ViewingMode, std::shared_ptr<IRenderingView>> m_rendering_views;

            bool m_initialized;
    };
}

#endif

