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
        io::File::writeFile("accidental-1.origin.png", originImage.data());
    }

    DrawDataPtr drawData;
    {
        DrawDataGenerator g;
        drawData = g.genDrawData(VTEST_SCORES + "/accidental-1.mscx");
        DrawDataRW::writeData("accidental-1.json", drawData);
    }

    Pixmap dataImage;
    {
        DrawDataConverter c;
        dataImage = c.drawDataToPixmap(drawData);
        io::File::writeFile("accidental-1.data.png", dataImage.data());
    }

    EXPECT_EQ(originImage, dataImage);
}
