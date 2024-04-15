/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "engraving/dom/mscore.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::draw;

static const Color REF_COLOR("#999999");
static const Color ADDED_COLOR("#ff0000");

Ret DrawDataConverter::drawDataToPng(const muse::io::path_t& dataFile, const muse::io::path_t& outFile)
{
    RetVal<DrawDataPtr> drawData = DrawDataRW::readData(dataFile);
    if (!drawData.ret) {
        return drawData.ret;
    }

    return saveAsPng(drawData.val, outFile);
}

Ret DrawDataConverter::drawDiffToPng(const muse::io::path_t& diffFile, const muse::io::path_t& refFile, const muse::io::path_t& outFile)
{
    RetVal<Diff> diff = DrawDataRW::readDiff(diffFile);
    if (!diff.ret) {
        return diff.ret;
    }

    RetVal<DrawDataPtr> refData;
    if (!refFile.empty()) {
        refData = DrawDataRW::readData(refFile);
        if (!refData.ret) {
            return refData.ret;
        }
    }

    Pixmap px(std::lrint(diff.val.dataAdded->viewport.width()), std::lrint(diff.val.dataAdded->viewport.height()));
    if (refData.val) {
        drawOnPixmap(px, refData.val, REF_COLOR);
    }

    drawOnPixmap(px, diff.val.dataAdded, ADDED_COLOR);

    return io::File::writeFile(outFile, px.data());
}

Ret DrawDataConverter::saveAsPng(const DrawDataPtr& data, const muse::io::path_t& path)
{
    Pixmap px = drawDataToPixmap(data);
    return io::File::writeFile(path, px.data());
}

Pixmap DrawDataConverter::drawDataToPixmap(const DrawDataPtr& data)
{
    IF_ASSERT_FAILED(data) {
        return Pixmap();
    }

    int width = std::lrint(data->viewport.width());
    int height = std::lrint(data->viewport.height());

    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    image.setDotsPerMeterX(std::lrint((DrawData::CANVAS_DPI * 1000) / mu::engraving::INCH));
    image.setDotsPerMeterY(std::lrint((DrawData::CANVAS_DPI * 1000) / mu::engraving::INCH));

    image.fill(Qt::white);

    Painter painter(&image, "DrawData");

    DrawDataPaint::paint(&painter, data);

    painter.endDraw();

    Pixmap png = Pixmap::fromQImage(image);
    return png;
}

void DrawDataConverter::drawOnPixmap(Pixmap& px, const DrawDataPtr& data, const Color& overlay)
{
    QImage qim = Pixmap::toQImage(px);
    qim.setDotsPerMeterX(std::lrint((DrawData::CANVAS_DPI * 1000) / mu::engraving::INCH));
    qim.setDotsPerMeterY(std::lrint((DrawData::CANVAS_DPI * 1000) / mu::engraving::INCH));

    Painter painter(&qim, "DrawData");
    DrawDataPaint::paint(&painter, data, overlay);
    painter.endDraw();

    px = Pixmap::fromQImage(qim);
}
