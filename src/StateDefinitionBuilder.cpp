
#include "StateDefinitionBuilder.h"

#include <fstream>

#include "Util.h"
#include "Logger.h"

auto MapNormalizer::createStatesList(const ProvinceList& provinces,
                                     const std::filesystem::path& state_info_path)
    -> StateList
{
    using namespace std::string_literals;

    std::ifstream state_info_file(state_info_path);
    StateList states;

    // CSV file is of the following format:
    //  <State ID>;<State Name>;<Manpower>;<Category>
    if(state_info_file) {
        auto line_num = 0;
        for(std::string line; std::getline(state_info_file, line); ++line_num) {
            // Skip over empty lines
            if(!line.empty()) {
                std::string raw_id, raw_name, raw_manpower, raw_category;
                StateID id;
                size_t manpower;

                auto last_comma = 0;
                auto comma = line.find_first_of(';');
                if(comma == std::string::npos) goto csvParseFailure;
                raw_id = line.substr(0, comma);

                last_comma = comma + 1;
                comma = line.find_first_of(';', last_comma);
                if(comma == std::string::npos) goto csvParseFailure;
                raw_name = line.substr(last_comma, comma - last_comma);

                last_comma = comma + 1;
                comma = line.find_first_of(';', last_comma);
                if(comma == std::string::npos) goto csvParseFailure;
                raw_manpower = line.substr(last_comma, comma - last_comma);

                // No need to search for the next comma, as there shouldn't _be_ one
                //  Just grab until the end of the line, damn the consequences
                last_comma = comma + 1;
                raw_category = line.substr(last_comma);

                // Now that we have each raw value, go ahead and try to parse them

                trim(raw_id);
                trim(raw_name);
                trim(raw_manpower);
                trim(raw_category);
                
                id = std::atoi(raw_id.c_str());
                manpower = std::atoi(raw_manpower.c_str());

                states[id] = {
                    id, raw_name, manpower, raw_category, std::vector<ProvinceID>()
                };

                continue;
csvParseFailure:
                writeError("Failed to parse CSV file on line #"s + std::to_string(line_num));
                return StateList();
            }
        }
    }

    for(auto&& province : provinces) {
        // Add 1 to the state ID, since that is not allowed to be 0
        auto state_id = province.state + 1;

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

