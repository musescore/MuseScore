/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <QTemporaryDir>

#include "engraving/infrastructure/mscreader.h"
#include "engraving/infrastructure/mscwriter.h"
#include "project/internal/projectvideosettings.h"

using namespace muse;
using namespace mu::engraving;
using namespace mu::project;

static VideoAttachmentSettings readAttachmentFromJson(const QString& json)
{
    QTemporaryDir dir;
    EXPECT_TRUE(dir.isValid());

    MscWriter::Params writerParams;
    writerParams.filePath = dir.path();
    writerParams.mode = MscIoMode::Dir;

    MscWriter writer(writerParams);
    EXPECT_TRUE(writer.open());
    writer.writeVideoSettingsJsonFile(ByteArray::fromQByteArray(json.toUtf8()));
    writer.close();

    MscReader::Params readerParams;
    readerParams.filePath = dir.path();
    readerParams.mode = MscIoMode::Dir;

    MscReader reader(readerParams);
    EXPECT_TRUE(reader.open());

    ProjectVideoSettings settings;
    EXPECT_TRUE(settings.read(reader));
    return settings.attachment();
}

TEST(ProjectVideoSettingsTests, MissingSettingsReadAsDefault)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    MscReader::Params readerParams;
    readerParams.filePath = dir.path();
    readerParams.mode = MscIoMode::Dir;

    MscReader reader(readerParams);
    ASSERT_TRUE(reader.open());

    ProjectVideoSettings settings;
    EXPECT_TRUE(settings.read(reader));

    EXPECT_FALSE(settings.attachment().isValid());
}

TEST(ProjectVideoSettingsTests, InvalidSettingsReadAsDefault)
{
    VideoAttachmentSettings attachment = readAttachmentFromJson("{");

    EXPECT_FALSE(attachment.isValid());
}

TEST(ProjectVideoSettingsTests, UnknownVersionReadsAsDefault)
{
    VideoAttachmentSettings attachment = readAttachmentFromJson(R"({"version":999,"attachment":{"path":"media/reference-picture.mp4"}})");

    EXPECT_FALSE(attachment.isValid());
}

TEST(ProjectVideoSettingsTests, MissingAttachmentReadsAsDefault)
{
    VideoAttachmentSettings attachment = readAttachmentFromJson(R"({"version":1})");

    EXPECT_FALSE(attachment.isValid());
}

TEST(ProjectVideoSettingsTests, WriteAndReadAttachment)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    MscWriter::Params writerParams;
    writerParams.filePath = dir.path();
    writerParams.mode = MscIoMode::Dir;

    MscWriter writer(writerParams);
    EXPECT_TRUE(writer.open());

    ProjectVideoSettings source;
    VideoAttachmentSettings attachment;
    attachment.path = "media/reference-picture.mp4";
    attachment.offsetMs = -1250;
    attachment.volume = 0.75f;
    attachment.balance = -0.25f;
    attachment.muted = true;
    attachment.solo = true;
    attachment.frameRate = 29.97;
    attachment.timecodeDisplayMode = VideoTimecodeDisplayMode::AboveBars;

    VideoHitPointSettings firstHitPoint;
    firstHitPoint.label = u"Door slam";
    firstHitPoint.timeMs = 3200;
    firstHitPoint.color = 0xD94A4A;
    attachment.hitPoints.push_back(firstHitPoint);

    VideoHitPointSettings secondHitPoint;
    secondHitPoint.label = u"Cut";
    secondHitPoint.timeMs = 9600;
    secondHitPoint.color = 0x3B94E5;
    attachment.hitPoints.push_back(secondHitPoint);

    source.setAttachment(attachment);
    EXPECT_TRUE(source.write(writer));
    writer.close();

    MscReader::Params readerParams;
    readerParams.filePath = dir.path();
    readerParams.mode = MscIoMode::Dir;

    MscReader reader(readerParams);
    ASSERT_TRUE(reader.open());

    ProjectVideoSettings restored;
    EXPECT_TRUE(restored.read(reader));

    EXPECT_EQ(restored.attachment(), attachment);
}
