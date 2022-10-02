
#include "RiversProject.h"

#include <fstream>
#include <cstring>
#include <memory>

#include "Logger.h"

#include "Constants.h"
#include "StatusCodes.h"
#include "MapData.h"
#include "Util.h"

#include "WorldNormalBuilder.h"

HMDT::Project::RiversProject::RiversProject(IRootMapProject& parent):
    m_parent_project(parent),
    m_rivers_bmp(nullptr)
{ }

/**
 * @brief Writes all continent data to root/$HEIGHTMAP_FILENAME
 *
 * @param root The root where the rivers should go
 * @param ec The error code
 *
 * @return True if the rivers was successfully saved, false otherwise
 */
auto HMDT::Project::RiversProject::save(const std::filesystem::path& root)
    -> MaybeVoid
{
    if(m_rivers_bmp == nullptr) {
        WRITE_ERROR("No rivers has been loaded, cannot save yet.");
        RETURN_ERROR(STATUS_NO_DATA_LOADED);
    }

    auto path = root / HEIGHTMAP_FILENAME;

    // Write the rivers to a file
    auto res = writeBMP(path, m_rivers_bmp);
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
auto HMDT::Project::RiversProject::load(const std::filesystem::path& root)
    -> MaybeVoid
{
    auto path = root / HEIGHTMAP_FILENAME;

    // If the file doesn't exist, then return false (we didn't actually load it
    //  after all), but don't set the error code as it is expected that the
    //  file may not exist
    if(std::error_code ec; !std::filesystem::exists(path, ec)) {
        RETURN_ERROR_IF(ec.value() != 0, ec);

        WRITE_WARN("No data to load! No rivers currently exists!");
        RETURN_ERROR(std::make_error_code(std::errc::no_such_file_or_directory));
    }

    return loadFile(path);
}

auto HMDT::Project::RiversProject::export_(const std::filesystem::path& root) const noexcept
    -> MaybeVoid
{
    MaybeVoid res;

    if(m_rivers_bmp != nullptr) {
        res = writeBMP2(root / RIVERS_FILENAME,
                             getMapData()->getRivers().lock().get(),
                             getMapData()->getWidth(), getMapData()->getHeight(),
                             1 /* depth */,
                             true /* is_greyscale */);
        RETURN_IF_ERROR(res);
    } else {
        WRITE_WARN("No imported rivers file exists.");

        std::stringstream prompt_ss;
        prompt_ss << "No custom Rivers file was added. Generate a blank template?";

        auto response = prompt(prompt_ss.str(), {"Yes", "No"});

        std::unique_ptr<unsigned char[]> rivers_data;

        res = response.andThen<std::monostate>([this, &res, &rivers_data, &root](const uint32_t& r)
            -> MaybeVoid
        {
            switch(r) {
                case 0:
                    res = generateTemplate(rivers_data, getMapData());
                    RETURN_IF_ERROR(res);

                    res = writeBMP2(root / RIVERS_FILENAME,
                                    rivers_data.get(),
                                    getMapData()->getWidth(),
                                    getMapData()->getHeight(),
                                    1 /* depth */,
                                    false /* is_greyscale */,
                                    BMPHeaderToUse::V4 /* version */,
                                    generateColorTable());
                    RETURN_IF_ERROR(res);
                    break;
                case 1:
                    WRITE_ERROR("Not generating a blank template, cannot export rivers.");
                    RETURN_ERROR(STATUS_NO_DATA_LOADED);
                    break;
                default:
                    WRITE_ERROR("Unexpected response from prompt ", r);
                    RETURN_ERROR(STATUS_UNEXPECTED_RESPONSE);
            }

            return STATUS_SUCCESS;
        });
        RETURN_IF_ERROR(res);
    }

    return STATUS_SUCCESS;
}

auto HMDT::Project::RiversProject::getRootParent() -> IRootProject& {
    return m_parent_project.getRootParent();
}

auto HMDT::Project::RiversProject::getRootParent() const
    -> const IRootProject&
{
    return m_parent_project.getRootParent();
}

auto HMDT::Project::RiversProject::getMapData() -> std::shared_ptr<MapData> {
    return m_parent_project.getMapData();
}

auto HMDT::Project::RiversProject::getMapData() const 
    -> const std::shared_ptr<MapData>
{
    return m_parent_project.getMapData();
}

void HMDT::Project::RiversProject::import(const ShapeFinder&, std::shared_ptr<MapData>) { }

bool HMDT::Project::RiversProject::validateData() {
    // We have nothing to really validate here
    return true;
}

auto HMDT::Project::RiversProject::getRootMapParent() -> IRootMapProject& {
    return m_parent_project.getRootMapParent();
}

auto HMDT::Project::RiversProject::getRootMapParent() const
    -> const IRootMapProject&
{
    return m_parent_project.getRootMapParent();
}

auto HMDT::Project::RiversProject::loadFile(const std::filesystem::path& path) noexcept
    -> MaybeVoid
{
    try {
        m_rivers_bmp.reset(new BitMap2);
    } catch(const std::bad_alloc& e) {
        WRITE_ERROR("Failed to allocate space for new bitmap: ", e.what());
        RETURN_ERROR(STATUS_BADALLOC);
    }

    auto res = readBMP(path, m_rivers_bmp);
    RETURN_IF_ERROR(res);

    WRITE_DEBUG(*m_rivers_bmp);

    if(auto d = getMapData()->getDimensions();
            d.first != m_rivers_bmp->info_header.v1.width ||
            d.second != m_rivers_bmp->info_header.v1.height)
    {
        WRITE_ERROR("Heightmap dimensions (",
                    m_rivers_bmp->info_header.v1.width, ", ",
                    m_rivers_bmp->info_header.v1.height, ") do not match the"
                    " previously loaded dimensions (", d.first, ", ", d.second,
                    ")");
        RETURN_ERROR(STATUS_DIMENSION_MISMATCH);
    }

    // Just in case the input image is not actually an 8-bit images
    uint8_t* rivers_data;
    if(auto bpp = m_rivers_bmp->info_header.v1.bitsPerPixel; bpp != 8) {
        WRITE_WARN("Heightmaps must be 8-bit greyscale images, not ", bpp, ". "
                   "Checking if the user is okay with converting it.");

        std::stringstream prompt_ss;
        prompt_ss << "Heightmaps must be an 8-bit greyscale image (loaded "
                     "image has BPP=" << bpp << "). Convert to 8-bit image?";

        auto response = prompt(prompt_ss.str(), {"Yes", "No"});
        if(response.error() == STATUS_CALLBACK_NOT_REGISTERED) {
            WRITE_WARN("No prompt callback registered, not going to convert the"
                       " input just to be safe.");
            response = 1U; // Do not convert unless the user explicitly says
                           //  that's okay.
        }
        RETURN_IF_ERROR(response);

        switch(*response) {
            case 0:
                res = convertBitMapTo8BPPGreyscale(*m_rivers_bmp);
                RETURN_IF_ERROR(res);
                break;
            case 1:
                WRITE_ERROR("Not converting input image. Cannot continue loading.");
                RETURN_ERROR(STATUS_INVALID_BIT_DEPTH);
                break;
            default:
                WRITE_ERROR("Unexpected response from prompt ", *response);
                RETURN_ERROR(STATUS_UNEXPECTED_RESPONSE);
        }
    }

    rivers_data = m_rivers_bmp->data.get();

    // Load rivers data into MapData
    // This operation is fairly simple, as we are not making any modifications
    //   to the data itself, and just loading it into memory
    std::memcpy(getMapData()->getRivers().lock().get(),
                rivers_data,
                getMapData()->getRiversSize());

    return STATUS_SUCCESS;
}

auto HMDT::Project::RiversProject::getBitMap() const
    -> MonadOptionalRef<const BitMap2>
{
    if(m_rivers_bmp != nullptr) {
        return *m_rivers_bmp;
    } else {
        return std::nullopt;
    }
}

auto HMDT::Project::RiversProject::generateTemplate(std::unique_ptr<uint8_t[]>& data,
                                                    std::shared_ptr<const MapData> map_data) noexcept
    -> MaybeVoid
{
    try {
        data.reset(new uint8_t[map_data->getRiversSize()]);
    } catch(const std::bad_alloc& e) {
        WRITE_ERROR(e.what());
        RETURN_ERROR(STATUS_BADALLOC);
    }

    for(auto i = 0U; i < map_data->getRiversSize(); ++i) {
    }

    return STATUS_SUCCESS;
}

auto HMDT::Project::RiversProject::generateColorTable() noexcept
    -> ColorTable
{
    return ColorTable {
        0 /* num_colors */,
        nullptr /* color_table */
    };
}

