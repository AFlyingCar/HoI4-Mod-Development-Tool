#ifndef MN_SOURCE_H
# define MN_SOURCE_H

#include <string>

namespace MapNormalizer::Log {
    /**
     * @brief Defines a logging message source
     */
    class Source {
        public:
            Source(const std::string& = "", const std::string& = "",
                   const std::string& = "", uint32_t = 0);

            const std::string& getModuleName() const;
            const std::string& getFileName() const;
            const std::string& getFunctionName() const;
            uint32_t getLineNumber() const;

            bool operator==(const Source&) const;

        private:
            //! The module the message originated from
            std::string m_module_name;

            //! The filename the message originated from
            std::string m_filename;

            //! The function name the message was logged in
            std::string m_function_name;

            //! The line number the message was logged at
            uint32_t m_line_number;
    };

    std::string getModuleName();
}

# ifdef WIN32
#  define FUNC_NAME __FUNCTION__
# else
#  define FUNC_NAME __func__
# endif

# define MN_LOG_SOURCE() MapNormalizer::Log::Source( \
        MapNormalizer::Log::getModuleName(), __FILE__, FUNC_NAME, __LINE__)

#endif

