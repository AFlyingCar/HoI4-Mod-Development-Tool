#ifndef MN_SOURCE_H
# define MN_SOURCE_H

# include <string>
# include <filesystem>

namespace MapNormalizer::Log {
    /**
     * @brief Defines a logging message source
     */
    class Source {
        public:
            Source(const std::filesystem::path& = "",
                   const std::filesystem::path& = "",
                   const std::string& = "", uint32_t = 0);

            const std::filesystem::path& getModulePath() const;
            const std::filesystem::path& getFileName() const;
            const std::string& getFunctionName() const;
            uint32_t getLineNumber() const;

            bool operator==(const Source&) const;

        private:
            //! The module the message originated from
            std::filesystem::path m_module_name;

            //! The filename the message originated from
            std::filesystem::path m_filename;

            //! The function name the message was logged in
            std::string m_function_name;

            //! The line number the message was logged at
            uint32_t m_line_number;
    };

    std::filesystem::path getModulePath();
}

# ifdef WIN32
#  define FUNC_NAME __FUNCTION__
# else
#  define FUNC_NAME __func__
# endif

/**
 * @brief Generates a MapNormalizer::Log::Source object for the point it was
 *        called from.
 */
# define MN_LOG_SOURCE() [](auto&& func_name) {                              \
    return MapNormalizer::Log::Source(                                       \
        MapNormalizer::Log::getModulePath(), __FILE__, func_name, __LINE__); \
}(FUNC_NAME)

#endif

