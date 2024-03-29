/**
 * @file Preferences.h
 *
 * @brief Declares the interface for Preferences.
 */

#ifndef PREFERENCES_H
# define PREFERENCES_H

# include <map>
# include <string>
# include <variant>
# include <filesystem>

# include "Maybe.h"
# include "Monad.h"
# include "Logger.h"
# include "Util.h"

# include "SectionProperties.h"

/**
 * @brief Creates the IsUserFacing name for the given property
 *
 * @param PROP_NAME The property.
 */
# define HMDT_SECTION_GET_PROP_IUF_NAME(PROP_NAME) \
    CONCAT(_, CONCAT(PROP_NAME, _IsUserFacing))

namespace HMDT {
    /**
     * @brief A class which represents the program Preferences
     */
    class Preferences {
        public:
            /**
             * @brief All possible value types
             */
            using ValueVariant = std::variant<int64_t, uint64_t, double, bool, std::string>;

            /**
             * @brief The callback to be called when a preference value is changed
             */
            using OnPreferenceChangeCallback = std::function<bool(const ValueVariant&, const ValueVariant&)>;

            /**
             * @brief Helper function which constructs a ValueVariant from a T
             * @details This should be used instead of constructing in-place, as
             *          it will take care of certain types of implicit
             *          conversions.
             *
             * @tparam T The type being passed into the ValueVariant
             * @param value The value being held by the ValueVariant
             *
             * @return A new ValueVariant
             */
            template<typename T>
            static ValueVariant buildValueVariant(const T& value) {
                if constexpr(std::is_same_v<std::decay_t<T>, const char*> ||
                             std::is_same_v<std::decay_t<T>, char*>)
                {
                    return std::string(value);
                } else {
                    return value;
                }
            }

            struct Config {
                //! The comment for this config
                std::string comment;

                //! The value for this config
                ValueVariant value;

                //! Whether this config requires the program to be restarted to take effect
                bool requires_restart;
            };

            /**
             * @brief A Group of config values
             */
            struct Group {
                //! The comment for this group
                std::string comment;

                //! The configs for this group name -> config
                std::map<std::string, Config> configs;
            };

            /**
             * @brief A Section of config Groups
             */
            struct Section {
                //! The comment for this section
                std::string comment;

# define X(TYPE, PROP_NAME, IS_USER_FACING)                          \
    TYPE PROP_NAME;                                                  \
    static constexpr bool HMDT_SECTION_GET_PROP_IUF_NAME(PROP_NAME) = IS_USER_FACING;

                HMDT_SECTION_PROPERTIES 
# undef X

                //! The groups for this section
                std::map<std::string, Group> groups;
            };

            using SectionMap = std::map<std::string, Section>;

            // Will initialize Preferences on the first call _AFTER_
            //   setConfigLocation has been set to a _VALID_ path.
            static Preferences& getInstance(bool = true);

            static std::string buildValuePath(const std::string&,
                                              const std::string&,
                                              const std::string&);

            static MonadOptional<std::tuple<std::string,
                                            std::string,
                                            std::string>> parseValuePath(const std::string&);

            // Unused after initialization
            void setConfigLocation(const std::filesystem::path&);
            const std::filesystem::path& getConfigLocation() const;

            void defineEnvVar(const std::string&, ValueVariant);

            void setDefaultValues(const SectionMap&);

            MonadOptionalRef<const Section> getSection(const std::string&,
                                                       const SectionMap&) const;
            MonadOptionalRef<Section> getSection(const std::string&,
                                                 SectionMap&);

            MonadOptionalRef<const Group> getGroup(const std::string&,
                                                   const SectionMap&) const;
            MonadOptionalRef<Group> getGroup(const std::string&,
                                             SectionMap&);

            const SectionMap& getSections() const;
            const SectionMap& getDefaultSections() const;

            // Get a preference value. Pass in path to the value as a string:
            //  "{Section}.{Group}.{Title}"
            /**
             * @brief Gets a preference value.
             *
             * @tparam T The type that is expected from the given value path.
             *
             * @param value_path The path to the config value. Given in the
             *                   format of "{Section}.{Group}.{Config}"
             *
             * @return An optional containing the preference value if the path
             *         exists and if it contains the expected type, otherwise
             *         std::nullopt is returned.
             */
            template<typename T>
            MonadOptional<T> getPreferenceValue(const std::string& value_path) const noexcept
            {
                return getPreferenceValue<T>(value_path, m_sections);
            }

            // Sets a preference value. value_path takes the same sort of argument
            //  as getPreferenceValue
            bool setPreferenceValue(const std::string& value_path, ValueVariant);

            MonadOptional<bool> doesPathRequireRestart(const std::string&) const noexcept;

            MaybeVoid setCallbackOnPreferenceChange(const std::string&,
                                                    OnPreferenceChangeCallback) noexcept;

            void resetToDefaults();

            MaybeVoid validateLoadedPreferenceTypes();

            void writeToJson(std::ostream&, bool = false) const;
            MaybeVoid writeToFile(bool = false) const;
            void writeToLog() const;

            bool isInitialized() const;

            bool isDirty() const;

            // WARN: INTERNAL FUNCTION ONLY FOR TESTING PURPOSES
            //  DO NOT USE FOR NON-TESTING PURPOSES
            void _reset() noexcept;

        protected:
            /**
             * @brief Gets a preference value.
             *
             * @tparam T The type that is expected from the given value path.
             *
             * @param value_path The path to the config value. Given in the
             *                   format of "{Section}.{Group}.{Config}"
             * @param section_map The SectionMap to get the value from.
             * @param expand_envvars Whether or not to expand $ENVVARs
             *
             * @return An optional containing the preference value if the path
             *         exists and if it contains the expected type, otherwise
             *         std::nullopt is returned.
             */
            template<typename T>
            MonadOptional<T> getPreferenceValue(const std::string& value_path,
                                                const SectionMap& section_map,
                                                bool expand_envvars = true) const noexcept
            {
                MonadOptionalRef<const ValueVariant> opt_value_var = getValueVariantForPathInSectionMap(value_path, section_map);
                return opt_value_var.andThen<T>(
                    [this, value_path, expand_envvars](Ref<const ValueVariant> value_var_ref)
                        -> MonadOptional<T>
                    {
                        auto& value_var = value_var_ref.get();
                    
                        // Parse $VARs first
                        if(expand_envvars) {
                            if(auto* value = std::get_if<std::string>(&value_var);
                                     value != nullptr)
                            {
                                if(value->size() > 0 && (*value)[0] == '$') {
                                    return parseEnvVar<T>(value->substr(1),
                                                          value_path);
                                }
                            }
                        }

                        if(std::holds_alternative<T>(value_var)) {
                            return std::get<T>(value_var);
                        } else {
                            WRITE_ERROR("Held type of '", value_path,
                                        "' does not match expected type of '",
                                        typeid(T).name(), "'.");
                            return std::nullopt;
                        }
                    });
            }

            /**
             * @brief Parses a $ENVVAR
             * @details If the env_var is "DEFAULT", then the default value for
             *          value_path will be returned.
             *
             * @tparam T The type that is expected for the given $ENVVAR
             * @param env_var The $ENVVAR to get
             * @param value_path The current config value path.
             *
             * @return The value of the $ENVVAR, or std::nullopt if the
             *         $ENVVAR's type doesn't match the requested type T.
             */
            template<typename T>
            MonadOptional<T> parseEnvVar(const std::string& env_var,
                                         const std::string& value_path) const
            {
                if(env_var == "DEFAULT") {
                    WRITE_DEBUG("Value at '", value_path, "' is $DEFAULT. Expanding...");
                    return getPreferenceValue<T>(value_path, m_default_sections, false);
                }

                if(m_env_vars.find(env_var) != m_env_vars.end()) {
                    if(std::holds_alternative<T>(m_env_vars.at(env_var))) {
                        return std::get<T>(m_env_vars.at(env_var));
                    } else {
                        WRITE_ERROR("Held type of $", env_var,
                                    " does not match expected type of '",
                                    typeid(T).name(), "'.");
 
                        return std::nullopt;
                    }
                } else {
                    return getPreferenceValue<T>(env_var);
                }
            }

            MonadOptionalRef<const ValueVariant> getValueVariantForPath(const std::string&) const;
            MonadOptionalRef<ValueVariant> getValueVariantForPath(const std::string&);

            MonadOptionalRef<const ValueVariant> getValueVariantForPathInSectionMap(const std::string&, const SectionMap&) const;
            MonadOptionalRef<ValueVariant> getValueVariantForPathInSectionMap(const std::string&, SectionMap&);

            // Load + Parse the config file, as well as all property values
            //  Note that we are not going to expand any "$" values here, that is
            //   done on 'get'
            void initialize() noexcept;

        private:
            Preferences();

            //! The path to the config file to load
            std::filesystem::path m_config_path;

            //! All sections of the config file
            SectionMap m_sections;

            //! All default values
            SectionMap m_default_sections;

            //! All defined config environment variables
            std::map<std::string, ValueVariant> m_env_vars;

            //! All registered callbacks to be called when a preference value changes
            std::map<std::string, OnPreferenceChangeCallback> m_on_pref_change_callbacks;

            //! Used to specify if initialize() has been called and succeeded
            bool m_initialized;

            /**
             * @brief Used to specify if the config values have been modified
             *        since they were last loaded or reset.
             */
            bool m_dirty;
    };

    bool operator==(const Preferences::Config&, const Preferences::Config&) noexcept;

/// @cond
# define PREF_BEGIN_DEF() {

# define PREF_BEGIN_DEFINE_SECTION(SEC_NAME, COMMENT) \
    { SEC_NAME, []() { HMDT::Preferences::Section _section; _section.comment = std::string("" COMMENT);

# define PREF_SECTION_DEFINE_PROPERTY(PROP_NAME, PROP_VAL) \
    _section. PROP_NAME = PROP_VAL ;

# define PREF_BEGIN_DEFINE_GROUP(GROUP_NAME, COMMENT) \
    _section.groups[ GROUP_NAME ] = HMDT::Preferences::Group { std::string(COMMENT), {

# define PREF_DEFINE_CONFIG(CONF_NAME, VALUE, COMMENT, REQRESTART) \
    { CONF_NAME, HMDT::Preferences::Config{            \
        std::string(COMMENT),                          \
        HMDT::Preferences::buildValueVariant( VALUE ), \
        REQRESTART }                                   \
    },

# define PREF_END_DEFINE_GROUP(GROUP_NAME) } };

# define PREF_END_DEFINE_SECTION() return _section; }() }

# define PREF_END_DEF() }
///@endcond

}

#endif

