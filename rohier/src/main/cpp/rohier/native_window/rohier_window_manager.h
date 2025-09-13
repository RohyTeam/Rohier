#ifndef ROHIER_WINDOW_MANAGER_H
#define ROHIER_WINDOW_MANAGER_H

#include <map>
#include <mutex>
#include <string>
#include "ace/xcomponent/native_interface_xcomponent.h"
#include "napi/native_api.h"

struct RohierNativeWindow {
  std::string id;
  OH_NativeXComponent *nativeXComponent{nullptr};
  void *nativeWindow{nullptr};
};

class RohierWindowManager {
public:
    static void initRohierWindowManager(napi_env env, napi_value exports);
    
    void addNativeXComponent(std::shared_ptr<RohierNativeWindow> &component);
    std::shared_ptr<RohierNativeWindow> getNativeXComponent(const std::string &windowId);
    void removeNativeXComponent(const std::string &windowId);
    
    ~RohierWindowManager();
    
    static RohierWindowManager *getInstance();
private:
    RohierWindowManager();
    
    std::map<std::string, std::shared_ptr<RohierNativeWindow>> windows_;
    std::mutex window_mutex;
    
    static std::once_flag flag_;
    static RohierWindowManager *instance_;
};


#endif // ROHIER_WINDOW_MANAGER_H
