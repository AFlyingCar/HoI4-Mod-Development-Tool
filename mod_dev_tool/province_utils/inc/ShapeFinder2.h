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
# include <memory>

# include "IGraphicsWorker.h"
# include "Types.h"
# include "BitMap.h"
# include "Monad.h"

namespace HMDT {
    class MapData;

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

            ShapeFinder(const BitMap*, IGraphicsWorker&, std::shared_ptr<MapData>);
            ShapeFinder(IGraphicsWorker&);
            ShapeFinder(ShapeFinder&&);

            ShapeFinder& operator=(ShapeFinder&&);

            const PolygonList& findAllShapes();

            void estop();

            Stage getStage() const;

            std::vector<Pixel>& getBorderPixels();
            std::map<uint32_t, Color>& getLabelToColorMap();
            PolygonList& getShapes();

            const BitMap* getImage() const;
            const std::vector<Pixel>& getBorderPixels() const;
            const std::map<uint32_t, Color>& getLabelToColorMap() const;
            const PolygonList& getShapes() const;

            static bool calculateAdjacency(const BitMap*, const uint32_t*,
                                           std::set<uint32_t>&, const Point2D&);
            static bool calculateAdjacency(const Dimensions&,
                                           const uint8_t*,
                                           const uint32_t*,
                                           std::set<uint32_t>&,
                                           const Point2D&);

            static MonadOptional<Point2D> getAdjacentPoint(const BitMap*,
                                                           const Point2D&,
                                                           Direction);
            static MonadOptional<Point2D> getAdjacentPoint(const Dimensions&,
                                                           const uint8_t*,
                                                           const Point2D&,
                                                           Direction);
            static MonadOptional<Pixel> getAdjacentPixel(const BitMap*,
                                                         const Point2D&,
                                                         Direction);
            static MonadOptional<Pixel> getAdjacentPixel(const Dimensions&,
                                                         const uint8_t*,
                                                         const Point2D&,
                                                         Direction);
        protected:
            using LabelShapeIdxMap = std::unordered_map<uint32_t, uint32_t>;

            uint32_t pass1();
            PolygonList& pass2(LabelShapeIdxMap&);

            bool mergeBorders(PolygonList&,
                              const LabelShapeIdxMap&);

            std::pair<uint32_t, Color> getLabelAndColor(const Point2D&,
                                                        const Color&);

            std::optional<uint32_t> finalize(PolygonList&);

            void outputStage(const std::filesystem::path&);

            uint32_t getRootLabel(uint32_t);

            MonadOptional<Point2D> getAdjacentPoint(const Point2D&, Direction) const;

            void buildShape(uint32_t, const Pixel&, PolygonList&,
                            LabelShapeIdxMap&);

            void calculateAdjacencies(PolygonList&) const;

        private:
            //! The graphics worker
            IGraphicsWorker& m_worker;

            //! The image to find shapes on
            const BitMap* m_image;

            //! The shared map data
            std::shared_ptr<MapData> m_map_data;

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

    std::string toString(const ShapeFinder::Stage&);
}

#endif

