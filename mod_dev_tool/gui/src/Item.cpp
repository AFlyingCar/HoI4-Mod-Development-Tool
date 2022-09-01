
#include "Item.h"

#include "Maybe.h"
#include "StatusCodes.h"

#include "Logger.h"

#include "ItemRegistrar.h"

namespace {
    HMDT::GUI::ItemTypeMap& getMutableRegisteredItemTypes() {
        static HMDT::GUI::ItemTypeMap registered_item_types = HMDT::GUI::genRegisteredItemTypes();

        return registered_item_types;
    }
}

const HMDT::GUI::ItemType::PostInitCallbackType HMDT::GUI::ItemType::DEFAULT_POSTINIT_CALLBACK = [](auto&&...) { return STATUS_SUCCESS; };

const HMDT::GUI::ItemTypeMap& HMDT::GUI::getRegisteredItemTypes()
{
    return getMutableRegisteredItemTypes();
}

auto HMDT::GUI::getItemType(const std::string& name) -> MaybeRef<const ItemType>
{
    if(getRegisteredItemTypes().count(name) != 0) {
        return getRegisteredItemTypes().at(name);
    } else {
        WRITE_ERROR("No such ItemType named ", name);
        RETURN_ERROR(STATUS_NO_SUCH_ITEM_TYPE);
    }
}

auto HMDT::GUI::addItem(const std::string& type_name, Window& window,
                        const std::vector<std::filesystem::path>& paths)
    -> MaybeVoid
{
    auto maybe_item_type = getItemType(type_name);
    RETURN_IF_ERROR(maybe_item_type);

    const auto& item_type = maybe_item_type->get();

    // Setup
    auto maybe_data = item_type.init_add_callback(window, paths);
    RETURN_IF_ERROR(maybe_data);

    {
        auto data = *maybe_data;

        // First set up the dispatcher to run the 'end' function
        uint32_t end_dispatcher_id;

        window.setupDispatcher([data, &window, &item_type](uint32_t dispatch_id) -> void {
            auto res = item_type.end_add_callback(window, data);
            WRITE_IF_ERROR(res);

            // remove ourselves
            res = window.teardownDispatcher(dispatch_id);
            WRITE_IF_ERROR(res);
        }).andThen([&end_dispatcher_id](const uint32_t& new_id) {
            end_dispatcher_id = new_id;
        });

        // Capture only by copy since we don't want to read data out of scope
        std::thread add_thread([end_dispatcher_id, data, &window, &item_type]()
            -> MaybeVoid
        {
            auto res = item_type.add_worker_callback(window, data);
            RETURN_IF_ERROR(res);

            res = window.notifyDispatcher(end_dispatcher_id);
            RETURN_IF_ERROR(res);

            return STATUS_SUCCESS;
        });

        auto res = item_type.post_start_add_callback(window, data);
        if(!IS_FAILURE(res)) {
            WRITE_DEBUG("Detaching worker thread.");
            // If start succeeded, then detach the thread
            add_thread.detach();
        } else {
            // Otherwise, wait for it to rejoin and just call end manually
            WRITE_DEBUG("Waiting for worker thread to rejoin.");
            add_thread.join();

            res = item_type.end_add_callback(window, data);
        }

        RETURN_IF_ERROR(res);
    }

    return STATUS_SUCCESS;
}

