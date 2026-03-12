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

#include "ffmpeglibhandler.h"

#if !defined(Q_OS_WIN)
#include <dlfcn.h>
#endif

#include "io/fileinfo.h"
#include "global/dlib.h"

#include "log.h"

namespace muse::media::ffmpeg::v7 {
#define RESOLVE_FROM(lib, name) do { \
        name = reinterpret_cast<decltype(name)>(getSymbol(lib, #name)); \
        if (!name) { LOGW() << "FFmpeg: missing symbol " #name; return false; } \
} while (0)

void* FFmpegLibHandler::getSymbol(void* lib, const char* name) const
{
    if (!lib) {
        return nullptr;
    }
    void* sym = muse::getLibFunc(lib, name);
    if (!sym) {
#if !defined(Q_OS_WIN)
        sym = dlsym(RTLD_DEFAULT, name);
#endif
    }
    return sym;
}

bool FFmpegLibHandler::loadApi()
{
    if (!m_avUtilLibrary || !m_avCodecLibrary || !m_avFormatLibrary || !m_swsScaleLibrary || !m_swResampleLibrary) {
        return false;
    }

    clearFunctions();

    // libavutil
    RESOLVE_FROM(m_avUtilLibrary, av_free);
    RESOLVE_FROM(m_avUtilLibrary, av_freep);
    RESOLVE_FROM(m_avUtilLibrary, av_rescale_q);
    RESOLVE_FROM(m_avUtilLibrary, av_image_fill_arrays);
    RESOLVE_FROM(m_avUtilLibrary, av_image_get_buffer_size);
    RESOLVE_FROM(m_avUtilLibrary, av_opt_set);
    RESOLVE_FROM(m_avUtilLibrary, av_opt_set_int);
    RESOLVE_FROM(m_avUtilLibrary, av_opt_set_pixel_fmt);
    RESOLVE_FROM(m_avUtilLibrary, av_frame_alloc);

    // libavformat
    RESOLVE_FROM(m_avFormatLibrary, av_guess_format);
    RESOLVE_FROM(m_avFormatLibrary, av_write_trailer);
    RESOLVE_FROM(m_avFormatLibrary, av_interleaved_write_frame);
    RESOLVE_FROM(m_avFormatLibrary, av_read_frame);
    RESOLVE_FROM(m_avFormatLibrary, avio_open);
    RESOLVE_FROM(m_avFormatLibrary, avio_close);
    RESOLVE_FROM(m_avFormatLibrary, avformat_alloc_context);
    RESOLVE_FROM(m_avFormatLibrary, avformat_new_stream);
    RESOLVE_FROM(m_avFormatLibrary, avformat_write_header);
    RESOLVE_FROM(m_avFormatLibrary, avformat_open_input);
    RESOLVE_FROM(m_avFormatLibrary, avformat_find_stream_info);
    RESOLVE_FROM(m_avFormatLibrary, avformat_close_input);
    RESOLVE_FROM(m_avFormatLibrary, avformat_free_context);
    RESOLVE_FROM(m_avFormatLibrary, avformat_alloc_output_context2);

    // libavcodec
    RESOLVE_FROM(m_avCodecLibrary, av_packet_alloc);
    RESOLVE_FROM(m_avCodecLibrary, av_packet_free);
    RESOLVE_FROM(m_avCodecLibrary, av_packet_unref);
    RESOLVE_FROM(m_avCodecLibrary, av_packet_rescale_ts);
    RESOLVE_FROM(m_avCodecLibrary, avcodec_alloc_context3);
    RESOLVE_FROM(m_avCodecLibrary, avcodec_find_encoder);
    RESOLVE_FROM(m_avCodecLibrary, avcodec_free_context);
    RESOLVE_FROM(m_avCodecLibrary, avcodec_open2);
    RESOLVE_FROM(m_avCodecLibrary, avcodec_parameters_from_context);
    RESOLVE_FROM(m_avCodecLibrary, avcodec_parameters_copy);
    RESOLVE_FROM(m_avCodecLibrary, avcodec_send_frame);
    RESOLVE_FROM(m_avCodecLibrary, avcodec_receive_packet);

    // libswscale
    RESOLVE_FROM(m_swsScaleLibrary, sws_getCachedContext);
    RESOLVE_FROM(m_swsScaleLibrary, sws_scale);

    return functionsValid();
}

bool FFmpegLibHandler::loadLib(const io::path_t& avUtilPath, const io::path_t& avCodecPath, const io::path_t& avFormatPath,
                               const io::path_t& swScalePath, const io::path_t& swResamplePath)
{
    unload();

    // Load avutil first, then swresample, swscale, avcodec, avformat (dependency order)
    if (!tryLoadPath(m_avUtilLibrary, avUtilPath)
        || !tryLoadPath(m_swResampleLibrary, swResamplePath)
        || !tryLoadPath(m_swsScaleLibrary, swScalePath)
        || !tryLoadPath(m_avCodecLibrary, avCodecPath)
        || !tryLoadPath(m_avFormatLibrary, avFormatPath)) {
        return false;
    }

    LOGI() << "FFmpeg loaded: avutil=" << avUtilPath << ", avcodec=" << avCodecPath << ", avformat=" << avFormatPath
           << ", swscale=" << swScalePath << ", swresample=" << swResamplePath;

    return true;
}

void FFmpegLibHandler::unload()
{
    closeLib(m_avFormatLibrary);
    closeLib(m_swResampleLibrary);
    closeLib(m_swsScaleLibrary);
    closeLib(m_avCodecLibrary);
    closeLib(m_avUtilLibrary);

    clearFunctions();
    m_version = 0;
    m_dir = io::path_t();
}

void FFmpegLibHandler::closeLib(void*& lib)
{
    if (lib) {
        muse::closeLib(lib);
        lib = nullptr;
    }
}

bool FFmpegLibHandler::tryLoadPath(void*& lib, const io::path_t& fullPath)
{
    if (!io::FileInfo::exists(fullPath)) {
        return false;
    }

    void* loaded = muse::loadLib(fullPath);
    if (loaded) {
        lib = loaded;
        return true;
    }

    return false;
}

bool FFmpegLibHandler::functionsValid() const
{
    return av_free && av_freep && av_rescale_q
           && av_image_fill_arrays && av_image_get_buffer_size
           && av_opt_set && av_opt_set_int && av_opt_set_pixel_fmt
           && av_guess_format && avformat_alloc_context && avformat_new_stream
           && avformat_write_header && av_write_trailer
           && avio_open && avio_close && av_interleaved_write_frame
           && avformat_open_input && avformat_find_stream_info
           && avformat_close_input && avformat_free_context
           && avformat_alloc_output_context2 && av_read_frame
           && avcodec_alloc_context3 && avcodec_find_encoder && avcodec_free_context
           && avcodec_open2 && avcodec_parameters_from_context && avcodec_parameters_copy
           && avcodec_send_frame && avcodec_receive_packet
           && av_frame_alloc
           && av_packet_alloc && av_packet_free && av_packet_unref && av_packet_rescale_ts
           && sws_getCachedContext && sws_scale;
}

void FFmpegLibHandler::clearFunctions()
{
    av_free = nullptr;
    av_freep = nullptr;
    av_rescale_q = nullptr;
    av_image_fill_arrays = nullptr;
    av_image_get_buffer_size = nullptr;
    av_opt_set = nullptr;
    av_opt_set_int = nullptr;
    av_opt_set_pixel_fmt = nullptr;
    av_guess_format = nullptr;
    avformat_alloc_context = nullptr;
    avformat_new_stream = nullptr;
    avformat_write_header = nullptr;
    av_write_trailer = nullptr;
    avio_open = nullptr;
    avio_close = nullptr;
    av_interleaved_write_frame = nullptr;
    avformat_open_input = nullptr;
    avformat_find_stream_info = nullptr;
    avformat_close_input = nullptr;
    avformat_free_context = nullptr;
    avformat_alloc_output_context2 = nullptr;
    av_read_frame = nullptr;
    avcodec_alloc_context3 = nullptr;
    avcodec_find_encoder = nullptr;
    avcodec_free_context = nullptr;
    avcodec_open2 = nullptr;
    avcodec_parameters_from_context = nullptr;
    avcodec_parameters_copy = nullptr;
    avcodec_send_frame = nullptr;
    avcodec_receive_packet = nullptr;
    av_frame_alloc = nullptr;
    av_packet_alloc = nullptr;
    av_packet_free = nullptr;
    av_packet_unref = nullptr;
    av_packet_rescale_ts = nullptr;
    sws_getCachedContext = nullptr;
    sws_scale = nullptr;
}
}
