#include "napi/native_api.h"
#include "rohier/native_window/rohier_window_manager.h"

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    RohierWindowManager::initRohierWindowManager(env, exports);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "rohier",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterRohierModule(void)
{
    napi_module_register(&demoModule);
}
