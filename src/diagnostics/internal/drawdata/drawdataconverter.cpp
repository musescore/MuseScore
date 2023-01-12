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
#include "drawdataconverter.h"

#include "global/io/file.h"
#include "draw/utils/drawdatarw.h"
#include "draw/utils/drawdatapaint.h"
#include "draw/painter.h"

#include "log.h"

using namespace mu;
using namespace mu::diagnostics;
using namespace mu::draw;

Ret DrawDataConverter::drawDataToPng(const io::path_t& dataFile, const io::path_t& outFile) const
{
    RetVal<DrawDataPtr> drawData = DrawDataRW::readData(dataFile);
    if (!drawData.ret) {
        return drawData.ret;
    }

    Pixmap png = drawDataToPixmap(drawData.val);
    return io::File::writeFile(outFile, png.data());
}

Pixmap DrawDataConverter::drawDataToPixmap(const DrawDataPtr& data) const
{
    IF_ASSERT_FAILED(data) {
        return Pixmap();
    }

    QPixmap px(std::lrint(data->viewport.width()), std::lrint(data->viewport.height()));

    Painter painter(&px, "DrawData");

    DrawDataPaint::paint(&painter, data);

    painter.endDraw();

    Pixmap png = Pixmap::fromQPixmap(px);
    return png;
}
