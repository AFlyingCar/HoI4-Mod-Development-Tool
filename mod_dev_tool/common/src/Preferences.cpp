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
#define HMDT_DEFINE_PROPERTY(PROPERTY, JTYPE, TYPE)                                 \
    if(name == STR(PROPERTY)) {                                                     \
        if(!section. HMDT_SECTION_GET_PROP_IUF_NAME(PROPERTY) ) {                   \
            WRITE_ERROR("Property '" STR(PROPERTY) "' is not user-facing.");        \
            return false;                                                           \
        }                                                                           \
        if(!jobject. CONCAT(is_, JTYPE) ()) {                                       \
            WRITE_ERROR("Property '" STR(PROPERTY) "' must be of type " STR(TYPE)); \
            return false;                                                           \
        }                                                                           \
        section. PROPERTY = jobject.get< TYPE >();                                  \
    } else

        // Add all new properties here for parsing
        HMDT_DEFINE_PROPERTY(showTitles, boolean, bool)

        // Final block, will run if none of the other properties are defined.
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
        .andThen<bool>([&value_var](Ref<ValueVariant> ref_val) {
            auto& val = ref_val.get();

            val = value_var;

            // TODO: Monadidc operations should be able to take functions
            //   which return void (this should essentially return a 
            //   MonadOptional<std::monostate>
            return true;
        }) != std::nullopt;
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
                return group.get().configs.at(config_name);
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
                return group.get().configs.at(config_name);
            }

            return std::nullopt;
        });
}

// Load + Parse the config file, as well as all property values
//  Note that we are not going to expand any "$" values here, that is
//   done on 'get'
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

        in >> config;

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

                        ValueVariant& value = group.configs[config_name];

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
    } else {
        WRITE_ERROR("Failed to open file ", m_config_path);
    }
}

void HMDT::Preferences::resetToDefaults() {
    WRITE_INFO("Resetting preferences to the defaults.");

    m_sections = m_default_sections;
}

bool HMDT::Preferences::validateLoadedPreferenceTypes() {
    // Use the default preferences to validate if each type matches the type
    //   found in the default list

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

            for(auto&& [config_name, config] : group.configs) {
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

                const auto& def_config = def_group.configs.at(config_name);

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

void HMDT::Preferences::writeToJson(std::ostream& out, bool pretty) const {
    using json = nlohmann::json;

    json root;

    // Build the root out of every section
    for(auto&& [sec_name, section] : m_sections) {
        json jsec;

        //////////////////////
        // Write every section property
#define X(_, PROP_NAME, IS_USER_FACING)  \
        if constexpr( IS_USER_FACING ) { \
            jsec["@" STR(PROP_NAME) ] = section. PROP_NAME ; \
        }

        HMDT_SECTION_PROPERTIES 

#undef X
        //////////////////////

        // Write every group
        for(auto&& [group_name, group] : section.groups) {
            json jgroup;

            // Write every config
            for(auto&& [config_name, config] : group.configs) {
                // Note that we need to get a special binding here so we can
                //   access config_name in the lambda
                const auto& cfg_name = config_name;
                std::visit([&](auto&& config_val) {
                    jgroup[cfg_name] = config_val;
                }, config);
            }

            jsec[group_name] = jgroup;
        }

        root[sec_name] = jsec;
    }

    if(pretty) {
        out << root.dump(4);
    } else {
        out << root;
    }
}

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
    m_initialized(false)
{ }

bool HMDT::Preferences::isInitialized() const {
    return m_initialized;
}

void HMDT::Preferences::_reset() noexcept {
    WRITE_WARN("_reset() called! This should only ever be called from the testing framework!");

    m_config_path = "";
    m_sections.clear();
    m_default_sections.clear();
    m_env_vars.clear();
    m_initialized = false;
}

