/**
 * @file GLEWInitializationException.h
 *
 * @brief Defines an exception which is thrown if GLEW fails to initialize.
 */

#ifndef GLEWINITIALIZATIONEXCEPTION_H
# define GLEWINITIALIZATIONEXCEPTION_H

# include <exception>
# include <cstdint>

namespace MapNormalizer::GUI::GL {
    /**
     * @brief An exception representing the case where GLEW failed to initialize
     */
    class GLEWInitializationException: public std::exception {
        public:
            GLEWInitializationException(uint32_t);

            virtual const char* what() const noexcept override;
        private:
            uint32_t m_reason;
    };
}

#endif

