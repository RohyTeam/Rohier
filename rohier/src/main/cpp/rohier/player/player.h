#ifndef ROHIER_PLAYER_H
#define ROHIER_PLAYER_H

#include "rohier/common/status.h"
#include "rohier/native_window/rohier_window_manager.h"
#include "rohier/source/video_source.h"
#include <cstdint>

class RohierPlayer {
public:
    RohierPlayer();
    ~RohierPlayer();
    
    RohierStatus Init(RohierNativeWindow* window);
    
    RohierStatus SetSource(VideoSource* source);
    
    RohierStatus Prepare();
    
    RohierStatus PlayAll();
    RohierStatus PlayVideo();
    RohierStatus PlayAudio();
    RohierStatus PlaySubtitle();
    
    RohierStatus PauseAll();
    RohierStatus PauseVideo();
    RohierStatus PauseAudio();
    RohierStatus PauseSubtitle();
    
    RohierStatus SetAudioOffset(int64_t offset);
    RohierStatus SetSubtitleOffset(int64_t offset);
    
    RohierStatus Seek(uint64_t position);
    
    RohierStatus SetSpeed(uint32_t speed);
    
    RohierStatus SelectTrack(uint32_t track);
    RohierStatus DeselectTrack(uint32_t track);
    
    RohierStatus Release();
};

#endif //ROHIER_PLAYER_H
