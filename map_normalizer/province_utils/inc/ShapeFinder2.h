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
            enum class Stage {
                START,
                PASS1,
                OUTPUT_PASS1,
                PASS2,
                OUTPUT_PASS2,
                MERGE_BORDERS,
                ERROR_CHECK,
                DONE
            };

            ShapeFinder(BitMap*);

            PolygonList findAllShapes();

            void estop();

            Stage getStage() const;

        protected:
            using LabelShapeIdxMap = std::unordered_map<uint32_t, uint32_t>;

            uint32_t pass1();
            PolygonList pass2(LabelShapeIdxMap&);

            bool mergeBorders(PolygonList&,
                              const LabelShapeIdxMap&);

            std::pair<uint32_t, Color> getLabelAndColor(const Point2D&,
                                                        const Color&);

            std::optional<uint32_t> errorCheckAllShapes(const PolygonList&);

            void outputStage(const std::string&);

            uint32_t getRootLabel(uint32_t);

            std::optional<Point2D> getAdjacentPixel(const Point2D&, Direction) const;

            void buildShape(uint32_t, const Pixel&, PolygonList&,
                            LabelShapeIdxMap&);

            void addPixelToShape(Polygon&, const Pixel&);

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

            //! Whether or not the find algorithm should stop
            bool m_do_estop;

            //! The stage the findAllShapes() algorithm is at.
            Stage m_stage;
    };

    std::string toString(const ShapeFinder::Stage&);
}

#endif

