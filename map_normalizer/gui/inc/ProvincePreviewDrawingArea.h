#ifndef PROVINCEPREVIEWDRAWINGAREA_H
# define PROVINCEPREVIEWDRAWINGAREA_H

# include <functional>

# include "gtkmm/drawingarea.h"
# include "gdkmm/pixbuf.h"
# include "gdkmm/event.h"

namespace MapNormalizer::GUI {
    class ProvincePreviewDrawingArea: public Gtk::DrawingArea {
        public:
            using DataPtr = std::weak_ptr<const unsigned char[]>;

            ProvincePreviewDrawingArea();
            virtual ~ProvincePreviewDrawingArea() = default;

            void setScale(double = 1.0, double = 1.0);

            void setData(DataPtr, uint32_t, uint32_t);

            bool isValid() const;

            void calcScale();

        protected:
            virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>&) override;

        private:
            DataPtr m_data;
            uint32_t m_width;
            uint32_t m_height;

            double m_scalex;
            double m_scaley;
    };
}

#endif

