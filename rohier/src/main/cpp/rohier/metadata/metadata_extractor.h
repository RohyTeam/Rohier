//
// Created on 2025/9/13.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef ROHIER_METADATA_EXTRACTOR_H
#define ROHIER_METADATA_EXTRACTOR_H

#include "rohier/metadata/video_metadata.h"
#include "rohier/source/video_source.h"
#include <memory>

class MetadataExtractor {
public:
    static std::shared_ptr<VideoMetadata> extractMetadata(VideoSource* source, bool cover);
};

#endif //ROHIER_METADATA_EXTRACTOR_H
