#include "rohier/native_window/rohier_window_manager.h"
#include "rohier/utils/napi_utils.h"

void Rohier_OnSurfaceCreatedCallback(OH_NativeXComponent *component, void *window) {
    char name[OH_XCOMPONENT_ID_LEN_MAX] = {0};
    uint64_t name_len = OH_XCOMPONENT_ID_LEN_MAX;
    if (component != nullptr) {
      OH_NativeXComponent_GetXComponentId(component, name, &name_len);
    }
    std::shared_ptr<RohierNativeWindow> temp = std::make_shared<RohierNativeWindow>();
    temp->nativeWindow = (OHNativeWindow*) window;
    temp->id = name;
    temp->nativeXComponent = component;
    RohierWindowManager::getInstance()->addNativeXComponent(temp);
}

void Rohier_OnSurfaceChangedCallback(OH_NativeXComponent *component, void *window) {
}

void Rohier_OnSurfaceDestroyedCallback(OH_NativeXComponent *component, void *window) {
    char name[OH_XCOMPONENT_ID_LEN_MAX] = {0};
    uint64_t name_len = OH_XCOMPONENT_ID_LEN_MAX;
    if (component != nullptr) {
      OH_NativeXComponent_GetXComponentId(component, name, &name_len);
    }
    RohierWindowManager::getInstance()->removeNativeXComponent(name);
}

void Rohier_DispatchTouchEventCallback(OH_NativeXComponent *component, void *window) {
}

void RohierWindowManager::initRohierWindowManager(napi_env env, napi_value exports) {
    OH_NativeXComponent *nativeXComponent = nullptr;
    static OH_NativeXComponent_Callback nativeXComponentCallback;
    napi_status status;
    napi_value exportInstance = nullptr;
    status = napi_get_named_property(env, exports, OH_NATIVE_XCOMPONENT_OBJ, &exportInstance);
    if (status == napi_ok) {
        status = napi_unwrap(env, exportInstance,
                             reinterpret_cast<void **>(&nativeXComponent));
        if (status == napi_ok) {
            nativeXComponentCallback.OnSurfaceCreated = Rohier_OnSurfaceCreatedCallback;
            nativeXComponentCallback.OnSurfaceChanged = Rohier_OnSurfaceChangedCallback;
            nativeXComponentCallback.OnSurfaceDestroyed = Rohier_OnSurfaceDestroyedCallback;
            nativeXComponentCallback.DispatchTouchEvent = Rohier_DispatchTouchEventCallback;
            OH_NativeXComponent_RegisterCallback(nativeXComponent, &nativeXComponentCallback);
        }
    }
}

RohierWindowManager *RohierWindowManager::instance_{nullptr};
std::once_flag RohierWindowManager::flag_;

void RohierWindowManager::addNativeXComponent(
    std::shared_ptr<RohierNativeWindow> &window) {
  std::unique_lock<std::mutex> _(window_mutex);
  windows_[window->id] = window;
}

std::shared_ptr<RohierNativeWindow> RohierWindowManager::getNativeXComponent(const std::string &windowId) {
  std::unique_lock<std::mutex> _(window_mutex);
  auto it = windows_.find(windowId);
  if (it == windows_.end()) {
    return nullptr;
  }
  return it->second;
}

void RohierWindowManager::removeNativeXComponent(const std::string &windowId) {
  std::unique_lock<std::mutex> _(window_mutex);
  windows_.erase(windowId);
}

RohierWindowManager::~RohierWindowManager() {
  std::unique_lock<std::mutex> _(window_mutex);
  windows_.clear();
}

RohierWindowManager::RohierWindowManager() {}

RohierWindowManager *RohierWindowManager::getInstance() {
  std::call_once(flag_, []() { instance_ = new RohierWindowManager(); });
  return instance_;
}