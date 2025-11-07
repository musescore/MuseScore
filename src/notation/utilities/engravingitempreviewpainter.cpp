/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "engravingitempreviewpainter.h"

#include "engraving/dom/masterscore.h"
#include "engraving/dom/textlinebase.h"
#include "engraving/rendering/paintoptions.h"
#include "engraving/style/defaultstyle.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse::draw;

/// Paint `element` centered inside `params.rect`. If `params.numStaffLines >
/// 0`, then the element is only centered horizontally; i.e. vertical alignment
/// is unchanged from the default so that item will appear at the correct height
/// on the staff.
void EngravingItemPreviewPainter::paintPreview(mu::engraving::EngravingItem* element, PaintParams& params)
{
    IF_ASSERT_FAILED(element && params.painter) {
        return;
    }

    Painter* painter = params.painter;
    painter->setPen(params.color);

    PointF rectOrigin = params.rect.center(); // draw element at center of cell by default
    if (params.numStaffLines > 0) {
        const double topLinePos = paintStaff(params); // draw dummy staff lines onto rect.
        rectOrigin.setY(topLinePos); // vertical position relative to staff instead of cell center.
    }

    painter->translate(rectOrigin);

    painter->translate(params.xoffset * params.spatium, params.yoffset * params.spatium); // additional offset for element only

    const double sizeRatio = params.spatium / gpaletteScore->style().spatium();
    painter->scale(sizeRatio, sizeRatio); // scale coordinates so element is drawn at correct size

    // calculate bbox
    engravingRender()->layoutItem(element);

    PointF elementOrigin = element->ldata()->bbox().center();

    if (params.numStaffLines > 0) {
        // y = 0 is position of the element's parent.
        // If the parent is the staff (or a segment on the staff) then
        // y = 0 corresponds to the position of the top staff line.
        elementOrigin.setY(0.0);
    }

    // shift coordinates so element is drawn at correct position
    painter->translate(-elementOrigin);

    paintItem(element, params);
}

void EngravingItemPreviewPainter::paintItem(mu::engraving::EngravingItem* element, PaintParams& params)
{
    IF_ASSERT_FAILED(element && params.painter) {
        return;
    }

    const auto doPaint = [](void* context, EngravingItem* item) {
        PaintParams* ctx = static_cast<PaintParams*>(context);
        Painter* painter = ctx->painter;

        painter->save();
        painter->translate(item->pos()); // necessary for drawing child items

        const Color colorBackup = item->getProperty(Pid::COLOR).value<Color>();
        const Color frameColorBackup = item->getProperty(Pid::FRAME_FG_COLOR).value<Color>();

        if (!ctx->useElementColors) {
            const Color color = ctx->color;
            item->setProperty(Pid::COLOR, color);
            item->setProperty(Pid::FRAME_FG_COLOR, color);
        }

        rendering::PaintOptions opt;
        opt.invertColors = ctx->colorsInversionEnabled;

        engravingRender()->drawItem(item, painter, opt);

        item->setProperty(Pid::COLOR, colorBackup);
        item->setProperty(Pid::FRAME_FG_COLOR, frameColorBackup);

        painter->restore();
    };

    element->scanElements(&params, doPaint);
}

/// Paint a staff centered within a QRect and return the distance from the
/// top of the QRect to the uppermost staff line.
double EngravingItemPreviewPainter::paintStaff(PaintParams& params)
{
    IF_ASSERT_FAILED(params.painter) {
        return 0.0;
    }

    Painter* painter = params.painter;
    painter->save();

    Color staffLinesColor(params.color);
    staffLinesColor.setAlpha(127); //reduce alpha of staff lines to half
    Pen pen(staffLinesColor);
    pen.setWidthF(engraving::DefaultStyle::defaultStyle().styleS(Sid::staffLineWidth).val() * params.spatium);
    painter->setPen(pen);

    const double staffHeight = params.spatium * (params.numStaffLines - 1);
    const double topLineDist = params.rect.center().y() - (staffHeight / 2.0);

    // lines bounded horizontally by edge of target (with small margin)
    constexpr double margin = 3.0;
    const double x1 = params.rect.left() + margin;
    const double x2 = params.rect.right() - margin;

    // draw staff lines with middle line centered vertically on target
    double y = topLineDist;
    for (int i = 0; i < params.numStaffLines; ++i) {
        painter->drawLine(LineF(x1, y, x2, y));
        y += params.spatium;
    }

    painter->restore();
    return topLineDist;
}
