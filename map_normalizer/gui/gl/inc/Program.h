#ifndef GL_PROGRAM_H
# define GL_PROGRAM_H

# include <initializer_list>
# include <vector>
# include <memory>

# include "Shader.h"

namespace MapNormalizer::GUI::GL {
    class Program {
        public:
            class LinkException: public std::exception {
                public:
                    LinkException(const std::string&);

                    virtual const char* what() const noexcept override;

                private:
                    std::string m_reason;
            };

            Program();

            Program(std::initializer_list<Shader>);
            Program(const std::vector<Shader>&);

            Program(const Program&);
            ~Program();

            Program& operator=(const Program&);

            void use(bool = true);

        private:
            void attachShader(const Shader&);

            template<typename Iter>
            void attachShaders(Iter begin, Iter end) {
                while(begin != end) {
                    attachShader(*begin);
                    ++begin;
                }
            }

            void linkProgram();

            std::shared_ptr<uint32_t> m_ref_count;

            uint32_t m_program_id;
    };
}

#endif

