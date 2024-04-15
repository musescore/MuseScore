/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "io/buffer.h"
#include "infrastructure/mscwriter.h"
#include "infrastructure/mscreader.h"

using namespace mu;
using namespace muse;
using namespace muse::io;
using namespace mu::engraving;

class Engraving_MsczFileTests : public ::testing::Test
{
public:
};

TEST_F(Engraving_MsczFileTests, MsczFile_WriteRead)
{
    //! CASE Writing and reading multiple datas

    //! GIVEN Some datas

    const ByteArray originScoreData("score");
    const ByteArray originImageData("image");
    const ByteArray originThumbnailData("thumbnail");

    //! DO Write datas
    ByteArray msczData;
    {
        Buffer buf(&msczData);
        MscWriter::Params params;
        params.device = &buf;
        params.filePath = "simple1.mscz";
        params.mode = MscIoMode::Zip;

        MscWriter writer(params);
        writer.open();

        writer.writeScoreFile(originScoreData);
        writer.writeThumbnailFile(originThumbnailData);
        writer.addImageFile(u"image1.png", originImageData);
    }

    //! CHECK Read and compare with origin
    {
        Buffer buf(&msczData);
        MscReader::Params params;
        params.device = &buf;
        params.filePath = "simple1.mscz";
        params.mode = MscIoMode::Zip;

        MscReader reader(params);
        reader.open();

        ByteArray scoreData = reader.readScoreFile();
        EXPECT_EQ(scoreData, originScoreData);

        ByteArray thumbnailData = reader.readThumbnailFile();
        EXPECT_EQ(thumbnailData, originThumbnailData);

        std::vector<String> images = reader.imageFileNames();
        ByteArray imageData = reader.readImageFile(u"image1.png");
        EXPECT_EQ(images.size(), 1);
        EXPECT_EQ(images.at(0), u"image1.png");
        EXPECT_EQ(imageData, originImageData);
    }
}
