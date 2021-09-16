
#ifndef IPROVINCEPREVIEWDRAWINGAREA_H
# define IPROVINCEPREVIEWDRAWINGAREA_H

# include <gtkmm/widget.h>

namespace MapNormalizer::GUI {
    class IProvincePreviewDrawingAreaBase {
        public:
            IProvincePreviewDrawingAreaBase();
            virtual ~IProvincePreviewDrawingAreaBase() = default;

            void setScale(double = 1.0, double = 1.0);

            virtual bool isValid() const = 0;

            void calcScale();

        protected:
            std::pair<double, double> getScale() const;
            std::pair<uint32_t, uint32_t> getDimensions() const;

            void setDimensions(uint32_t, uint32_t);

            virtual Gtk::Widget* getParent() = 0;

        private:
            double m_scalex;
            double m_scaley;

            uint32_t m_width;
            uint32_t m_height;
    };

    template<typename BaseGtkWidget>
    class IProvincePreviewDrawingArea: public IProvincePreviewDrawingAreaBase,
                                       public BaseGtkWidget
    { };
}

#endif

