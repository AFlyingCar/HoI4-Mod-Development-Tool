
#include "StateDefinitionBuilder.h"

#include <fstream>

auto MapNormalizer::createStatesList(const ProvinceList& provinces,
                                     const std::filesystem::path& state_info_path)
    -> StateList
{
    std::ifstream state_info_file(state_info_path);
    StateList states;

    if(state_info_file) {
        // TODO: Load info about various states
    }

    for(auto&& province : provinces) {
        auto state_id = province.state;

        if(states.count(state_id) != 0) {
            states.at(state_id).provinces.push_back(province.id);
        } else {
            std::string state_name = "";
            size_t manpower = 0;
            std::string category = "";
            // TODO: If state_info_file was loaded, get the name of this state
            // TODO: If state_info_file was loaded, get the manpower of this state
            // TODO: If state_info_file was loaded, get the category of this state

            states[state_id] = {
                state_id, state_name, manpower, category, std::vector<ProvinceID>()
            };
        }
    }

    return states;
}

