//
// Created on 2025/9/13.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef ROHIER_VIDEO_SOURCE_H
#define ROHIER_VIDEO_SOURCE_H

#include <string>

/**
 * 视频来源
 * 网络视频不配置 headers 等参数，因为 Rohier 在 ArkTS 侧有一个代理系统，创建视频源是直接通过代理创建，之后返回代理 url 即可。
 */
struct VideoSource {
    std::string path; 
    bool isUrl; // http://, https://, webdav:// etc,
};

#endif //ROHIER_VIDEO_SOURCE_H
