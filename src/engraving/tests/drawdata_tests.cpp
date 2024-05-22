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

#include <cstring>

#include "draw/types/drawdata.h"
#include "draw/painter.h"
#include "draw/bufferedpaintprovider.h"
#include "draw/utils/drawdatarw.h"
#include "draw/utils/drawdatacomp.h"

#include "global/io/file.h"

#include "devtools/drawdata/drawdataconverter.h"
#include "devtools/drawdata/drawdatagenerator.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace muse::draw;
using namespace mu::engraving;

const muse::io::path_t DATA_ROOT(engraving_tests_DATA_ROOT);
const muse::io::path_t VTEST_SCORES = DATA_ROOT + "/../../../vtest/scores";

class Engraving_DrawDataTests : public ::testing::Test
{
public:
};

void saveAsPng(const muse::io::path_t& path, const DrawDataPtr& data)
{
    DrawDataConverter c;
    Pixmap px = c.drawDataToPixmap(data);
    io::File::writeFile(path, px.data());
}

void saveDiff(const muse::io::path_t& path, const DrawDataPtr& origin, const DrawDataPtr& diff)
{
    DrawDataConverter c;
    Pixmap px(std::lrint(origin->viewport.width()), std::lrint(origin->viewport.height()));
    c.drawOnPixmap(px, origin, Color("#999999"));
    c.drawOnPixmap(px, diff, Color("#ff0000"));
    io::File::writeFile(path, px.data());
}

TEST_F(Engraving_DrawDataTests, DISABLED_Rw)
{
    DrawDataPtr origin;
    {
        DrawDataGenerator g(muse::modularity::globalCtx());
        origin = g.genDrawData(VTEST_SCORES + "/accidental-1.mscx");
        DrawDataRW::writeData("rw_data.origin.json", origin);
    }

    DrawDataPtr readed;
    {
        readed = DrawDataRW::readData("rw_data.origin.json").val;
    }

    muse::SetCompareRealPrecision(3);

    EXPECT_EQ(origin->item.chilren.size(), readed->item.chilren.size());

    for (size_t i = 0; i < origin->item.chilren.size(); ++i) {
        const DrawData::Item& originObj = origin->item.chilren.at(i);
        const DrawData::Item& readedObj = readed->item.chilren.at(i);
        EXPECT_EQ(originObj.datas.size(), readedObj.datas.size());

        for (size_t j = 0; j < originObj.datas.size(); ++j) {
            const DrawData::Data& originData = originObj.datas.at(j);
            const DrawData::Data& readedData = readedObj.datas.at(j);
            // state
            const DrawData::State& originState = origin->states.at(originData.state);
            const DrawData::State& readedState = readed->states.at(readedData.state);

            EXPECT_EQ(originState.pen, readedState.pen);
            EXPECT_EQ(originState.brush, readedState.brush);
            EXPECT_EQ(originState.font, readedState.font);
            EXPECT_EQ(originState.transform, readedState.transform);
            EXPECT_EQ(originState.isAntialiasing, readedState.isAntialiasing);
            EXPECT_EQ(originState.compositionMode, readedState.compositionMode);

            // data
            EXPECT_EQ(originData.paths, readedData.paths);
            EXPECT_EQ(originData.polygons, readedData.polygons);
            EXPECT_EQ(originData.texts, readedData.texts);
        }
    }
}

TEST_F(Engraving_DrawDataTests, SimpleDraw)
{
    DrawDataPtr data;
    // paint
    {
        std::shared_ptr<BufferedPaintProvider> prv = std::make_shared<BufferedPaintProvider>();
        Painter p(prv, "test");

        p.setViewport(RectF(0, 0, 450, 450));

        p.setAntialiasing(true);
        p.beginObject("page_1");

        PointF pos(120, 240);
        p.translate(pos);

        Pen pen(Color::GREEN);
        p.setPen(pen);
        p.beginObject("line_1");
        p.drawLine(0, 0, 120, 0);
        p.endObject();

        {
            p.beginObject("line_2");
            p.drawLine(0, 20, 120, 20);

            {
                p.beginObject("line_2.2");
                p.drawLine(0, 40, 120, 40);
                p.endObject();
            }
            p.endObject();
        }

        p.translate(-pos);

        p.endObject(); // page_1
        p.endDraw();

        data = prv->drawData();
    }

    DrawDataRW::writeData("1_data.json", data);

    // convert
    {
        DrawDataConverter c;
        Pixmap px = c.drawDataToPixmap(data);
        io::File::writeFile("1_data.png", px.data());
    }
}

TEST_F(Engraving_DrawDataTests, ScoreDraw)
{
    Pixmap originImage;
    {
        DrawDataGenerator g(muse::modularity::globalCtx());
        originImage = g.genImage(VTEST_SCORES + "/accidental-1.mscx");
        io::File::writeFile("2_accidental-1.origin.png", originImage.data());
    }

    {
        PainterItemMarker::enabled = false;
        DrawDataGenerator g(muse::modularity::globalCtx());
        DrawDataPtr drawData = g.genDrawData(VTEST_SCORES + "/accidental-1.mscx");
        DrawDataRW::writeData("2_accidental-1_no_objects.json", drawData);
        PainterItemMarker::enabled = true;
    }

    DrawDataPtr drawData;
    {
        DrawDataGenerator g(muse::modularity::globalCtx());
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

TEST_F(Engraving_DrawDataTests, DrawDiff)
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

    DrawDataRW::writeData("3_origin.json", origin);
    saveAsPng("3_origin.png", origin);

    DrawDataRW::writeData("3_test.json", test);
    saveAsPng("3_test.png", test);

    DrawDataRW::writeData("3_added.json", diff.dataAdded);
    saveAsPng("3_added.png", diff.dataAdded);

    DrawDataRW::writeData("3_removed.json", diff.dataRemoved);
    saveAsPng("3_removed.png", diff.dataRemoved);

    saveDiff("3_diff.png", origin, diff.dataAdded);

    const DrawDataPtr& dd = diff.dataAdded;
    EXPECT_EQ(dd->item.chilren.size(), 1);
    const DrawData::Item& ddo = dd->item.chilren.at(0);
    EXPECT_EQ(ddo.datas.size(), 1);
    const DrawData::Data& ddd = ddo.datas.at(0);
    EXPECT_EQ(ddd.polygons.size(), 1);
}

TEST_F(Engraving_DrawDataTests, ScoreDrawDiff)
{
    DrawDataPtr data1;
    {
        DrawDataGenerator g(muse::modularity::globalCtx());
        data1 = g.genDrawData(VTEST_SCORES + "/accidental-1.mscx");
    }

    DrawDataPtr data2;
    {
        DrawDataGenerator g(muse::modularity::globalCtx());
        data2 = g.genDrawData(VTEST_SCORES + "/accidental-2.mscx");
    }

    Diff diff = DrawDataComp::compare(data1, data2);

    DrawDataRW::writeData("4_data1.json", data1);
    saveAsPng("4_data1.png", data1);

    DrawDataRW::writeData("4_data2.json", data2);
    saveAsPng("4_data2.png", data2);

    DrawDataRW::writeData("4_diff.added.json", diff.dataAdded);
    saveAsPng("4_added.png", diff.dataAdded);

    saveDiff("4_diff.png", data1, diff.dataAdded);
}
