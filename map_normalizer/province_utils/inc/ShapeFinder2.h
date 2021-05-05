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
            using LabelToColorMap = std::map<uint32_t, Color>;

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

            ShapeFinder(const BitMap*);
            ShapeFinder();
            ShapeFinder(ShapeFinder&&);

            ShapeFinder& operator=(ShapeFinder&&);

            const PolygonList& findAllShapes();

            void estop();

            Stage getStage() const;

            uint32_t getLabelMatrixSize();
            uint32_t* getLabelMatrix();
            std::vector<Pixel>& getBorderPixels();
            std::map<uint32_t, Color>& getLabelToColorMap();
            PolygonList& getShapes();

            const BitMap* getImage() const;
            uint32_t getLabelMatrixSize() const;
            const uint32_t* getLabelMatrix() const;
            const std::vector<Pixel>& getBorderPixels() const;
            const std::map<uint32_t, Color>& getLabelToColorMap() const;
            const PolygonList& getShapes() const;

        protected:
            using LabelShapeIdxMap = std::unordered_map<uint32_t, uint32_t>;

            uint32_t pass1();
            PolygonList& pass2(LabelShapeIdxMap&);

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

        private:
            //! The image to find shapes on
            const BitMap* m_image;

            //! The size of the label matrix
            uint32_t m_label_matrix_size;

            //! A flat array containing the label for each pixel
            uint32_t* m_label_matrix;

            //! A mapping of each label -> that label's root (key == value => key is already the root)
            std::unordered_map<uint32_t, uint32_t> m_label_parents;

            //! A vector of every border pixel
            std::vector<Pixel> m_border_pixels;

            //! The color of each label
            LabelToColorMap m_label_to_color;

            //! Whether or not the find algorithm should stop
            bool m_do_estop;

            //! The stage the findAllShapes() algorithm is at.
            Stage m_stage;

            //! The last list of shapes that were found
            PolygonList m_shapes;
    };

    void addPixelToShape(Polygon&, const Pixel&);

    PolygonList createPolygonListFromLabels(uint32_t*, uint32_t, uint32_t,
                                            const ShapeFinder::LabelToColorMap&,
                                            const BitMap*);

    std::string toString(const ShapeFinder::Stage&);
}

#endif

