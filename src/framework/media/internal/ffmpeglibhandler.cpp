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

#include "internal/ffmpegutils.h"

#include "log.h"

using namespace muse::media;

static int versionFromAVFormatPath(const muse::io::path_t& path)
{
    std::string name = muse::io::filename(path, true).toStdString();
    int avVer = -1;
#if defined(Q_OS_MAC)
    // libavformat.60.dylib
    size_t dot = name.rfind('.');
    if (dot != std::string::npos && dot > 0) {
        size_t verStart = name.rfind('.', dot - 1);
        if (verStart != std::string::npos && verStart + 1 < dot) {
            try {
                avVer = std::stoi(name.substr(verStart + 1, dot - verStart - 1));
            } catch (...) {}
        }
    }
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    // libavformat.so.60
    size_t so = name.rfind(".so.");
    if (so != std::string::npos && so + 4 < name.size()) {
        try {
            avVer = std::stoi(name.substr(so + 4));
        } catch (...) {}
    }
#elif defined(Q_OS_WIN)
    // avformat-60.dll
    size_t dash = name.find('-');
    if (dash != std::string::npos && dash + 1 < name.size()) {
        size_t dot = name.find('.', dash);
        if (dot != std::string::npos) {
            try {
                avVer = std::stoi(name.substr(dash + 1, dot - dash - 1));
            } catch (...) {}
        }
    }
#endif

    for (const auto& [ver, componentsVersions]: FFMPEG_COMPONENTS_VERSIONS) {
        if (componentsVersions.avFormatVersion == avVer) {
            return ver;
        }
    }

    return 0;
}

#define RESOLVE_FROM(lib, name) do { \
        m_functions.name = reinterpret_cast<decltype(m_functions.name)>(getSymbol(lib, #name)); \
        if (!m_functions.name) { LOGW() << "FFmpeg: missing symbol " #name; return false; } \
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
    if (!m_avUtilLibrary || !m_avCodecLibrary || !m_avFormatLibrary || !m_swsScaleLibrary) {
        return false;
    }

    m_functions = FFmpegFunctions();

    // libavutil
    RESOLVE_FROM(m_avUtilLibrary, av_free);
    RESOLVE_FROM(m_avUtilLibrary, av_freep);
    RESOLVE_FROM(m_avUtilLibrary, av_rescale_q);
    RESOLVE_FROM(m_avUtilLibrary, av_image_fill_arrays);
    RESOLVE_FROM(m_avUtilLibrary, av_image_get_buffer_size);
    RESOLVE_FROM(m_avUtilLibrary, av_opt_set_int);
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

    return m_functions.isValid();
}

bool FFmpegLibHandler::loadLib(const io::path_t& avUtilPath, const io::path_t& avCodecPath, const io::path_t& avFormatPath,
                               const io::path_t& swScalePath)
{
    unload();

    // Load avutil first, then avcodec, swscale, avformat (dependency order)
    if (!tryLoadPath(m_avUtilLibrary, avUtilPath)
        || !tryLoadPath(m_avCodecLibrary, avCodecPath)
        || !tryLoadPath(m_swsScaleLibrary, swScalePath)
        || !tryLoadPath(m_avFormatLibrary, avFormatPath)) {
        return false;
    }

    m_version = versionFromAVFormatPath(avFormatPath);
    m_dir = io::dirpath(avFormatPath);

    LOGI() << "FFmpeg loaded: avutil=" << avUtilPath << ", avcodec=" << avCodecPath << ", avformat=" << avFormatPath
           << ", swscale=" << swScalePath;

    return true;
}

void FFmpegLibHandler::unload()
{
    closeLib(m_avFormatLibrary);
    closeLib(m_swsScaleLibrary);
    closeLib(m_avCodecLibrary);
    closeLib(m_avUtilLibrary);

    m_functions = FFmpegFunctions();
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
