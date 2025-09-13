//
// Created on 2025/9/13.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef ROHIER_STATUS_H
#define ROHIER_STATUS_H

#include <cstdint>

enum RohierStatus {
    RohierStatus_Success = 0,
    RohierStatus_SourceNotAccessible = 10001,
    RohierStatus_PlayerNotPrepared = 10002,
};

#endif //ROHIER_STATUS_H
