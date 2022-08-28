#ifndef ITEM_H
# define ITEM_H

# include <any>
# include <set>
# include <string>
# include <functional>
# include <filesystem>

# include "Maybe.h"

# include "Window.h"

namespace HMDT::GUI {
    struct ItemType {
        //! The name of the item
        std::string name;

        //! The description of the item
        std::string description;

        //! The icon of the item
        std::string icon;

        /**
         * @brief A list of default files that will be overridden/invalidated
         *        along with whatever files this item explicitly adds/modifies.
         * @details These files will be included as blank files unless
         *          explicitly added separately.
         */
        std::set<std::string> extra_overrides;

        //! Called once to initialize the add item routine
        std::function<Maybe<std::any>(Window&, const std::filesystem::path&)> init_add_callback;

        //! Called once after initialization in a std::thread
        std::function<MaybeVoid(Window&, std::any)> add_worker_callback;

        //! Called once after the std::thread has been created
        std::function<MaybeVoid(Window&, std::any)> post_start_add_callback;

        //! Called once when this item is done getting added.
        std::function<MaybeVoid(Window&, std::any)> end_add_callback;

        //! Called when this item is removed
        std::function<MaybeVoid(Window&)> on_remove_callback;
    };

    using ItemTypeMap = std::map<std::string, ItemType>;

    const ItemTypeMap& getRegisteredItemTypes();
    MaybeRef<const ItemType> getItemType(const std::string&);

    MaybeVoid addItem(const std::string&, Window&, const std::filesystem::path&);
}

/**
 * @brief Defines an item type
 *
 * @param ... Every field of ItemType
 */
# define DEFINE_ITEM_TYPE(...) \
    DEFINE_ITEM_TYPE_IMPL(HMDT_UNIQUE_NAME(__DEFINE_ITEM_TYPE), __VA_ARGS__)

/**
 * @brief Implementation for defining an item type
 * @details Will also use X(DEFINITION_NAME) to do something with the name
 *          after creating the object.
 *
 * @param DEFINITION_NAME The name of this item type definition
 * @param ... Every field of ItemType
 */
# define DEFINE_ITEM_TYPE_IMPL(DEFINITION_NAME, ...)                           \
    do {                                                                       \
        static HMDT::GUI::ItemType DEFINITION_NAME =                           \
            HMDT::GUI::ItemType { __VA_ARGS__ } ;                              \
        X(DEFINITION_NAME);                                                    \
    } while(0)                                                                 \

#endif

