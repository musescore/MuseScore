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

#include <cstring>

#include "draw/types/drawdata.h"
#include "draw/painter.h"
#include "draw/bufferedpaintprovider.h"
#include "draw/utils/drawdatarw.h"
#include "draw/utils/drawdatacomp.h"

#include "global/io/file.h"

#include "diagnostics/internal/drawdata/drawdataconverter.h"
#include "diagnostics/internal/drawdata/drawdatagenerator.h"

#include "log.h"

using namespace mu;
using namespace mu::draw;
using namespace mu::diagnostics;

const io::path_t DATA_ROOT(diagnostics_tests_DATA_ROOT);
const io::path_t VTEST_SCORES = DATA_ROOT + "/../../../vtest/scores";

class Diagnostics_DrawDataTests : public ::testing::Test
{
public:
};

void saveAsPng(const io::path_t& path, const DrawDataPtr& data)
{
    DrawDataConverter c;
    Pixmap px = c.drawDataToPixmap(data);
    io::File::writeFile(path, px.data());
}

void saveDiff(const io::path_t& path, const DrawDataPtr& origin, const DrawDataPtr& diff)
{
    DrawDataConverter c;
    Pixmap px(std::lrint(origin->viewport.width()), std::lrint(origin->viewport.height()));
    c.drawOnPixmap(px, origin, Color("#999999"));
    c.drawOnPixmap(px, diff, Color("#ff0000"));
    io::File::writeFile(path, px.data());
}

TEST_F(Diagnostics_DrawDataTests, SimpleDraw)
{
    DrawDataPtr data;
    // paint
    {
        std::shared_ptr<BufferedPaintProvider> prv = std::make_shared<BufferedPaintProvider>();
        Painter p(prv, "test");

        PointF pos(120, 240);
        p.translate(pos);

        Pen pen(Color::GREEN);
        p.setPen(pen);
        p.drawLine(0, 0, 120, 0);
        p.drawLine(0, 20, 120, 20);

        p.translate(-pos);

        p.endDraw();

        data = prv->drawData();
    }

    // convert
    {
        DrawDataConverter c;
        Pixmap px = c.drawDataToPixmap(data);
        io::File::writeFile("paint.png", px.data());
    }
}

TEST_F(Diagnostics_DrawDataTests, ScoreDraw)
{
    Pixmap originImage;
    {
        DrawDataGenerator g;
        originImage = g.genImage(VTEST_SCORES + "/accidental-1.mscx");
        io::File::writeFile("2_accidental-1.origin.png", originImage.data());
    }

    DrawDataPtr drawData;
    {
        DrawDataGenerator g;
        drawData = g.genDrawData(VTEST_SCORES + "/accidental-1.mscx");
        DrawDataRW::writeData("2_accidental-1.json", drawData);
    }

    Pixmap dataImage;
    {
        DrawDataConverter c;
        dataImage = c.drawDataToPixmap(drawData);
        io::File::writeFile("2_accidental-1.data.png", dataImage.data());
    }

    EXPECT_EQ(originImage, dataImage);
}

TEST_F(Diagnostics_DrawDataTests, DrawDiff)
{
    DrawDataPtr origin;
    // paint 1
    {
        std::shared_ptr<BufferedPaintProvider> prv = std::make_shared<BufferedPaintProvider>();
        Painter p(prv, "test");

        RectF viewport(0, 0, 300, 300);
        p.setViewport(viewport);
        p.setWindow(viewport);

        p.setBrush(Color::WHITE);
        p.drawRect(viewport);

        p.setPen(Color::GREEN);

        PointF pos(60, 30);
        p.translate(pos);

        p.drawLine(0, 0, 120, 0);
        p.drawLine(0, 20, 120, 20);

        p.translate(-pos);

        p.endDraw();

        origin = prv->drawData();
    }

    DrawDataPtr test;
    // paint 2
    {
        std::shared_ptr<BufferedPaintProvider> prv = std::make_shared<BufferedPaintProvider>();
        Painter p(prv, "test");

        RectF viewport(0, 0, 300, 300);
        p.setViewport(viewport);
        p.setWindow(viewport);

        p.setBrush(Color::WHITE);
        p.drawRect(viewport);

        p.setPen(Color::GREEN);

        PointF pos(60, 30);
        p.translate(pos);

        p.drawLine(0, 0, 120, 0);
        p.drawLine(0, 22, 120, 22);

        p.translate(-pos);

        p.endDraw();

        test = prv->drawData();
    }

    // compare

    Diff diff = DrawDataComp::compare(origin, test);

    saveAsPng("dd_origin.png", origin);
    saveAsPng("dd_test.png", test);
    saveAsPng("dd_added.png", diff.dataAdded);
    saveAsPng("dd_removed.png", diff.dataRemoved);
    saveDiff("dd_diff.png", origin, diff.dataAdded);

    const DrawDataPtr& dd = diff.dataAdded;
    EXPECT_EQ(dd->objects.size(), 1);
    const DrawData::Object& ddo = dd->objects.at(0);
    EXPECT_EQ(ddo.datas.size(), 1);
    const DrawData::Data& ddd = ddo.datas.at(0);
    EXPECT_EQ(ddd.polygons.size(), 1);
}

TEST_F(Diagnostics_DrawDataTests, ScoreDrawDiff)
{
    DrawDataPtr data1;
    {
        DrawDataGenerator g;
        data1 = g.genDrawData(VTEST_SCORES + "/accidental-1.mscx");
    }

    DrawDataPtr data2;
    {
        DrawDataGenerator g;
        data2 = g.genDrawData(VTEST_SCORES + "/accidental-2.mscx");
    }

    Diff diff = DrawDataComp::compare(data1, data2);

    DrawDataRW::writeData("4_data1.json", data1);
    saveAsPng("4_data1.png", data1);

    DrawDataRW::writeData("4_data2.json", data2);
    saveAsPng("4_data2.png", data2);

    saveAsPng("4_added.png", diff.dataAdded);

    saveDiff("4_diff.png", data1, diff.dataAdded);
}
