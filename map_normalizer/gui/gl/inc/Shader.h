/**
 * @file Shader.h
 *
 * @brief Defines the Shader class
 */

#ifndef GL_SHADER_H
# define GL_SHADER_H

# include <string>
# include <string_view>
# include <memory>

namespace MapNormalizer::GUI::GL {
    /**
     * @brief A class which represents a single GLSL shader
     *
     * @details This class is reference counted, to prevent accidentally leaking
     *          GL shader memory
     */
    class Shader {
        public:
            /**
             * @brief The possible shader types
             */
            enum class Type {
                VERTEX,   //!< A Vertex Shader
                FRAGMENT, //!< A Fragment Shader
            };

            /**
             * @brief An exception which represents a GLSL shader compilation
             *        exception.
             */
            class CompileException: public std::exception {
                public:
                    CompileException(Type, const std::string&, const std::string&);

                    virtual const char* what() const noexcept override;

                    Type getType() const noexcept;

                private:
                    //! The type of shader the exception was thrown for.
                    Type m_type;

                    //! The GLSL source code which failed.
                    std::string m_source;

                    //! The reason the exception was thrown
                    std::string m_reason;

                    //! The full string 'what' of what went wrong
                    std::string m_what;
            };


            Shader(Type, const std::string&);
            Shader(const Shader&);

            ~Shader();

            Shader& operator=(const Shader&);

            uint32_t getID() const;

        private:
            static uint32_t typeToGL(Type);

            //! The reference counter
            std::shared_ptr<uint32_t> m_ref_count;

            //! The GL Shader ID
            uint32_t m_shader_id;
    };
}

namespace std {
    string_view to_string(const MapNormalizer::GUI::GL::Shader::Type&);
}

#endif

