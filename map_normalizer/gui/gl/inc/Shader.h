#ifndef GL_SHADER_H
# define GL_SHADER_H

# include <string>
# include <memory>

namespace MapNormalizer::GUI::GL {
    class Shader {
        public:
            enum class Type {
                VERTEX,
                FRAGMENT,
            };

            class CompileException: public std::exception {
                public:
                    CompileException(Type, const std::string&);

                    virtual const char* what() const noexcept override;

                    Type getType() const noexcept;

                private:
                    Type m_type;
                    std::string m_reason;
            };


            Shader(Type, const std::string&);
            Shader(const Shader&);

            ~Shader();

            Shader& operator=(const Shader&);

            uint32_t getID() const;

        private:
            static uint32_t typeToGL(Type);

            std::shared_ptr<uint32_t> m_ref_count;

            uint32_t m_shader_id;
    };
}

#endif

