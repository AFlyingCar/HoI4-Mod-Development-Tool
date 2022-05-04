
#include "MainWindowPropertiesPanePart.h"

#include "Driver.h"
#include "SelectionManager.h"

auto HMDT::GUI::MainWindowPropertiesPanePart::getProvincePropertiesPane()
    -> ProvincePropertiesPane&
{
    return *m_province_properties_pane;
}

auto HMDT::GUI::MainWindowPropertiesPanePart::getStatePropertiesPane()
    -> StatePropertiesPane&
{
    return *m_state_properties_pane;
}

auto HMDT::GUI::MainWindowPropertiesPanePart::getProvincePropertiesPane() const
    -> const ProvincePropertiesPane&
{
    return *m_province_properties_pane;
}

auto HMDT::GUI::MainWindowPropertiesPanePart::getStatePropertiesPane() const
    -> const StatePropertiesPane&
{
    return *m_state_properties_pane;
}

/**
 * @brief Builds the properties pane, which is where properties about a selected
 *        province/state will go.
 *
 * @return The frame that the properties will be placed into
 */
Gtk::Frame* HMDT::GUI::MainWindowPropertiesPanePart::buildPropertiesPane(Gtk::Paned* pane)
{
    Gtk::Frame* properties_frame = new Gtk::Frame();
    setActiveChild(properties_frame);

    pane->pack2(*properties_frame, false, false);

    // Province Tab
    auto properties_tab = addActiveWidget<Gtk::Notebook>();

    {
        m_province_properties_pane.reset(new ProvincePropertiesPane);
        m_province_properties_pane->init();
        properties_tab->append_page(m_province_properties_pane->getParent(), "Province");
    }

    // State Tab
    {
        m_state_properties_pane.reset(new StatePropertiesPane);
        m_state_properties_pane->init();

        properties_tab->append_page(m_state_properties_pane->getParent(), "State");
    }

    pane->property_position().signal_changed().connect([this]() {
        if(m_province_properties_pane) {
            m_province_properties_pane->onResize();
        }

        if(m_state_properties_pane) {
            m_state_properties_pane->onResize();
        }
    });

    setActiveChild();

    // Finish extra setup in case we have a project loaded
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        auto& project = opt_project->get();
        auto& map_project = project.getMapProject();

        // Provinces Tab
        if(auto selected = SelectionManager::getInstance().getSelectedProvinces(); !selected.empty())
        {
            auto* province = &selected.begin()->get();
            auto label = province->id;
            m_province_properties_pane->setProvince(province,
                                                    map_project.getPreviewData(label),
                                                    selected.size() > 1);
        }
    }

    return properties_frame;
}

void HMDT::GUI::MainWindowPropertiesPanePart::onProjectOpened() {
    m_province_properties_pane->onProjectOpened();
}

