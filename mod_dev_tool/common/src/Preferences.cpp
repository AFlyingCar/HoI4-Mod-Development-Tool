#include "Preferences.h"

#include <fstream>

#include "nlohmann/json.hpp"

#include "Util.h"

namespace HMDT {
    /**
     * @brief Parse a single Section property
     * @details This is not placed in the Preferences class to prevent polluting
     *          the header file with any nlohmann/json* includes.
     *
     * @param section The section to parse the property for
     * @param name The name of the property to parse
     * @param jobject The json object holding the property to parse
     *
     * @return True if the property was successfully parsed, false otherwise.
     */
    bool parseProperty(Preferences::Section& section, const std::string& name,
                       const nlohmann::json& jobject)
    {

        // Note that we have to define this here, since it makes things easier
        //   for the below macro, and nlohmann::json doesn't define is_bool
        //   for some reason.
#define is_bool is_boolean

#define X(TYPE, PROPERTY, IS_USER_FACING)                          \
    if(name == STR(PROPERTY)) {                                                     \
        if constexpr(! IS_USER_FACING ) {                   \
            WRITE_ERROR("Property '" STR(PROPERTY) "' is not user-facing.");        \
            return false;                                                           \
        }                                                                           \
        if(!jobject. CONCAT(is_, TYPE) ()) {                                       \
            WRITE_ERROR("Property '" STR(PROPERTY) "' must be of type " STR(TYPE)); \
            return false;                                                           \
        }                                                                           \
        section. PROPERTY = jobject.get< TYPE >();                                  \
    } else

        HMDT_SECTION_PROPERTIES 

#undef X

    // But that reason may be a good one, so undefine it again to be safe
#undef is_bool

        // Final block, will run if the given property is unrecognized
        {
            WRITE_ERROR("Unknown property '", name, '\'');

            // NOTE: We don't return false here because this shouldn't be a failing
            //   error
        }

        return true;
    }
}

// Will initialize Preferences on the first call _AFTER_
//   setConfigLocation has been set to a _VALID_ path.
HMDT::Preferences& HMDT::Preferences::getInstance(bool try_init) {
    static HMDT::Preferences instance;

    // Only attempt to initialize if try_init is true
    if(try_init && !instance.m_initialized) {
        instance.initialize();
    }

    return instance;
}

/**
 * @brief Builds a config value path
 *
 * @param section_name The section name
 * @param group_name The group name
 * @param config_name The config name
 *
 * @return A string in the format of "Section.Group.Config"
 */
std::string HMDT::Preferences::buildValuePath(const std::string& section_name,
                                              const std::string& group_name,
                                              const std::string& config_name)
{
    return section_name + "." + group_name + "." + config_name;
}

/**
 * @brief Parses a value path into a triple
 *
 * @param path The path to parse
 *
 * @return A triple containing the components of the path, or std::nullopt if
 *         'path' is not a valid path.
 */
auto HMDT::Preferences::parseValuePath(const std::string& path)
    -> MonadOptional<std::tuple<std::string, std::string, std::string>>
{
    // There should be no more than 2 separator for SECTION.GROUP.CONFIG
    if(std::count(path.begin(), path.end(), '.') != 2) {
        WRITE_ERROR("Invalid path '", path, "'. Format should be 'Section.Group.Config'");
        return std::nullopt;
    }

    // Parse out the section+group name and the config name
    auto first_sep = path.find('.');
    auto last_sep = path.find('.', first_sep + 1);

    std::string section_name = path.substr(0, first_sep);
    std::string group_name = path.substr(first_sep + 1, last_sep - first_sep - 1);
    std::string config_name = path.substr(last_sep + 1);

    return std::make_tuple(section_name, group_name, config_name);
}

/**
 * @brief Sets the config file location to search for. Will not be used after
 *        initialization.
 *
 * @param path The config file location.
 */
void HMDT::Preferences::setConfigLocation(const std::filesystem::path& path) {
    m_config_path = path;
}

/**
 * @brief Gets the config file location that will be/was loaded.
 */
const std::filesystem::path& HMDT::Preferences::getConfigLocation() const {
    return m_config_path;
}

/**
 * @brief Defines a config $ENVVAR
 *
 * @param var_name The $ENVVAR to define
 * @param value The value to define the $ENVVAR as
 */
void HMDT::Preferences::defineEnvVar(const std::string& var_name,
                                     ValueVariant value)
{
    m_env_vars[var_name] = value;
}

void HMDT::Preferences::setDefaultValues(const SectionMap& default_sections) {
    m_default_sections = default_sections;
}

/**
 * @brief Sets a single config value at the given path.
 *
 * @param value_path The path to the config value. Given in the format of
 *                   "{Section}.{Group}.{Config}"
 * @param value_var The value to set the config value to.
 *
 * @return True if the config was successfully found and the value set, false
 *         otherwise.
 */
bool HMDT::Preferences::setPreferenceValue(const std::string& value_path,
                                           ValueVariant value_var)
{
    // First, verify that the type we were given is what we are actually
    //   expecting to see
    auto default_value = getValueVariantForPathInSectionMap(value_path,
                                                            m_default_sections);
    if(!default_value) {
        WRITE_ERROR("Invalid preference path '", value_path, '\'');
        return false;
    }

    if(default_value->get().index() != value_var.index()) {
        WRITE_ERROR("Provided preference value for '", value_path, "' does not "
                    "match expected type. Expected ",
                    default_value->get().index(), ", got ", value_var.index());
        return false;
    }

    return getValueVariantForPath(value_path)
        .andThen<bool>([this, &value_var](Ref<ValueVariant> ref_val) {
            auto& val = ref_val.get();

            val = value_var;

            m_dirty = true;

            return true;
        }).orElse(false);
}

/**
 * @brief Gets the Section named 'section_name'
 *
 * @param section_name The name of the section.
 *
 * @return The Section of name 'section_name'
 */
auto HMDT::Preferences::getSection(const std::string& section_name,
                                   const SectionMap& section_map) const
    -> MonadOptionalRef<const Section>
{
    if(section_map.count(section_name) != 0) {
        return section_map.at(section_name);
    }

    return std::nullopt;
}

/**
 * @brief Gets the Section named 'section_name'
 *
 * @param section_name The name of the section.
 *
 * @return The Section of name 'section_name'
 */
auto HMDT::Preferences::getSection(const std::string& section_name,
                                   SectionMap& section_map)
    -> MonadOptionalRef<Section>
{
    if(section_map.count(section_name) != 0) {
        return section_map.at(section_name);
    }

    return std::nullopt;
}

/**
 * @brief Gets the Group found at group_path
 *
 * @param group_path The path to the given group. Given in the format of
 *                   "{Section}.{Group}"
 *
 * @return The Group found at 'group_path' if it exists, std::nullopt otherwise.
 */
auto HMDT::Preferences::getGroup(const std::string& group_path,
                                 const SectionMap& section_map) const
    -> MonadOptionalRef<const Group>
{
    // There should be exactly 1 separator for SECTION.GROUP
    if(std::count(group_path.begin(), group_path.end(), '.') != 1) {
        WRITE_ERROR("Invalid group path '", group_path, "'. Format should be 'Section.Group'");
        return std::nullopt;
    }

    // Parse out the section+group name and the config name
    auto last_sep = group_path.rfind('.');
    std::string section_name = group_path.substr(0, last_sep);
    std::string group_name = group_path.substr(last_sep + 1);

    // Now look up the group in the section (assuming the section exists)
    return getSection(section_name, section_map)
        .andThen<Ref<const Group>>([&group_name](Ref<const Section> section)
            -> MonadOptionalRef<const Group>
        {
            if(section.get().groups.count(group_name) != 0) {
                return section.get().groups.at(group_name);
            }

            return std::nullopt;
        });
}

/**
 * @brief Gets the Group found at group_path
 *
 * @param group_path The path to the given group. Given in the format of
 *                   "{Section}.{Group}"
 *
 * @return The Group found at 'group_path' if it exists, std::nullopt otherwise.
 */
auto HMDT::Preferences::getGroup(const std::string& group_path,
                                 SectionMap& section_map)
    -> MonadOptionalRef<Group>
{
    // There should be exactly 1 separator for SECTION.GROUP
    if(std::count(group_path.begin(), group_path.end(), '.') != 1) {
        WRITE_ERROR("Invalid group path '", group_path, "'. Format should be 'Section.Group'");
        return std::nullopt;
    }

    // Parse out the section+group name and the config name
    auto last_sep = group_path.rfind('.');
    std::string section_name = group_path.substr(0, last_sep);
    std::string group_name = group_path.substr(last_sep + 1);

    // Now look up the group in the section (assuming the section exists)
    return getSection(section_name, section_map)
        .andThen<Ref<Group>>([&group_name](Ref<Section> section)
            -> MonadOptionalRef<Group>
        {
            if(section.get().groups.count(group_name) != 0) {
                return section.get().groups.at(group_name);
            }

            return std::nullopt;
        });
}

const HMDT::Preferences::SectionMap& HMDT::Preferences::getSections() const {
    return m_sections;
}

auto HMDT::Preferences::getDefaultSections() const -> const SectionMap& {
    return m_default_sections;
}

/**
 * @brief Gets the Config value found at value_path
 *
 * @param value_path The path to the given config value. Given in the format of
 *                   "{Section}.{Group}.{Config}"
 *
 * @return The Config value found at 'value_path' if it exists, std::nullopt
 *         otherwise.
 */
auto HMDT::Preferences::getValueVariantForPath(const std::string& value_path) const
    -> MonadOptionalRef<const ValueVariant>
{
    return getValueVariantForPathInSectionMap(value_path, m_sections);
}

/**
 * @brief Gets the Config value found at value_path
 *
 * @param value_path The path to the given config value. Given in the format of
 *                   "{Section}.{Group}.{Config}"
 *
 * @return The Config value found at 'value_path' if it exists, std::nullopt
 *         otherwise.
 */
auto HMDT::Preferences::getValueVariantForPath(const std::string& value_path)
    -> MonadOptionalRef<ValueVariant>
{
    return getValueVariantForPathInSectionMap(value_path, m_sections);
}

/**
 * @brief Gets the Config value found at value_path
 *
 * @param value_path The path to the given config value. Given in the format of
 *                   "{Section}.{Group}.{Config}"
 *
 * @return The Config value found at 'value_path' if it exists, std::nullopt
 *         otherwise.
 */
auto HMDT::Preferences::getValueVariantForPathInSectionMap(const std::string& value_path,
                                                           const SectionMap& section_map) const
    -> MonadOptionalRef<const ValueVariant>
{
    // There should be exactly 2 separators for SECTION.GROUP.CONFIG
    if(std::count(value_path.begin(), value_path.end(), '.') != 2) {
        WRITE_ERROR("Invalid config path '", value_path, "'. Format should be 'Section.Group.Config'");
        return std::nullopt;
    }

    // Parse out the section+group name and the config name
    auto last_sep = value_path.rfind('.');
    std::string section_group_name = value_path.substr(0, last_sep);
    std::string config_name = value_path.substr(last_sep + 1);

    // Now look up the config in the group (assuming the group exists)
    return getGroup(section_group_name, section_map)
        .andThen<Ref<const ValueVariant>>([&config_name](Ref<const Group> group)
            -> MonadOptionalRef<const ValueVariant>
        {
            if(group.get().configs.count(config_name) != 0) {
                return group.get().configs.at(config_name).second;
            }

            return std::nullopt;
        });
}

/**
 * @brief Gets the Config value found at value_path
 *
 * @param value_path The path to the given config value. Given in the format of
 *                   "{Section}.{Group}.{Config}"
 *
 * @return The Config value found at 'value_path' if it exists, std::nullopt
 *         otherwise.
 */
auto HMDT::Preferences::getValueVariantForPathInSectionMap(const std::string& value_path,
                                                           SectionMap& section_map)
    -> MonadOptionalRef<ValueVariant>
{
    // There should be exactly 2 separators for SECTION.GROUP.CONFIG
    if(std::count(value_path.begin(), value_path.end(), '.') != 2) {
        WRITE_ERROR("Invalid config-value path '", value_path, "'");
        return std::nullopt;
    }

    // Parse out the section+group name and the config name
    auto last_sep = value_path.rfind('.');
    std::string section_group_name = value_path.substr(0, last_sep);
    std::string config_name = value_path.substr(last_sep + 1);

    // Now look up the config in the group (assuming the group exists)
    return getGroup(section_group_name, section_map)
        .andThen<Ref<ValueVariant>>([&config_name](Ref<Group> group)
            -> MonadOptionalRef<ValueVariant>
        {
            if(group.get().configs.count(config_name) != 0) {
                return group.get().configs.at(config_name).second;
            }

            return std::nullopt;
        });
}

/**
 * @brief Loads and parses the config file, as well as all property values.
 *
 * @details Does not expand any "$" values here, that is done on 'get'
 */
void HMDT::Preferences::initialize() noexcept {
    using json = nlohmann::json;

    // Don't load anything if the config path hasn't been set yet.
    if(!std::filesystem::exists(m_config_path)) {
        return;
    }

    WRITE_INFO("Initializing Preferences from ", m_config_path);

    // Load the config file into a json object
    if(std::ifstream in(m_config_path); in) {
        json config;

        config = json::parse(in,
                             nullptr /* callback */,
                             true /* allow_exceptions */,
                             true /* ignore_comments */);

        if(!config.is_object()) {
            WRITE_ERROR("Config top-level type must be OBJECT.");
            return;
        }

        // For every section
        for(auto&& [sec_name, jsection] : config.items()) {
            WRITE_DEBUG("Parsing section '", sec_name, '\'');

            if(!jsection.is_object()) {
                WRITE_ERROR(sec_name, ": Sections must be of type OBJECT.");
                return;
            }

            Section& section = m_sections[sec_name];

            // For every group/property in the section
            for(auto&& [grp_prop_name, jobj] : jsection.items()) {
                WRITE_DEBUG("Parsing section item '", grp_prop_name, '\'');

                bool is_property = (grp_prop_name.front() == '@');

                if(is_property) {
                    WRITE_DEBUG(grp_prop_name, ": is a property.");

                    // Parse the property (the name will be 1 past the start to
                    //   get rid of the '@' sign)
                    if(!parseProperty(section, grp_prop_name.substr(1), jobj)) {
                        WRITE_ERROR(grp_prop_name, ": Failed to parse property");
                        return;
                    }
                } else {
                    WRITE_DEBUG(grp_prop_name, ": is a group.");

                    if(!jobj.is_object()) {
                        WRITE_ERROR(grp_prop_name, ": Groups must be of type OBJECT.");
                        return;
                    }

                    Group& group = section.groups[grp_prop_name];

                    // For every config in the group
                    for(auto&& [config_name, jconfig] : jobj.items()) {
                        WRITE_DEBUG("Parsing config value '", config_name, '\'');

                        ValueVariant& value = group.configs[config_name].second;

                        if(jconfig.is_boolean()) {
                            value = jconfig.get<bool>();
                        } else if(jconfig.is_number_integer()) {
                            value = jconfig.get<int64_t>();
                        } else if(jconfig.is_number_unsigned()) {
                            value = jconfig.get<uint64_t>();
                        } else if(jconfig.is_number_float()) {
                            value = jconfig.get<double>();
                        } else if(jconfig.is_string()) {
                            value = jconfig.get<std::string>();
                        } else if(jconfig.is_array()) {
                            WRITE_ERROR("ARRAY config values are not supported.");
                            return;
                        } else if(jconfig.is_object() || jconfig.is_null()) {
                            WRITE_ERROR("Config values cannot be OBJECT or NULL.");
                            return;
                        }
                    }
                }
            }
        }

        m_initialized = true;
        m_dirty = false;
    } else {
        WRITE_ERROR("Failed to open file ", m_config_path);
    }
}

/**
 * @brief Resets all configs to the default settings
 */
void HMDT::Preferences::resetToDefaults() {
    WRITE_INFO("Resetting preferences to the defaults.");

    m_sections = m_default_sections;
    m_dirty = false;
}

/**
 * @brief Validates that every loaded type matches the default config types.
 *
 * @return True if the loaded preference types correctly match the default
 *         preference types, false otherwise.
 */
bool HMDT::Preferences::validateLoadedPreferenceTypes() {
    uint32_t errors_found = 0;

    for(auto&& [sec_name, section] : m_sections) {
        // Warn and skip each additional section that is not tracked
        if(m_default_sections.count(sec_name) == 0) {
            WRITE_WARN("Default sections does not contain '", sec_name,
                       "'. It likely won't be used, and will not be checked.");
            continue;
        }

        const auto& def_section = m_default_sections.at(sec_name);

        // No need to check properties for errors here
        for(auto&& [group_name, group] : section.groups) {
            // Warn and skip each additional section that is not tracked
            if(def_section.groups.count(group_name) == 0) {
                WRITE_WARN("Default groups of '", sec_name,
                           "' does not contain group '", group_name,
                           "'. It likely won't be used, and will not be checked.");
                continue;
            }

            const auto& def_group = def_section.groups.at(group_name);

            for(auto&& [config_name, config_pair] : group.configs) {
                auto&& [_, config] = config_pair;

                // Warn and skip each additional section that is not tracked
                if(def_group.configs.count(config_name) == 0) {
                    WRITE_WARN("Default configs of '", sec_name, '.', group_name,
                               "' does not contain config '", config_name,
                               "'. It likely won't be used, and will not be checked.");
                    continue;
                }

                // First check if it's a $ENVVAR
                if(auto* value = std::get_if<std::string>(&config);
                         value != nullptr)
                {
                    if(value->size() > 0 && (*value)[0] == '$') {
                        WRITE_DEBUG("$ENVVAR detected. Cannot perform "
                                    "type-checking on load.");
                        continue;
                    }
                }

                const auto& def_config = def_group.configs.at(config_name).second;

                // Now we verify that the types are valid.
                if(def_config.index() != config.index()) {
                    // TODO: Can we dump out the name of the type as well?
                    WRITE_ERROR("Config value '", sec_name, '.', group_name,
                                '.', config_name, "' does not contain the "
                                "expected correct type. Expected type #", 
                                def_config.index(), ", got #", config.index());
                    ++errors_found;
                }
            }
        }
    }

    WRITE_INFO("Finished validating log file and found ", errors_found, " errors.");

    return errors_found == 0;
}

/**
 * @brief Serializes the loaded configs to JSON
 *
 * @param out The stream to write the JSON to.
 * @param pretty Whether to pretty-ify the serialized JSON
 */
void HMDT::Preferences::writeToJson(std::ostream& out, bool pretty) const {
    // Only indent if 'pretty == true'
    uint32_t indent_amt = pretty ? 4 : 0; // TODO: 4 should be a constant

    constexpr char INDENT_CHAR = ' ';

    // Note that this isn't an enum-class as we want to do math with it
    enum IndentLevel {
        NONE = 0,
        SECTION,
        GROUP,
        CONFIG
    };

    auto gen_indent = [indent_amt](IndentLevel level) {
        return std::string(level * indent_amt, INDENT_CHAR);
    };

    // NOTE: We do our own custom serializer here because we want to be able to
    //   also output things like comments
    // TODO: Can we somehow support non-defined comments that the user inserts?

    uint32_t section_count = 0;
    out << "{" << std::endl;
    for(auto&& [sec_name, section] : m_sections) {
        ++section_count;

        // Output the comment for this section if there is one
        if(!section.comment.empty()) {
            out << gen_indent(IndentLevel::SECTION)
                << "// " << section.comment
                << std::endl;
        }

        // Start outputting the section
        out << gen_indent(IndentLevel::SECTION)
            << '"' << sec_name << "\": {"
            << std::endl;

        // TODO: Output user-facing properties

        uint32_t group_count = 0;
        for(auto&& [grp_name, group] : section.groups) {
            ++group_count;

            // Output the comment for this group if there is one
            if(!group.comment.empty()) {
                out << gen_indent(IndentLevel::GROUP)
                    << "// " << group.comment
                    << std::endl;
            }

            // Start outputting the group
            out << gen_indent(IndentLevel::GROUP)
                << '"' << grp_name << "\": {"
                << std::endl;

            uint32_t config_count = 0;
            for(auto&& [config_name, comment_config] : group.configs) {
                auto&& [comment, config] = comment_config;

                ++config_count;

                // Output the comment for this config if there is one
                if(!comment.empty()) {
                    out << gen_indent(IndentLevel::CONFIG)
                        << "// " << comment
                        << std::endl;
                }

                // Output a comment with the default value for this config if
                //   there is a default value
                getValueVariantForPathInSectionMap(
                        Preferences::buildValuePath(sec_name, grp_name, config_name),
                        m_default_sections)
                    .andThen<bool>(
                        [&out, &gen_indent](Ref<const ValueVariant> value_var_ref)
                        {
                            auto& value_var = value_var_ref.get();

                            std::visit([&out, &gen_indent](auto& value) {
                                out << gen_indent(IndentLevel::CONFIG)
                                    << "// Default: " << nlohmann::json(value)
                                    << std::endl;
                            }, value_var);

                            return true;
                        });

                // Start outputting the config. Use nlohmann::json here for
                //   simplicity :)
                out << gen_indent(IndentLevel::CONFIG)
                    << '"' << config_name << "\": "
                    << std::visit([](auto& config_val) {
                            return nlohmann::json(config_val);
                        }, config);

                // Output trailing comma if necessary
                if(config_count < group.configs.size()) {
                    out << ',' << std::endl;
                }
                out << std::endl;
            }

            // Output closing brace with trailing comma
            out << gen_indent(IndentLevel::GROUP) << "}";
            if(group_count < section.groups.size()) {
                out << ',' << std::endl;
            }
            out << std::endl;
        }

        // Output closing brace with trailing comma
        out << gen_indent(IndentLevel::SECTION) << "}";
        if(section_count < m_sections.size()) {
            out << ',' << std::endl;
        }
        out << std::endl;
    }
    out << "}" << std::endl;
}

/**
 * @brief Serializes the loaded configs to a JSON file.
 *
 * @param pretty Whether to pretty-ify the serialized JSON
 *
 * @return True if the serialization to a file succeeded, false otherwise.
 */
bool HMDT::Preferences::writeToFile(bool pretty) const {
    if(m_config_path.empty()) {
        WRITE_ERROR("Cannot write preferences to config file as the path has "
                    "not been set yet.");
        return false;
    }

    if(!std::filesystem::exists(m_config_path.parent_path())) {
        WRITE_ERROR("Cannot write preferences to non-existant directory ",
                    m_config_path.parent_path());
        return false;
    }

    if(std::ofstream out(m_config_path); out) {
        writeToJson(out, pretty);

        return true;
    } else {
        return false;
    }
}

/**
 * @brief Serializes the loaded configs to the info log
 */
void HMDT::Preferences::writeToLog() const {
    std::stringstream ss;
    writeToJson(ss);

    WRITE_INFO(ss.str());
}

HMDT::Preferences::Preferences():
    m_config_path(),
    m_sections(),
    m_default_sections(),
    m_env_vars(),
    m_initialized(false),
    m_dirty(false)
{ }

bool HMDT::Preferences::isInitialized() const {
    return m_initialized;
}

bool HMDT::Preferences::isDirty() const {
    return m_dirty;
}

void HMDT::Preferences::_reset() noexcept {
    WRITE_WARN("_reset() called! This should only ever be called from the testing framework!");

    m_config_path = "";
    m_sections.clear();
    m_default_sections.clear();
    m_env_vars.clear();
    m_initialized = false;
    m_dirty = false;
}

