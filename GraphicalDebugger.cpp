
#include "GraphicalDebugger.h"

#include <mutex> // std::mutex
#include <chrono> // std::chrono
#include <iostream> // std::cerr
#include <thread> // std::this_thread

#include "ShapeFinder.h"

#ifdef ENABLE_GRAPHICS 
# include <SDL.h>
#endif

static std::mutex graphics_debug_mutex;

void MapNormalizer::graphicsWorker(BitMap* image, unsigned char* disp_data,
                                   bool& done)
{
#ifndef ENABLE_GRAPHICS 
    (void)image;
    (void)disp_data;
    (void)done;
#else
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL! " << SDL_GetError() << std::endl;
        return;
    }

    SDL_Window* window = SDL_CreateWindow("Shape Finder Debugger",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          image->width * 10,
                                          image->height * 10,
                                          SDL_WINDOW_SHOWN
                                          );

    if(window == nullptr) {
        std::cerr << "Failed to create SDL window! " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    SDL_Surface* win_surface = SDL_GetWindowSurface(window);

    if(win_surface == nullptr) {
        std::cerr << "Failed to get window surface! " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(disp_data, image->width,
                                                    image->height, 24,
                                                    3 * image->width,
                                                    0xFF0000, 0xFF00, 0xFF, 0);

    SDL_Rect dest = { 0, 0, image->width * 10, image->height * 10 };

    SDL_Event event;

    while(!done) {
        while(SDL_PollEvent(&event) != 0) {
            switch(event.type) {
                case SDL_QUIT:
                    done = true;
                    std::exit(0);
            }
        }

        // SDL_FillRect(win_surface, nullptr, 0);

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

void MapNormalizer::writeDebugColor(unsigned char* debug_data, uint32_t w,
                                    uint32_t x, uint32_t y, Color c)
{
#ifndef ENABLE_GRAPHICS
    (void)debug_data;
    (void)w;
    (void)x;
    (void)y;
    (void)c;
#else
    if(debug_data != nullptr) {
        using namespace std::chrono_literals;

        std::this_thread::sleep_for(0.01s);

        uint32_t index = xyToIndex(w * 3, x * 3, y);

        graphics_debug_mutex.lock();

        debug_data[index] = c.r;
        debug_data[index + 1] = c.g;
        debug_data[index + 2] = c.b;

        graphics_debug_mutex.unlock();
    }
#endif
}

