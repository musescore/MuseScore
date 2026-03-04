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

#include "ffmpegloader.h"

#include "io/fileinfo.h"
#include "io/path.h"

#include "ffmpegutils.h"
#include "internal/ffmpeglibhandler.h"

#include "log.h"

using namespace muse::media;
using namespace muse;

namespace {
io::paths_t defaultSearchPaths()
{
    io::paths_t paths;
#if defined(Q_OS_MAC)
    paths.push_back("/opt/homebrew/lib");
    paths.push_back("/usr/local/lib");
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    paths.push_back("/usr/lib/x86_64-linux-gnu");
    paths.push_back("/usr/lib64");
    paths.push_back("/usr/lib");
#elif defined(Q_OS_WIN)
    // Windows: rely on PATH or user-specified path
#endif
    return paths;
}

struct FFmpegLibPaths {
    io::path_t avUtilPath;
    io::path_t avCodecPath;
    io::path_t avFormatPath;
    io::path_t swScalePath;
};

FFmpegLibPaths libraryPathsForVersion(int ffmpegVer, const io::paths_t& searchPaths)
{
    const std::string verStr = std::to_string(ffmpegVer);

    FFmpegVersionInfo ffmpegVersionInfo;
    for (const auto& [ffmpegVersion, componentsVersions] : FFMPEG_COMPONENTS_VERSIONS) {
        if (ffmpegVersion == ffmpegVer) {
            ffmpegVersionInfo = componentsVersions;
            break;
        }
    }

    if (!ffmpegVersionInfo.isValid()) {
        return {};
    }

    io::path_t avutilName, avcodecName, avformatName, swscaleName;
#if defined(Q_OS_MAC)
    avutilName = io::path_t("libavutil." + std::to_string(ffmpegVersionInfo.avUtilVersion) + ".dylib");
    avcodecName = io::path_t("libavcodec." + std::to_string(ffmpegVersionInfo.avCodecVersion) + ".dylib");
    avformatName = io::path_t("libavformat." + std::to_string(ffmpegVersionInfo.avFormatVersion) + ".dylib");
    swscaleName = io::path_t("libswscale." + std::to_string(ffmpegVersionInfo.swScaleVersion) + ".dylib");
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    avutilName = io::path_t("libavutil.so." + std::to_string(ffmpegVersionInfo.avUtilVersion));
    avcodecName = io::path_t("libavcodec.so." + std::to_string(ffmpegVersionInfo.avCodecVersion));
    avformatName = io::path_t("libavformat.so." + verStr);
    swscaleName = io::path_t("libswscale.so." + std::to_string(ffmpegVersionInfo.swScaleVersion));
#elif defined(Q_OS_WIN)
    avutilName = io::path_t("avutil-" + std::to_string(ffmpegVersionInfo.avUtilVersion) + ".dll");
    avcodecName = io::path_t("avcodec-" + std::to_string(ffmpegVersionInfo.avCodecVersion) + ".dll");
    avformatName = io::path_t("avformat-" + std::to_string(ffmpegVersionInfo.avFormatVersion) + ".dll");
    swscaleName = io::path_t("swscale-" + std::to_string(ffmpegVersionInfo.swScaleVersion) + ".dll");
#endif

    FFmpegLibPaths result;
    for (const io::path_t& dir : searchPaths) {
        io::path_t avutilPath = dir.appendingComponent(avutilName);
        io::path_t avcodecPath = dir.appendingComponent(avcodecName);
        io::path_t avformatPath = dir.appendingComponent(avformatName);
        io::path_t swscalePath = dir.appendingComponent(swscaleName);
        if (io::FileInfo::exists(avutilPath) && io::FileInfo::exists(avcodecPath)
            && io::FileInfo::exists(avformatPath) && io::FileInfo::exists(swscalePath)) {
            result.avUtilPath = avutilPath;
            result.avCodecPath = avcodecPath;
            result.avFormatPath = avformatPath;
            result.swScalePath = swscalePath;
            return result;
        }
    }

    return result;
}

FFmpegLibPaths findLibraryPaths(const io::path_t& configPath)
{
    FFmpegLibPaths result;

    io::paths_t searchPaths;
    if (!configPath.empty() && io::FileInfo::exists(configPath)) {
        searchPaths.push_back(io::FileInfo(configPath).entryType() == io::EntryType::Dir
                              ? configPath : io::dirpath(configPath));
    }

    for (const io::path_t& p : defaultSearchPaths()) {
        searchPaths.push_back(p);
    }

    for (const auto& [ffmpegVer, _] : FFMPEG_COMPONENTS_VERSIONS) {
        result = libraryPathsForVersion(ffmpegVer, searchPaths);
        if (!result.avFormatPath.empty()) {
            return result;
        }
    }

    return result;
}
}

std::shared_ptr<FFmpegLibHandler> FFmpegLoader::load(const io::path_t& ffmpegLibsDir)
{
    const FFmpegLibPaths paths = findLibraryPaths(ffmpegLibsDir);
    if (paths.avFormatPath.empty()) {
        return nullptr;
    }

    std::shared_ptr<FFmpegLibHandler> libHandlerPtr = std::make_shared<FFmpegLibHandler>();
    if (libHandlerPtr->loadLib(paths.avUtilPath, paths.avCodecPath, paths.avFormatPath, paths.swScalePath)
        && libHandlerPtr->loadApi()) {
        LOGD() << "FFmpeg loaded, version: " << libHandlerPtr->version();
        return libHandlerPtr;
    }

    LOGW() << "FFmpeg libraries not found";
    return nullptr;
}
