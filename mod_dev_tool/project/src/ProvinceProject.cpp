
#include "ProvinceProject.h"

#include <fstream>
#include <cerrno>
#include <cstring>

#include "Constants.h"
#include "MapData.h"
#include "Util.h"

#include "ShapeFinder2.h"

#include "Logger.h"

HMDT::Project::ProvinceProject::ProvinceProject(IMapProject& parent_project):
    m_parent_project(parent_project),
    m_provinces()
{
}

HMDT::Project::ProvinceProject::~ProvinceProject() {
}

bool HMDT::Project::ProvinceProject::save(const std::filesystem::path& path,
                                          std::error_code& ec)
{
    if(m_provinces.empty()) {
        WRITE_DEBUG("Nothing to write!");
        return true;
    }

    return saveShapeLabels(path, ec) && saveProvinceData(path, ec);
}

bool HMDT::Project::ProvinceProject::load(const std::filesystem::path& path,
                                          std::error_code& ec)
{
    return loadProvinceData(path, ec) && loadShapeLabels(path, ec);
}

void HMDT::Project::ProvinceProject::import(const ShapeFinder& sf, std::shared_ptr<MapData>)
{
    m_provinces = createProvincesFromShapeList(sf.getShapes());
}

bool HMDT::Project::ProvinceProject::validateData() {
    // We have nothing to really validate here
    return true;
}

auto HMDT::Project::ProvinceProject::getMapData() -> std::shared_ptr<MapData> {
    return m_parent_project.getMapData();
}

auto HMDT::Project::ProvinceProject::getMapData() const
    -> const std::shared_ptr<MapData>
{
    return m_parent_project.getMapData();
}

/**
 * @brief Writes all shape label data to a file.
 *
 * @param root The root where the shape label data should be written to
 *
 * @return True if the data was able to be successfully written, false otherwise.
 */
bool HMDT::Project::ProvinceProject::saveShapeLabels(const std::filesystem::path& root,
                                                std::error_code& ec)
{
    auto path = root / SHAPEDATA_FILENAME;

    // write the shape finder data in a way that we can re-load it later
    if(std::ofstream out(path, std::ios::binary | std::ios::out); out)
    {
        out << SHAPEDATA_MAGIC;

        writeData(out, getMapData()->getWidth(), 
                       getMapData()->getHeight());

        // Write the entire label matrix to the file
        out.write(reinterpret_cast<const char*>(getMapData()->getLabelMatrix().lock().get()),
                  getMapData()->getMatrixSize() * sizeof(uint32_t));
        out << '\0';
    } else {
        ec = std::error_code(static_cast<int>(errno), std::generic_category());
        WRITE_ERROR("Failed to open file ", path, ". Reason: ", std::strerror(errno));
        return false;
    }

    return true;
}

/**
 * @brief Writes all province data to a .csv file (the same sort of file as
 *        would be loaded by HoI4
 *
 * @param root The root where the csv file should be written to
 *
 * @return True if the file was able to be successfully written, false otherwise.
 */
bool HMDT::Project::ProvinceProject::saveProvinceData(const std::filesystem::path& root,
                                                 std::error_code& ec)
{
    auto path = root / PROVINCEDATA_FILENAME;

    if(std::ofstream out(path); out) {
        // Write one line to the CSV for each province
        for(auto&& province : m_provinces) {
            out << province.id << ';'
                << static_cast<int>(province.unique_color.r) << ';'
                << static_cast<int>(province.unique_color.g) << ';'
                << static_cast<int>(province.unique_color.b) << ';'
                << province.type << ';'
                << (province.coastal ? "true" : "false")
                << ';' << province.terrain << ';'
                << province.continent << ';'
                << province.bounding_box.bottom_left.x << ';'
                << province.bounding_box.bottom_left.y << ';'
                << province.bounding_box.top_right.x << ';'
                << province.bounding_box.top_right.y << ';'
                << province.state
                << std::endl;
        }
    } else {
        ec = std::error_code(static_cast<int>(errno), std::generic_category());
        WRITE_ERROR("Failed to open file ", path, ". Reason: ", std::strerror(errno));
        return false;
    }

    return true;
}

/**
 * @brief Loads all shape label data out of $root/SHAPEDATA_FILENAME
 *
 * @param root The root where the shapedata is found
 *
 * @return True if the data was able to be loaded successfully, false otherwise
 */
bool HMDT::Project::ProvinceProject::loadShapeLabels(const std::filesystem::path& root,
                                                std::error_code& ec)
{
    auto path = root / SHAPEDATA_FILENAME;

    if(!std::filesystem::exists(path)) {
        WRITE_WARN("File ", path, " does not exist.");
        return false;
    } else if(std::ifstream in(path, std::ios::binary | std::ios::in); in) {
        unsigned char magic[4];
        uint32_t width = 0;
        uint32_t height = 0;

        // Read in all header information first, and make sure that we were 
        //  successful
        if(!safeRead(in, &magic, &width, &height)) {
            ec = std::error_code(static_cast<int>(errno), std::generic_category());
            WRITE_ERROR("Failed to read in header information. Reason: ", std::strerror(errno));
            return false;
        }

        // Validate that the width + height for the shape data matches what we
        //   expect.
        if((width * height) != getMapData()->getMatrixSize()) {
            ec = std::make_error_code(std::errc::invalid_argument);
            WRITE_ERROR("Loaded shape data size (", (width * height), ") does not match expected matrix size of (", getMapData()->getMatrixSize(), ")");
            return false;
        }

        auto label_matrix = getMapData()->getLabelMatrix().lock();

        if(!safeRead(label_matrix.get(), getMapData()->getMatrixSize() * sizeof(uint32_t), in))
        {
            ec = std::error_code(static_cast<int>(errno), std::generic_category());
            WRITE_ERROR("Failed to read full label matrix. Reason: ", std::strerror(errno));
            return false;
        }
    } else {
        ec = std::error_code(static_cast<int>(errno), std::generic_category());
        WRITE_ERROR("Failed to open file ", path, ". Reason: ", std::strerror(errno));
        return false;
    }

    return true;
}

/**
 * @brief Load all province-level data from $root/PROVINCEDATA_FILENAME, the
 *        same type of .csv file loaded by HoI4.
 *
 * @param root The path to the root where the .csv file is
 *
 * @return True if the file was able to be successfully loaded, false otherwise.
 */
bool HMDT::Project::ProvinceProject::loadProvinceData(const std::filesystem::path& root,
                                                 std::error_code& ec)
{
    auto path = root / PROVINCEDATA_FILENAME;

    if(!std::filesystem::exists(path, ec)) {
        WRITE_WARN("File ", path, " does not exist.");
        return false;
    } else if(std::ifstream in(path); in) {
        std::string line;

        // Make sure we don't have any provinces in the list first
        m_provinces.clear();

        // Get every line from the CSV file for parsing
        for(uint32_t line_num = 1; std::getline(in, line); ++line_num) {
            if(line.empty()) continue;

            std::stringstream ss(line);

            Province prov;

            // Attempt to parse the entire CSV line, we expect it to look like:
            //  ID;R;G;B;ProvinceType;IsCoastal;TerrainType;ContinentID;BB.BottomLeft.X;BB.BottomLeft.Y;BB.TopRight.X;BB.TopRight.Y;StateID
            if(!parseValuesSkipMissing<';'>(ss, &prov.id,
                                                &prov.unique_color.r,
                                                &prov.unique_color.g,
                                                &prov.unique_color.b,
                                                &prov.type,
                                                &prov.coastal,
                                                &prov.terrain,
                                                &prov.continent,
                                                &prov.bounding_box.bottom_left.x,
                                                &prov.bounding_box.bottom_left.y,
                                                &prov.bounding_box.top_right.x,
                                                &prov.bounding_box.top_right.y,
                                                &prov.state, true))
            {
                ec = std::make_error_code(std::errc::bad_message);
                WRITE_ERROR("Failed to parse line #", line_num, ": '", line, "'");
                return false;
            }

            m_provinces.push_back(prov);
        }

        WRITE_DEBUG("Loaded information for ",
                   m_provinces.size(), " provinces");
    } else {
        ec = std::error_code(static_cast<int>(errno), std::generic_category());
        WRITE_ERROR("Failed to open file ", path, ". Reason: ", std::strerror(errno));
        return false;
    }

    return true;
}

HMDT::ProvinceList& HMDT::Project::ProvinceProject::getProvinces() {
    return m_provinces;
}

const HMDT::ProvinceList& HMDT::Project::ProvinceProject::getProvinces() const {
    return m_provinces;
}

