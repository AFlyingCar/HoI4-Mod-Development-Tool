
#include "GraphicalDebugger.h"

#include <mutex> // std::mutex
#include <chrono> // std::chrono
#include <iostream> // std::cerr
#include <thread> // std::this_thread

#define NOMINMAX
#include <cmath>
#include <algorithm>

#include "BitMap.h" // BitMap
#include "ShapeFinder.h"
#include "Options.h"
#include "Logger.h"

#ifdef ENABLE_GRAPHICS
# include <SDL.h> // SDL_*
#endif

#ifdef ENABLE_GRAPHICS
#include <condition_variable>
#include <atomic>

static std::mutex graphics_debug_mutex;

static std::mutex main_thread_should_unpause_mutex;
static std::condition_variable main_thread_should_unpause_cv;
static std::atomic<bool> main_thread_should_pause;
#endif

// Maximum width + height that SDL will create a window for
static const auto MAX_SCREEN_WIDTH = 16384;
static const auto MAX_SCREEN_HEIGHT = 16384;

// Minimum width + height that I want to allow
static const auto MIN_SCREEN_WIDTH = 100;
static const auto MIN_SCREEN_HEIGHT = 100;
static const auto NUM_PIX_REQ_SLEEP = 10000;

#ifdef ENABLE_GRAPHICS
static bool should_sleep = true;
#endif

/**
 * @brief Worker function which manages displaying graphical debugging data.
 *
 * @param image The image being debugged
 * @param disp_data The pixel data to display
 * @param done A boolean for this to know when the algorithm is done.
 */
void MapNormalizer::graphicsWorker(BitMap* image, unsigned char* disp_data,
                                   bool& done)
{
#ifndef ENABLE_GRAPHICS
    (void)image;
    (void)disp_data;
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

    auto width = std::min(image->info_header.width, MAX_SCREEN_WIDTH);
    auto height = std::min(image->info_header.height, MAX_SCREEN_HEIGHT);

    if(width < MIN_SCREEN_WIDTH)
        width *= 10;
    if(height < MIN_SCREEN_HEIGHT)
        height *= 10;

    using namespace std::string_literals;

    writeDebug("Creating a window of size ("s + std::to_string(width) + ',' +
               std::to_string(height) + ")");

    ::should_sleep = (image->info_header.width * image->info_header.height) < NUM_PIX_REQ_SLEEP;

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
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(disp_data, image->info_header.width,
                                                    image->info_header.height, 24,
                                                    3 * image->info_header.width,
                                                    0xFF, 0xFF00, 0xFF0000, 0);

    SDL_Rect dest = { 0, 0, image->info_header.width * 10, image->info_header.height * 10 };

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
                    }
            }
        }

        graphics_debug_mutex.lock();
        SDL_BlitScaled(surface, nullptr, win_surface, &dest);
        graphics_debug_mutex.unlock();

        SDL_UpdateWindowSurface(window);
    }

kill_graphics_worker:
    SDL_DestroyWindow(window);
    SDL_Quit();
#endif
}

/**
 * @brief Writes a color to the screen
 *
 * @param debug_data The color data location to write into
 * @param w The width of the color data
 * @param x The x coordinate to write to
 * @param y The y coordinate to write to
 * @param c The color to write
 */
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

        debug_data[index] = c.r;
        debug_data[index + 1] = c.g;
        debug_data[index + 2] = c.b;

#ifdef ENABLE_GRAPHICS
        if(!prog_opts.no_gui)
            graphics_debug_mutex.unlock();
#endif
    }
}

void MapNormalizer::checkForPause() {
#ifdef ENABLE_GRAPHICS
    if(main_thread_should_pause) {
        writeDebug("Going to sleep now!");
        std::unique_lock<std::mutex> lk(main_thread_should_unpause_mutex);
        main_thread_should_unpause_cv.wait(lk, []() -> bool { return !main_thread_should_pause; });
    }
#endif
}

