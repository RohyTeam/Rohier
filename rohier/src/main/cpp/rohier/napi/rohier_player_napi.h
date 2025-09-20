//
// Created on 2025/9/17.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef ROHIER_ROHIER_PLAYER_NAPI_H
#define ROHIER_ROHIER_PLAYER_NAPI_H

#include "rohier/player/rohier_player.h"
#include <napi/native_api.h>

class RohierPlayerNapi {
public:
    explicit RohierPlayerNapi();
    static napi_value init_napi(napi_env env, napi_value exports);
    static void finalize(napi_env env, void* native_object, void* finalize_hint);
private:
    ~RohierPlayerNapi();
    
    static std::map<std::string, std::shared_ptr<RohierPlayer>> players_;
    
    static napi_value func_new(napi_env env, napi_callback_info info);
    static napi_value func_init(napi_env env, napi_callback_info info);
    static napi_value func_prepare(napi_env env, napi_callback_info info);
    static napi_value func_play(napi_env env, napi_callback_info info);
    
    napi_env _env;
    napi_ref _wrapper;
    
    RohierPlayer* rohier_player_;
};

#endif //ROHIER_ROHIER_PLAYER_NAPI_H
