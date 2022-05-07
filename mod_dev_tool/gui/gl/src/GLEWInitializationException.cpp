
#include "GLEWInitializationException.h"

#include <GL/glew.h>

HMDT::GUI::GL::GLEWInitializationException::GLEWInitializationException(uint32_t reason):
    m_reason(reason)
{ }

const char* HMDT::GUI::GL::GLEWInitializationException::what() const noexcept {
    return reinterpret_cast<const char*>(glewGetErrorString(m_reason));
}

