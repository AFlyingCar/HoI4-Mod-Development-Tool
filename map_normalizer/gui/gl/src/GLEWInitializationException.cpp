
#include "GLEWInitializationException.h"

#include <GL/glew.h>

MapNormalizer::GUI::GL::GLEWInitializationException::GLEWInitializationException(uint32_t reason):
    m_reason(reason)
{ }

const char* MapNormalizer::GUI::GL::GLEWInitializationException::what() const noexcept
{
    return reinterpret_cast<const char*>(glewGetErrorString(m_reason));
}

