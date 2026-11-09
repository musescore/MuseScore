/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include <algorithm>

#include "io/fileinfo.h"
#include "io/path.h"

#if defined(Q_OS_WIN)
#include <QSettings>
#endif

using namespace muse::media;
using namespace muse;

namespace muse::media {
io::paths_t defaultSearchPaths()
{
    io::paths_t paths;
#if defined(Q_OS_WIN)
    QSettings settings(
        "HKEY_LOCAL_MACHINE\\Software\\FFmpeg\\Providers",
        QSettings::NativeFormat
        );

    for (const QString& child : settings.childGroups()) {
        settings.beginGroup(child);
        QVariant value = settings.value("InstallPath");
        if (value.isValid()) {
            QString path = value.toString();
            if (!path.isEmpty()) {
                paths.push_back(path.replace('\\', '/') + "/bin");
            }
        }
        settings.endGroup(); // Back to parent group.
    }
#elif defined(Q_OS_MAC)
    RetVal<io::paths_t> providers = io::Dir::scanFiles("/Library/Application Support/FFmpeg/Providers", { "*" },
                                                       io::ScanMode::FoldersInCurrentDir);
    if (providers.ret) {
        for (const io::path_t& dir : providers.val) {
            paths.push_back(dir + "/lib");
        }
    }
    paths.push_back("/opt/homebrew/lib");
    paths.push_back("/usr/local/lib");
#else
    paths.push_back("/usr/lib/x86_64-linux-gnu");
    paths.push_back("/usr/lib64");
    paths.push_back("/usr/lib");
#endif
    return paths;
}

static FFmpegLibPaths libraryPathsForVersion(int ffmpegVer, const io::path_t& searchDir, FFmpegCandidateOrigin origin)
{
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
    avformatName = io::path_t("libavformat.so." + std::to_string(ffmpegVersionInfo.avFormatVersion));
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
        result.ffmpegVersion = ffmpegVer;
        result.origin = origin;
        result.searchDir = searchDir;
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

    return FFMPEG_INVALID_VERSION;
}

static io::path_t normalizedSearchPath(const io::path_t& path)
{
    const io::FileInfo fileInfo(path);
    const String canonicalPath = fileInfo.canonicalFilePath();
    return canonicalPath.empty() ? io::path_t(fileInfo.absoluteFilePath()) : io::path_t(canonicalPath);
}

static String searchPathKey(const io::path_t& path)
{
#if defined(Q_OS_WIN)
    return path.toString().toLower();
#else
    return path.toString();
#endif
}

static bool containsSearchPath(const io::paths_t& paths, const io::path_t& path)
{
    const String pathKey = searchPathKey(path);
    return std::find_if(paths.cbegin(), paths.cend(), [&pathKey](const io::path_t& candidate) {
        return searchPathKey(candidate) == pathKey;
    }) != paths.cend();
}

static io::paths_t normalizedSearchPaths(const io::paths_t& paths)
{
    io::paths_t result;
    result.reserve(paths.size());

    for (const io::path_t& path : paths) {
        const io::path_t normalizedPath = normalizedSearchPath(path);
        if (normalizedPath.empty() || containsSearchPath(result, normalizedPath)) {
            continue;
        }

        result.push_back(normalizedPath);
    }

    std::sort(result.begin(), result.end(), [](const io::path_t& first, const io::path_t& second) {
        return searchPathKey(first) < searchPathKey(second);
    });
    return result;
}

static void appendLibraryPathsForAllVersions(FFmpegLibPathsList& result, const io::path_t& searchPath,
                                             FFmpegCandidateOrigin origin)
{
    for (const auto& [ffmpegVersion, _] : FFMPEG_COMPONENTS_VERSIONS) {
        FFmpegLibPaths paths = libraryPathsForVersion(ffmpegVersion, searchPath, origin);
        if (!paths.avFormatPath.empty()) {
            result.push_back(std::move(paths));
        }
    }
}

FFmpegLibPathsList findLibraryPaths(const io::path_t& configPath)
{
    return findLibraryPaths(configPath, defaultSearchPaths());
}

FFmpegLibPathsList findLibraryPaths(const io::path_t& configPath, const io::paths_t& defaultPaths)
{
    FFmpegLibPathsList result;
    const io::paths_t automaticSearchPaths = normalizedSearchPaths(defaultPaths);

    if (!configPath.empty() && io::FileInfo::exists(configPath)) {
        const io::path_t configuredSearchPath = normalizedSearchPath(
            io::FileInfo(configPath).entryType() == io::EntryType::Dir ? configPath : io::dirpath(configPath));

        if (!containsSearchPath(automaticSearchPaths, configuredSearchPath)) {
            appendLibraryPathsForAllVersions(result, configuredSearchPath, FFmpegCandidateOrigin::Configured);
        }
    }

    FFmpegLibPathsList automaticCandidates;
    for (const io::path_t& path : automaticSearchPaths) {
        appendLibraryPathsForAllVersions(automaticCandidates, path, FFmpegCandidateOrigin::Automatic);
    }

    std::sort(automaticCandidates.begin(), automaticCandidates.end(), [](const FFmpegLibPaths& first, const FFmpegLibPaths& second) {
        if (first.ffmpegVersion != second.ffmpegVersion) {
            return first.ffmpegVersion > second.ffmpegVersion;
        }
        return searchPathKey(first.searchDir) < searchPathKey(second.searchDir);
    });

    result.insert(result.end(), automaticCandidates.cbegin(), automaticCandidates.cend());

    return result;
}

std::optional<io::path_t> configuredPathToPersist(const io::path_t& requestedPath, const FFmpegLibPaths& loadedCandidate)
{
    if (requestedPath.empty()) {
        return io::path_t();
    }

    const io::FileInfo requestedFileInfo(requestedPath);
    if (!requestedFileInfo.exists()) {
        return std::nullopt;
    }

    const io::path_t requestedSearchPath = normalizedSearchPath(
        requestedFileInfo.entryType() == io::EntryType::Dir ? requestedPath : io::dirpath(requestedPath));
    if (searchPathKey(requestedSearchPath) != searchPathKey(loadedCandidate.searchDir)) {
        return std::nullopt;
    }

    return requestedSearchPath;
}
}
