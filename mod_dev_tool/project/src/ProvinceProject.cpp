
#include "ProvinceProject.h"

#include <fstream>
#include <cerrno>
#include <cstring>

#include "Constants.h"
#include "MapData.h"
#include "Util.h"
#include "StatusCodes.h"

#include "ShapeFinder2.h"

#include "Logger.h"

HMDT::Project::ProvinceProject::ProvinceProject(IMapProject& parent_project):
    m_parent_project(parent_project),
    m_provinces()
{
}

HMDT::Project::ProvinceProject::~ProvinceProject() {
}

auto HMDT::Project::ProvinceProject::save(const std::filesystem::path& path)
    -> MaybeVoid
{
    if(m_provinces.empty()) {
        WRITE_DEBUG("Nothing to write!");
        return STATUS_SUCCESS;
    }

    auto shapelabels_result = saveShapeLabels(path);
    RETURN_IF_ERROR(shapelabels_result);

    auto provdata_result = saveProvinceData(path);
    RETURN_IF_ERROR(provdata_result);

    return STATUS_SUCCESS;
}

auto HMDT::Project::ProvinceProject::load(const std::filesystem::path& path)
    -> MaybeVoid
{
    auto provdata_result = loadProvinceData(path);
    if(provdata_result.error() == std::errc::no_such_file_or_directory) {
        provdata_result = STATUS_SUCCESS;
    }
    RETURN_IF_ERROR(provdata_result);

    auto shapelabels_result = loadShapeLabels(path);
    RETURN_IF_ERROR(shapelabels_result);

    return STATUS_SUCCESS;
}

void HMDT::Project::ProvinceProject::import(const ShapeFinder& sf, std::shared_ptr<MapData>)
{
    m_provinces = createProvincesFromShapeList(sf.getShapes());
}

bool HMDT::Project::ProvinceProject::validateData() {
    // We have nothing to really validate here
    return true;
}

HMDT::Project::IProject& HMDT::Project::ProvinceProject::getRootParent() {
    return m_parent_project.getRootParent();
}

HMDT::Project::IMapProject& HMDT::Project::ProvinceProject::getRootMapParent() {
    return m_parent_project.getRootMapParent();
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
auto HMDT::Project::ProvinceProject::saveShapeLabels(const std::filesystem::path& root)
    -> MaybeVoid
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
        WRITE_ERROR("Failed to open file ", path);
        RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
    }

    return STATUS_SUCCESS;
}

/**
 * @brief Writes all province data to a .csv file (the same sort of file as
 *        would be loaded by HoI4
 *
 * @param root The root where the csv file should be written to
 *
 * @return True if the file was able to be successfully written, false otherwise.
 */
auto HMDT::Project::ProvinceProject::saveProvinceData(const std::filesystem::path& root)
    -> MaybeVoid
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
        WRITE_ERROR("Failed to open file ", path);
        RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
    }

    return STATUS_SUCCESS;
}

/**
 * @brief Loads all shape label data out of $root/SHAPEDATA_FILENAME
 *
 * @param root The root where the shapedata is found
 *
 * @return True if the data was able to be loaded successfully, false otherwise
 */
auto HMDT::Project::ProvinceProject::loadShapeLabels(const std::filesystem::path& root)
    -> MaybeVoid
{
    auto path = root / SHAPEDATA_FILENAME;

    if(std::error_code ec; !std::filesystem::exists(path, ec)) {
        RETURN_ERROR_IF(ec.value() != 0, ec);

        WRITE_WARN("File ", path, " does not exist.");
        return std::make_error_code(std::errc::no_such_file_or_directory);
    } else if(std::ifstream in(path, std::ios::binary | std::ios::in); in) {
        unsigned char magic[4];
        uint32_t width = 0;
        uint32_t height = 0;

        // Read in all header information first, and make sure that we were 
        //  successful
        if(!safeRead(in, &magic, &width, &height)) {
            WRITE_ERROR("Failed to read in header information.");
            RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
        }

        // Validate that the width + height for the shape data matches what we
        //   expect.
        if((width * height) != getMapData()->getMatrixSize()) {
            WRITE_ERROR("Loaded shape data size (", (width * height), ") does not match expected matrix size of (", getMapData()->getMatrixSize(), ")");
            RETURN_ERROR(std::make_error_code(std::errc::invalid_argument));
        }

        auto label_matrix = getMapData()->getLabelMatrix().lock();

        if(!safeRead(label_matrix.get(), getMapData()->getMatrixSize() * sizeof(uint32_t), in))
        {
            WRITE_ERROR("Failed to read full label matrix.");
            RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
        }
    } else {
        WRITE_ERROR("Failed to open file ", path);
        RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
    }

    return STATUS_SUCCESS;
}

/**
 * @brief Load all province-level data from $root/PROVINCEDATA_FILENAME, the
 *        same type of .csv file loaded by HoI4.
 *
 * @param root The path to the root where the .csv file is
 *
 * @return True if the file was able to be successfully loaded, false otherwise.
 */
auto HMDT::Project::ProvinceProject::loadProvinceData(const std::filesystem::path& root)
    -> MaybeVoid
{
    auto path = root / PROVINCEDATA_FILENAME;

    if(std::error_code ec; !std::filesystem::exists(path, ec)) {
        RETURN_ERROR_IF(ec.value() != 0, ec);

        WRITE_WARN("File ", path, " does not exist.");
        return std::make_error_code(std::errc::no_such_file_or_directory);
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
                WRITE_ERROR("Failed to parse line #", line_num, ": '", line, "'");
                RETURN_ERROR(std::make_error_code(std::errc::bad_message));
            }

            m_provinces.push_back(prov);
        }

        WRITE_DEBUG("Loaded information for ",
                   m_provinces.size(), " provinces");
    } else {
        WRITE_ERROR("Failed to open file ", path);
        RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
    }

    return STATUS_SUCCESS;
}

HMDT::ProvinceList& HMDT::Project::ProvinceProject::getProvinces() {
    return m_provinces;
}

const HMDT::ProvinceList& HMDT::Project::ProvinceProject::getProvinces() const {
    return m_provinces;
}

