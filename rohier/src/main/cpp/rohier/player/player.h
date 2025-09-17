#ifndef ROHIER_PLAYER_H
#define ROHIER_PLAYER_H

#include "rohier/common/status.h"
#include "rohier/metadata/video_metadata.h"
#include "rohier/native_window/rohier_window_manager.h"
#include "rohier/source/video_source.h"
#include <cstdint>

class RohierPlayer {
public:
    RohierPlayer();
    ~RohierPlayer();
    
    RohierStatus init(RohierNativeWindow* window);
    
    RohierStatus set_source(VideoSource* source);
    
    RohierStatus prepare();
    
    RohierStatus play_all();
    RohierStatus play_video();
    RohierStatus play_audio();
    RohierStatus play_subtitle();
    
    RohierStatus pause_all();
    RohierStatus pause_video();
    RohierStatus pause_audio();
    RohierStatus pause_subtitle();
    
    RohierStatus set_audio_offset(int64_t offset);
    RohierStatus set_subtitle_offset(int64_t offset);
    
    RohierStatus seek(uint64_t position);
    
    RohierStatus set_speed(uint32_t speed);
    
    RohierStatus select_track(uint32_t track);
    RohierStatus deselect_track(uint32_t track);
    
    RohierStatus release();
    
private:
    std::shared_ptr<VideoMetadata> videoMetadata_;
};

#endif //ROHIER_PLAYER_H
