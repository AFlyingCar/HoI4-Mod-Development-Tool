
#include "ProvinceRenderingView.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "GLShaderSources.h"

#include "Logger.h"
#include "Util.h"

#include "MapDrawingAreaGL.h"

void MapNormalizer::GUI::GL::ProvinceRenderingView::init() {
    MapRenderingViewBase::init();

    // Build the outlines shader
    m_outline_shader = Program{Shader(Shader::Type::VERTEX,
                               ShaderSources::province_outline_vertex),
                        Shader(Shader::Type::FRAGMENT,
                               ShaderSources::province_outline_fragment)
                       };

    // Build the selection shader
    m_selection_shader = Program{Shader(Shader::Type::VERTEX,
                                 ShaderSources::province_selection_vertex),
                          Shader(Shader::Type::FRAGMENT,
                                 ShaderSources::province_selection_fragment)
                         };
}

void MapNormalizer::GUI::GL::ProvinceRenderingView::render() {
    // Render the normal map first
    MapRenderingViewBase::render();

    // Now render the outlines on top of it
#if 0
    {
        m_outline_shader.use();
        setupUniforms();
        m_outline_texture.activate();

        drawMapVAO();

        m_outline_shader.use(false);
    }
#endif
    
    // Then render the selected province (if there is one selected) on top of that
    //  But, obviously, only do so if there _is_ a selection
    if(auto selection = getOwningGLDrawingArea()->getSelection(); selection) {
        m_selection_shader.use();

        auto&& [bl, tr] = selection->bounding_box;
        auto&& [width, height] = calcDims(selection->bounding_box);

        auto posx = bl.x;
        auto posy = tr.y;

        float scale_factor = static_cast<float>(getOwningGLDrawingArea()->getScaleFactor());

        // Calculate a custom transform for selection
        glm::mat4 transform = glm::mat4{1.0f};
        transform = glm::translate(transform, glm::vec3{posx, posy, 0} *
                                              scale_factor);
        transform = glm::scale(transform, glm::vec3{width, height, 1} *
                                          glm::vec3{scale_factor, scale_factor, 1});

        // Set up the transformation matrix
        m_selection_shader.uniform("transform", transform);
        m_selection_shader.uniform("map_texture", 1);

        m_selection_texture.activate();

        // The drawn selection is still a square, so just go ahead and use the
        //  same VAO
        drawMapVAO();

        m_selection_shader.use(false);
    }
}

void MapNormalizer::GUI::GL::ProvinceRenderingView::onMapDataChanged(std::shared_ptr<const MapData> map_data)
{
    // First build the base map texture
    MapRenderingViewBase::onMapDataChanged(map_data);

#if 0
    auto [iwidth, iheight] = map_data->getDimensions();

    // Now build the outline texture
    {
        m_outline_texture.setTextureUnitID(Texture::Unit::TEX_UNIT0);

        m_outline_texture.bind();
        {
            // map_texture->setWrapping(Texture::Axis::S, Texture::WrapMode::CLAMP_TO_EDGE);
            // map_texture->setWrapping(Texture::Axis::T, Texture::WrapMode::CLAMP_TO_EDGE);

            m_outline_texture.setFiltering(Texture::FilterType::MAG, Texture::Filter::LINEAR);
            m_outline_texture.setFiltering(Texture::FilterType::MIN, Texture::Filter::LINEAR);

            m_outline_texture.setTextureData(Texture::Format::RED,
                                             iwidth, iheight, map_data->getProvinceOutlines().lock().get());
        }
        m_outline_texture.bind(false);
    }
#endif

    // Only build the selection texture when the selection has changed, not here
    //   as we have no data to actually build
}

void MapNormalizer::GUI::GL::ProvinceRenderingView::onSelectionChanged(std::optional<IMapDrawingAreaBase::SelectionInfo> selection)
{
    // If there is no selection, then don't actually do anything, we just won't
    //  render using the selection shader+texture
    if(selection) {
        WRITE_DEBUG("Rebuilding the selection texture...");

        auto [iwidth, iheight] = calcDims(selection->bounding_box);

        m_selection_texture.setTextureUnitID(Texture::Unit::TEX_UNIT1);

        m_selection_texture.bind();
        {
            // Use NEAREST rather than LINEAR to prevent weird outlines around
            //  the textures
            m_selection_texture.setFiltering(Texture::FilterType::MAG, Texture::Filter::NEAREST);
            m_selection_texture.setFiltering(Texture::FilterType::MIN, Texture::Filter::NEAREST);

            m_selection_texture.setTextureData(Texture::Format::RGBA,
                                               iwidth, iheight,
                                               selection->data.get());
        }
        m_selection_texture.bind(false);

        WRITE_DEBUG("Done.");
    }
}

auto MapNormalizer::GUI::GL::ProvinceRenderingView::getPrograms() -> ProgramList
{
    return { getMapProgram(), m_outline_shader, m_selection_shader };
}

