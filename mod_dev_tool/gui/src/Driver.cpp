
#include "Driver.h"

#include "Application.h"
#include "Util.h"
#include "Logger.h"

HMDT::GUI::Driver& HMDT::GUI::Driver::getInstance() {
    static Driver instance;

    return instance;
}

HMDT::GUI::Driver::Driver(): m_app(), m_project(nullptr)
{ }

HMDT::GUI::Driver::~Driver() {
}

bool HMDT::GUI::Driver::initialize() {
    WRITE_INFO("Driver initializing.");

    m_app = Glib::RefPtr<Application>(new Application());

    auto result = registerResourceFile(HMDT_GLIB_RESOURCES,
                                       getExecutablePath(),
                                       HMDT_RESOURCES);
    RETURN_VALUE_IF_ERROR(result, false);

    result = registerResourceFile(HMDT_GLIB_IONICONS_RESOURCES,
                                  getExecutablePath(),
                                  HMDT_IONICONS_RESOURCES);
    RETURN_VALUE_IF_ERROR(result, false);

    return true;
}

/**
 * @brief Registers a new resource pack
 *
 * @param res_name The filename/id of the resource pack
 * @param res_path The base path where the resource pack is located
 * @param res_prefix The prefix to use when looking up resources in the pack
 *
 * @return STATUS_SUCCESS on success, or an error code on failure.
 */
auto HMDT::GUI::Driver::registerResourceFile(const std::string& res_name,
                                             const std::filesystem::path& res_path,
                                             const std::string& res_prefix) noexcept
    -> MaybeVoid
{
    auto path = res_path / res_name;

    WRITE_INFO("Registering resource file ", res_name, " located at ",
               res_path, " with prefix ", res_prefix);

    try {
        WRITE_DEBUG("Loading resources from ", path, "...");
        m_resources[res_name] = {
            Gio::Resource::create_from_file(path.generic_string()),
            res_prefix
        };
        m_resources[res_name].resource->register_global(); // Make sure it can be accessed anywhere
        WRITE_DEBUG("Done.");
    } catch(const Glib::FileError& e) {
        WRITE_ERROR(e.what());
        RETURN_ERROR(std::make_error_code(std::errc::no_such_file_or_directory));
    } catch(const Gio::ResourceError& e) {
        WRITE_ERROR(e.what());
        RETURN_ERROR(STATUS_RESOURCE_LOAD_FAILURE);
    }

    return STATUS_SUCCESS;
}

void HMDT::GUI::Driver::run() {
    WRITE_INFO("Beginning application.");
    m_app->run();
}

/**
 * @brief Gets the main project object, if one has been loaded
 *
 * @return The main project object, or std::nullopt if one has not been loaded.
 */
auto HMDT::GUI::Driver::getProject() const -> OptionalReference<const HProject>
{
    if(m_project) {
        return *m_project;
    } else {
        return std::nullopt;
    }
}

/**
 * @brief Gets the main project object, if one has been loaded
 *
 * @return The main project object, or std::nullopt if one has not been loaded.
 */
auto HMDT::GUI::Driver::getProject() -> OptionalReference<HProject> {
    if(m_project) {
        return *m_project;
    } else {
        return std::nullopt;
    }
}

/**
 * @brief Sets the main project object.
 *
 * @param project 
 */
void HMDT::GUI::Driver::setProject(UniqueProject&& project) {
    m_project = std::move(project);
}

/**
 * @brief Unloads the main project object.
 */
void HMDT::GUI::Driver::setProject() {
    m_project = nullptr;
}

/**
 * @brief Gets a resource pack
 *
 * @param id The ID of the resource pack to get
 *
 * @return The project's resources
 */
auto HMDT::GUI::Driver::getResourcePack(const std::string& id) const noexcept
    -> Maybe<ResourcePack>
{
    if(m_resources.count(id) == 0) {
        RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
    } else {
        return m_resources.at(id);
    }
}


/**
 * @brief Gets a resource as an icon
 *
 * @param icon_name The name of the icon
 * @param width The expected width of the icon. If the value is -1, then the
 *              resource width is not scaled
 * @param height The expected height of the icon. If the value is -1, then the
 *              resource width is not scaled
 *
 * @return The requested resource as an icon, scaled to widthxheight
 */
Glib::RefPtr<Gdk::Pixbuf> HMDT::GUI::Driver::getIcon(const std::string& icon_name,
                                                     std::int32_t width,
                                                     std::int32_t height) noexcept
{
    auto icon_theme = Gtk::IconTheme::get_for_screen(m_app->getMainWindow()->get_screen());

    try {
        Glib::RefPtr<Gdk::Pixbuf> pixbuf = icon_theme->load_icon(
                icon_name,
                width,
                Gtk::ICON_LOOKUP_GENERIC_FALLBACK);

        return pixbuf;
    } catch(const Gtk::IconThemeError& ex) {
        WRITE_ERROR("Failed to load icon ", icon_name, ": ", ex.what());

        return Gdk::Pixbuf::create_from_resource(
                "/com/aflyingcar/HoI4ModDevelopmentTool/textures/missing.png",
                width, height, true /* preserve_aspect_ratio */
            );
    }
}

/**
 * @brief Gets a resource as a raw input stream
 *
 * @param id The ID of the resource pack to get the resource from
 * @param res_name The path to the resource in the resource pack
 *
 * @return The requested resource as a raw input stream, or an error code if
 *         lookup of the resource fails
 */
auto HMDT::GUI::Driver::getResourceStream(const std::string& id,
                                          const std::string& res_name) const noexcept
    -> Maybe<Glib::RefPtr<Gio::InputStream>>
{
    auto pack = getResourcePack(id);
    RETURN_IF_ERROR(pack);

    try {
        WRITE_DEBUG("OpenStream(", pack->prefix + "/" + res_name, ')');
        return pack->resource->open_stream(pack->prefix + "/" + res_name);
    } catch(...) {
        RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
    }
}

/**
 * @brief Gets a resource as a pixbuf
 *
 * @param id The ID of the resource pack to get the resource from
 * @param res_name The path to the resource in the resource pack
 * @param width The expected width of the pixbuf. If the value is -1, then the
 *              resource width is not scaled
 * @param height The expected height of the pixbuf. If the value is -1, then the
 *               resource height is not scaled
 * @param preserve_aspect_ratio Whether the aspect ratio should be preserved
 *                              when scaling
 *
 * @return The requested resource as a pixbuf scaled to widthxheight, or an
 *         error code if lookup of the resource fails
 */
auto HMDT::GUI::Driver::getResourcePixbuf(const std::string& id,
                                          const std::string& res_name,
                                          int width, int height,
                                          bool preserve_aspect_ratio) noexcept
    -> Maybe<Glib::RefPtr<Gdk::Pixbuf>>
{
    auto pack = getResourcePack(id);
    RETURN_IF_ERROR(pack);

    auto res_path = pack->prefix + "/" + res_name;

    if(auto it = m_resource_pixbuf_cache.find(res_path, width, height,
                                              preserve_aspect_ratio);
            it != m_resource_pixbuf_cache.end())
    {
        Glib::RefPtr<Gdk::Pixbuf> pb = it->second;
        return pb;
    }
    else {
        try {
            WRITE_DEBUG("CreateFromResource(", res_path, ')');
            auto pb = Gdk::Pixbuf::create_from_resource(res_path,
                                                        width, height,
                                                        preserve_aspect_ratio);

            auto res = m_resource_pixbuf_cache.insert({res_path, width, height,
                                                       preserve_aspect_ratio},
                                                      pb);
            if(!res) {
                WRITE_ERROR("Failed to insert resource ", res_path,
                            " into cache.");
            }

            return pb;
        } catch(...) {
            RETURN_ERROR(STATUS_VALUE_NOT_FOUND);
        }
    }
}

/**
 * @brief Gets a Pixbuf to represent an error texture
 *
 * @return A Pixbuf to be used in the event that resource acquisition fails.
 */
auto HMDT::GUI::Driver::getFailurePixbuf() const noexcept -> Pixbuf {
    // TODO: We should use an actual (repeatable) texture, instead of a 1x1
    static unsigned char _error_buffer[3] = { 0 };

    return Gdk::Pixbuf::create_from_data(
            _error_buffer,
            Gdk::COLORSPACE_RGB,
            false /* has_alpha */,
            8 /* bits_per_sample */,
            1 /* width */, 1 /* height */,
            0 /* rowstride */
        );
}

