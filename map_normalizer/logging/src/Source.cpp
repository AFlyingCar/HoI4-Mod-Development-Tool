
#include "Source.h"

MapNormalizer::Log::Source::Source(const std::string& module_name,
                                   const std::string& filename,
                                   const std::string& function_name,
                                   uint32_t line_number):
    m_module_name(module_name),
    m_filename(filename),
    m_function_name(function_name),
    m_line_number(line_number)
{ }

const std::string& MapNormalizer::Log::Source::getModuleName() const {
    return m_module_name;
}

const std::string& MapNormalizer::Log::Source::getFileName() const {
    return m_filename;
}

const std::string& MapNormalizer::Log::Source::getFunctionName() const {
    return m_function_name;
}

uint32_t MapNormalizer::Log::Source::getLineNumber() const {
    return m_line_number;
}


bool MapNormalizer::Log::Source::operator==(const Source& other) const {
    return m_module_name == other.m_module_name &&
           m_filename == other.m_filename &&
           m_function_name == other.m_function_name &&
           m_line_number == other.m_line_number;
}

std::string MapNormalizer::Log::getModuleName() {
#ifdef WIN32
#else
#endif
    return "";
}

