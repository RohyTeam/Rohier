#include "rohier_player_napi.h"
#include "rohier/utils/napi_utils.h"
#include "rohier/utils/rohier_logger.h"

std::map<std::string, std::shared_ptr<RohierPlayer>> RohierPlayerNapi::players_;

napi_value RohierPlayerNapi::func_init(napi_env env, napi_callback_info info) {
    ROHIER_INFO("RohierPlayerNapi", "Initializing rohier player napi");
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_value jsThis;
    if (napi_ok != napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr)) {
        ROHIER_ERROR("RohierPlayerNapi", "Failed to get parameters");
        return nullptr;
    }
    ROHIER_INFO("RohierPlayerNapi", "Successfully got parameters");

    std::string surfaceId;
    NapiUtils::JsValueToString(env, args[0], 2048, surfaceId);

    RohierNativeWindow* native_window = RohierWindowManager::getInstance()->getNativeXComponent(surfaceId).get();

    if (!native_window) {
        ROHIER_ERROR("RohierPlayerNapi", "Cannot find native window");
        return nullptr;
    }
    
    std::shared_ptr<RohierPlayer> rohier_player = std::make_shared<RohierPlayer>();
    RohierPlayerNapi::players_[surfaceId] = rohier_player;
    rohier_player->init(native_window);

    return nullptr;
}

napi_value RohierPlayerNapi::func_prepare(napi_env env, napi_callback_info info) {
    ROHIER_INFO("RohierPlayerNapi", "Preparing rohier player");
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    napi_value jsThis;
    
    if (napi_ok != napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr)) {
        ROHIER_ERROR("RohierPlayerNapi", "Failed to get parameters");
        return nullptr;
    }
    
    std::string surfaceId;
    NapiUtils::JsValueToString(env, args[0], 2048, surfaceId);
    
    std::shared_ptr<RohierPlayer> rohier_player = RohierPlayerNapi::players_[surfaceId];
    
    if (!rohier_player || !rohier_player.get()) {
        ROHIER_ERROR("RohierPlayerNapi", "Cannot find targeted rohier player");
        return nullptr;
    }
    
    std::string dest;
    NapiUtils::JsValueToString(env, args[1], 2048, dest);
    
    bool isUrl;
    napi_get_value_bool(env, args[2], &isUrl);

    VideoSource* source = new VideoSource {
        .path = dest,
        .isUrl = isUrl
    };

    rohier_player->prepare(source);
    return nullptr;
}

napi_value RohierPlayerNapi::func_play(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {};
    napi_value jsThis;
    
    if (napi_ok != napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr)) {
        ROHIER_ERROR("RohierPlayerNapi", "Failed to get parameters");
        return nullptr;
    }
    
    std::string surfaceId;
    NapiUtils::JsValueToString(env, args[0], 2048, surfaceId);
    
    std::shared_ptr<RohierPlayer> rohier_player = RohierPlayerNapi::players_[surfaceId];
    
    if (!rohier_player || !rohier_player.get()) {
        ROHIER_ERROR("RohierPlayerNapi", "Cannot find targeted rohier player");
        return nullptr;
    }
    
    rohier_player->start();

    return nullptr;
}

napi_value RohierPlayerNapi::init_napi(napi_env env, napi_value exports) {
    ROHIER_INFO("RohierPlayerNapi", "Initializing rohier player napi");
    napi_property_descriptor desc[] = {
        { "init", nullptr, func_init, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "prepare", nullptr, func_prepare, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "play", nullptr, func_play, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    
    if (napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc) == napi_ok) {
        ROHIER_INFO("RohierPlayerNapi", "Successfully exported rohier player napi");
    } else {
        ROHIER_INFO("RohierPlayerNapi", "Failed to export rohier player napi");
    }
    return exports;
}