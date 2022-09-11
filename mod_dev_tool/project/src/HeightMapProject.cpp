
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

    // Try to open the heightmap file for saving.
    // TODO: We really need to do some error checking from writeBMP
#if 0
    writeBMP(path, m_heightmap_bmp.get());
#else
    writeBMP(path,
             getMapData()->getHeightMap().lock().get(),
             getMapData()->getWidth(), getMapData()->getHeight(),
             1 /* depth */,
             true /* is_greyscale */);
#endif

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
    // TODO: writeBMP does not actually return any errors out to us, so we
    //  need to be careful here in case it does fail
    // TODO: Do we want to export from MapData's heightmap? Or just use the
    //       BitMap object?
    writeBMP(root / HEIGHTMAP_FILENAME,
             getMapData()->getHeightMap().lock().get(),
             getMapData()->getWidth(), getMapData()->getHeight(),
             1 /* depth */,
             true /* is_greyscale */);

    {
        // Normal map is a 24-bit bitmap
        auto normal_data_size = getMapData()->getWidth() *
                                getMapData()->getWidth() * 3;
        std::unique_ptr<unsigned char[]> normal_data(new unsigned char[normal_data_size]);

        // TODO: Do we need to do any error checking?
        generateWorldNormalMap(m_heightmap_bmp.get(), normal_data.get());

        // TODO: Also do error-checking here as well :)
        writeBMP(root / NORMALMAP_FILENAME, normal_data.get(),
                 getMapData()->getWidth(), getMapData()->getHeight());
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

auto HMDT::Project::HeightMapProject::loadFile(const std::filesystem::path& path)
    -> MaybeVoid
{
    m_heightmap_bmp.reset(readBMP(path));

    if(m_heightmap_bmp == nullptr) {
        WRITE_ERROR("Failed to open bitmap ", path);
        RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
    }

    WRITE_DEBUG(*m_heightmap_bmp);

    // Load heightmap data into MapData
    // This operation is fairly simple, as we are not making any modifications
    //   to the data itself, and just loading it into memory
    std::memcpy(getMapData()->getHeightMap().lock().get(),
                m_heightmap_bmp->data,
                getMapData()->getHeightMapSize());

    return STATUS_SUCCESS;
}

