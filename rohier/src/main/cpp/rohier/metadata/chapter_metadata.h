//
// Created on 2025/9/13.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef ROHIER_CHAPTER_METADATA_H
#define ROHIER_CHAPTER_METADATA_H

#include <cstdint>
#include <string>

struct ChapterMetadata {
    std::string title;
    int64_t start = 0;
    int64_t end = 0;
};

#endif //ROHIER_CHAPTER_METADATA_H
