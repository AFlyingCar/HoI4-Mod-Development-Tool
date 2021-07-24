#ifndef GLEWINITIALIZATIONEXCEPTION_H
# define GLEWINITIALIZATIONEXCEPTION_H

# include <exception>
# include <cstdint>

namespace MapNormalizer::GUI::GL {
    class GLEWInitializationException: public std::exception {
        public:
            GLEWInitializationException(uint32_t);

            virtual const char* what() const noexcept override;
        private:
            uint32_t m_reason;
    };
}

#endif

