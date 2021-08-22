#ifndef GL_TEXTURE_H
# define GL_TEXTURE_H

# include <cstdint>
# include <typeinfo>
# include <queue>
# include <string>

namespace MapNormalizer::GUI::GL {
    class Texture {
        public:

            class TextureActivationFailure: public std::exception {
                public:
                    TextureActivationFailure(const std::string&);

                    virtual const char* what() const noexcept override;

                private:
                    std::string m_reason;
            };

            enum class Unit {
                TEX_UNIT0, TEX_UNIT1, TEX_UNIT2, TEX_UNIT3, TEX_UNIT4,
                TEX_UNIT5, TEX_UNIT6, TEX_UNIT7, TEX_UNIT8, TEX_UNIT9,
                TEX_UNIT10, TEX_UNIT11, TEX_UNIT12, TEX_UNIT13, TEX_UNIT14,
                TEX_UNIT15,

                INVALID = -1
            };

            enum class Target {
                TEX_1D,
                TEX_2D,
            };

            enum class Format {
                RED, GREEN, BLUE, ALPHA,
                RGB,
                RGBA
            };

            enum class Axis {
                S, T
            };

            enum class WrapMode {
                REPEAT,
                MIRRORED_REPEAT,
                CLAMP_TO_EDGE,
                CLAMP_TO_BORDER
            };

            enum class FilterType {
                MAG, MIN
            };

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

            template<typename T>
            void setTextureData(Format format, uint32_t width, uint32_t height, const T* data) {
                setTextureData(format, width, height,
                               typeToDataType(typeid(T)), data);
            }

            uint32_t getTextureUnitID();
            uint32_t getTextureID();

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
                                const void*);

        private:
            uint32_t m_texture_id;
            uint32_t m_texture_unit;
            Target m_target;
    };
}

#endif

