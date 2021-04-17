/**
 * @file GraphicalDebugger.cpp
 *
 * @brief File for managing the graphical debugger window of the program.
 */

#include "GraphicalDebugger.h"

#include <mutex> // std::mutex
#include <chrono> // std::chrono
#include <iostream> // std::cerr
#include <thread> // std::this_thread

// Define this to prevent the #define-ing of the min() and max() functions.
#define NOMINMAX
#include <cmath>
#include <algorithm>

#include "BitMap.h"
#include "Options.h"
#include "Logger.h"
#include "Util.h"

// Conditional macro for enabling/disabling the graphical debugger entirely
#ifdef ENABLE_GRAPHICS

# include <SDL.h> // SDL_*
# include <condition_variable>
# include <atomic>

/**
 * @brief The mutex for preventing writeDebugColor from writing while
 *        graphicsWorker is drawing the image.
 */
static std::mutex graphics_debug_mutex;

//! The mutex for watching if the main thread should unpause.
static std::mutex main_thread_should_unpause_mutex;

//! The conditional variable for watching if the main thread should unpause.
static std::condition_variable main_thread_should_unpause_cv;

//! The atomic boolean for tracking if the main thread should be paused.
static std::atomic<bool> main_thread_should_pause;
#endif

//! The maximum width of the created SDL window.
static const auto MAX_SCREEN_WIDTH = 16384;
//! The maximum height of the created SDL window.
static const auto MAX_SCREEN_HEIGHT = 16384;

//! The minimum width of the created SDL window.
static const auto MIN_SCREEN_WIDTH = 100;
//! The minimum height of the created SDL window.
static const auto MIN_SCREEN_HEIGHT = 100;

/**
 * @brief The number of pixels required before should_sleep will be false by
 *        default.
 * @details Value is for a 512 x 512 dimension image.
 */
static const auto NUM_PIX_REQ_SLEEP = 262144;

#ifdef ENABLE_GRAPHICS
/**
 * @brief Whether or not the main thread should perform a sleep operation to
 *        artificially slow down the program.
 */
static bool should_sleep = true;
#endif

MapNormalizer::GraphicsWorker& MapNormalizer::GraphicsWorker::getInstance() {
    static GraphicsWorker instance;

    return instance;
}

/*
 * @brief Initializes the GraphicsWorker
 *
 * @param image The image being debugged
 * @param debug_data The pixel data to display
 */
void MapNormalizer::GraphicsWorker::init(BitMap* image,
                                         unsigned char* debug_data)
{
    m_image = image;
    m_debug_data = debug_data;
}

/**
 * @brief Worker function which manages displaying graphical debugging data.
 *
 * @param done A boolean for this to know when the algorithm is done.
 */
void MapNormalizer::GraphicsWorker::work(bool& done)
{
#ifndef ENABLE_GRAPHICS
    (void)done;
#else

    // Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL! " << SDL_GetError() << std::endl;
        return;
    }

    SDL_DisplayMode current;
    if(SDL_GetCurrentDisplayMode(0, &current) != 0) {
        std::cerr << "Failed to get current display mode! " << SDL_GetError()
                  << std::endl;
        return;
    }

    // (8/10) of the current screen resolution is the maximum supported
    const auto MAX_SCREEN_WIDTH = (8 * current.w) / 10;
    const auto MAX_SCREEN_HEIGHT = (8 * current.h) / 10;

    auto width = std::min(m_image->info_header.width, MAX_SCREEN_WIDTH);
    auto height = std::min(m_image->info_header.height, MAX_SCREEN_HEIGHT);

    if(width < MIN_SCREEN_WIDTH)
        width *= 10;
    if(height < MIN_SCREEN_HEIGHT)
        height *= 10;

    using namespace std::string_literals;

    writeDebug("Creating a window of size ("s + std::to_string(width) + ',' +
               std::to_string(height) + ")");

    ::should_sleep = (m_image->info_header.width * m_image->info_header.height) < NUM_PIX_REQ_SLEEP;

    // Create SDL window to write to
    SDL_Window* window = SDL_CreateWindow("Shape Finder Debugger",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          width, height,
                                          SDL_WINDOW_SHOWN
                                          );

    if(window == nullptr) {
        writeError("Failed to create SDL window! "s + SDL_GetError());
        SDL_Quit();
        return;
    }

    SDL_Surface* win_surface = SDL_GetWindowSurface(window);

    if(win_surface == nullptr) {
        writeError("Failed to get window surface! "s + SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    // Create initial RGB surface
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(m_debug_data, m_image->info_header.width,
                                                    m_image->info_header.height, 24,
                                                    3 * m_image->info_header.width,
                                                    0xFF0000, 0xFF00, 0xFF, 0);

    SDL_Rect dest = { 0, 0, m_image->info_header.width * 10, m_image->info_header.height * 10 };

    SDL_Event event;

    // Keep going until the calling thread finishes
    while(!done) {
        while(SDL_PollEvent(&event) != 0) {
            switch(event.type) {
                case SDL_QUIT:
                    // Notify the calling thread that we are done
                    done = true;

                    // Make sure that we still kill everything even if the main thread is paused.
                    main_thread_should_pause = false;
                    main_thread_should_unpause_cv.notify_all();

                    std::exit(0); // No need to break here
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym) {
                        case ' ':
                            main_thread_should_pause = !main_thread_should_pause;

                            // If we are now false, then notify the cv of this change
                            if(!main_thread_should_pause) {
                                writeDebug("Notifying main thread that it should wake up!");
                                main_thread_should_unpause_cv.notify_all();
                            }
                            break;
                        case 's':
                            should_sleep = !should_sleep;
                            break;
                        case SDLK_ESCAPE:
                            // Notify the calling thread that we are done
                            done = true;

                            // Make sure that we still kill everything even if the main thread is paused.
                            main_thread_should_pause = false;
                            main_thread_should_unpause_cv.notify_all();

                            std::exit(0); // No need to break here

                            break;
                    }
            }
        }

        SDL_BlitScaled(surface, nullptr, win_surface, &dest);

        SDL_UpdateWindowSurface(window);
    }

kill_graphics_worker:
    SDL_DestroyWindow(window);
    SDL_Quit();
#endif
}

void MapNormalizer::GraphicsWorker::writeDebugColor(uint32_t x, uint32_t y,
                                                    Color c)
{
    if(m_debug_data != nullptr && !prog_opts.no_gui) {
        using namespace std::chrono_literals;

        uint32_t w = m_image->info_header.width;

        // if(should_sleep)
        //     std::this_thread::sleep_for(0.01s);

        uint32_t index = xyToIndex(w * 3, x * 3, y);

        // graphics_debug_mutex.lock();

        // Make sure we swap B and R (because BMP format sucks)
        m_debug_data[index] = c.b;
        m_debug_data[index + 1] = c.g;
        m_debug_data[index + 2] = c.r;

        // graphics_debug_mutex.unlock();
    }
}

void MapNormalizer::GraphicsWorker::resetDebugData() {
    if(m_debug_data != nullptr && !prog_opts.no_gui) {
        auto data_size = m_image->info_header.width * m_image->info_header.height * 3;

        std::copy(m_image->data, m_image->data + data_size, m_debug_data);
    }
}

void MapNormalizer::GraphicsWorker::resetDebugDataAt(const Point2D& point) {
    if(m_debug_data != nullptr && !prog_opts.no_gui) {
        uint32_t index = xyToIndex(m_image, point.x, point.y);

        m_debug_data[index] = m_image->data[index];
    }
}

const unsigned char* MapNormalizer::GraphicsWorker::getDebugData() const {
    return m_debug_data;
}

const MapNormalizer::BitMap* MapNormalizer::GraphicsWorker::getImage() const {
    return m_image;
}

/**
 * @brief Writes a color to the screen.
 *
 * @param debug_data The color data location to write into
 * @param w The width of the color data
 * @param x The x coordinate to write to
 * @param y The y coordinate to write to
 * @param c The color to write
 */
[[deprecated]]
void MapNormalizer::writeDebugColor(unsigned char* debug_data, uint32_t w,
                                    uint32_t x, uint32_t y, Color c)
{
    if(debug_data != nullptr) {
        using namespace std::chrono_literals;

#ifdef ENABLE_GRAPHICS
        if(!prog_opts.no_gui && should_sleep)
            std::this_thread::sleep_for(0.01s);
#endif

        uint32_t index = xyToIndex(w * 3, x * 3, y);

#ifdef ENABLE_GRAPHICS
        if(!prog_opts.no_gui)
            graphics_debug_mutex.lock();
#endif

        // Make sure we swap B and R (because BMP format sucks)
        debug_data[index] = c.b;
        debug_data[index + 1] = c.g;
        debug_data[index + 2] = c.r;

#ifdef ENABLE_GRAPHICS
        if(!prog_opts.no_gui)
            graphics_debug_mutex.unlock();
#endif
    }
}

/**
 * @brief Checks if the main thread should pause execution.
 */
void MapNormalizer::checkForPause() {
    // If graphics aren't enabled, then there is no graphical thread to check.
    //  So don't bother doing anything in that case then.
#ifdef ENABLE_GRAPHICS
    if(main_thread_should_pause) {
        writeDebug("Going to sleep now!");
        std::unique_lock<std::mutex> lk(main_thread_should_unpause_mutex);
        main_thread_should_unpause_cv.wait(lk, []() -> bool { return !main_thread_should_pause; });
    }
#endif
}

