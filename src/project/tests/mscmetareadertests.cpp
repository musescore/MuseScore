/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include <filesystem>

#include <QString>

#include "global/io/path.h"

#include "project/internal/mscmetareader.h"

namespace stdfs = std::filesystem;
using namespace Qt::StringLiterals;
using namespace mu::project;

namespace {
muse::io::path_t getDataPath(const stdfs::path& relativePath)
{
    const auto absolutePath = stdfs::path(project_test_DATA_ROOT) / relativePath;
    return absolutePath.generic_string();
}
}

TEST(ProjectMscMetaReaderTests, testReadFromMeta)
{
    auto metaReader = std::make_shared<MscMetaReader>();
    muse::RetVal<ProjectMeta> maybeMeta = metaReader->readMeta(getDataPath("from_meta/from_meta.mscx"));
    ASSERT_TRUE(maybeMeta.ret);

    const ProjectMeta& meta = maybeMeta.val;
    EXPECT_EQ(meta.title, u"Title from tag"_s);
    EXPECT_TRUE(meta.subtitle.isEmpty());
    EXPECT_EQ(meta.composer, u"Composer from tag"_s);
    EXPECT_EQ(meta.arranger, u"Arranger from tag"_s);
    EXPECT_EQ(meta.lyricist, u"Lyricist from tag"_s);
    EXPECT_EQ(meta.translator, u"Translator from tag"_s);
    EXPECT_EQ(meta.copyright, u"Copyright from tag"_s);
    EXPECT_EQ(meta.creationDate, QDate(2025, 12, 6));

    EXPECT_EQ(meta.partsCount, 1);
    EXPECT_FALSE(meta.thumbnail.isNull());
    EXPECT_TRUE(meta.source.isEmpty());
    EXPECT_TRUE(meta.audioComUrl.isEmpty());
    EXPECT_TRUE(meta.platform.isEmpty());
    EXPECT_TRUE(meta.musescoreVersion.isEmpty());
    EXPECT_EQ(meta.musescoreRevision, 0);
    EXPECT_EQ(meta.mscVersion, 0);
}

TEST(ProjectMscMetaReaderTests, testReadFromMetaAndBox)
{
    auto metaReader = std::make_shared<MscMetaReader>();
    muse::RetVal<ProjectMeta> maybeMeta = metaReader->readMeta(getDataPath("from_meta_and_box/from_meta_and_box.mscx"));
    ASSERT_TRUE(maybeMeta.ret);

    const ProjectMeta& meta = maybeMeta.val;
    EXPECT_EQ(meta.title, u"Title from box"_s);
    EXPECT_EQ(meta.subtitle, u"Subtitle from box"_s);
    EXPECT_EQ(meta.composer, u"Composer from box"_s);
    EXPECT_EQ(meta.arranger, u"Arranger from tag"_s);
    EXPECT_EQ(meta.lyricist, u"Lyricist from box"_s);
    EXPECT_EQ(meta.translator, u"Translator from tag"_s);
    EXPECT_EQ(meta.copyright, u"Copyright from tag"_s);
    EXPECT_EQ(meta.creationDate, QDate(2025, 12, 6));

    EXPECT_EQ(meta.partsCount, 1);
    EXPECT_FALSE(meta.thumbnail.isNull());
    EXPECT_TRUE(meta.source.isEmpty());
    EXPECT_TRUE(meta.audioComUrl.isEmpty());
    EXPECT_TRUE(meta.platform.isEmpty());
    EXPECT_TRUE(meta.musescoreVersion.isEmpty());
    EXPECT_EQ(meta.musescoreRevision, 0);
    EXPECT_EQ(meta.mscVersion, 0);
}

TEST(ProjectMscMetaReaderTests, testReadCloudInfo)
{
    auto metaReader = std::make_shared<MscMetaReader>();
    muse::RetVal<CloudProjectInfo> maybeCloudInfo = metaReader->readCloudProjectInfo(getDataPath("from_meta/from_meta.mscx"));
    ASSERT_TRUE(maybeCloudInfo.ret);

    const CloudProjectInfo& cloudInfo = maybeCloudInfo.val;
    EXPECT_TRUE(cloudInfo.sourceUrl.isEmpty());
    EXPECT_EQ(cloudInfo.revisionId, 42);
    EXPECT_TRUE(cloudInfo.name.isEmpty());
}
