#include "rohier_player_napi.h"
#include "rohier/utils/napi_utils.h"

static thread_local napi_ref g_ref = nullptr;

RohierPlayerNapi::RohierPlayerNapi() : _env(nullptr), _wrapper(nullptr) {}

RohierPlayerNapi::~RohierPlayerNapi() {
    napi_delete_reference(this->_env, this->_wrapper);
}

void RohierPlayerNapi::finalize(napi_env env, void *native_object, [[maybe_unused]] void *finalize_hint) {
    RohierPlayerNapi* casted_object = reinterpret_cast<RohierPlayerNapi*>(native_object);
    if (casted_object->rohier_player_)
        casted_object->rohier_player_->release();
    delete casted_object;
}

napi_value RohierPlayerNapi::func_new(napi_env env, napi_callback_info info) {
    napi_value new_target;
    napi_get_new_target(env, info, &new_target);
    
    if (new_target != nullptr) {
        size_t arg_c = 0;
        napi_value args[0];
        napi_value jsThis;
        napi_get_cb_info(env, info, &arg_c, args, &jsThis, nullptr);
        
        RohierPlayerNapi* native_object = new RohierPlayerNapi();
        
        native_object->_env = env;
        napi_status status = napi_wrap(env, jsThis, reinterpret_cast<void*>(native_object), RohierPlayerNapi::finalize, nullptr, &native_object->_wrapper);
        if (status != napi_ok) {
            delete native_object;
            return jsThis;
        }
        
        return jsThis;
    } else {
        size_t arg_c = 0;
        napi_value args[0];
        napi_get_cb_info(env, info, &arg_c, args, nullptr, nullptr);
    
        napi_value cons;
        napi_get_reference_value(env, g_ref, &cons);
        
        napi_value instance;
        napi_new_instance(env, cons, arg_c, args, &instance);
    
        return instance;
    }
}

napi_value RohierPlayerNapi::func_init(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_value jsThis;
    if (napi_ok != napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr)) {
        return nullptr;
    }
    
    RohierPlayerNapi* native_object;
    napi_unwrap(env, jsThis, reinterpret_cast<void**>(&native_object));
    
    std::string surfaceId;
    NapiUtils::JsValueToString(env, args[0], 2048, surfaceId);

    native_object->rohier_player_->init(RohierWindowManager::getInstance()->getNativeXComponent(surfaceId).get());
    return nullptr;
}

napi_value RohierPlayerNapi::func_prepare(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    napi_value jsThis;
    if (napi_ok != napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr)) {
        return nullptr;
    }
    
    RohierPlayerNapi* native_object;
    napi_unwrap(env, jsThis, reinterpret_cast<void**>(&native_object));
    
    std::string dest;
    NapiUtils::JsValueToString(env, args[0], 2048, dest);
    
    bool isUrl;
    napi_get_value_bool(env, args[1], &isUrl);

    VideoSource* source = new VideoSource {
        .path = dest,
        .isUrl = isUrl
    };

    native_object->rohier_player_->prepare(source);
    return nullptr;
}

napi_value RohierPlayerNapi::func_play(napi_env env, napi_callback_info info) {
    size_t argc = 0;
    napi_value args[0] = {};
    napi_value jsThis;
    if (napi_ok != napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr)) {
        return nullptr;
    }
    
    RohierPlayerNapi* native_object;
    napi_unwrap(env, jsThis, reinterpret_cast<void**>(&native_object));
    
    native_object->rohier_player_->start();

    return nullptr;
}

napi_value RohierPlayerNapi::init_napi(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        { "init", nullptr, func_init, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "prepare", nullptr, func_prepare, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "play", nullptr, func_play, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    
    napi_value cons; // idk what does this mean, i just do what the doc tell me to do
    napi_define_class(env, "RohyAssSubtitleRenderer", NAPI_AUTO_LENGTH, func_new, nullptr, 4, desc, &cons);
    napi_create_reference(env, cons, 1, &g_ref);
    napi_set_named_property(env, exports, "RohyAssSubtitleRenderer", cons);
    
    return exports;
}