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

#include "engraving/dom/actionicon.h"
#include "engraving/style/defaultstyle.h"
#include "engraving/dom/masterscore.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse::draw;

/// Paint an EngravingItem - include a staff if specified in the params.
void EngravingItemPreviewPainter::paintPreview(mu::engraving::EngravingItem* element, PaintParams& params)
{
    IF_ASSERT_FAILED(element && params.painter) {
        return;
    }

    Painter* painter = params.painter;
    painter->setPen(params.color);

    if (element->isActionIcon()) {
        paintPreviewForActionIcon(element, params);
        return; // never draw staff for icon elements
    }

    PointF origin = params.rect.center(); // draw element at center of cell by default
    if (params.drawStaff) {
        const double topLinePos = paintStaff(params); // draw dummy staff lines onto rect.
        origin.setY(topLinePos); // vertical position relative to staff instead of cell center.
    }

    painter->translate(origin);

    painter->translate(params.xoffset * params.spatium, params.yoffset * params.spatium); // additional offset for element only

    paintPreviewForItem(element, params);
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
        const bool colorsInversionEnabledBackup = item->colorsInversionEnabled();

        item->setColorsInverionEnabled(ctx->colorsInversionEnabled);

        if (!ctx->useElementColors) {
            const Color color = ctx->color;
            item->setProperty(Pid::COLOR, color);
            item->setProperty(Pid::FRAME_FG_COLOR, color);
        }

        engravingRender()->drawItem(item, painter);

        item->setColorsInverionEnabled(colorsInversionEnabledBackup);
        item->setProperty(Pid::COLOR, colorBackup);
        item->setProperty(Pid::FRAME_FG_COLOR, frameColorBackup);

        painter->restore();
    };

    element->scanElements(&params, doPaint);
}

/// Paint an icon element so that it fills a QRect, preserving aspect ratio, and
/// leaving a small margin around the edges.
void EngravingItemPreviewPainter::paintPreviewForActionIcon(mu::engraving::EngravingItem* element, PaintParams& params)
{
    IF_ASSERT_FAILED(element && element->isActionIcon() && params.painter) {
        return;
    }

    Painter* painter = params.painter;
    painter->save();

    double DPIscaling = (mu::engraving::DPI / mu::engraving::DPI_F) / params.dpi;

    ActionIcon* action = toActionIcon(element);
    action->setFontSize(ActionIcon::DEFAULT_FONT_SIZE * params.mag * DPIscaling);

    engravingRender()->layoutItem(action);

    painter->translate(params.rect.center() - action->ldata()->bbox().center());
    engravingRender()->drawItem(action, painter);
    painter->restore();
}

/// Paint a 5 line staff centered within a QRect and return the distance from the
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

    constexpr int numStaffLines = 5;
    const double staffHeight = params.spatium * (numStaffLines - 1);
    const double topLineDist = params.rect.center().y() - (staffHeight / 2.0);

    // lines bounded horizontally by edge of target (with small margin)
    constexpr double margin = 3.0;
    const double x1 = params.rect.left() + margin;
    const double x2 = params.rect.right() - margin;

    // draw staff lines with middle line centered vertically on target
    double y = topLineDist;
    for (int i = 0; i < numStaffLines; ++i) {
        painter->drawLine(LineF(x1, y, x2, y));
        y += params.spatium;
    }

    painter->restore();
    return topLineDist;
}

/// Paint a non-icon element centered at the origin of the painter's coordinate
/// system. If drawStaff is true then the element is only centered horizontally;
/// i.e. vertical alignment is unchanged from the default so that item will appear
/// at the correct height on the staff.
void EngravingItemPreviewPainter::paintPreviewForItem(mu::engraving::EngravingItem* element, PaintParams& params)
{
    IF_ASSERT_FAILED(element && !element->isActionIcon() && params.painter) {
        return;
    }

    Painter* painter = params.painter;
    painter->save();

    mu::engraving::MScore::pixelRatio = mu::engraving::DPI / params.dpi;

    const double sizeRatio = params.spatium / gpaletteScore->style().spatium();
    painter->scale(sizeRatio, sizeRatio); // scale coordinates so element is drawn at correct size

    // calculate bbox
    engravingRender()->layoutItem(element);

    PointF origin = element->ldata()->bbox().center();

    if (params.drawStaff) {
        // y = 0 is position of the element's parent.
        // If the parent is the staff (or a segment on the staff) then
        // y = 0 corresponds to the position of the top staff line.
        origin.setY(0.0);
    }

    painter->translate(-1.0 * origin); // shift coordinates so element is drawn at correct position

    paintItem(element, params);
    painter->restore();
}
