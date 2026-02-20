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
#include "engraving/dom/shadownote.h"
#include "draw/painter.h"
#include "draw/internal/qpainterprovider.h"

#include <QPainter>
#include <QLinearGradient>

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;
using namespace muse::draw;

void PreviewMeasure::setScore(const Score* score)
{
    m_score = score;
}

bool PreviewMeasure::isValid() const
{
    if (!m_score || !m_score->inputState().noteEntryMode()) {
        return false;
    }

    const ShadowNote* note = m_score->shadowNote();
    return (note && note->visible() && note->isBeyondScore()) || m_score->inputState().beyondScore();
}

muse::RectF PreviewMeasure::rect() const
{
    if (!isValid()) {
        return muse::RectF();
    }

    const Measure* lastMeasure = m_score->lastMeasure();
    const PointF lastMeasurePos = lastMeasure->canvasPos();

    const Fraction tick = lastMeasure->endTick();
    const double spatium = m_score->style().spatium();
    const double previewWidth = 6 * spatium;

    RectF rect = RectF(lastMeasurePos.x() + lastMeasure->width(), lastMeasurePos.y(),
                       previewWidth, lastMeasure->height());

    const Staff* firstStaff = m_score->staves().front();
    const double firstStaffLineWidth = m_score->style().styleAbsolute(Sid::staffLineWidth) * firstStaff->staffMag(tick);

    const Staff* lastStaff = m_score->staves().back();
    const double lastStaffLineWidth = m_score->style().styleAbsolute(Sid::staffLineWidth) * lastStaff->staffMag(tick);

    rect.adjust(0.0, -0.5 * firstStaffLineWidth, 0.0, 0.5 * lastStaffLineWidth);

    return rect;
}

void PreviewMeasure::paint(Painter* painter)
{
    if (!isValid()) {
        return;
    }

    const Measure* lastMeasure = m_score->lastMeasure();
    const System* lastSystem = lastMeasure->system();

    if (!lastMeasure) {
        return;
    }

    const Fraction tick = lastMeasure->endTick();
    const double spatium = m_score->style().spatium();
    const double previewWidth = 6 * spatium;

    const PointF measurePos = lastMeasure->canvasPos();
    const double startX = measurePos.x() + lastMeasure->width();

    QColor lineColor = [&] {
        const Color color = configuration()->noteInputPreviewColor();
        if (color.isValid() && color != configuration()->selectionColor()) {
            return color.toQColor();
        } else {
            return configuration()->selectionColor(m_score->inputState().voice());
        }
    }();

    for (staff_idx_t staffIdx = 0; staffIdx < m_score->nstaves(); ++staffIdx) {
        Staff* staff = m_score->staff(staffIdx);
        SysStaff* sysStaff = lastSystem->staff(staffIdx);

        if (!sysStaff->show()) {
            continue;
        }

        const int staffLines = staff->lines(tick);
        const double lineDist = staff->staffType(tick)->lineDistance().toAbsolute(staff->staffMag(tick) * spatium);
        const double lineWidth = m_score->style().styleAbsolute(Sid::staffLineWidth) * staff->staffMag(tick);

        const double staffY = measurePos.y() + sysStaff->y();

        paintStaffLines(painter, PointF(startX, staffY), previewWidth, staffLines, lineDist, lineWidth, lineColor);
    }
}

void PreviewMeasure::paintStaffLines(Painter* painter, const PointF& pos, double width, int lines, double lineDist, double lineWidth,
                                     QColor lineColor)
{
    if (lines <= 0 || width <= 0) {
        return;
    }

    QPainterProvider* qpainterProvider = dynamic_cast<QPainterProvider*>(painter->provider().get());
    QPainter* qpainter = qpainterProvider->qpainter();

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
