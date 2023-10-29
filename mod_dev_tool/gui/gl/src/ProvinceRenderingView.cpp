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

#include "Driver.h"

#include "MapDrawingAreaGL.h"

/**
 * @brief Initializes a ProvinceRenderingView
 */
void HMDT::GUI::GL::ProvinceRenderingView::init() {
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

void HMDT::GUI::GL::ProvinceRenderingView::beginRender() {
    MapRenderingViewBase::beginRender();

    m_texture.activate();
}

/**
 * @brief Renders the base map, then the current selection, and then the map
 *        outlines on top of that.
 */
void HMDT::GUI::GL::ProvinceRenderingView::render() {
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
        m_selection_shader.uniform("selection", getSelectionTexture());
        m_selection_shader.uniform("label_matrix", getLabelTexture());

        // All other uniforms
        std::vector<uint32_t> selection_ids;
        std::transform(selections.begin(),
                       selections.end(),
                       std::back_inserter(selection_ids),
                       [](const auto& s) {
                           return s.id.hash();
                       });

        m_selection_shader.uniform("province_labels", selection_ids);
        m_selection_shader.uniform("num_selected", static_cast<uint32_t>(selection_ids.size()));

        m_selection_shader.uniform("selection_color", Color{ 255, 0, 0 });

        getSelectionTexture().activate();
        getLabelTexture().activate();

        // The drawn selection is still a square, so just go ahead and use the
        //  same VAO
        drawMapVAO();

        // Render adjacencies only if we are selecting a single province
        if(getOwningGLDrawingArea()->shouldDrawAdjacencies() && selections.size() == 1) {
            if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
                auto& map_project = opt_project->get().getMapProject();

                setupUniforms();

                // Set up the textures
                m_selection_shader.uniform("selection", getSelectionTexture());
                m_selection_shader.uniform("label_matrix", getLabelTexture());

                // All other uniforms
                std::set<uint32_t> adjacent_ids;
                {
                    // We only have one selection here, but do a loop anyway in case
                    //   I change my mind on doing that later
                    for(auto&& selection_info : selections) {
                        if(!map_project.getProvinceProject().isValidProvinceID(selection_info.id))
                        {
                            WRITE_WARN("Unable to render adjacency for invalid province ID ", selection_info.id);
                            continue;
                        }

                        const auto& selection = map_project.getProvinceProject().getProvinceForID(selection_info.id);

                        std::transform(selection.adjacent_provinces.begin(),
                                       selection.adjacent_provinces.end(),
                                       std::inserter(adjacent_ids, adjacent_ids.begin()),
                                       [](auto& id) -> uint32_t {
                                           return id.hash();
                                       });
                    }
                }

                m_selection_shader.uniform("province_labels",
                                           std::vector<uint32_t>(adjacent_ids.begin(),
                                                                 adjacent_ids.end()));
                m_selection_shader.uniform("num_selected", static_cast<uint32_t>(adjacent_ids.size()));

                m_selection_shader.uniform("selection_color", Color{ 255, 0, 255 });

                getSelectionTexture().activate();
                getLabelTexture().activate();

                // The drawn selection is still a square, so just go ahead and use the
                //  same VAO
                drawMapVAO();
            }
        }

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

void HMDT::GUI::GL::ProvinceRenderingView::setupUniforms() {
    getMapProgram().uniform("map_texture", m_texture);
}

/**
 * @brief Rebuilds the outline province outlines texture
 */
void HMDT::GUI::GL::ProvinceRenderingView::onMapDataChanged(std::shared_ptr<const MapData> map_data)
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
                                     iwidth, iheight,
                                     map_data->getProvinceColors().lock().get());
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
void HMDT::GUI::GL::ProvinceRenderingView::onSelectionChanged(std::optional<IMapDrawingAreaBase::SelectionInfo> selection)
{
    // TODO: We should show adjacency here if that option is turned on
}

auto HMDT::GUI::GL::ProvinceRenderingView::getPrograms() -> ProgramList {
    return { getMapProgram(), m_outline_shader, m_selection_shader };
}

const std::string& HMDT::GUI::GL::ProvinceRenderingView::getVertexShaderSource() const
{
    return ShaderSources::provinceview_vertex;
}

const std::string& HMDT::GUI::GL::ProvinceRenderingView::getFragmentShaderSource() const
{
    return ShaderSources::provinceview_fragment;
}

auto HMDT::GUI::GL::ProvinceRenderingView::getMapTexture() -> Texture& {
    return m_texture;
}

auto HMDT::GUI::GL::ProvinceRenderingView::getLabelTexture() -> Texture& {
    return m_label_texture;
}

