/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "previewmeasure.h"

#include "engraving/dom/staff.h"
#include "engraving/dom/measure.h"
#include "draw/painter.h"
#include "draw/internal/qpainterprovider.h"

#include <QPainter>
#include <QLinearGradient>

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;
using namespace muse::draw;

void PreviewMeasure::paint(Painter* painter, const NoteInputState& state)
{
    const Score* score = state.staff()->score();
    const Measure* lastMeasure = score->lastMeasure();
    const System* lastSystem = lastMeasure->system();

    if (!lastMeasure) {
        return;
    }

    // Keep in sync with NotationInteraction::previewMeasureRect
    const Fraction tick = lastMeasure->endTick();
    const double spatium = score->style().spatium();
    const double previewWidth = 6 * spatium;

    const PointF measurePos = lastMeasure->canvasPos();
    const double startX = measurePos.x() + lastMeasure->width();

    for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        Staff* staff = score->staff(staffIdx);
        SysStaff* sysStaff = lastSystem->staff(staffIdx);

        if (!sysStaff->show()) {
            continue;
        }

        const int staffLines = staff->lines(tick);
        const double lineDist = staff->staffType(tick)->lineDistance().toMM(staff->staffMag(tick) * spatium);
        const double lineWidth = score->style().styleMM(Sid::staffLineWidth) * staff->staffMag(tick);

        const double staffY = measurePos.y() + sysStaff->y();

        paintStaffLines(painter, PointF(startX, staffY), previewWidth, staffLines, lineDist, lineWidth);
    }
}

void PreviewMeasure::paintStaffLines(Painter* painter, const PointF& pos, double width, int lines, double lineDist, double lineWidth)
{
    if (lines <= 0 || width <= 0) {
        return;
    }

    QPainterProvider* qpainterProvider = dynamic_cast<QPainterProvider*>(painter->provider().get());
    QPainter* qpainter = qpainterProvider->qpainter();

    QColor lineColor = configuration()->noteInputPreviewColor().toQColor();

    QLinearGradient gradient(pos.x(), pos.y(), pos.x() + width, pos.y());
    gradient.setColorAt(0, lineColor);
    lineColor.setAlpha(0);
    gradient.setColorAt(1, lineColor);

    qpainter->setBrush(Qt::NoBrush);
    qpainter->setPen(QPen(gradient, lineWidth, Qt::SolidLine, Qt::FlatCap));

    for (int i = 0; i < lines; ++i) {
        double y = pos.y() + i * lineDist;
        qpainter->drawLine(QLineF(pos.x(), y, pos.x() + width, y));
    }
}
