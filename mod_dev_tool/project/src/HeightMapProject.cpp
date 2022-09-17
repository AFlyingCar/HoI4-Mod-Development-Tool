
#include "HeightMapProject.h"

#include <fstream>
#include <cstring>
#include <memory>

#include "Logger.h"

#include "Constants.h"
#include "StatusCodes.h"
#include "MapData.h"

#include "WorldNormalBuilder.h"

HMDT::Project::HeightMapProject::HeightMapProject(IMapProject& parent):
    m_parent_project(parent),
    m_heightmap_bmp(nullptr)
{ }

/**
 * @brief Writes all continent data to root/$HEIGHTMAP_FILENAME
 *
 * @param root The root where the heightmap should go
 * @param ec The error code
 *
 * @return True if the heightmap was successfully saved, false otherwise
 */
auto HMDT::Project::HeightMapProject::save(const std::filesystem::path& root)
    -> MaybeVoid
{
    if(m_heightmap_bmp == nullptr) {
        WRITE_ERROR("No heightmap has been loaded, cannot save yet.");
        RETURN_ERROR(STATUS_NO_DATA_LOADED);
    }

    auto path = root / HEIGHTMAP_FILENAME;

    // Write the heightmap to a file
    auto res = writeBMP(path, m_heightmap_bmp);
    RETURN_IF_ERROR(res);

    return STATUS_SUCCESS;
}

/**
 * @brief Loads all continent data from a file
 *
 * @param root The root where the continent data file should be found
 * @param ec The error code
 *
 * @return True if data was loaded correctly, false otherwise
 */
auto HMDT::Project::HeightMapProject::load(const std::filesystem::path& root)
    -> MaybeVoid
{
    auto path = root / HEIGHTMAP_FILENAME;

    // If the file doesn't exist, then return false (we didn't actually load it
    //  after all), but don't set the error code as it is expected that the
    //  file may not exist
    if(std::error_code ec; !std::filesystem::exists(path, ec)) {
        RETURN_ERROR_IF(ec.value() != 0, ec);

        WRITE_WARN("No data to load! No heightmap currently exists!");
        RETURN_ERROR(std::make_error_code(std::errc::no_such_file_or_directory));
    }

    return loadFile(path);
}

auto HMDT::Project::HeightMapProject::export_(const std::filesystem::path& root) const noexcept
    -> MaybeVoid
{
    // TODO: Do we want to export from MapData's heightmap? Or just use the
    //       BitMap object?
    auto res = writeBMP2(root / HEIGHTMAP_FILENAME,
                         getMapData()->getHeightMap().lock().get(),
                         getMapData()->getWidth(), getMapData()->getHeight(),
                         1 /* depth */,
                         true /* is_greyscale */);
    RETURN_IF_ERROR(res);

    {
        // Normal map is a 24-bit bitmap
        auto normal_data_size = getMapData()->getWidth() *
                                getMapData()->getWidth() * 3;
        std::unique_ptr<unsigned char[]> normal_data;
        try {
            normal_data.reset(new unsigned char[normal_data_size]);
        } catch(const std::bad_alloc& e) {
            WRITE_ERROR("Failed to allocate enough space for the world normal data.");
            RETURN_ERROR(STATUS_BADALLOC);
        }

        res = generateWorldNormalMap(*m_heightmap_bmp, normal_data.get());
        RETURN_IF_ERROR(res);

        res = writeBMP2(root / NORMALMAP_FILENAME, normal_data.get(),
                        getMapData()->getWidth(), getMapData()->getHeight(),
                        3 /* depth */);
        RETURN_IF_ERROR(res);
    }

    return STATUS_SUCCESS;
}

auto HMDT::Project::HeightMapProject::getRootParent() -> IRootProject& {
    return m_parent_project.getRootParent();
}

auto HMDT::Project::HeightMapProject::getMapData() -> std::shared_ptr<MapData> {
    return m_parent_project.getMapData();
}

auto HMDT::Project::HeightMapProject::getMapData() const 
    -> const std::shared_ptr<MapData>
{
    return m_parent_project.getMapData();
}

void HMDT::Project::HeightMapProject::import(const ShapeFinder&, std::shared_ptr<MapData>) { }

bool HMDT::Project::HeightMapProject::validateData() {
    // We have nothing to really validate here
    return true;
}

auto HMDT::Project::HeightMapProject::getRootMapParent() -> IMapProject& {
    return m_parent_project.getRootMapParent();
}

auto HMDT::Project::HeightMapProject::loadFile(const std::filesystem::path& path) noexcept
    -> MaybeVoid
{
    try {
        m_heightmap_bmp.reset(new BitMap2);
    } catch(std::bad_alloc e) {
        WRITE_ERROR("Failed to allocate space for new bitmap: ", e.what());
        RETURN_ERROR(STATUS_BADALLOC);
    }

    auto res = readBMP(path, m_heightmap_bmp);
    RETURN_IF_ERROR(res);

    WRITE_DEBUG(*m_heightmap_bmp);

    // Load heightmap data into MapData
    // This operation is fairly simple, as we are not making any modifications
    //   to the data itself, and just loading it into memory
    std::memcpy(getMapData()->getHeightMap().lock().get(),
                m_heightmap_bmp->data.get(),
                getMapData()->getHeightMapSize());

    return STATUS_SUCCESS;
}

auto HMDT::Project::HeightMapProject::getBitMap() const
    -> MonadOptionalRef<const BitMap2>
{
    if(m_heightmap_bmp != nullptr) {
        return *m_heightmap_bmp;
    } else {
        return std::nullopt;
    }
}

