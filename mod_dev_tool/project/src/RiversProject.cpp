
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

#include "ProjectNode.h"
#include "PropertyNode.h"

HMDT::Project::RiversProject::RiversProject(IRootMapProject& parent):
    m_parent_project(parent),
    m_rivers_bmp(nullptr)
{ }

/**
 * @brief Writes all continent data to root/$RIVERS_FILENAME
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

    auto path = root / RIVERS_FILENAME;

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
    auto path = root / RIVERS_FILENAME;

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

        res = response.andThen<std::monostate>([this, &res, &root](const uint32_t& r)
            -> MaybeVoid
        {
            switch(r) {
                case 0:
                    res = writeTemplate(root / RIVERS_FILENAME);
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
        WRITE_ERROR("Rivers dimensions (",
                    m_rivers_bmp->info_header.v1.width, ", ",
                    m_rivers_bmp->info_header.v1.height, ") do not match the"
                    " previously loaded dimensions (", d.first, ", ", d.second,
                    ")");
        RETURN_ERROR(STATUS_DIMENSION_MISMATCH);
    }

    // Just in case the input image is not actually an 8-bit images
    uint8_t* rivers_data;
    if(auto bpp = m_rivers_bmp->info_header.v1.bitsPerPixel; bpp != 8) {
        WRITE_ERROR("Rivers must be 8-bit images, not ", bpp, ".");
        RETURN_ERROR(STATUS_INVALID_BIT_DEPTH);
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

auto HMDT::Project::RiversProject::writeTemplate(const std::filesystem::path& path) const noexcept
    -> MaybeVoid
{
    std::unique_ptr<unsigned char[]> rivers_data;

    auto res = generateTemplate(rivers_data);
    RETURN_IF_ERROR(res);

    res = writeBMP2(path,
                    rivers_data.get(),
                    getMapData()->getWidth(),
                    getMapData()->getHeight(),
                    1 /* depth */,
                    false /* is_greyscale */,
                    BMPHeaderToUse::V4 /* version */,
                    generateColorTable());
    RETURN_IF_ERROR(res);

    return STATUS_SUCCESS;
}

auto HMDT::Project::RiversProject::generateTemplate(std::unique_ptr<uint8_t[]>& data) const noexcept
    -> MaybeVoid
{
    try {
        data.reset(new uint8_t[getMapData()->getRiversSize()]);
    } catch(const std::bad_alloc& e) {
        WRITE_ERROR(e.what());
        RETURN_ERROR(STATUS_BADALLOC);
    }

    for(auto i = 0U; i < getMapData()->getRiversSize(); ++i) {
        auto province_label = getMapData()->getProvinces().lock()[i];

        if(!getRootMapParent().getProvinceProject().isValidProvinceID(province_label))
        {
            WRITE_ERROR("Province ID ", province_label, " at river index ", i,
                        " is not valid.");
            RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
        }

        auto prov_type = getRootMapParent().getProvinceProject().getProvinceForID(province_label).type;

        switch(prov_type) {
            case ProvinceType::LAND:
                data[i] = 12;
                break;
            case ProvinceType::LAKE:
            case ProvinceType::SEA:
                data[i] = 13;
                break;
            case ProvinceType::UNKNOWN:
                data[i] = 14;
                break;
        }
    }

    return STATUS_SUCCESS;
}

/**
 * @brief Generates a color table for rivers.
 * @details See https://hoi4.paradoxwikis.com/Map_modding#Rivers for a
 *          description on what the individual values mean.
 *          Note that we also add in a few extra colors which are ignored by the
 *          base game for comments
 *
 * @return A ColorTable object
 */
auto HMDT::Project::RiversProject::generateColorTable() noexcept
    -> ColorTable
{
    // NOTE: Color values will differ slightly from the above link, since those
    //   are listed in RGB format, but BitMap requires BGR
    return ColorTable {
        15 /* num_colors */,
        std::unique_ptr<RGBQuad[]>(new RGBQuad[15]{
            { { 0, 0xFF, 0, 0 } },    // 0 The source of a river
            { { 0, 0, 0xFF, 0 } },    // 1 Flow-in source. Used to join multiple 'source' paths into one river.
            { { 0xFF, 0xFC, 0, 0 } }, // 2 Flow-out source. Used to branch outwards from one river.
            { { 0xFF, 0xE1, 0, 0 } }, // 3 River with narrowest texture.
            { { 0xFF, 0xC8, 0, 0 } }, // 4
            { { 0xFF, 0x96, 0, 0 } }, // 5
            { { 0xFF, 0x64, 0, 0 } }, // 6 River with wide texture.
            { { 0xFF, 0, 0, 0 } },    // 7
            { { 0xE1, 0, 0, 0 } },    // 8
            { { 0xC8, 0, 0, 0 } },    // 9
            { { 0x96, 0, 0, 0 } },    // 10
            { { 0x64, 0, 0, 0 } },    // 11 River with widest texture.

            { { 0xFF, 0xFF, 0xFF, 0 } }, // LAND COMMENT
            { { 0x7A, 0x7A, 0x7A, 0 } }, // SEA + LAKE COMMENT
            { { 0, 0, 0, 0 } }, // UNKNOWN COMMENT
        }) /* color_table */
    };
}

auto HMDT::Project::RiversProject::visit(const std::function<MaybeVoid(std::shared_ptr<Hierarchy::INode>)>& visitor) const noexcept
    -> Maybe<std::shared_ptr<Hierarchy::INode>>
{
    auto rivers_project_node = std::make_shared<Hierarchy::ProjectNode>("Rivers");

    auto result = visitor(rivers_project_node);
    RETURN_IF_ERROR(result);

    // Heightmap only has a single property under it, the heightmap itself which
    //   cannot be easily manipulated as a primitive, so we simply expose a
    //   single constant string view
    // TODO: Should we instead actually not expose anything?
    auto rivers_node = std::make_shared<Hierarchy::ConstPropertyNode<std::string>>("Rivers");

    result = visitor(rivers_node);
    RETURN_IF_ERROR(result);

    rivers_project_node->addChild(rivers_node);

    return rivers_project_node;
}

