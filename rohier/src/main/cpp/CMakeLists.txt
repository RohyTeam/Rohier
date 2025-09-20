cmake_minimum_required(VERSION 3.10)
project(RohierPlayer)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-int-conversion -Wl,-Bsymbolic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-int-conversion -Wl,-Bsymbolic")

set(THIRDPARTY_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")

set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

file(GLOB THIRDPARTY_INCLUDES "${THIRDPARTY_LIBRARY_DIR}/*/${OHOS_ARCH}/include")
list(APPEND INCLUDE_DIRS ${THIRDPARTY_INCLUDES})

include_directories(${INCLUDE_DIRS})

set(SOURCES
    rohier/napi/rohier_napi.cpp
    rohier/napi/rohier_player_napi.cpp
    rohier/decoder/video/ohcodec_video_decoder.cpp
    rohier/decoder/audio/ohcodec_audio_decoder.cpp
    rohier/demuxer/ohcodec_demuxer.cpp
    rohier/metadata/metadata_extractor.cpp
    rohier/metadata/metadata.cpp
    rohier/native_window/rohier_window_manager.cpp
    rohier/player/rohier_player.cpp
    rohier/utils/napi_utils.cpp
    rohier/utils/safe_utils.cpp
)

add_library(rohier SHARED ${SOURCES})

set(SYSTEM_LIBS
    z
    c
    m
    EGL
    GLESv3
    uv
)

set(OHOS_LIBS
    ace_napi.z
    hilog_ndk.z
    native_media_core
    native_media_adec
    native_media_vdec
    native_media_avdemuxer
    native_media_avsource
    native_media_codecbase
    native_media_acodec
    ace_ndk.z
    native_drm
    native_window
    native_buffer
    ohaudio
)

set(THIRDPARTY_LIBS
    ${THIRDPARTY_LIBRARY_DIR}/avisynth/${OHOS_ARCH}/lib/libavisynth.so.11
    ${THIRDPARTY_LIBRARY_DIR}/brotli/${OHOS_ARCH}/lib/libbrotlicommon-static.a
    ${THIRDPARTY_LIBRARY_DIR}/brotli/${OHOS_ARCH}/lib/libbrotlidec-static.a
    ${THIRDPARTY_LIBRARY_DIR}/brotli/${OHOS_ARCH}/lib/libbrotlienc-static.a
    ${THIRDPARTY_LIBRARY_DIR}/bzip2_1_0_8/${OHOS_ARCH}/lib/libbz2.a
    ${THIRDPARTY_LIBRARY_DIR}/davs2/${OHOS_ARCH}/lib/libdavs2.a
    ${THIRDPARTY_LIBRARY_DIR}/fdk-aac/${OHOS_ARCH}/lib/libfdk-aac.a
    ${THIRDPARTY_LIBRARY_DIR}/ffmpeg/${OHOS_ARCH}/lib/libavcodec.so.62
    ${THIRDPARTY_LIBRARY_DIR}/ffmpeg/${OHOS_ARCH}/lib/libavdevice.so.62
    ${THIRDPARTY_LIBRARY_DIR}/ffmpeg/${OHOS_ARCH}/lib/libavfilter.so.11
    ${THIRDPARTY_LIBRARY_DIR}/ffmpeg/${OHOS_ARCH}/lib/libavformat.so.62
    ${THIRDPARTY_LIBRARY_DIR}/ffmpeg/${OHOS_ARCH}/lib/libavutil.so.60
    ${THIRDPARTY_LIBRARY_DIR}/ffmpeg/${OHOS_ARCH}/lib/libswresample.so.6
    ${THIRDPARTY_LIBRARY_DIR}/ffmpeg/${OHOS_ARCH}/lib/libswscale.so.9
    ${THIRDPARTY_LIBRARY_DIR}/fontconfig/${OHOS_ARCH}/lib/libfontconfig.so.1
    ${THIRDPARTY_LIBRARY_DIR}/freetype2/${OHOS_ARCH}/lib/libfreetype.a
    ${THIRDPARTY_LIBRARY_DIR}/fribidi/${OHOS_ARCH}/lib/libfribidi.so.0
    ${THIRDPARTY_LIBRARY_DIR}/harfbuzz/${OHOS_ARCH}/lib/libharfbuzz.so.0
    ${THIRDPARTY_LIBRARY_DIR}/jbigkit/${OHOS_ARCH}/lib/libjbig.a
    ${THIRDPARTY_LIBRARY_DIR}/json-c/${OHOS_ARCH}/lib/libjson-c.a
    ${THIRDPARTY_LIBRARY_DIR}/kvazaar/${OHOS_ARCH}/lib/libkvazaar.so.7
    ${THIRDPARTY_LIBRARY_DIR}/lame/${OHOS_ARCH}/lib/libmp3lame.so.0
    ${THIRDPARTY_LIBRARY_DIR}/lcevc/${OHOS_ARCH}/lib/liblcevc_dec_api.so
    ${THIRDPARTY_LIBRARY_DIR}/lcevc/${OHOS_ARCH}/lib/liblcevc_dec_core.so
    ${THIRDPARTY_LIBRARY_DIR}/lcms2/${OHOS_ARCH}/lib/liblcms2.so.2
    ${THIRDPARTY_LIBRARY_DIR}/libaom/${OHOS_ARCH}/lib/libaom.a
    ${THIRDPARTY_LIBRARY_DIR}/libass/${OHOS_ARCH}/lib/libass.so.9
    ${THIRDPARTY_LIBRARY_DIR}/libbluray/${OHOS_ARCH}/lib/libbluray.so.3
    ${THIRDPARTY_LIBRARY_DIR}/libdav1d/${OHOS_ARCH}/lib/libdav1d.so.7
    ${THIRDPARTY_LIBRARY_DIR}/libdeflate/${OHOS_ARCH}/lib/libdeflate.so.0
    ${THIRDPARTY_LIBRARY_DIR}/libiconv/${OHOS_ARCH}/lib/libcharset.so.1
    ${THIRDPARTY_LIBRARY_DIR}/libiconv/${OHOS_ARCH}/lib/libiconv.so.2
    ${THIRDPARTY_LIBRARY_DIR}/libjpeg-turbo/${OHOS_ARCH}/lib/libjpeg.so.62
    ${THIRDPARTY_LIBRARY_DIR}/libjpeg-turbo/${OHOS_ARCH}/lib/libturbojpeg.so.0
    ${THIRDPARTY_LIBRARY_DIR}/libogg/${OHOS_ARCH}/lib/libogg.a
    ${THIRDPARTY_LIBRARY_DIR}/libpng/${OHOS_ARCH}/lib/libpng16.a
    ${THIRDPARTY_LIBRARY_DIR}/libudfread/${OHOS_ARCH}/lib/libudfread.so.3
    ${THIRDPARTY_LIBRARY_DIR}/libunibreak/${OHOS_ARCH}/lib/libunibreak.so.6
    ${THIRDPARTY_LIBRARY_DIR}/libvpx/${OHOS_ARCH}/lib/libvpx.a
    ${THIRDPARTY_LIBRARY_DIR}/libwebp/${OHOS_ARCH}/lib/libsharpyuv.so.0
    ${THIRDPARTY_LIBRARY_DIR}/libwebp/${OHOS_ARCH}/lib/libwebp.so.7
    ${THIRDPARTY_LIBRARY_DIR}/libwebp/${OHOS_ARCH}/lib/libwebpdecoder.so.3
    ${THIRDPARTY_LIBRARY_DIR}/libwebp/${OHOS_ARCH}/lib/libwebpdemux.so.2
    ${THIRDPARTY_LIBRARY_DIR}/libwebp/${OHOS_ARCH}/lib/libwebpmux.so.3
    ${THIRDPARTY_LIBRARY_DIR}/libxml2/${OHOS_ARCH}/lib/libxml2.so.2
    ${THIRDPARTY_LIBRARY_DIR}/libzip/${OHOS_ARCH}/lib/libzip.so.5
    ${THIRDPARTY_LIBRARY_DIR}/openapv/${OHOS_ARCH}/lib/liboapv.so.2
    ${THIRDPARTY_LIBRARY_DIR}/openh264/${OHOS_ARCH}/lib/libopenh264.so.7
    ${THIRDPARTY_LIBRARY_DIR}/openjpeg/${OHOS_ARCH}/lib/libopenjp2.so.7
    ${THIRDPARTY_LIBRARY_DIR}/openssl/${OHOS_ARCH}/lib/libcrypto.a
    ${THIRDPARTY_LIBRARY_DIR}/openssl/${OHOS_ARCH}/lib/libssl.a
    ${THIRDPARTY_LIBRARY_DIR}/opus/${OHOS_ARCH}/lib/libopus.a
    ${THIRDPARTY_LIBRARY_DIR}/rav1e/${OHOS_ARCH}/lib/librav1e.a
    ${THIRDPARTY_LIBRARY_DIR}/speex/${OHOS_ARCH}/lib/libspeex.so.1
    ${THIRDPARTY_LIBRARY_DIR}/tiff/${OHOS_ARCH}/lib/libtiff.so.6
    ${THIRDPARTY_LIBRARY_DIR}/tiff/${OHOS_ARCH}/lib/libtiffxx.so.6
    ${THIRDPARTY_LIBRARY_DIR}/uavs3d/${OHOS_ARCH}/lib/libuavs3d.so
    ${THIRDPARTY_LIBRARY_DIR}/vvenc/${OHOS_ARCH}/lib/libvvenc.so.1.13
    ${THIRDPARTY_LIBRARY_DIR}/x264/${OHOS_ARCH}/lib/libx264.so.164
    ${THIRDPARTY_LIBRARY_DIR}/x265/${OHOS_ARCH}/lib/libx265.so
    ${THIRDPARTY_LIBRARY_DIR}/x265/${OHOS_ARCH}/lib/libx265.so.215
    ${THIRDPARTY_LIBRARY_DIR}/x265/${OHOS_ARCH}/lib/libx265_main10.so
    ${THIRDPARTY_LIBRARY_DIR}/x265/${OHOS_ARCH}/lib/libx265_main12.so
    ${THIRDPARTY_LIBRARY_DIR}/xavs2/${OHOS_ARCH}/lib/libxavs2.a
    ${THIRDPARTY_LIBRARY_DIR}/xvidcore/${OHOS_ARCH}/lib/libxvidcore.so.4
    ${THIRDPARTY_LIBRARY_DIR}/xz/${OHOS_ARCH}/lib/liblzma.so.5
    ${THIRDPARTY_LIBRARY_DIR}/zlib_1_3_1/${OHOS_ARCH}/lib/libz.so.1
    ${THIRDPARTY_LIBRARY_DIR}/zstd_1_5_6/${OHOS_ARCH}/lib/libzstd.so.1
)

target_link_directories(rohier PRIVATE ${THIRDPARTY_LIBRARY_DIR}/ffmpeg/${OHOS_ARCH}/lib)

target_link_libraries(rohier PRIVATE
    ${OHOS_LIBS}
    ${SYSTEM_LIBS}
    ${THIRDPARTY_LIBS}
)