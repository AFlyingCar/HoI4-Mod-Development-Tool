
#include "Types.h"

#include "Util.h"

/**
 * @brief Outputs the given point to the given stream.
 *
 * @param stream The stream to output into.
 * @param p The Point to output.
 *
 * @return The given stream after output.
 */
std::ostream& operator<<(std::ostream& stream, const MapNormalizer::Point2D& p)
{
    return (stream << '(' << p.x << ',' << p.y << ')');
}

/**
 * @brief Outputs the given color to the given stream.
 *
 * @param stream The stream to output into.
 * @param c The Color to output.
 * @return The given stream after output.
 */
std::ostream& operator<<(std::ostream& stream, const MapNormalizer::Color& c) {
    return (stream << std::hex << colorToRGB(c));
}

/**
 * @brief Outputs the given ProvinceType to the given stream.
 *
 * @param stream The stream to output into.
 * @param prov_type The ProvinceType to output.
 * @return The given stream after output.
 */
std::ostream& operator<<(std::ostream& stream,
                         const MapNormalizer::ProvinceType& prov_type)
{
    switch(prov_type) {
        case MapNormalizer::ProvinceType::LAND:
            return (stream << "land");
        case MapNormalizer::ProvinceType::LAKE:
            return (stream << "lake");
        case MapNormalizer::ProvinceType::SEA:
            return (stream << "sea");
        case MapNormalizer::ProvinceType::UNKNOWN:
        default:
            return (stream << "UNKNOWN{" << static_cast<int>(prov_type) << "}");
    }
}

/**
 * @brief Outputs the given State to the given stream.
 *
 * @param stream The stream to output into.
 * @param state The State to output.
 *
 * @return The given stream after output.
 */
std::ostream& operator<<(std::ostream& stream, const MapNormalizer::State& state)
{
    stream << "state = {" << std::endl;
    stream << "    id = " << state.id << std::endl;
    stream << "    name = \"" << state.name << '"' << std::endl;
    stream << "    manpower = " << state.manpower << std::endl;
    stream << "    state_category = " << state.category << std::endl;
    stream << "    provinces = {" << std::endl << "        ";

    for(auto&& prov : state.provinces) stream << prov << " ";

    stream << std::endl << "    }" << std::endl;

    stream << "    history = {" << std::endl;
    // TODO: Effects? Should we load these in, or let the user input them?

#if 0
    stream << "        victory_points = {" << std::endl;
    // TODO: Same for victory points. Should we try loading them in from somewhere?
    // For future reference, format is <province id> <points>
    stream << "        }" << std::endl;
#endif

    stream << "        buildings = {" << std::endl;
    // TODO: Finally, same for buildings. Should we try loading them in from somewhere?
    // For future reference, format is <building> = <amount> and:
    //     <province id> = { <building> = <amount> }
    stream << "        }" << std::endl;
    stream << "    }" << std::endl;
    stream << "}" << std::endl;

    return stream;
}

