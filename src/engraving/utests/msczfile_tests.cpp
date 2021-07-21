/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include <QByteArray>
#include <QBuffer>

#include "io/mscwriter.h"
#include "io/mscreader.h"

using namespace mu::engraving;

class MsczFileTests : public ::testing::Test
{
public:
};

TEST_F(MsczFileTests, MsczFile_WriteRead)
{
    //! CASE Writing and reading multiple datas

    //! GIVEN Some datas

    const QByteArray originScoreData("score");
    const QByteArray originImageData("image");
    const QByteArray originThumbnailData("thumbnail");

    //! DO Write datas
    QByteArray msczData;
    {
        QBuffer buf(&msczData);
        MscWriter::Params params;
        params.device = &buf;
        params.filePath = "simple1.mscz";
        params.mode = MscIoMode::Zip;

        MscWriter writer(params);
        writer.open();

        writer.writeScoreFile(originScoreData);
        writer.writeThumbnailFile(originThumbnailData);
        writer.addImageFile("image1.png", originImageData);
    }

    //! CHECK Read and compare with origin
    {
        QBuffer buf(&msczData);
        MscReader::Params params;
        params.device = &buf;
        params.filePath = "simple1.mscz";
        params.mode = MscIoMode::Zip;

        MscReader reader(params);
        reader.open();

        QByteArray scoreData = reader.readScoreFile();
        EXPECT_EQ(scoreData, originScoreData);

        QByteArray thumbnailData = reader.readThumbnailFile();
        EXPECT_EQ(thumbnailData, originThumbnailData);

        std::vector<QString> images = reader.imageFileNames();
        QByteArray imageData = reader.readImageFile("image1.png");
        EXPECT_EQ(images.size(), 1);
        EXPECT_EQ(images.at(0), "image1.png");
        EXPECT_EQ(imageData, originImageData);
    }
}
