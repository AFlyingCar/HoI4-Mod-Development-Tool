
#include "MapProject.h"

#include <fstream>
#include <cstring>
#include <cerrno>

#include "Logger.h"
#include "Constants.h"
#include "Util.h"

#include "Project.h"

MapNormalizer::Project::MapProject::MapProject(IProject& parent_project):
    m_shape_finder(nullptr),
    m_image(nullptr),
    m_parent_project(parent_project)
{
}

MapNormalizer::Project::MapProject::~MapProject() {
}

bool MapNormalizer::Project::MapProject::save(const std::filesystem::path& path)
{
    if(!std::filesystem::exists(path)) {
        writeDebug("Creating directory ", path);
        std::filesystem::create_directory(path);
    }

    return saveShapeLabels(path) && saveProvinceData(path);
}

bool MapNormalizer::Project::MapProject::load(const std::filesystem::path& path)
{
    // If there is no root path for this subproject, then don't bother trying
    //  to load
    if(!std::filesystem::exists(path)) {
        return true;
    }

    auto inputs_root = dynamic_cast<HoI4Project&>(m_parent_project).getInputsRoot();
    auto input_provincemap_path = inputs_root / INPUT_PROVINCEMAP_FILENAME;
    if(!std::filesystem::exists(input_provincemap_path)) {
        writeWarning("Source import image does not exist, unable to finish loading data.");
        return false;
    } else {
        m_image = readBMP(input_provincemap_path);
        if(m_image == nullptr) {
            writeWarning("Failed to read imported image.");
            return false;
        }
    }

    return loadShapeLabels(path) && loadProvinceData(path);
}

////////////////////////////////////////////////////////////////////////////////

bool MapNormalizer::Project::MapProject::saveShapeLabels(const std::filesystem::path& root)
{
    if(m_shape_finder->getShapes().empty()) {
        writeDebug("Nothing to write!");
        return true;
    }

    auto path = root / SHAPEDATA_FILENAME;

    // write the shape finder data in a way that we can re-load it later
    if(std::ofstream out(path, std::ios::binary | std::ios::out); out)
    {
        out << SHAPEDATA_MAGIC;

        writeData(out, m_shape_finder->getImage()->info_header.width, 
                       m_shape_finder->getImage()->info_header.height);

        // Write the entire label matrix to the file
        out.write(reinterpret_cast<const char*>(m_shape_finder->getLabelMatrix()),
                  m_shape_finder->getLabelMatrixSize() * sizeof(uint32_t));
        out << '\0';
    } else {
        return false;
    }

    return true;
}

bool MapNormalizer::Project::MapProject::saveProvinceData(const std::filesystem::path& root)
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool MapNormalizer::Project::MapProject::loadShapeLabels(const std::filesystem::path& root)
{
    auto path = root / SHAPEDATA_FILENAME;

    if(!std::filesystem::exists(path)) {
        writeWarning("File ", path, " does not exist.");
        return false;
    } else if(std::ifstream in(path, std::ios::binary | std::ios::in); in)
    {
        unsigned char magic[4];
        uint32_t width = 0;
        uint32_t height = 0;

        // Read in all header information first, and make sure that we were 
        //  successful
        if(!safeRead(in, &magic, &width, &height)) {
            writeError("Failed to read in header information. Reason: ", std::strerror(errno));
            return false;
        }

        uint32_t label_matrix_size = width * height;
        uint32_t* label_matrix = new uint32_t[label_matrix_size];

        if(!safeRead(label_matrix, label_matrix_size * sizeof(uint32_t), in)) {
            writeError("Failed to read full label matrix. Reason: ", std::strerror(errno));

            delete[] label_matrix;
            return false;
        }

        // TODO: We also need to load in the CSV file that stores unique color data 
        PolygonList shapes = createPolygonListFromLabels(label_matrix, width,
                                                         height, {}, m_image);

        // TODO: We now need to save this data somewhere
    } else {
        writeError("Failed to open file ", path, ". Reason: ", std::strerror(errno));
        return false;
    }

    return true;
}

bool MapNormalizer::Project::MapProject::loadProvinceData(const std::filesystem::path& root)
{
    return true;
}

void MapNormalizer::Project::MapProject::setShapeFinder(ShapeFinder&& shape_finder)
{
    m_shape_finder.reset(new ShapeFinder(std::move(shape_finder)));
}

