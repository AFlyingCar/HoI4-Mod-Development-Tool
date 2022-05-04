/**
 * @file Texture.h
 *
 * @brief Defines the Texture class
 */

#ifndef GL_TEXTURE_H
# define GL_TEXTURE_H

# include <cstdint>
# include <typeinfo>
# include <queue>
# include <string>
# include <optional>

namespace HMDT::GUI::GL {
    /**
     * @brief Represents an OpenGL texture
     */
    class Texture {
        public:
            /**
             * @brief Represents each valid texture unit (at least 16 must be
             *        supported)
             */
            enum class Unit {
                TEX_UNIT0, TEX_UNIT1, TEX_UNIT2, TEX_UNIT3, TEX_UNIT4,
                TEX_UNIT5, TEX_UNIT6, TEX_UNIT7, TEX_UNIT8, TEX_UNIT9,
                TEX_UNIT10, TEX_UNIT11, TEX_UNIT12, TEX_UNIT13, TEX_UNIT14,
                TEX_UNIT15,

                INVALID = -1
            };

            /**
             * @brief Represents each valid Texture target
             */
            enum class Target {
                TEX_1D,
                TEX_2D,
            };

            /**
             * @brief Represents each valid GL format
             */
            enum class Format {
                RED, GREEN, BLUE, ALPHA,
                RGB,
                RGBA,
                RED32I,
                RED32UI
            };

            /**
             * @brief A GL texture axis for wrapping
             */
            enum class Axis {
                S, T
            };

            /**
             * @brief Defines each GL wrapping mode
             */
            enum class WrapMode {
                REPEAT,
                MIRRORED_REPEAT,
                CLAMP_TO_EDGE,
                CLAMP_TO_BORDER
            };

            /**
             * @brief GL filtering types
             */
            enum class FilterType {
                MAG, MIN
            };

            /**
             * @brief GL filters 
             */
            enum class Filter {
                NEAREST, LINEAR
            };

            Texture();
            ~Texture();

            Texture(const Texture&) = delete;
            Texture& operator=(const Texture&) = delete;

            void setTarget(Target);

            void setWrapping(Axis, WrapMode);
            void setFiltering(FilterType, Filter);
            void setTextureUnitID(Unit);

            /**
             * @brief Sends texture data to the GPU.
             *
             * @details Implicitly calls bind()
             *
             * @tparam T The type of data being passed in
             *
             * @param internal_format The format of the data. Currently, both
             *                        the CPU-side and GPU-side format must
             *                        match.
             * @param width The width of the data
             * @param height The height of the data
             * @param data The data to send to the GPU
             */
            template<typename T>
            void setTextureData(Format internal_format,
                                uint32_t width, uint32_t height, const T* data,
                                std::optional<uint32_t> format = std::nullopt)
            {
                setTextureData(internal_format, width, height,
                               typeToDataType(typeid(T)), data, format);
            }

            uint32_t getTextureUnitID() const;
            uint32_t getTextureID() const;
            uint32_t getWidth() const;
            uint32_t getHeight() const;
            std::pair<uint32_t, uint32_t> getDimensions() const;

            void bind(bool = true);
            uint32_t activate();

        protected:
            static uint32_t typeToDataType(const std::type_info&);

            static uint32_t targetToGLTarget(Target);
            static uint32_t formatToGLFormat(Format);
            static uint32_t axisToGLAxis(Axis);
            static uint32_t wrapToGLWrap(WrapMode);
            static uint32_t filterToGLFilter(Filter);
            static uint32_t filterTypeToGLFilterType(FilterType);
            static uint32_t unitToGLUnit(Unit);

            void setTextureData(Format, uint32_t, uint32_t, uint32_t,
                                const void*, std::optional<uint32_t>);

        private:
            //! The texture ID
            uint32_t m_texture_id;

            //! The texture unit this takes up
            uint32_t m_texture_unit;

            //! The target of this texture
            Target m_target;

            uint32_t m_width;
            uint32_t m_height;
    };
}

#endif

