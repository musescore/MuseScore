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

#include "io/msczwriter.h"
#include "io/msczreader.h"

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
        MsczWriter writer(&buf);
        writer.setFilePath("simple1.mscz");
        writer.open();

        writer.writeScore(originScoreData);
        writer.writeThumbnail(originThumbnailData);
        writer.addImage("image1.png", originImageData);
    }

    //! CHECK Read and compare with origin
    {
        QBuffer buf(&msczData);
        MsczReader reader(&buf);
        reader.setFilePath("simple1.mscz");
        reader.open();

        QByteArray scoreData = reader.readScore();
        EXPECT_EQ(scoreData, originScoreData);

        QByteArray thumbnailData = reader.readThumbnail();
        EXPECT_EQ(thumbnailData, originThumbnailData);

        std::vector<QString> images = reader.imageFileNames();
        QByteArray imageData = reader.readImage("image1.png");
        EXPECT_EQ(images.size(), 1);
        EXPECT_EQ(images.at(0), "image1.png");
        EXPECT_EQ(imageData, originImageData);
    }
}
