/**
 * @file StateRenderingView.cpp
 *
 * @file Defines the StateRenderingView class
 */

#include "StateRenderingView.h"

#include <GL/glew.h>

#include "GLShaderSources.h"

#include "Logger.h"
#include "Util.h"

#include "Driver.h"

#include "MapDrawingAreaGL.h"
#include "GuiUtils.h"

/**
 * @brief Initializes a StateRenderingView
 */
void MapNormalizer::GUI::GL::StateRenderingView::init() {
    MapRenderingViewBase::init();

#if 0
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
#endif


    {
        // We set these values away from the texture, as there is no real need
        //  to set them multiple times.
        m_state_id_texture.setTextureUnitID(Texture::Unit::TEX_UNIT2);

        m_state_id_texture.setWrapping(Texture::Axis::S, Texture::WrapMode::REPEAT);
        m_state_id_texture.setWrapping(Texture::Axis::T, Texture::WrapMode::REPEAT);

        m_state_id_texture.setFiltering(Texture::FilterType::MAG, Texture::Filter::NEAREST);
        m_state_id_texture.setFiltering(Texture::FilterType::MIN, Texture::Filter::NEAREST);

        updateStateIDTexture();
    }
}

void MapNormalizer::GUI::GL::StateRenderingView::beginRender() {
    MapRenderingViewBase::beginRender();

    // Only update the state ID texture if the matrix has changed
    if(m_map_data != nullptr &&
       m_last_state_id_matrix_updated_tag != m_map_data->getStateIDMatrixUpdatedTag())
    {
        updateStateIDTexture();
    }
}

void MapNormalizer::GUI::GL::StateRenderingView::updateStateIDTexture() {
    if(m_map_data != nullptr) {
        if(auto state_id_mtx = m_map_data->getStateIDMatrix(); !state_id_mtx.expired())
        {
            auto [iwidth, iheight] = m_map_data->getDimensions();

            WRITE_DEBUG("Updating State ID Matrix Texture.");

            m_state_id_texture.bind();
            {
                m_state_id_texture.setTextureData(Texture::Format::RED32UI,
                                                  iwidth, iheight,
                                                  state_id_mtx.lock().get(),
                                                  GL_RED_INTEGER);
            }
            m_state_id_texture.bind(false);

            // Make sure we update what the current tag is
            m_last_state_id_matrix_updated_tag = m_map_data->getStateIDMatrixUpdatedTag();
        }
    }
}

/**
 * @brief Renders the base map, then the current selection, and then the map
 *        outlines on top of that.
 */
void MapNormalizer::GUI::GL::StateRenderingView::render() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Return early if m_map_data is null, as that likely means we do not have
    //  any state_id_matrix texture uploaded
    if(m_map_data == nullptr) return;

    if(auto opt_map_project = Driver::getInstance().getProject(); opt_map_project)
    {
        auto& map_project = opt_map_project->get().getMapProject();

        getMapProgram().uniform("tex_dimensions", glm::ivec2(m_state_id_texture.getWidth(),
                                                             m_state_id_texture.getHeight()));
        getMapProgram().uniform("state_id_matrix", m_state_id_texture);
        m_state_id_texture.activate();

        // Render the normal map first for each state that exists
        for(auto&& [id, state] : map_project.getStates()) {
            getMapProgram().uniform("state_color", state.color);
            getMapProgram().uniform("state_id", id);

            MapRenderingViewBase::render();
        }
    }

    // TODO: Render selections
}

void MapNormalizer::GUI::GL::StateRenderingView::setupUniforms() {
    getMapProgram().uniform("state_id_matrix", getStateIDMatrixTexture());
}

const std::string& MapNormalizer::GUI::GL::StateRenderingView::getVertexShaderSource() const
{
    return ShaderSources::stateview_vertex;
}

const std::string& MapNormalizer::GUI::GL::StateRenderingView::getFragmentShaderSource() const
{
    return ShaderSources::stateview_fragment;
}

/**
 * @brief
 */
void MapNormalizer::GUI::GL::StateRenderingView::onMapDataChanged(std::shared_ptr<const MapData> map_data)
{
    m_map_data = map_data;
}

/**
 * @brief If a selection is provided, then the selection area texture is rebuilt
 *
 * @param selection Info about the selected province, or std::nullopt
 */
void MapNormalizer::GUI::GL::StateRenderingView::onSelectionChanged(std::optional<IMapDrawingAreaBase::SelectionInfo> selection)
{
    // TODO: We should show adjacency here if that option is turned on
}

auto MapNormalizer::GUI::GL::StateRenderingView::getStateIDMatrixTexture()
    -> Texture&
{
    return m_state_id_texture;
}

