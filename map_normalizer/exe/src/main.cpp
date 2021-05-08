/**
 * @file main.cpp
 *
 * @brief The starting point of the program
 */

#include <thread>

#include "ArgParser.h"
#include "Options.h"

#include "Logger.h"

#include "Interfaces.h"

MapNormalizer::ProgramOptions MapNormalizer::prog_opts;

/**
 * @brief The starting point of the program.
 *
 * @param argc The number of arguments.
 * @param argv The arguments.
 *
 * @return 1 upon failure, 0 upon success.
 */
int main(int argc, char** argv) {
    bool done = false;
    std::thread log_thread([&done]() {
        MapNormalizer::Log::Logger::getInstance().work(done);
    });

    // Parse the command-line arguments
    MapNormalizer::prog_opts = MapNormalizer::parseArgs(argc, argv);

    // Figure out if we should stop now based on the status of the parsing
    switch(MapNormalizer::prog_opts.status) {
        case 1:
            done = true;
            log_thread.join();
            return 1;
        case 2:
            done = true;
            log_thread.join();
            return 0;
        case 0:
        default:
            break;
    }

    int result = MapNormalizer::runApplication();

    done = true;
    log_thread.join();

    return result;
}

