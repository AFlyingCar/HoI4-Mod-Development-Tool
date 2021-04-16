/**
 * @file ShapeFinder2.h
 *
 * @brief Defines functions for finding and validating shapes in an input .BMP.
 */

#ifndef SHAPEFINDER2_H
# define SHAPEFINDER2_H

# include <map>
# include <unordered_map>
# include <optional>

# include "Types.h"
# include "BitMap.h"

namespace MapNormalizer {
    /**
     * @brief Holds all state information about connected component labeling.
     */
    class ShapeFinder {
        public:
            ShapeFinder(BitMap*);

            PolygonList findAllShapes();
            
        protected:
            uint32_t pass1();
            PolygonList pass2(std::map<uint32_t, uint32_t>&);

            bool mergeBorders(PolygonList&,
                              const std::map<uint32_t, uint32_t>&);

            std::pair<uint32_t, Color> getLabelAndColor(const Point2D&,
                                                        const Color&);

            std::optional<uint32_t> errorCheckAllShapes(const PolygonList&);

            void outputStage(const std::string&);

            uint32_t getRootLabel(uint32_t);

            std::optional<Point2D> getAdjacentPixel(Point2D, Direction,
                                                    Direction = Direction::NONE) const;

        private:
            //! The image to find shapes on
            BitMap* m_image;

            //! The size of the label matrix
            uint32_t m_label_matrix_size;

            //! A flat array containing the label for each pixel
            uint32_t* m_label_matrix;

            //! A mapping of each label -> that label's root (key == value => key is already the root)
            std::unordered_map<uint32_t, uint32_t> m_label_parents;

            //! A vector of every border pixel
            std::vector<Pixel> m_border_pixels;

            //! The color of each label
            std::map<uint32_t, Color> m_label_to_color;
    };
}

#endif

