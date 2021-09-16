/**
 * @file Program.h
 *
 * @brief Defines the Program class
 */

#ifndef GL_PROGRAM_H
# define GL_PROGRAM_H

# include <initializer_list>
# include <vector>
# include <memory>
# include <any>

# include "Shader.h"

namespace MapNormalizer::GUI::GL {
    /**
     * @brief Represents a fully compiled and linked GLSL Shader Program
     */
    class Program {
        public:
            /**
             * @brief An exception which is thrown when a GLSL link error
             *        occurs
             */
            class LinkException: public std::exception {
                public:
                    LinkException(const std::string&);

                    virtual const char* what() const noexcept override;

                private:
                    //! The reason for the link error
                    std::string m_reason;
            };

            Program();

            Program(std::initializer_list<Shader>);
            Program(const std::vector<Shader>&);

            Program(const Program&);
            ~Program();

            Program& operator=(const Program&);

            void use(bool = true);

            bool uniform(const std::string&, const std::any&) /* throws */;

        private:
            void attachShader(const Shader&);

            /**
             * @brief Attaches every Shader in the range to this program
             *
             * @tparam Iter The iterator type
             *
             * @param begin The start of the range of Shaders
             * @param end The end of the range of Shaders
             */
            template<typename Iter>
            void attachShaders(Iter begin, Iter end) {
                while(begin != end) {
                    attachShader(*begin);
                    ++begin;
                }
            }

            void linkProgram();

            //! The reference counter
            std::shared_ptr<uint32_t> m_ref_count;

            //! The GLSL program ID
            uint32_t m_program_id;
    };
}

#endif

