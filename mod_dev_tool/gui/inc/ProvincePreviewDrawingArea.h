#ifndef PROVINCEPREVIEWDRAWINGAREA_H
# define PROVINCEPREVIEWDRAWINGAREA_H

# include <functional>

# include "gtkmm/drawingarea.h"
# include "gdkmm/pixbuf.h"
# include "gdkmm/event.h"

# include "IProvincePreviewDrawingArea.h"

namespace HMDT::GUI {
    /**
     * @brief The area where a singlee province preview can get drawn
     */
    class ProvincePreviewDrawingArea: public IProvincePreviewDrawingArea<Gtk::DrawingArea>
    {
        public:
            using DataPtr = std::weak_ptr<const unsigned char[]>;

            ProvincePreviewDrawingArea();
            virtual ~ProvincePreviewDrawingArea() = default;

            void setData(DataPtr, uint32_t, uint32_t);

            virtual bool isValid() const override;

        protected:
            virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>&) override;

            virtual Gtk::Widget* getParent() override;

        private:
            DataPtr m_data;
    };
}

#endif

