
#include "ItemAddFunctions.h"

#include <libintl.h>

#include "gtkmm.h"

#include "Constants.h"
#include "StatusCodes.h"

#include "Driver.h"
#include "MainWindow.h"
#include "GraphicalDebugger.h"
#include "ProgressBarDialog.h"

namespace {
    struct AddProvinceMapData {
        //! The path that was added
        std::filesystem::path path;

        //! The progress bar dialog cancel button
        Gtk::Button* cancel_button;

        //! The progress bar dialog done button
        Gtk::Button* done_button;

        //! The drawing area of the window
        std::shared_ptr<HMDT::GUI::IMapDrawingAreaBase> drawing_area;

        //! The shared map data
        std::shared_ptr<HMDT::MapData> map_data;

        //! The shape finder
        std::shared_ptr<HMDT::ShapeFinder> shape_finder;

        //! The progress bar dialog window
        std::shared_ptr<HMDT::GUI::ProgressBarDialog> progress_dialog;

        //! A shared rectangle for communicating with the dispatcher
        std::shared_ptr<HMDT::Rectangle> rectangle;

        //! A shared boolean for communicating if an estop was triggered
        std::shared_ptr<bool> did_estop;

        //! The id of the Dispatcher for updating UI elements
        uint32_t ui_dispatcher_id;
    };
}

////////////////////////////////////////////////////////////////////////////////
// Province Maps

auto HMDT::GUI::initAddProvinceMap(Window& window,
                                   const std::vector<std::filesystem::path>& paths)
    -> Maybe<std::any>
{
    MainWindow& main_window = dynamic_cast<MainWindow&>(window);

    if(paths.empty()) {
        WRITE_ERROR("Expected at least 1 path, got 0.");
        RETURN_ERROR(STATUS_EXPECTED_PATHS);
    } else if(paths.size() > 1) {
        WRITE_WARN("Got multiple paths, but we only expected 1. Only going to use the first one.");
    }

    const auto& path = paths.front();

    // First, load the image into memory
    BitMap* image = nullptr;
    if(image = readBMP(path); image == nullptr) {
        WRITE_ERROR("Failed to read bitmap from ", path);
        return false;
    }

    // We now need a new array that the graphics worker can use to display the
    //  rendered image
    std::shared_ptr<MapData> map_data(new MapData(image->info_header.width,
                                                  image->info_header.height));


    // Make sure that the drawing area is sized correctly to draw the entire
    //  image
    window.resize(image->info_header.width, image->info_header.height);

    // Set up the data structure here
    AddProvinceMapData data {
        path /* path */,
        nullptr /* cancel_button */,
        nullptr /* done_button */,
        main_window.getDrawingArea() /* drawing_area */,
        map_data /* map_data */,
        std::make_shared<ShapeFinder>(image, GraphicsWorker::getInstance(), map_data) /* shape_finder */,
        std::make_shared<ProgressBarDialog>(window, gettext("Loading..."), "", true) /* progress_dialog */,
        std::shared_ptr<Rectangle>(new Rectangle{0, 0, static_cast<uint32_t>(image->info_header.width),
                                                       static_cast<uint32_t>(image->info_header.height)}) /* rectangle */,
        std::make_shared<bool>(false) /* did_estop */,
        0 /* ui_dispatcher_id */
    };

    // Set up the drawing area's map data
    data.drawing_area->setMapData(map_data);

    // Set up the Progress Bar Dialog
    {
        data.progress_dialog->setShowText(true);

        // We add the buttons manually here because we need to be able to set their
        //  sensitivity manually
        // TODO: Do we even want a Done button? Or should we just close the
        //  dialog when we're done?
        data.done_button = data.progress_dialog->add_button("OK", Gtk::RESPONSE_OK);
        data.done_button->set_sensitive(false);

        data.cancel_button = data.progress_dialog->add_button("Cancel", Gtk::RESPONSE_CANCEL);

        data.progress_dialog->show_all();
    }

    // Set up the graphics worker callback
    auto last_stage = data.shape_finder->getStage();

    // Set up the dispatcher to run all Gtk code that needs to be updated during
    //   the runtime of the thread
    window.setupDispatcher([data, last_stage](uint32_t) mutable {
        data.drawing_area->graphicsUpdateCallback(*data.rectangle);

        auto stage = data.shape_finder->getStage();
        float fstage = static_cast<uint32_t>(stage);
        float fraction = fstage / static_cast<uint32_t>(ShapeFinder::Stage::DONE);

        // TODO: Do we want to calculate a more precise fraction based on
        //   progress during a given stage?

        if(stage != last_stage) {
            last_stage = stage;
            data.progress_dialog->setText(toString(stage));
        }
        data.progress_dialog->setFraction(fraction);
    }).andThen([&data](uint32_t id) {
        data.ui_dispatcher_id = id;
    });

    WRITE_DEBUG("Data is holding dispatcher id=", data.ui_dispatcher_id);

    // Set up the graphical worker
    // This is how ShapeFinder communicates to the DrawingArea about how to
    //   update the screen
    {
        auto& worker = GraphicsWorker::getInstance();

        // No memory leak here, since the data will get deleted either at program
        //  exit, or when the next value is loaded
        worker.init(map_data);
        worker.resetDebugData();

        // Call update immediately to clear the entire screen
        worker.updateCallback({0, 0, static_cast<uint32_t>(image->info_header.width),
                                     static_cast<uint32_t>(image->info_header.height)});

        // TODO: Is it safe to capture the window by reference? It should always
        //   exist while this callback might be used
        worker.setWriteCallback([rect_ptr = data.rectangle,
                                 ui_dispatcher_id = data.ui_dispatcher_id,
                                 &window](const Rectangle& r)
        {
            // Set the rectangle such that the dispatcher will be able to
            //   access it
            *rect_ptr = r;

            auto res = window.notifyDispatcher(ui_dispatcher_id);
            WRITE_IF_ERROR(res);
        });
    }

    return data;
}

HMDT::MaybeVoid HMDT::GUI::postStartAddProvinceMap(Window& window, std::any data)
{
    AddProvinceMapData apd_data = std::any_cast<AddProvinceMapData>(data);

    // Run the progress bar dialog
    // If the user cancels the action, then we need to kill sf_worker ASAP
    if(auto response = apd_data.progress_dialog->run();
            response == Gtk::RESPONSE_DELETE_EVENT ||
            response == Gtk::RESPONSE_CANCEL)
    {
        apd_data.shape_finder->estop();
        *apd_data.did_estop = true;

        WRITE_WARN("ShapeFinder ESTOP triggered. Halting shape finder asap!");
        return STATUS_SHAPEFINDER_ESTOP;
    }
    
    return STATUS_SUCCESS;
}

/**
 * @brief Continues to add a province map 
 *
 * @param window
 * @param data
 *
 * @return 
 */
HMDT::MaybeVoid HMDT::GUI::addProvinceMapWorker(Window& window, std::any data) {
    AddProvinceMapData apd_data = std::any_cast<AddProvinceMapData>(data);
    auto& worker = GraphicsWorker::getInstance();

    // Wait for the entire algorithm to run to completion
    auto shapes = apd_data.shape_finder->findAllShapes();

    auto* image = apd_data.shape_finder->getImage();

    auto prov_ptr = apd_data.map_data->getProvinces().lock();

    // Redraw the new image so we can properly show how it should look in the
    //  final output
    // TODO: Do we still want to do this here? Would it not be better to do
    //  it later on?
    for(auto&& shape : shapes) {
        for(auto&& pixel : shape.pixels) {
            // Write to both the output data and into the displayed data
            writeColorTo(prov_ptr.get(), image->info_header.width,
                         pixel.point.x, pixel.point.y,
                         shape.unique_color);

            worker.writeDebugColor(pixel.point.x, pixel.point.y,
                                   shape.unique_color);
        }
    }

    // One final callback update so that the map drawer has the latest
    //  graphical information
    worker.updateCallback({0, 0, static_cast<uint32_t>(image->info_header.width),
                                 static_cast<uint32_t>(image->info_header.height)});

    WRITE_INFO("Detected ", shapes.size(), " shapes.");

    // Disable the cancel button (we've already finished), and enable the
    //  done button so that the user can close the box and move on
    apd_data.done_button->set_sensitive(true);
    apd_data.cancel_button->set_sensitive(false);

    return STATUS_SUCCESS;
}

HMDT::MaybeVoid HMDT::GUI::endAddProvinceMap(Window& window, std::any data) {
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        auto& project = opt_project->get();

        AddProvinceMapData apd_data = std::any_cast<AddProvinceMapData>(data);
        auto& worker = GraphicsWorker::getInstance();

        // Make sure we destroy our own dispatcher
        WRITE_DEBUG("Tearing down the ui dispatcher.");
        auto res = window.teardownDispatcher(apd_data.ui_dispatcher_id);
        RETURN_IF_ERROR(res);

        // Note: We reset the zoom here so that we can ensure that the drawing
        //  area actually updates the image.
        // TODO: Why do I have to do this? What about this PR has caused this
        //  to suddenly be required?
        apd_data.drawing_area->resetZoom();

        worker.resetWriteCallback();

        // Don't finish importing if we stopped early
        if(*apd_data.did_estop) {
            return STATUS_SHAPEFINDER_ESTOP;
        }

        WRITE_DEBUG("Assigning the found data to the map project.");
        project.getMapProject().import(*apd_data.shape_finder, apd_data.map_data);

        WRITE_INFO("Calculating coastal provinces...");
        project.getMapProject().calculateCoastalProvinces();

        // We need to re-assign the data into the drawing area to update the
        //   texture on the drawing area
        // TODO: For OpenGL we should probably use a Pixel Buffer Object (PBO)
        //   so that the texture data doesn't have to be uploaded twice and
        //   instead can be drawn as it is getting generated
        WRITE_DEBUG("Assigning the found data into the drawing area.");
        apd_data.drawing_area->setMapData(apd_data.map_data);

        std::filesystem::path input_root = project.getInputsRoot();

        WRITE_DEBUG("Copying the province map into ", input_root);
        if(!std::filesystem::exists(input_root)) {
            std::filesystem::create_directory(input_root);
        }

        // TODO: We should actually do two things here:
        //  1) If filename == input_full_path: do nothing
        //  2) Otherwise ask if they want to overrite/replace the province map
        if(auto input_full_path = input_root / INPUT_PROVINCEMAP_FILENAME;
           !std::filesystem::exists(input_full_path))
        {
            std::filesystem::copy_file(apd_data.path, input_full_path);
        }
    } else {
        return STATUS_NO_PROJECT_LOADED;
    }

    return STATUS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

auto HMDT::GUI::addHeightMap(Window& window,
                             const std::vector<std::filesystem::path>& paths)
    -> Maybe<std::any>
{
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        auto& project = opt_project->get();

        project.getMapProject().getHeightMapProject().setPromptCallback(
            [&window](const std::string& message,
                      const std::vector<std::string>& opts,
                      const Project::IProject::PromptType& /*type*/)
                -> uint32_t
            {
                // TODO: We should make use of the PromptType here

                // Create a dialog with no buttons, as we will just add the ones
                //   specified in opts.
                Gtk::MessageDialog dialog(window,
                                          message,
                                          true /* use_markup */,
                                          Gtk::MESSAGE_QUESTION /* type */,
                                          Gtk::BUTTONS_NONE /* buttons */);
                for(uint32_t i = 0; i < opts.size(); ++i) {
                    dialog.add_button(opts.at(i), i);
                }

                return dialog.run();
            });

        WRITE_DEBUG("Loading new heightmap into HeightMapProject.");
        auto res = project.getMapProject().getHeightMapProject().loadFile(paths.front());

        // Make sure we reset before checking for errors
        project.getMapProject().getHeightMapProject().resetPromptCallback();

        RETURN_IF_ERROR(res);
    }

    return STATUS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

auto HMDT::GUI::addRivers(Window& window,
                          const std::vector<std::filesystem::path>& paths)
    -> Maybe<std::any>
{
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        auto& project = opt_project->get();

        project.getMapProject().getRiversProject().setPromptCallback(
            [&window](const std::string& message,
                      const std::vector<std::string>& opts,
                      const Project::IProject::PromptType& /*type*/)
                -> uint32_t
            {
                // TODO: We should make use of the PromptType here

                // Create a dialog with no buttons, as we will just add the ones
                //   specified in opts.
                Gtk::MessageDialog dialog(window,
                                          message,
                                          true /* use_markup */,
                                          Gtk::MESSAGE_QUESTION /* type */,
                                          Gtk::BUTTONS_NONE /* buttons */);
                for(uint32_t i = 0; i < opts.size(); ++i) {
                    dialog.add_button(opts.at(i), i);
                }

                return dialog.run();
            });

        WRITE_DEBUG("Loading new rivers map into RiversProject.");
        auto res = project.getMapProject().getRiversProject().loadFile(paths.front());

        // Make sure we reset before checking for errors
        project.getMapProject().getRiversProject().resetPromptCallback();

        RETURN_IF_ERROR(res);
    }

    return STATUS_SUCCESS;
}

