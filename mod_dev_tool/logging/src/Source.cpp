
#include "Source.h"

#ifdef WIN32
# include <libloaderapi.h>
#else
# include <dlfcn.h>
# include <unistd.h>
#endif

HMDT::Log::Source::Source(const std::filesystem::path& module_name,
                          const std::filesystem::path& filename,
                          const std::string& function_name,
                          uint32_t line_number):
    m_module_name(module_name),
    m_filename(filename),
    m_function_name(function_name),
    m_line_number(line_number)
{ }

const std::filesystem::path& HMDT::Log::Source::getModulePath() const {
    return m_module_name;
}

const std::filesystem::path& HMDT::Log::Source::getFileName() const {
    return m_filename;
}

const std::string& HMDT::Log::Source::getFunctionName() const {
    return m_function_name;
}

uint32_t HMDT::Log::Source::getLineNumber() const {
    return m_line_number;
}


bool HMDT::Log::Source::operator==(const Source& other) const {
    return m_module_name == other.m_module_name &&
           m_filename == other.m_filename &&
           m_function_name == other.m_function_name &&
           m_line_number == other.m_line_number;
}

std::filesystem::path HMDT::Log::getModulePath() {
#ifdef WIN32
# ifdef _MSC_VER
#  define RETURN_ADDRESS() _ReturnAddress()
# else
#  define RETURN_ADDRESS() __builtin_return_address(0)
# endif

    if(HMODULE handle; GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                                         reinterpret_cast<LPCSTR>(RETURN_ADDRESS()),
                                         &handle))
    {
        CHAR filename[MAX_PATH];
        GetModuleFileNameA(handle, filename, MAX_PATH);

        return filename;
    }
#else
    // Get the calling function and find out which module that is a part of
    //  https://stackoverflow.com/a/2156531
    if(Dl_info info; dladdr(__builtin_return_address(0), &info) != 0) {
        return info.dli_fname;
    }
#endif

    return "";
}

