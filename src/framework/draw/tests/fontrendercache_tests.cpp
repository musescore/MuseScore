/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
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

#include "draw/internal/fontrendercache.h"

#include "global/io/file.h"

using namespace muse;
using namespace muse::draw;

class Draw_FontRenderCacheTests : public ::testing::Test
{
public:
};

static FaceKey testFace()
{
    return FaceKey(FontDataKey(u"Edwin"), Font::Type::Text, 20);
}

static GlyphImage testImage(const RectF& rect)
{
    GlyphImage image;
    image.rect = rect;
    image.range = 3.5f;
    image.sdf.width = 64;
    image.sdf.height = 72;
    image.sdf.bitmap = ByteArray("sdf-data");
    return image;
}

TEST_F(Draw_FontRenderCacheTests, MalformedFile)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    FontRenderCache cache;
    cache.setCacheDirPath(io::path_t(dir.path()));

    const io::path_t malformedFilePath = io::path_t(dir.path()).appendingComponent("Edwin_7_0_0_20_[bad|20|0|0|1|1|1].sdf");
    ASSERT_TRUE(io::File::writeFile(malformedFilePath, ByteArray("sdf-data")).success());

    GlyphImage image;
    EXPECT_NO_THROW(image = cache.load(testFace(), 7));

    EXPECT_TRUE(image.isNull());
}

TEST_F(Draw_FontRenderCacheTests, StoreLoad)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    const FaceKey face = testFace();

    GlyphImage storedImage = testImage(RectF(-0.03125, -14.328125, 18.765625, 20.09375));

    FontRenderCache storeCache;
    storeCache.setCacheDirPath(io::path_t(dir.path()));
    storeCache.store(face, 7, storedImage);

    //! DO Load from a new cache instance, like after an application restart
    FontRenderCache loadCache;
    loadCache.setCacheDirPath(io::path_t(dir.path()));
    GlyphImage loadedImage = loadCache.load(face, 7);

    EXPECT_EQ(loadedImage.sdf.width, storedImage.sdf.width);
    EXPECT_EQ(loadedImage.sdf.height, storedImage.sdf.height);
    EXPECT_DOUBLE_EQ(loadedImage.rect.x(), storedImage.rect.x());
    EXPECT_DOUBLE_EQ(loadedImage.rect.y(), storedImage.rect.y());
    EXPECT_DOUBLE_EQ(loadedImage.rect.width(), storedImage.rect.width());
    EXPECT_DOUBLE_EQ(loadedImage.rect.height(), storedImage.rect.height());
    EXPECT_FLOAT_EQ(loadedImage.range, storedImage.range);
}
