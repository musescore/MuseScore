/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
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

#include <gtest/gtest.h>

#include <algorithm>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include "media/internal/ffmpegutils.h"

using namespace muse;
using namespace muse::media;

namespace {
QString libraryFileName(const QString& component, int version)
{
#if defined(Q_OS_MAC)
    return QStringLiteral("lib%1.%2.dylib").arg(component).arg(version);
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    return QStringLiteral("lib%1.so.%2").arg(component).arg(version);
#elif defined(Q_OS_WIN)
    return QStringLiteral("%1-%2.dll").arg(component).arg(version);
#endif
}

void createFile(const QString& path)
{
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.close();
}

void createLibraries(const QString& dirPath, FFmpegVersion version, bool complete = true)
{
    ASSERT_TRUE(QDir().mkpath(dirPath));

    const auto versionIt = std::find_if(FFMPEG_COMPONENTS_VERSIONS.cbegin(), FFMPEG_COMPONENTS_VERSIONS.cend(),
                                        [version](const auto& item) { return item.first == version; });
    ASSERT_NE(versionIt, FFMPEG_COMPONENTS_VERSIONS.cend());

    const FFmpegVersionInfo& components = versionIt->second;
    const QDir dir(dirPath);
    createFile(dir.filePath(libraryFileName(QStringLiteral("avutil"), components.avUtilVersion)));
    createFile(dir.filePath(libraryFileName(QStringLiteral("avcodec"), components.avCodecVersion)));
    createFile(dir.filePath(libraryFileName(QStringLiteral("avformat"), components.avFormatVersion)));
    createFile(dir.filePath(libraryFileName(QStringLiteral("swscale"), components.swScaleVersion)));
    if (complete) {
        createFile(dir.filePath(libraryFileName(QStringLiteral("swresample"), components.swResampleVersion)));
    }
}

QString canonicalPath(const QString& path)
{
    return QFileInfo(path).canonicalFilePath();
}
}

TEST(FFmpegUtilsTests, RecognizesFFmpeg9FromAVFormatPath)
{
    const QString fileName = libraryFileName(QStringLiteral("avformat"), 63);
    EXPECT_EQ(versionFromAVFormatPath(io::path_t(fileName)), FFMPEG_V9);
}

TEST(FFmpegUtilsTests, AutomaticCandidatesPreferNewestVersion)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QString ffmpeg8Dir = tempDir.filePath(QStringLiteral("ffmpeg8"));
    const QString ffmpeg9Dir = tempDir.filePath(QStringLiteral("ffmpeg9"));
    createLibraries(ffmpeg8Dir, FFMPEG_V8);
    createLibraries(ffmpeg9Dir, FFMPEG_V9);

    const FFmpegLibPathsList candidates = findLibraryPaths({}, { io::path_t(ffmpeg8Dir), io::path_t(ffmpeg9Dir) });

    ASSERT_EQ(candidates.size(), 2u);
    EXPECT_EQ(candidates[0].ffmpegVersion, FFMPEG_V9);
    EXPECT_EQ(candidates[0].origin, FFmpegCandidateOrigin::Automatic);
    EXPECT_EQ(candidates[1].ffmpegVersion, FFMPEG_V8);
    EXPECT_EQ(candidates[1].origin, FFmpegCandidateOrigin::Automatic);
}

TEST(FFmpegUtilsTests, CustomPathRetainsPriority)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QString customDir = tempDir.filePath(QStringLiteral("custom"));
    const QString automaticDir = tempDir.filePath(QStringLiteral("automatic"));
    createLibraries(customDir, FFMPEG_V8);
    createLibraries(automaticDir, FFMPEG_V9);

    const FFmpegLibPathsList candidates = findLibraryPaths(io::path_t(customDir), { io::path_t(automaticDir) });

    ASSERT_EQ(candidates.size(), 2u);
    EXPECT_EQ(candidates[0].ffmpegVersion, FFMPEG_V8);
    EXPECT_EQ(candidates[0].origin, FFmpegCandidateOrigin::Configured);
    EXPECT_EQ(candidates[0].searchDir.toQString(), canonicalPath(customDir));
    EXPECT_EQ(candidates[1].ffmpegVersion, FFMPEG_V9);
    EXPECT_EQ(candidates[1].origin, FFmpegCandidateOrigin::Automatic);
}

TEST(FFmpegUtilsTests, RegisteredConfiguredPathRemainsAutomatic)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QString ffmpeg8Dir = tempDir.filePath(QStringLiteral("ffmpeg8"));
    const QString ffmpeg9Dir = tempDir.filePath(QStringLiteral("ffmpeg9"));
    createLibraries(ffmpeg8Dir, FFMPEG_V8);
    createLibraries(ffmpeg9Dir, FFMPEG_V9);

    const FFmpegLibPathsList candidates = findLibraryPaths(io::path_t(ffmpeg8Dir),
                                                           { io::path_t(ffmpeg8Dir), io::path_t(ffmpeg9Dir) });

    ASSERT_EQ(candidates.size(), 2u);
    EXPECT_EQ(candidates[0].ffmpegVersion, FFMPEG_V9);
    EXPECT_EQ(candidates[0].origin, FFmpegCandidateOrigin::Automatic);
    EXPECT_EQ(candidates[1].ffmpegVersion, FFMPEG_V8);
    EXPECT_EQ(candidates[1].origin, FFmpegCandidateOrigin::Automatic);
}

TEST(FFmpegUtilsTests, IncompleteCandidateIsSkipped)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QString incompleteDir = tempDir.filePath(QStringLiteral("incomplete"));
    const QString completeDir = tempDir.filePath(QStringLiteral("complete"));
    createLibraries(incompleteDir, FFMPEG_V9, false);
    createLibraries(completeDir, FFMPEG_V8);

    const FFmpegLibPathsList candidates = findLibraryPaths({}, { io::path_t(incompleteDir), io::path_t(completeDir) });

    ASSERT_EQ(candidates.size(), 1u);
    EXPECT_EQ(candidates[0].ffmpegVersion, FFMPEG_V8);
}

TEST(FFmpegUtilsTests, SameVersionCandidatesUseStablePathOrder)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QString secondDir = tempDir.filePath(QStringLiteral("z-provider"));
    const QString firstDir = tempDir.filePath(QStringLiteral("a-provider"));
    createLibraries(secondDir, FFMPEG_V9);
    createLibraries(firstDir, FFMPEG_V9);

    const FFmpegLibPathsList candidates = findLibraryPaths({}, { io::path_t(secondDir), io::path_t(firstDir) });

    ASSERT_EQ(candidates.size(), 2u);
    EXPECT_EQ(candidates[0].searchDir.toQString(), canonicalPath(firstDir));
    EXPECT_EQ(candidates[1].searchDir.toQString(), canonicalPath(secondDir));
}

TEST(FFmpegUtilsTests, PersistsOnlyRequestedLoadedPath)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QString requestedDir = tempDir.filePath(QStringLiteral("requested"));
    const QString fallbackDir = tempDir.filePath(QStringLiteral("fallback"));
    createLibraries(requestedDir, FFMPEG_V8);
    createLibraries(fallbackDir, FFMPEG_V9);

    const FFmpegLibPathsList candidates = findLibraryPaths(io::path_t(requestedDir), { io::path_t(fallbackDir) });
    ASSERT_EQ(candidates.size(), 2u);

    const std::optional<io::path_t> requestedUpdate = configuredPathToPersist(io::path_t(requestedDir), candidates[0]);
    ASSERT_TRUE(requestedUpdate.has_value());
    EXPECT_EQ(requestedUpdate->toQString(), canonicalPath(requestedDir));

    EXPECT_FALSE(configuredPathToPersist(io::path_t(requestedDir), candidates[1]).has_value());
}

TEST(FFmpegUtilsTests, EmptyRequestedPathClearsConfiguration)
{
    FFmpegLibPaths automaticCandidate;
    automaticCandidate.searchDir = io::path_t("/automatic");

    const std::optional<io::path_t> update = configuredPathToPersist({}, automaticCandidate);
    ASSERT_TRUE(update.has_value());
    EXPECT_TRUE(update->empty());
}
