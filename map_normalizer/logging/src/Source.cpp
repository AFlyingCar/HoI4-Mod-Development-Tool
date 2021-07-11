
#include "Source.h"

#ifdef WIN32
# include <libloaderapi.h>
#else
# include <dlfcn.h>
# include <unistd.h>
#endif

MapNormalizer::Log::Source::Source(const std::filesystem::path& module_name,
                                   const std::filesystem::path& filename,
                                   const std::string& function_name,
                                   uint32_t line_number):
    m_module_name(module_name),
    m_filename(filename),
    m_function_name(function_name),
    m_line_number(line_number)
{ }

const std::filesystem::path& MapNormalizer::Log::Source::getModulePath() const {
    return m_module_name;
}

const std::filesystem::path& MapNormalizer::Log::Source::getFileName() const {
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

std::filesystem::path MapNormalizer::Log::getModulePath(void* address) {
#ifdef WIN32
    if(HMODULE handle; GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                                         reinterpret_cast<LPCSTR>(address),
                                         &handle))
    {
        CHAR filename[MAX_PATH];
        GetModuleFileNameA(handle, filename, MAX_PATH);

        return filename;
    }
#else
    // Get the calling function and find out which module that is a part of
    //  https://stackoverflow.com/a/2156531
    address = __builtin_return_address(0);

    if(Dl_info info; dladdr(address, &info) != 0) {
        return info.dli_fname;
    }
#endif

    return "";
}

