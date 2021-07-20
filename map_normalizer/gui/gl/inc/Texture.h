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


            enum class Target {
                TEX_1D,
                TEX_2D,
            };

            enum class Format {
                RGB,
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

            template<typename T>
            void setTextureData(Format format, uint32_t width, uint32_t height, const T* data) {
                setTextureData(format, width, height,
                               typeToDataType(typeid(T)), data);
            }

            void activate();
            void deactivate();

            static std::queue<uint32_t> getAvailableTexUnitIds();

        protected:
            static uint32_t typeToDataType(const std::type_info&);

            static uint32_t targetToGLTarget(Target);
            static uint32_t formatToGLFormat(Format);
            static uint32_t axisToGLAxis(Axis);
            static uint32_t wrapToGLWrap(WrapMode);
            static uint32_t filterToGLFilter(Filter);
            static uint32_t filterTypeToGLFilterType(FilterType);

            void setTextureData(Format, uint32_t, uint32_t, uint32_t,
                                const void*);

        private:
            static constexpr uint32_t INVALID_TEXTURE_UNIT_ID = -1;

            static std::queue<uint32_t> available_tex_unit_ids;
            static uint32_t getNextAvailableTextureUnitID();
            static void freeTextureUnitID(uint32_t&);

            uint32_t m_texture_id;
            uint32_t m_texture_unit;
            Target m_target;
    };
}

#endif

