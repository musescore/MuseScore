/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

extern "C" {
#include "thirdparty/ffmpeg/v7/libavcodec/avcodec.h"
#include "thirdparty/ffmpeg/v7/libavformat/avformat.h"
#include "thirdparty/ffmpeg/v7/libswscale/swscale.h"
}

#include "io/path.h"

namespace muse::media::ffmpeg::v7 {
class FFmpegLibHandler
{
public:
    FFmpegLibHandler() = default;

    bool loadLib(const io::path_t& avUtilPath, const io::path_t& avCodecPath, const io::path_t& avFormatPath, const io::path_t& swScalePath,
                 const io::path_t& swResamplePath);
    bool loadApi();
    void unload();

    // libavutil
    int64_t (*av_rescale_q)(int64_t a, AVRational bq, AVRational cq) = nullptr;
    int (*av_image_fill_arrays)(uint8_t* dst_data[4], int dst_linesize[4], const uint8_t* src, AVPixelFormat pix_fmt, int width, int height,
                                int align) = nullptr;
    int (*av_image_get_buffer_size)(AVPixelFormat pix_fmt, int width, int height, int align) = nullptr;
    int (*av_opt_set_int)(void* obj, const char* name, int64_t val, int search_flags) = nullptr;
    void*(*av_malloc)(size_t size) = nullptr;

    // libavformat
    AVStream*(*avformat_new_stream)(AVFormatContext* s, const AVCodec* c) = nullptr;
    int (*avformat_write_header)(AVFormatContext* s, AVDictionary** options) = nullptr;
    int (*av_write_trailer)(AVFormatContext* s) = nullptr;
    int (*avio_open)(AVIOContext** s, const char* url, int flags) = nullptr;
    int (*avio_close)(AVIOContext* s) = nullptr;
    int (*av_interleaved_write_frame)(AVFormatContext* s, AVPacket* pkt) = nullptr;
    int (*avformat_open_input)(AVFormatContext** ctx, const char* url, const AVInputFormat* fmt, AVDictionary** options) = nullptr;
    int (*avformat_find_stream_info)(AVFormatContext* ic, AVDictionary** options) = nullptr;
    void (*avformat_close_input)(AVFormatContext** s) = nullptr;
    void (*avformat_free_context)(AVFormatContext* s) = nullptr;
    int (*avformat_alloc_output_context2)(AVFormatContext** ctx, const AVOutputFormat* oformat, const char* format_name,
                                          const char* filename) = nullptr;
    int (*av_read_frame)(AVFormatContext* s, AVPacket* pkt) = nullptr;
    AVFormatContext*(*avformat_alloc_context)(void) = nullptr;
    AVIOContext*(*avio_alloc_context)(unsigned char* buffer, int buffer_size, int write_flag, void* opaque,
                                      int (*read_packet)(void* opaque, uint8_t* buf, int buf_size),
                                      int (*write_packet)(void* opaque, const uint8_t* buf, int buf_size),
                                      int64_t (*seek)(void* opaque, int64_t offset, int whence)) = nullptr;
    void (*avio_context_free)(AVIOContext** s) = nullptr;

    // libavcodec
    AVCodecContext*(*avcodec_alloc_context3)(const AVCodec* codec) = nullptr;
    const AVCodec*(*avcodec_find_encoder)(AVCodecID id) = nullptr;
    const AVCodec*(*avcodec_find_decoder)(AVCodecID id) = nullptr;
    void (*avcodec_free_context)(AVCodecContext** avctx) = nullptr;
    int (*avcodec_open2)(AVCodecContext* avctx, const AVCodec* codec, AVDictionary** options) = nullptr;
    int (*avcodec_parameters_from_context)(AVCodecParameters* par, const AVCodecContext* codec) = nullptr;
    int (*avcodec_parameters_to_context)(AVCodecContext* codec, const AVCodecParameters* par) = nullptr;
    int (*avcodec_parameters_copy)(AVCodecParameters* dst, const AVCodecParameters* src) = nullptr;
    int (*avcodec_send_frame)(AVCodecContext* avctx, const AVFrame* frame) = nullptr;
    int (*avcodec_receive_packet)(AVCodecContext* avctx, AVPacket* avpkt) = nullptr;
    int (*avcodec_send_packet)(AVCodecContext* avctx, const AVPacket* avpkt) = nullptr;
    int (*avcodec_receive_frame)(AVCodecContext* avctx, AVFrame* frame) = nullptr;
    AVFrame*(*av_frame_alloc)(void) = nullptr;
    void (*av_frame_free)(AVFrame** frame) = nullptr;
    AVPacket*(*av_packet_alloc)(void) = nullptr;
    void (*av_packet_free)(AVPacket** pkt) = nullptr;
    void (*av_packet_unref)(AVPacket* pkt) = nullptr;
    void (*av_packet_rescale_ts)(AVPacket* pkt, AVRational tb_src, AVRational tb_dst) = nullptr;

    // libswscale
    SwsContext*(*sws_getCachedContext)(SwsContext* context, int srcW, int srcH, AVPixelFormat srcFormat, int dstW, int dstH,
                                       AVPixelFormat dstFormat, int flags, SwsFilter* srcFilter, SwsFilter* dstFilter,
                                       const double* param) = nullptr;
    int (*sws_scale)(struct SwsContext* c, const uint8_t* const srcSlice[], const int srcStride[], int srcSliceY, int srcSliceH,
                     uint8_t* const dst[], const int dstStride[]) = nullptr;

private: void* getSymbol(void* lib, const char* name) const;
    bool tryLoadPath(void*& lib, const io::path_t& fullPath);
    void closeLib(void*& lib);
    void clearFunctions();
    bool functionsValid() const;

    void* m_avUtilLibrary = nullptr;
    void* m_avCodecLibrary = nullptr;
    void* m_avFormatLibrary = nullptr;
    void* m_swsScaleLibrary = nullptr;
    void* m_swResampleLibrary = nullptr;
};
}
