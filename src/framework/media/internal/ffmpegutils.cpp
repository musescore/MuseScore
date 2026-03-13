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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ffmpegutils.h"

#include "io/fileinfo.h"
#include "io/path.h"

using namespace muse::media;
using namespace muse;

namespace muse::media {
static io::paths_t defaultSearchPaths()
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

static FFmpegLibPaths libraryPathsForVersion(int ffmpegVer, const io::path_t& searchDir)
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

    io::path_t avutilName, avcodecName, avformatName, swscaleName, swresampleName;
#if defined(Q_OS_MAC)
    avutilName = io::path_t("libavutil." + std::to_string(ffmpegVersionInfo.avUtilVersion) + ".dylib");
    avcodecName = io::path_t("libavcodec." + std::to_string(ffmpegVersionInfo.avCodecVersion) + ".dylib");
    avformatName = io::path_t("libavformat." + std::to_string(ffmpegVersionInfo.avFormatVersion) + ".dylib");
    swscaleName = io::path_t("libswscale." + std::to_string(ffmpegVersionInfo.swScaleVersion) + ".dylib");
    swresampleName = io::path_t("libswresample." + std::to_string(ffmpegVersionInfo.swResampleVersion) + ".dylib");
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    avutilName = io::path_t("libavutil.so." + std::to_string(ffmpegVersionInfo.avUtilVersion));
    avcodecName = io::path_t("libavcodec.so." + std::to_string(ffmpegVersionInfo.avCodecVersion));
    avformatName = io::path_t("libavformat.so." + verStr);
    swscaleName = io::path_t("libswscale.so." + std::to_string(ffmpegVersionInfo.swScaleVersion));
    swresampleName = io::path_t("libswresample.so." + std::to_string(ffmpegVersionInfo.swResampleVersion));
#elif defined(Q_OS_WIN)
    avutilName = io::path_t("avutil-" + std::to_string(ffmpegVersionInfo.avUtilVersion) + ".dll");
    avcodecName = io::path_t("avcodec-" + std::to_string(ffmpegVersionInfo.avCodecVersion) + ".dll");
    avformatName = io::path_t("avformat-" + std::to_string(ffmpegVersionInfo.avFormatVersion) + ".dll");
    swscaleName = io::path_t("swscale-" + std::to_string(ffmpegVersionInfo.swScaleVersion) + ".dll");
    swresampleName = io::path_t("swresample-" + std::to_string(ffmpegVersionInfo.swResampleVersion) + ".dll");
#endif

    FFmpegLibPaths result;
    io::path_t avutilPath = searchDir.appendingComponent(avutilName);
    io::path_t avcodecPath = searchDir.appendingComponent(avcodecName);
    io::path_t avformatPath = searchDir.appendingComponent(avformatName);
    io::path_t swscalePath = searchDir.appendingComponent(swscaleName);
    io::path_t swresamplePath = searchDir.appendingComponent(swresampleName);
    if (io::FileInfo::exists(avutilPath) && io::FileInfo::exists(avcodecPath)
        && io::FileInfo::exists(avformatPath) && io::FileInfo::exists(swscalePath)
        && io::FileInfo::exists(swresamplePath)) {
        result.avUtilPath = avutilPath;
        result.avCodecPath = avcodecPath;
        result.avFormatPath = avformatPath;
        result.swScalePath = swscalePath;
        result.swResamplePath = swresamplePath;
        return result;
    }

    return result;
}

FFmpegVersion versionFromAVFormatPath(const io::path_t& path)
{
    std::string name = io::filename(path, true).toStdString();
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

    for (const auto& [ver, componentsVersions] : FFMPEG_COMPONENTS_VERSIONS) {
        if (componentsVersions.avFormatVersion == avVer) {
            return ver;
        }
    }

    return FFMPEG_INVALID_VERION;
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

    for (const io::path_t& path : searchPaths) {
        for (const auto& [ffmpegVer, _] : FFMPEG_COMPONENTS_VERSIONS) {
            result = libraryPathsForVersion(ffmpegVer, path);
            if (!result.avFormatPath.empty()) {
                return result;
            }
        }
    }

    return result;
}
}
