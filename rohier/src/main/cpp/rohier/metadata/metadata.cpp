#include "rohier/metadata/video_metadata.h"
#include "rohier/utils/rohier_logger.h"

int64_t time_base_to_ms(AVRational time_base, int64_t value) {
    return value * 1000 * time_base.num / time_base.den;
}

OH_AVFormat* VideoMetadata::get_ohcodec_track(OH_AVSource* oh_src, int32_t track_index) {
    ROHIER_INFO("VideoMetadata", "Getting OHCodec track metadata info for track %{public}d", track_index);
    return OH_AVSource_GetTrackFormat(oh_src, track_index);
}

AVStream* VideoMetadata::get_ffmpeg_track(AVFormatContext* av_fmt, int32_t track_index) {
    return av_fmt->streams[track_index];
}

// TODO: 处理一下不存在视频轨道的情况
TrackMetadata* VideoMetadata::find_best_video_track() {
    return std::find_if(this->tracks.begin(), this->tracks.end(), [](TrackMetadata track) { return track.track_type == TrackType::TrackType_Video; }).base();
}

// TODO: 处理一下不存在音轨的情况
TrackMetadata* VideoMetadata::find_best_audio_track() {
    return std::find_if(this->tracks.begin(), this->tracks.end(), [](TrackMetadata track) { return track.track_type == TrackType::TrackType_Audio; }).base();
}
