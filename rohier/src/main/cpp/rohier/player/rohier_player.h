//
// Created on 2025/9/16.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef ROHIER_ROHIER_DECODER_H
#define ROHIER_ROHIER_DECODER_H

#include "rohier/common/status.h"
#include "rohier/decoder/audio/audio_decoder.h"
#include "rohier/decoder/video/video_decoder.h"
#include "rohier/demuxer/demuxer.h"
#include "rohier/metadata/video_metadata.h"
#include "rohier/native_window/rohier_window_manager.h"
#include "rohier/source/video_source.h"
#include "rohier/utils/pointer_management.h"

#include <cstdint>
#include <multimedia/player_framework/native_avdemuxer.h>
#include <multimedia/player_framework/native_avsource.h>
#include <multimedia/player_framework/native_avcodec_base.h>
#include <multimedia/player_framework/native_avformat.h>
#include <multimedia/player_framework/native_avbuffer.h>
#include <fcntl.h>
#include <ohaudio/native_audiostream_base.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include "libavutil/dict.h"
}

class RohierPlayer {
public:
    RohierPlayer();
    ~RohierPlayer();
    
    RohierStatus init(RohierNativeWindow* window);
    RohierStatus prepare(VideoSource* source);
    RohierStatus start();
    RohierStatus stop();
    RohierStatus release();
    RohierStatus seek(int64_t position);
    
    std::shared_ptr<VideoMetadata> get_metadata();
    AVFormatContext* get_ffmpeg_avformat_context();
    OH_AVSource* get_ohcodec_avsource();
    
private:
    void release_threads();
    
private:
    RohierNativeWindow* window_;
    
    int* fd;
    
    int64_t current_position;
    
    std::atomic<bool> started = {false};
    
    std::shared_ptr<VideoMetadata> video_metadata_;
    AVFormatContext* av_format_context;
    OH_AVSource* oh_avsource;
    
    std::shared_ptr<Demuxer> demuxer_;
    std::shared_ptr<VideoDecoder> video_decoder_;
    std::shared_ptr<AudioDecoder> audio_decoder_;
    
    VideoCodecContext* video_context_;
    AudioCodecContext* audio_context_;
    
    std::mutex mutex_;
    std::mutex seek_mutex_;
    
    std::condition_variable seek_condition;
    
    std::unique_ptr<std::thread> video_decode_input_thread_ = nullptr;
    std::unique_ptr<std::thread> video_decode_output_thread_ = nullptr;
    std::unique_ptr<std::thread> audio_decode_input_thread_ = nullptr;
    std::unique_ptr<std::thread> audio_decode_output_thread_ = nullptr;
    
    void thread_video_decode_input();
    void thread_video_decode_output();
    void thread_audio_decode_input();
    void thread_audio_decode_output();
    
    static constexpr int64_t MICROSECOND = 1000000;
    static constexpr int BALANCE_VALUE = 2;
};

#endif //ROHIER_ROHIER_DECODER_H
