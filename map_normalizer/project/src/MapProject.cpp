
#include "MapProject.h"

#include "Logger.h"

MapNormalizer::Project::MapProject::MapProject() {
}

MapNormalizer::Project::MapProject::~MapProject() {
}

bool MapNormalizer::Project::MapProject::save(const std::filesystem::path& path)
{
    if(!std::filesystem::exists(path)) {
        writeDebug("Creating directory ", path);
        std::filesystem::create_directory(path);
    }

    if(m_shape_finder.getShapes().empty()) {
        writeDebug("Nothing to write!");
        return true;
    }

    const char SDAT_MAGIC[] = { 'S', 'D', 'A', 'T' };

    // write the shape finder data in a way that we can re-load it later
    if(std::ofstream out(path / "shapedata.bin", std::ios::binary | std::ios::out); out)
    {
        out << SDAT_MAGIC << m_shape_finder.getImage()->info_header.width
                          << m_shape_finder.getImage()->info_header.height;

        // Write the entire label matrix to the file
        out.write(reinterpret_cast<const char*>(m_shape_finder.getLabelMatrix()),
                  m_shape_finder.getLabelMatrixSize() * sizeof(uint32_t));
        out << '\0';
    } else {
        return false;
    }

    return true;
}

bool MapNormalizer::Project::MapProject::load(const std::filesystem::path& path)
{
    return true;
}

void MapNormalizer::Project::MapProject::setShapeFinder(ShapeFinder&& shape_finder)
{
    m_shape_finder = std::move(shape_finder);
}

