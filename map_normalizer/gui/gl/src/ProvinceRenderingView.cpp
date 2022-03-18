/**
 * @file ProvinceRenderingView.cpp
 *
 * @file Defines the ProvinceRenderingView class
 */

#include "ProvinceRenderingView.h"

#include <GL/glew.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "GLShaderSources.h"

#include "Logger.h"
#include "Util.h"

#include "Driver.h"

#include "MapDrawingAreaGL.h"
#include "GuiUtils.h"

/**
 * @brief Initializes a ProvinceRenderingView
 */
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

    // Build default texture
    {
        // TODO: This path shouldn't be hardcoded, we should get it instead from
        //   the build system/from a ResourceManager
        auto stream = Driver::getInstance().getResources()->open_stream("/com/aflyingcar/MapNormalizerTools/textures/selection.bmp");

        std::unique_ptr<BitMap> selection_bmp(new BitMap);
        if(readBMP(stream, selection_bmp.get()) == nullptr) {
            WRITE_ERROR("Failed to load selection texture!");

            m_selection_texture.setTextureUnitID(Texture::Unit::TEX_UNIT3);

            m_selection_texture.bind();

            m_selection_texture.setTextureData(Texture::Format::RGBA, 1, 1,
                                               (uint8_t*)0);
            m_selection_texture.bind(false);
        } else {
            auto iwidth = selection_bmp->info_header.width;
            auto iheight = selection_bmp->info_header.height;

            m_selection_texture.setTextureUnitID(Texture::Unit::TEX_UNIT3);

            m_selection_texture.bind();
            {
                // Use NEAREST rather than LINEAR to prevent weird outlines around
                //  the textures
                m_selection_texture.setFiltering(Texture::FilterType::MAG, Texture::Filter::LINEAR);
                m_selection_texture.setFiltering(Texture::FilterType::MIN, Texture::Filter::LINEAR);

                m_selection_texture.setWrapping(Texture::Axis::S, Texture::WrapMode::REPEAT);
                m_selection_texture.setWrapping(Texture::Axis::T, Texture::WrapMode::REPEAT);

                m_selection_texture.setTextureData(Texture::Format::RGBA,
                                                   iwidth, iheight,
                                                   selection_bmp->data);
            }
            m_selection_texture.bind(false);
        }
    }
}

void MapNormalizer::GUI::GL::ProvinceRenderingView::beginRender() {
    MapRenderingViewBase::beginRender();

    m_texture.activate();
}

/**
 * @brief Renders the base map, then the current selection, and then the map
 *        outlines on top of that.
 */
void MapNormalizer::GUI::GL::ProvinceRenderingView::render() {
    // Render the normal map first
    MapRenderingViewBase::render();

    // Then render the selected province (if there is one selected) on top of that
    //  But, obviously, only do so if there _is_ a selection
    if(auto selections = getOwningGLDrawingArea()->getSelections(); !selections.empty())
    {
        m_selection_shader.use();

        setupUniforms();

        // TODO: What the hell is wrong with the transform?!?!?!
        float scale_factor = static_cast<float>(getOwningGLDrawingArea()->getScaleFactor());
        glm::mat4 transform = glm::mat4{1.0f};
        transform = glm::scale(transform, glm::vec3{scale_factor, scale_factor, 1});
        // m_selection_shader.uniform("transform", transform);

        // Set up the textures
        m_selection_shader.uniform("selection", m_selection_texture);
        m_selection_shader.uniform("label_matrix", getLabelTexture());

        // All other uniforms
        std::vector<uint32_t> selection_ids;
        std::transform(selections.begin(), selections.end(), std::back_inserter(selection_ids), [](const auto& s) { return s.id; });

        m_selection_shader.uniform("province_labels", selection_ids);
        m_selection_shader.uniform("num_selected", static_cast<uint32_t>(selection_ids.size()));

        m_selection_texture.activate();
        getLabelTexture().activate();

        // The drawn selection is still a square, so just go ahead and use the
        //  same VAO
        drawMapVAO();

        m_selection_shader.use(false);
    }

    // Now render the outlines on top of everything
    {
        m_outline_shader.use();
        setupUniforms();

        float scale_factor = static_cast<float>(getOwningGLDrawingArea()->getScaleFactor());
        glm::mat4 transform = glm::mat4{1.0f};
        transform = glm::scale(transform, glm::vec3{scale_factor, scale_factor, 1});
        m_selection_shader.uniform("transform", transform);
        m_outline_shader.uniform("map_texture", m_outline_texture);

        m_outline_texture.activate();

        drawMapVAO();

        m_outline_shader.use(false);
    }
}

void MapNormalizer::GUI::GL::ProvinceRenderingView::setupUniforms() {
    getMapProgram().uniform("map_texture", m_texture);
}

/**
 * @brief Rebuilds the outline province outlines texture
 */
void MapNormalizer::GUI::GL::ProvinceRenderingView::onMapDataChanged(std::shared_ptr<const MapData> map_data)
{
    // First build the base map texture
    {
        auto [iwidth, iheight] = map_data->getDimensions();

        m_texture.setTextureUnitID(Texture::Unit::TEX_UNIT0);

        m_texture.bind();
        {
            // map_texture->setWrapping(Texture::Axis::S, Texture::WrapMode::CLAMP_TO_EDGE);
            // map_texture->setWrapping(Texture::Axis::T, Texture::WrapMode::CLAMP_TO_EDGE);

            m_texture.setFiltering(Texture::FilterType::MAG, Texture::Filter::LINEAR);
            m_texture.setFiltering(Texture::FilterType::MIN, Texture::Filter::LINEAR);

            m_texture.setTextureData(Texture::Format::RGB,
                                      iwidth, iheight, map_data->getProvinces().lock().get());
        }
        m_texture.bind(false);

        ////////////////////////////////////////////////////////////////////////////

        m_label_texture.setTextureUnitID(Texture::Unit::TEX_UNIT1);

        m_label_texture.bind();
        {
            WRITE_DEBUG("Building label matrix texture.");
            m_label_texture.setWrapping(Texture::Axis::S, Texture::WrapMode::REPEAT);
            m_label_texture.setWrapping(Texture::Axis::T, Texture::WrapMode::REPEAT);

            m_label_texture.setFiltering(Texture::FilterType::MAG, Texture::Filter::NEAREST);
            m_label_texture.setFiltering(Texture::FilterType::MIN, Texture::Filter::NEAREST);

            m_label_texture.setTextureData(Texture::Format::RED32UI,
                                           iwidth, iheight,
                                           map_data->getLabelMatrix().lock().get(),
                                           GL_RED_INTEGER);
        }
        m_label_texture.bind(false);
    }

    auto [iwidth, iheight] = map_data->getDimensions();

    // Now build the outline texture
    {
        m_outline_texture.setTextureUnitID(Texture::Unit::TEX_UNIT4);

        m_outline_texture.bind();
        {
            // map_texture->setWrapping(Texture::Axis::S, Texture::WrapMode::CLAMP_TO_EDGE);
            // map_texture->setWrapping(Texture::Axis::T, Texture::WrapMode::CLAMP_TO_EDGE);

            m_outline_texture.setFiltering(Texture::FilterType::MAG, Texture::Filter::LINEAR);
            m_outline_texture.setFiltering(Texture::FilterType::MIN, Texture::Filter::LINEAR);

            m_outline_texture.setTextureData(Texture::Format::RGBA,
                                             iwidth, iheight, map_data->getProvinceOutlines().lock().get());
        }
        m_outline_texture.bind(false);
    }

    // Only build the selection texture when the selection has changed, not here
    //   as we have no data to actually build
}

/**
 * @brief If a selection is provided, then the selection area texture is rebuilt
 *
 * @param selection Info about the selected province, or std::nullopt
 */
void MapNormalizer::GUI::GL::ProvinceRenderingView::onSelectionChanged(std::optional<IMapDrawingAreaBase::SelectionInfo> selection)
{
    // TODO: We should show adjacency here if that option is turned on
}

auto MapNormalizer::GUI::GL::ProvinceRenderingView::getPrograms() -> ProgramList
{
    return { getMapProgram(), m_outline_shader, m_selection_shader };
}

const std::string& MapNormalizer::GUI::GL::ProvinceRenderingView::getVertexShaderSource() const
{
    return ShaderSources::provinceview_vertex;
}

const std::string& MapNormalizer::GUI::GL::ProvinceRenderingView::getFragmentShaderSource() const
{
    return ShaderSources::provinceview_fragment;
}

auto MapNormalizer::GUI::GL::ProvinceRenderingView::getMapTexture() -> Texture& {
    return m_texture;
}

auto MapNormalizer::GUI::GL::ProvinceRenderingView::getLabelTexture() -> Texture&
{
    return m_label_texture;
}

