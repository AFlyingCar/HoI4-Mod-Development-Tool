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
void HMDT::GUI::GL::StateRenderingView::init() {
    MapRenderingViewBase::init();

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

void HMDT::GUI::GL::StateRenderingView::beginRender() {
    MapRenderingViewBase::beginRender();

    // Only update the state ID texture if the matrix has changed
    if(m_map_data != nullptr &&
       m_last_state_id_matrix_updated_tag != m_map_data->getStateIDMatrixUpdatedTag())
    {
        updateStateIDTexture();
    }
}

void HMDT::GUI::GL::StateRenderingView::updateStateIDTexture() {
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
void HMDT::GUI::GL::StateRenderingView::render() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Return early if m_map_data is null, as that likely means we do not have
    //  any state_id_matrix texture uploaded
    if(m_map_data == nullptr) return;

    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        auto& map_project = opt_project->get().getMapProject();
        auto& history_project = opt_project->get().getHistoryProject();

        getMapProgram().uniform("tex_dimensions", glm::ivec2(m_state_id_texture.getWidth(),
                                                             m_state_id_texture.getHeight()));
        getMapProgram().uniform("state_id_matrix", m_state_id_texture);
        m_state_id_texture.activate();

        // Set uniforms related to selection
        {
            getMapProgram().uniform("selection", getSelectionTexture());
            getSelectionTexture().activate();

            auto selections = getOwningGLDrawingArea()->getSelections();

            std::set<uint32_t> selection_ids;
            std::transform(selections.begin(), selections.end(),
                           std::inserter(selection_ids, selection_ids.begin()),
                           [&map_project](const auto& s) {
                               return map_project.getProvinceProject().isValidProvinceLabel(s.id) ?
                                      map_project.getProvinceProject().getProvinceForLabel(s.id).state :
                                      0;
                           });

            getMapProgram().uniform("selected_state_ids",
                                    std::vector<uint32_t>(selection_ids.begin(),
                                                          selection_ids.end()));
            getMapProgram().uniform("num_selected", static_cast<uint32_t>(selection_ids.size()));
        }

        // Render the normal map first for each state that exists
        for(auto&& [id, state] : history_project.getStateProject().getStates()) {
            if(state.provinces.empty()) continue;

            getMapProgram().uniform("state_color", state.color);
            getMapProgram().uniform("state_id", id);

            MapRenderingViewBase::render();
        }
    }

    // TODO: Render selections
}

void HMDT::GUI::GL::StateRenderingView::setupUniforms() {
    getMapProgram().uniform("state_id_matrix", getStateIDMatrixTexture());
}

const std::string& HMDT::GUI::GL::StateRenderingView::getVertexShaderSource() const
{
    return ShaderSources::stateview_vertex;
}

const std::string& HMDT::GUI::GL::StateRenderingView::getFragmentShaderSource() const
{
    return ShaderSources::stateview_fragment;
}

/**
 * @brief
 */
void HMDT::GUI::GL::StateRenderingView::onMapDataChanged(std::shared_ptr<const MapData> map_data)
{
    m_map_data = map_data;
}

/**
 * @brief If a selection is provided, then the selection area texture is rebuilt
 *
 * @param selection Info about the selected province, or std::nullopt
 */
void HMDT::GUI::GL::StateRenderingView::onSelectionChanged(std::optional<IMapDrawingAreaBase::SelectionInfo> selection)
{
    // TODO: We should show adjacency here if that option is turned on
}

auto HMDT::GUI::GL::StateRenderingView::getStateIDMatrixTexture() -> Texture& {
    return m_state_id_texture;
}

