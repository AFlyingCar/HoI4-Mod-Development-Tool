
#include "ProvinceProject.h"

#include <fstream>
#include <cerrno>
#include <cstring>

#include "Constants.h"
#include "MapData.h"
#include "Util.h"
#include "StatusCodes.h"
#include "Options.h"
#include "BitMap.h"

#include "ShapeFinder2.h"

#include "Logger.h"

#include "HoI4Project.h"

#include "ProjectNode.h"
#include "ProvinceNode.h"
#include "NodeKeyNames.h"

HMDT::Project::ProvinceProject::ProvinceProject(IRootMapProject& parent_project):
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
    if(getRootParent().getToolVersion() <= "0.25.0"_V) {
        WRITE_WARN("Tool version mismatch. Attempting to load province data "
                   "from version ", getRootParent().getToolVersion());

        // Make sure we load in the 0 -> EMPTY_UUID first, as that will never be
        //  loaded from the province data file
        m_oldid_to_uuid[0] = EMPTY_UUID;

        auto provdata_result = loadProvinceData(path);
        if(provdata_result.error() == std::errc::no_such_file_or_directory) {
            provdata_result = STATUS_SUCCESS;
        }
        RETURN_IF_ERROR(provdata_result);

        WRITE_DEBUG("oldid_to_uuid = {", joinMap(m_oldid_to_uuid.begin(),
                                                 m_oldid_to_uuid.end(),
                                                 ", "), "}");

        auto shapelabels_result = loadShapeLabels(path);
        RETURN_IF_ERROR(shapelabels_result);
    } else {
        auto provdata_result = loadProvinceData2(path);
        if(provdata_result.error() == std::errc::no_such_file_or_directory) {
            provdata_result = STATUS_SUCCESS;
        }
        RETURN_IF_ERROR(provdata_result);

        auto shapelabels_result = loadShapeLabels2(path);
        RETURN_IF_ERROR(shapelabels_result);
    }

    // Note that order is important here, graphics data _must_ be built before
    //   the outlines
    buildGraphicsData();
    buildProvinceOutlines();

    // Rebuild the uuid->id map last
    rebuildUUIDToIDMap();

    return STATUS_SUCCESS;
}

auto HMDT::Project::ProvinceProject::export_(const std::filesystem::path& root) const noexcept
    -> MaybeVoid
{
    // First create the export path if it doesn't exist
    if(std::error_code fs_ec; !std::filesystem::exists(root, fs_ec)) {
        RETURN_ERROR_IF(fs_ec.value() != 0 &&
                        fs_ec != std::errc::no_such_file_or_directory,
                        fs_ec);

        auto result = std::filesystem::create_directory(root, fs_ec);

        RETURN_ERROR_IF(!result, fs_ec);
    }

    MaybeVoid result;

    // Next, export the provinces.bmp file.
    {
        auto exportable_colors = getProvinceColorsForExport();
        RETURN_ERROR_IF(exportable_colors == nullptr, STATUS_UNEXPECTED);

        // TODO: Should we also specify a BMP header version? Default=V4
        auto result = writeBMP2(
                root / PROVINCES_FILENAME,
                exportable_colors.get(),
                getMapData()->getWidth(),
                getMapData()->getHeight()
            );
        RETURN_IF_ERROR(result);
    }

    // Next, export the definition.csv file.
    result = saveProvinceData(root, true);
    RETURN_IF_ERROR(result);

    // Next, export supply_nodes.txt and railways.txt
    {
        if(std::ofstream supply_nodes(root / "supply_nodes.txt"); supply_nodes)
        {
            // Level ProvinceID
            // for(auto&& province : m_provinces) {
                // TODO
                // NOTE: Level is defined as 1 by default. This is only changed
                //   in common/buildings/00_buildings.txt, so we will need to
                //   limit the max to whatever is defined in there (either the
                //   vanilla version or an overridden version defined in this
                //   mod)
            // }
        } else {
            WRITE_ERROR("Failed to open file ", root / "supply_nodes.txt");
            RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
        }

        if(std::ofstream railways(root / "railways.txt"); railways) {
            // TODO
        } else {
            WRITE_ERROR("Failed to open file ", root / "railways.txt");
            RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
        }
    }

    return STATUS_SUCCESS;
}

void HMDT::Project::ProvinceProject::import(const ShapeFinder& sf, std::shared_ptr<MapData>)
{
    m_provinces = createProvincesFromShapeList(sf.getShapes());

    // Clear out the province preview data
    m_data_cache.clear();

    buildProvinceOutlines();

    // Rebuild the uuid->id map last
    rebuildUUIDToIDMap();
}

bool HMDT::Project::ProvinceProject::validateData() {
    // We have nothing to really validate here
    return true;
}

HMDT::Project::IRootProject& HMDT::Project::ProvinceProject::getRootParent() {
    return m_parent_project.getRootParent();
}

const HMDT::Project::IRootProject& HMDT::Project::ProvinceProject::getRootParent() const
{
    return m_parent_project.getRootParent();
}

HMDT::Project::IRootMapProject& HMDT::Project::ProvinceProject::getRootMapParent()
{
    return m_parent_project.getRootMapParent();
}

const HMDT::Project::IRootMapProject& HMDT::Project::ProvinceProject::getRootMapParent() const
{
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

        auto num_bytes = getMapData()->getProvincesSize() * sizeof(UUID);

        // Write the entire label matrix to the file
        WRITE_DEBUG("Writing province ID data [", getMapData()->getWidth(),
                    " by ", getMapData()->getHeight(), ": ", num_bytes,
                    " bytes.");
        out.write(reinterpret_cast<const char*>(getMapData()->getProvinces().lock().get()),
                  num_bytes);
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
 * @param is_export Whether or not to include extra (i.e: non-HoI4) data, or
 *                  only data that is used by HoI4
 *
 * @return True if the file was able to be successfully written, false otherwise.
 */
auto HMDT::Project::ProvinceProject::saveProvinceData(const std::filesystem::path& root,
                                                      bool is_export) const noexcept
    -> MaybeVoid
{
    auto path = root / PROVINCEDATA_FILENAME;

    if(std::ofstream out(path); out) {
        const auto& continents = getRootMapParent().getContinentProject().getContinentList();

        bool assume_unknown_continents = false;

        // Write one line to the CSV for each province
        for(auto&& [id, province] : m_provinces) {
            // If we are exporting, then we need to output a numeric ID number,
            //   not the internal UUID we use
            if(is_export) {
                // For provinces that have been merged with another, skip
                //   actually writing them when exporting because we want to
                //   only export their parent's information
                if(province.parent_id != INVALID_PROVINCE) {
                    continue;
                }

                // Sanity check
                RETURN_ERROR_IF(m_uuid_to_oldid.count(id) == 0,
                                STATUS_VALUE_NOT_FOUND);

                out << getIDForProvinceID(id) << ';';
            } else {
                out << province.id << ';';
            }

            out << static_cast<int>(province.unique_color.r) << ';'
                << static_cast<int>(province.unique_color.g) << ';'
                << static_cast<int>(province.unique_color.b) << ';'
                << province.type << ';'
                << (province.coastal ? "true" : "false")
                << ';' << province.terrain << ';';

            if(!is_export) {
                out << province.continent << ';'
                    << province.bounding_box.bottom_left.x << ';'
                    << province.bounding_box.bottom_left.y << ';'
                    << province.bounding_box.top_right.x << ';'
                    << province.bounding_box.top_right.y << ';'
                    << province.state << ';'
                    << province.parent_id;
            } else {
                auto index = getIndexInSet(continents, province.continent);
                if(IS_FAILURE(index)) {
                    // Make sure we don't prompt the user for every single issue
                    if(!assume_unknown_continents) {
                        WRITE_WARN("Unknown continent '", province.continent,
                                   "' detected for province ID=", province.id);

                        std::stringstream ss;
                        ss << "An unknown continent '" << province.continent
                           << "' was detected for province ID=" << province.id
                           << ".\nContinuing will assume all unknown "
                              "continents are blank/0.";
                        auto result = prompt(ss.str(),
                                             {"Continue", "Stop Exporting"},
                                             PromptType::ERROR);

                        if(IS_FAILURE(result) || *result == 1) {
                            RETURN_IF_ERROR(index);
                        } else {
                            assume_unknown_continents = true;
                        }
                    }

                    index = 0;
                } else {
                    // Continents are 1 based, so convert the index to the ID
                    ++(*index);
                }

                out << *index;
            }

            out << std::endl;
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
        auto prov_matrix = getMapData()->getProvinces().lock();

        if(!safeRead(label_matrix.get(), getMapData()->getMatrixSize() * sizeof(uint32_t), in))
        {
            WRITE_ERROR("Failed to read full label matrix.");
            RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
        }

        // Generate the Provinces matrix
        bool err = false;
        parallelTransform(label_matrix.get() /* first */,
                          label_matrix.get() + (width * height) /* last */,
                          prov_matrix.get() /* dest */,
                          [this, &err](uint32_t& oldid) -> ProvinceID
                          {
                              if(m_oldid_to_uuid.count(oldid) == 0) {
                                  WRITE_ERROR("Failed to find ", oldid, " in map.");
                                  err = true;
                              }

                              const auto& newid = m_oldid_to_uuid.at(oldid);

                              // Convert the old ID to a hash to be the new "label"
                              oldid = newid.hash();

                              return newid;
                          });
        RETURN_ERROR_IF(err, STATUS_VALUE_NOT_FOUND);

        if(prog_opts.debug) {
            auto path = getRootParent().getDebugRoot();
            auto lmfname = path / "label_matrix.raw";
            auto pmfname = path / "prov_matrix.raw";

            if(!std::filesystem::exists(path)) {
                std::filesystem::create_directory(path);
            }

            WRITE_DEBUG("Writing label matrix (", getMapData()->getMatrixSize(),
                        " bytes) to ", lmfname);

            if(std::ofstream out(lmfname, std::ios::binary | std::ios::out); out)
            {
                out.write(reinterpret_cast<char*>(label_matrix.get()),
                          getMapData()->getMatrixSize());
            }

            WRITE_DEBUG("Writing province matrix (", getMapData()->getProvincesSize(),
                        " bytes) to ", pmfname);

            if(std::ofstream out(pmfname, std::ios::binary | std::ios::out); out)
            {
                out.write(reinterpret_cast<char*>(prov_matrix.get()),
                          getMapData()->getProvincesSize());
            }
        }
    } else {
        WRITE_ERROR("Failed to open file ", path);
        RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
    }

    return STATUS_SUCCESS;
}

auto HMDT::Project::ProvinceProject::loadShapeLabels2(const std::filesystem::path& root)
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
        if(auto input_size = width * height * sizeof(UUID);
                input_size != getMapData()->getProvincesSize() * sizeof(UUID))
        {
            WRITE_ERROR("Loaded shape data size (", input_size, ") does not "
                        "match expected matrix size of (",
                        getMapData()->getProvincesSize() * sizeof(UUID),
                        ")");
            RETURN_ERROR(std::make_error_code(std::errc::invalid_argument));
        }

        auto prov_matrix = getMapData()->getProvinces().lock();
        auto label_matrix = getMapData()->getLabelMatrix().lock();

        WRITE_DEBUG("Reading provinces into prov_matrix! sizeof(HMDT::UUID)=",
                    sizeof(HMDT::UUID));

        if(!safeRead(prov_matrix.get(),
                     getMapData()->getProvincesSize() * sizeof(UUID), in))
        {
            WRITE_ERROR("Failed to read full provinces matrix.");
            RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
        }

        std::atomic<uint32_t> num_trans = 0;

        // Now build the label matrix as well
        parallelTransform(prov_matrix.get() /* first */,
                          prov_matrix.get() + (width * height) /* last */,
                          label_matrix.get() /* dest */,
                          [&num_trans](const ProvinceID& prov_id) -> uint32_t {
                              ++num_trans;
                              return prov_id.hash();
                          });

        // Double check that we actually processed all of the UUIDs like we were
        //   supposed to
        RETURN_ERROR_IF(num_trans != width * height, STATUS_UNEXPECTED);

        if(prog_opts.debug) {
            auto path = getRootParent().getDebugRoot();
            auto lmfname = path / "label_matrix.raw";
            auto pmfname = path / "prov_matrix.raw";

            if(!std::filesystem::exists(path)) {
                std::filesystem::create_directory(path);
            }

            WRITE_DEBUG("Writing label matrix (", getMapData()->getMatrixSize(),
                        " bytes) to ", lmfname);

            if(std::ofstream out(lmfname, std::ios::binary | std::ios::out); out)
            {
                out.write(reinterpret_cast<char*>(label_matrix.get()),
                          getMapData()->getMatrixSize());
            }

            WRITE_DEBUG("Writing province matrix (", getMapData()->getProvincesSize(),
                        " bytes) to ", pmfname);

            if(std::ofstream out(pmfname, std::ios::binary | std::ios::out); out)
            {
                out.write(reinterpret_cast<char*>(prov_matrix.get()),
                          getMapData()->getProvincesSize());
            }
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

            WRITE_DEBUG("Parsing CSV line ", line);

            std::stringstream ss(line);

            Province prov;

            prov.parent_id = INVALID_PROVINCE;

            // Attempt to parse the entire CSV line, we expect it to look like:
            //  ID;R;G;B;ProvinceType;IsCoastal;TerrainType;ContinentID;BB.BottomLeft.X;BB.BottomLeft.Y;BB.TopRight.X;BB.TopRight.Y;StateID
            uint32_t id;
            if(!parseValuesSkipMissing<';'>(ss, &id,
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

            if(m_oldid_to_uuid.count(id) == 0) {
                // Create the new UUID for the old ID and give it to the
                //   province to hold onto
                prov.id = m_oldid_to_uuid[id];

                WRITE_DEBUG("Mapping old province ID ", id, " to ", prov.id);
            }

            m_provinces[prov.id] = prov;
        }

        WRITE_DEBUG("Loaded information for ",
                   m_provinces.size(), " provinces");
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
auto HMDT::Project::ProvinceProject::loadProvinceData2(const std::filesystem::path& root)
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

            prov.parent_id = INVALID_PROVINCE;

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
                                                &prov.state, true,
                                                &prov.parent_id, true))
            {
                WRITE_ERROR("Failed to parse line #", line_num, ": '", line, "'");
                RETURN_ERROR(std::make_error_code(std::errc::bad_message));
            }

            // Sanity check
            if(m_provinces.count(prov.id) != 0) {
                WRITE_WARN("Province with id ", prov.id, " already exists! Are "
                           "there two provinces listed in ", path, " which "
                           "share an ID?");
            }

            // Add the province into the vector
            m_provinces[prov.id] = prov;
        }

        // Post load processing
        // Take care of any additional linking that needs to be done after all
        //  Province objects exist
        for(auto&& [prov_id, prov] : m_provinces) {
            // Make sure that each province is added to its own parent's list of
            //   children
            if(isValidProvinceID(prov.parent_id)) {
                m_provinces[prov.parent_id].children.insert(prov_id);
            }
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

/**
 * @brief Will build the province preview for the given province. If more than
 *        MAX_CACHED_PROVINCE_PREVIEWS are already stored, then the least
 *        accessed preview will be kicked out of the cache before a new preview
 *        is constructed.
 *
 * @param province_ptr
 */
void HMDT::Project::ProvinceProject::buildProvinceCache(const Province* province_ptr)
{
    const auto& province = *province_ptr;
    auto id = province.id;

    // If there are too many cached provinces, then remove the least accessed
    //  one (which should be the first one in the cache)
    if(m_data_cache.size() > MAX_CACHED_PROVINCE_PREVIEWS) {
        // Do not clear out the first value if it is the one we are trying to
        //  create.
        if(m_data_cache.begin()->first == id) {
            m_data_cache.erase(std::next(m_data_cache.begin()));
        } else {
            m_data_cache.erase(m_data_cache.begin());
        }
    }

    auto& data = m_data_cache[id];

    WRITE_DEBUG("No preview data for province ", id, ". Building...");

    // Some references first, to make the following code easier to read
    //  id also starts at 1, so make sure we offset it down
    auto label_matrix = getMapData()->getProvinces().lock();
    auto iwidth = getMapData()->getWidth();

    auto&& bb = province.bounding_box;
    auto&& [width, height] = calcDims(bb);

    constexpr auto depth = 4;

    // Make sure we 0-initialize the array
    //  We use a depth of 4 since we have RGBA
    data.reset(new unsigned char[width * height * depth]());

    WRITE_DEBUG("Allocated space for ", width * height * depth, " bytes.");
    for(auto x = bb.bottom_left.x; x < bb.top_right.x; ++x) {
        for(auto y = bb.top_right.y; y < bb.bottom_left.y; ++y) {
            // Get the index into the label matrix
            auto lindex = xyToIndex(iwidth, x, y);

            // Offset the x,y so that we get 0-preview width/height
            auto relx = x - bb.bottom_left.x;
            auto rely = y - bb.top_right.y;

            // Index into the cached data
            //  Need a special one since the depth is different from the
            //  stored graphics data
            auto dindex = xyToIndex(width * depth, relx * depth, rely);

            auto label = label_matrix[lindex];

            if(label == id) {
                // ARGB
                *reinterpret_cast<uint32_t*>(&data[dindex]) = PROVINCE_HIGHLIGHT_COLOR;
            }
        }
    }

    WRITE_DEBUG("Done.");

    if(prog_opts.debug) {
        auto path = getRootParent().getDebugRoot();
        auto fname = path / (std::string("prov_preview") + std::to_string(id) + ".pam");

        if(!std::filesystem::exists(path)) {
            std::filesystem::create_directory(path);
        }

        WRITE_DEBUG("Writing province ", width, 'x', height, " (", id, ") to ", fname);

        if(std::ofstream out(fname); out) {
            out << "P7\nWIDTH " << width << "\nHEIGHT " << height << "\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
            for(auto i = 0; i < width * height * 4; i += 4) {
                // Write Alpha first
                out.write(reinterpret_cast<char*>(&data[i + 3]), 1);
                out.write(reinterpret_cast<char*>(&data[i + 1]), 3);
            }
        }
    }
}

void HMDT::Project::ProvinceProject::buildProvinceOutlines() {
    auto prov_outline_data = getMapData()->getProvinceOutlines().lock();
    auto graphics_data = getMapData()->getProvinceColors().lock();

    auto [width, height] = getMapData()->getDimensions();
    Dimensions dimensions{width, height};

    bool failure = false;

    // Go over the map again and build extra data that depends on the previously
    //  re-built province data
    for(uint32_t x = 0; x < width; ++x) {
        for(uint32_t y = 0; y < height; ++y) {
            auto lindex = xyToIndex(width, x, y);
            auto label = getMapData()->getProvinces().lock()[lindex];
            auto gindex = xyToIndex(width * 4, x * 4, y);

            if(!isValidProvinceID(label)) {
                WRITE_WARN("ProvinceID matrix has label ", label,
                           " at position (", x, ',', y, "), which was not "
                           "found in the list of loaded provinces.");

                if(!failure) {
                    failure = true;

                    WRITE_DEBUG("m_provinces=", [this]() {
                        std::stringstream ss;

                        for(auto it = m_provinces.begin(); it != m_provinces.end(); ++it) {
                            if(it != m_provinces.begin()) ss << ", ";
                            ss << it->first;
                        }

                        return ss.str();
                    }().c_str());
                }

                continue;
            }

            // Look up the province itself
            auto& province = getProvinceForID(label);

            // Recalculate adjacencies for this pixel
            auto is_adjacent = ShapeFinder::calculateAdjacency(dimensions,
                                                               graphics_data.get(),
                                                               getMapData()->getProvinces().lock().get(),
                                                               province.adjacent_provinces,
                                                               {x, y});
            // If this pixel is adjacent to any others, then make it visible as
            //  an outline
            if(is_adjacent) {
                prov_outline_data[gindex] = 0xFF;
                prov_outline_data[gindex + 1] = 0xFF;
                prov_outline_data[gindex + 2] = 0xFF;
                prov_outline_data[gindex + 3] = 0xFF;
            }
        }
    }
}

/**
 * @brief Builds the graphics data array
 */
void HMDT::Project::ProvinceProject::buildGraphicsData() {
    // Rebuild the graphics data
    auto [width, height] = getMapData()->getDimensions();

    auto prov_matrix = getMapData()->getProvinces().lock();

    auto graphics_data = getMapData()->getProvinceColors().lock();

    // Rebuild the map_data array and the adjacency lists
    for(uint32_t x = 0; x < width; ++x) {
        for(uint32_t y = 0; y < height; ++y) {
            // Get the index into the prov matrix
            auto lindex = xyToIndex(width, x, y);

            // Get the index into the graphics data
            //  3 == the depth
            auto gindex = xyToIndex(width * 3, x * 3, y);

            auto id = prov_matrix[lindex];

            // Error check
            if(!isValidProvinceID(id)) {
                WRITE_WARN("Province matrix has ID ", id,
                           " at position (", x, ',', y, "), which does not exist.");
                continue;
            }

            // Rebuild color data
            auto& province = getProvinceForID(id);

            // Flip the colors from RGB to BGR because BitMap is a bad format
            graphics_data[gindex] = province.unique_color.b;
            graphics_data[gindex + 1] = province.unique_color.g;
            graphics_data[gindex + 2] = province.unique_color.r;
        }
    }
}

/**
 * @brief Gets the province colors in a form that's ready to be exported.
 *
 * @return A new flat array containing all pixel colors, or nullptr if a failure
 *         occurs
 */
auto HMDT::Project::ProvinceProject::getProvinceColorsForExport() const noexcept
    -> std::unique_ptr<unsigned char[]>
{
    auto province_colors = getMapData()->getProvinceColors().lock();
    auto prov_matrix = getMapData()->getProvinces().lock();
    auto [width, height] = getMapData()->getDimensions();

    std::unique_ptr<unsigned char[]> exportable_colors(new unsigned char[getMapData()->getProvinceColorsSize()]);

    // TODO: Can we parallelize this?
    for(uint32_t x = 0; x < width; ++x) {
        for(uint32_t y = 0; y < height; ++y) {
            // Get the index into the prov matrix
            auto lindex = xyToIndex(width, x, y);

            // Get the index into the graphics data
            //  3 == the depth
            auto gindex = xyToIndex(width * 3, x * 3, y);

            auto id = prov_matrix[lindex];

            // Error check
            if(!isValidProvinceID(id)) {
                WRITE_WARN("Province matrix has ID ", id,
                           " at position (", x, ',', y, "), which does not exist.");
                continue;
            }

            // Rebuild color data
            auto maybe_root = getRootProvinceParent(id);
            // TODO: We should really figure out how to return the error code up
            //   from here. The reason we can't is because we cannot build a
            //   Maybe<unique_ptr>
            RETURN_VALUE_IF_ERROR(maybe_root, nullptr);

            exportable_colors[gindex] = maybe_root->get().unique_color.r;
            exportable_colors[gindex + 1] = maybe_root->get().unique_color.g;
            exportable_colors[gindex + 2] = maybe_root->get().unique_color.b;
        }
    }

    return exportable_colors;
}

/**
 * @brief Gets province preview data for the given ID
 *
 * @param id
 *
 * @return The preview data, or nullptr if the ID does not exist
 */
auto HMDT::Project::ProvinceProject::getPreviewData(ProvinceID id)
    -> ProvinceDataPtr
{
    if(isValidProvinceID(id)) {
        return getPreviewData(&getProvinceForID(id));
    }

    return nullptr;
}

/**
 * @brief Gets the preview data for the given province. If no data currently
 *        exists, construct it and cache it.
 *
 * @param province_ptr
 *
 * @return The preview data. This method should never return nullptr
 */
auto HMDT::Project::ProvinceProject::getPreviewData(const Province* province_ptr)
    -> ProvinceDataPtr
{
    const auto& province = *province_ptr;
    auto id = province.id;

    auto data = m_data_cache[id];

    // If there is no cached data for the given province ID, then generate the
    //  data for the preview
    if(data == nullptr) {
        buildProvinceCache(province_ptr);
    }

    // Reset access time, cycles the element to the end of the FIFO queue
    data = m_data_cache[id];
    m_data_cache.erase(id);
    m_data_cache[id] = data;

    return data;
}

auto HMDT::Project::ProvinceProject::getOldIDToUUIDMap() const noexcept
    -> const std::unordered_map<uint32_t, UUID>&
{
    return m_oldid_to_uuid;
}

uint32_t HMDT::Project::ProvinceProject::getIDForProvinceID(const ProvinceID& id) const noexcept
{
    return m_uuid_to_oldid.at(id);
}

/**
 * @brief Rebuilds the mapping of UUID->ID.
 * @details This should be called every time m_provinces changes/is updated.
 */
void HMDT::Project::ProvinceProject::rebuildUUIDToIDMap() noexcept {
    // Clear the old map out first
    m_uuid_to_oldid.clear();

    uint32_t i = 0;
    for(auto&& [id, _] : m_provinces) {
        m_uuid_to_oldid[id] = ++i;
    }
}

/**
 * @brief Builds the project hierarchy tree for ProvinceProject
 *
 * @param visitor The visitor callback
 *
 * @return The root node for ProvinceProject
 */
auto HMDT::Project::ProvinceProject::visit(const std::function<MaybeVoid(std::shared_ptr<Hierarchy::INode>)>& visitor) const noexcept
    -> Maybe<std::shared_ptr<Hierarchy::INode>>
{
    auto province_project_node = std::make_shared<Hierarchy::ProjectNode>(Hierarchy::ProjectKeys::PROVINCES);

    auto result = visitor(province_project_node);
    RETURN_IF_ERROR(result);

    result = visitProvinces(visitor)
        .andThen([&province_project_node](auto provinces_group_node) -> MaybeVoid {
            auto result = province_project_node->addChild(provinces_group_node);
            RETURN_IF_ERROR(result);

            return STATUS_SUCCESS;
        });
    RETURN_IF_ERROR(result);

    return province_project_node;
}

/**
 * @brief Builds the group node for holding all provinces
 *
 * @param visitor The visitor callback
 *
 * @return A GroupNode that holds all ProvinceNodes for this project
 */
auto HMDT::Project::ProvinceProject::visitProvinces(const std::function<MaybeVoid(std::shared_ptr<Hierarchy::INode>)>& visitor) const noexcept
    -> Maybe<std::shared_ptr<Hierarchy::IGroupNode>>
{
    // This should be a static group since the number of provinces will not
    //   change unless a new province map is loaded
    auto provinces_group_node = std::make_shared<Hierarchy::GroupNode>(Hierarchy::GroupKeys::PROVINCES);

    auto result = visitor(provinces_group_node);
    RETURN_IF_ERROR(result);

    for(auto&& [id, province] : m_provinces) {
        auto province_id = id;
        auto name = std::to_string(province.id);

        auto province_node = std::make_shared<Hierarchy::ProvinceNode>(name);

        result = visitor(province_node);
        RETURN_IF_ERROR(result);

        result = province_node->setID([_this=const_cast<ProvinceProject*>(this),
                                       province_id]() -> auto&
            {
                return _this->m_provinces[province_id].id;
            }, visitor);
        RETURN_IF_ERROR(result);

        result = province_node->setColor([_this=const_cast<ProvinceProject*>(this),
                                          province_id]() -> const auto&
            {
                return _this->m_provinces[province_id].unique_color;
            }, visitor);
        RETURN_IF_ERROR(result);

        result = province_node->setProvinceType([_this=const_cast<ProvinceProject*>(this),
                                                 province_id]() -> auto& {
                return _this->m_provinces[province_id].type;
            }, visitor);
        RETURN_IF_ERROR(result);

        result = province_node->setCoastal([_this=const_cast<ProvinceProject*>(this),
                                            province_id]() -> auto&
            {
                return _this->m_provinces[province_id].coastal;
            }, visitor);
        RETURN_IF_ERROR(result);

        result = province_node->setTerrain([_this=const_cast<ProvinceProject*>(this),
                                            province_id]() -> auto&
            {
                return _this->m_provinces[province_id].terrain;
            }, visitor);
        RETURN_IF_ERROR(result);

        result = province_node->setContinent([_this=const_cast<ProvinceProject*>(this),
                                              province_id]() -> auto&
            {
                return _this->m_provinces[province_id].continent;
            }, visitor);
        RETURN_IF_ERROR(result);

        // TODO: States still use uint32_t for ID numbering. This needs to be
        //   changed over to UUID before we can safely implement province->state
        //   linkage.
#if 0
        result = province_node->setState(province.state, visitor);
        RETURN_IF_ERROR(result);

        result = province_node->setAdjacentProvinces(province.adjacent_provinces, visitor);
        RETURN_IF_ERROR(result);
#endif

        provinces_group_node->addChild(name, province_node);
    }

    return provinces_group_node;
}

