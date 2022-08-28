
#include "ItemRegistrar.h"

////////////////////////////////////////////////////////////////////////////////
// Include all headers necessary for ItemTypes.inl

#include "StatusCodes.h"

#include "ItemAddFunctions.h"

////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Generates a map of every item type
 *
 * @return 
 */
HMDT::GUI::ItemTypeMap HMDT::GUI::genRegisteredItemTypes() {
    HMDT::GUI::ItemTypeMap item_type_map;

#define X(NAME) item_type_map[NAME .name] = NAME
#include "ItemTypes.inl"
#undef X

    return item_type_map;
}


