
#include "MainWindowDrawingAreaPart.h"

#include "Constants.h"
#include "Logger.h"
#include "Util.h"

#include "InterruptableScrolledWindow.h"
#include "SelectionManager.h"
#include "Driver.h"

void HMDT::GUI::MainWindowDrawingAreaPart::MainWindowDrawingAreaPart::buildDrawingArea()
{
    // Setup the box+area for the map image to render
    auto drawing_window = nameWidget("drawing_window",
                                     addWidget<InterruptableScrolledWindow>());

    auto setup_drawing_area = [](std::shared_ptr<IMapDrawingAreaBase> drawing_area)
    {
        drawing_area->setOnProvinceSelectCallback([](uint32_t x, uint32_t y) {
            if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
                auto& project = opt_project->get();
                auto& map_project = project.getMapProject();
                auto& history_project = project.getHistoryProject();

                auto map_data = project.getMapProject().getMapData();
                auto lmatrix = map_data->getProvinces().lock();

                // If the click happens outside of the bounds of the image, then
                //   deselect the province
                if(x > map_data->getWidth() || y > map_data->getHeight()) {
                    SelectionManager::getInstance().clearProvinceSelection();

                    return;
                }

                // Get the label for the pixel that got clicked on
                auto label = lmatrix[xyToIndex(map_data->getWidth(), x, y)];

                WRITE_DEBUG("Selecting province with ID ", label);
                SelectionManager::getInstance().selectProvince(label);

                // If this is a valid province, then select the state that it is
                //  a part of (if it is a part of one at all, that is)
                if(map_project.getProvinceProject().isValidProvinceID(label))
                {
                    auto& prov = map_project.getProvinceProject().getProvinceForID(label);

                    // Make sure we check for if the state ID is valid first so
                    //  that we deselect the state for provinces that aren't in
                    //  one
                    if(history_project.getStateProject().isValidStateID(prov.state))
                    {
                        SelectionManager::getInstance().selectState(prov.state);
                    } else {
                        SelectionManager::getInstance().clearStateSelection();
                    }
                } else {
                    SelectionManager::getInstance().clearStateSelection();
                }
            }
        });

        drawing_area->setOnMultiProvinceSelectionCallback([](uint32_t x, uint32_t y)
        {
            if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
                auto& project = opt_project->get();
                auto& map_project = project.getMapProject();

                auto map_data = map_project.getMapData();
                auto lmatrix = map_data->getProvinces().lock();

                // Multiselect out of bounds will simply not add to the selections
                if(x > map_data->getWidth() || y > map_data->getHeight()) {
                    return;
                }

                auto label = lmatrix[xyToIndex(map_data->getWidth(), x, y)];

                // Go over the list of already selected provinces and check if
                //  we have clicked on one that is _already_ selected
                const auto& selected_labels = SelectionManager::getInstance().getSelectedProvinceLabels();
                bool is_already_selected = selected_labels.count(label);

                // Perform a check here to make sure we don't go over the maximum
                //   number of selectable provinces if we are not deselecting one
                if(!is_already_selected && selected_labels.size() == MAX_SELECTED_PROVINCES)
                {
                    WRITE_WARN("Maximum number of provinces selected! Cannot select more than ",
                               MAX_SELECTED_PROVINCES, " at once!");
                    return;
                }

                // Do not mark this province as selected if we are deselecting it
                if(is_already_selected) {
                    SelectionManager::getInstance().removeProvinceSelection(label);
                } else {
                    SelectionManager::getInstance().addProvinceSelection(label);
                }

                // If this is a valid province, then select the state that it is
                //  a part of (if it is a part of one at all, that is)
                if(map_project.getProvinceProject().isValidProvinceID(label)) {
                    auto& prov = map_project.getProvinceProject().getProvinceForID(label);

                    // Don't bother checking for if it's valid or not, as
                    //  MapProject will do that for us
                    SelectionManager::getInstance().addStateSelection(prov.state);
                }
            }
        });
    };

    // Setup each drawing area type
    m_gl_drawing_area.reset(new GL::MapDrawingArea);
    setup_drawing_area(m_gl_drawing_area);

    // Setup initially enabled drawing area
    auto drawing_area = m_drawing_area = m_gl_drawing_area;

    // Set up a signal callback to zoom in and out when performing CTRL+ScrollWhell
    drawing_window->signalOnScroll().connect([drawing_area](GdkEventScroll* event)
    {
        if(event->state & GDK_CONTROL_MASK) {
            switch(event->direction) {
                case GDK_SCROLL_UP:
                    drawing_area->zoom(GL::MapDrawingArea::ZoomDirection::IN);
                    break;
                case GDK_SCROLL_DOWN:
                    drawing_area->zoom(GL::MapDrawingArea::ZoomDirection::OUT);
                    break;
                case GDK_SCROLL_SMOOTH:
                    drawing_area->zoom(-event->delta_y * ZOOM_FACTOR);
                    break;
                default: // We don't care about _LEFT or _RIGHT
                    break;
            }
            return true;
        }

        return false;
    });

    // Set up a signal callback to zoom in and out when pressing NumpadADD and NumpadSUB
    // CTRL+r will reset zoom level
    drawing_window->add_events(Gdk::KEY_PRESS_MASK);
    drawing_window->signal_key_press_event().connect([drawing_area](GdkEventKey* event)
    {
        switch(event->keyval) {
            case GDK_KEY_KP_Add:
                drawing_area->zoom(GL::MapDrawingArea::ZoomDirection::IN);
                break;
            case GDK_KEY_KP_Subtract:
                drawing_area->zoom(GL::MapDrawingArea::ZoomDirection::OUT);
                break;
            case GDK_KEY_r:
                if(event->state & GDK_CONTROL_MASK) {
                    drawing_area->zoom(GL::MapDrawingArea::ZoomDirection::RESET);
                }
                break;
        }

        return false;
    });

    // Place the drawing area in a scrollable window
    // drawing_window->add(*drawing_area->self());
    m_drawing_box.reset(new Gtk::Box());
    m_drawing_box->pack_start(*drawing_area->self(), Gtk::PACK_SHRINK);
    drawing_window->add(*m_drawing_box);
    drawing_window->show_all();
}

auto HMDT::GUI::MainWindowDrawingAreaPart::getDrawingArea()
    -> std::shared_ptr<IMapDrawingAreaBase>
{
    return m_drawing_area;
}

void HMDT::GUI::MainWindowDrawingAreaPart::setShouldDrawAdjacencies(bool should_draw_adjacencies)
{
    m_drawing_area->setShouldDrawAdjacencies(should_draw_adjacencies);
}

bool HMDT::GUI::MainWindowDrawingAreaPart::shouldDrawAdjacencies() const {
    return m_drawing_area->shouldDrawAdjacencies();
}

