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
    RohierStatus_NotImplemented = 1,
    RohierStatus_NotInitialized = 2,
    RohierStatus_WindowNotFound = 3,
    RohierStatus_AlreadyInitialized = 4,
    RohierStatus_IllegalMetadata = 5,
    RohierStatus_FailedToConfigureDecoder = 6,
    RohierStatus_DecoderNotFound = 7,
    RohierStatus_FailedToSetSurface = 8,
    RohierStatus_NotPrepared = 9,
    RohierStatus_FailedToStartDecoder = 10,
    RohierStatus_FailedToPushBufferToDecoder = 11,
    RohierStatus_FailedToFreeBufferFromDecoder = 12,
    RohierStatus_FailedToStopDecoder = 13,
    RohierStatus_AlreadyStarted = 14,
    RohierStatus_FailedToPrepareDecoder = 15,
    RohierStatus_FailedToCreateThread = 16,
    RohierStatus_FailedToCreateDemuxer = 17,
    RohierStatus_FailedToGetSourceFormat = 18,
    RohierStatus_DemuxerNotFound = 19,
    RohierStatus_FailedToReadSampleFromDemuxer = 20,
    RohierStatus_FailedToGetBufferAttrFromDemuxer = 21,
    RohierStatus_FailedToSetDecoderCallbacks = 22,
    RohierStatus_FailedToSetNativeWindowScalingMode = 23,
    RohierStatus_FailedToPrepareDemuxer = 24,
    RohierStatus_FailedToSeek = 25,
    RohierStatus_NotStartedYet = 26,
    RohierStatus_SourceNotAccessible = 10001,
    RohierStatus_PlayerNotPrepared = 10002,
};

#endif //ROHIER_STATUS_H
