/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
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

#include "notevelocityoverlay.h"

#include <algorithm>
#include <limits>

#include <QMouseEvent>
#include <QPainter>

using namespace mu::notation;

constexpr static qreal EDGE_HIT_MARGIN_PX = 4.0;
constexpr static qreal BAR_HALF_WIDTH_MARGIN_PX = 1.0; // keeps adjacent chord bars from visually touching

NoteVelocityOverlay::NoteVelocityOverlay(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::LeftButton);
}

void NoteVelocityOverlay::setRects(const QVector<RectData>& rects)
{
    m_rects = rects;
    update();
}

const QVector<NoteVelocityOverlay::RectData>& NoteVelocityOverlay::rects() const
{
    return m_rects;
}

void NoteVelocityOverlay::updateRect(int index, const RectData& rect)
{
    if (index < 0 || index >= m_rects.size()) {
        return;
    }

    m_rects[index] = rect;
    update();
}

void NoteVelocityOverlay::setFillColor(const QColor& color)
{
    m_fillColor = color;
    update();
}

void NoteVelocityOverlay::setSelectedFillColor(const QColor& color)
{
    m_selectedFillColor = color;
    update();
}

void NoteVelocityOverlay::setModifiedFillColor(const QColor& color)
{
    m_modifiedFillColor = color;
    update();
}

void NoteVelocityOverlay::setBorderColor(const QColor& color)
{
    m_borderColor = color;
    update();
}

void NoteVelocityOverlay::paint(QPainter* painter)
{
    if (m_rects.isEmpty()) {
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(m_borderColor, 1.0));

    // Bars are stored in back-to-front paint order (see header comment) - simply painting each
    // one's fully opaque body in order reproduces the stacked/overlapping look of a DAW velocity
    // lane, with no extra bookkeeping needed here.
    for (const RectData& rect : m_rects) {
        const qreal leftPx = rect.leftN * width() + BAR_HALF_WIDTH_MARGIN_PX;
        const qreal rightPx = rect.rightN * width() - BAR_HALF_WIDTH_MARGIN_PX;
        const qreal topPx = rect.yTopN * height();
        const qreal basePx = rect.y0N * height();

        const QRectF barRect(leftPx, topPx, std::max(0.0, rightPx - leftPx), std::max(0.0, basePx - topPx));

        painter->setBrush(rect.selected ? m_selectedFillColor : (rect.userModified ? m_modifiedFillColor : m_fillColor));
        painter->drawRect(barRect);
    }
}

int NoteVelocityOverlay::hitTestPx(const QPointF& posPx) const
{
    // Only bars whose horizontal span contains the click are candidates - chord columns never
    // overlap in X, so this alone isolates the relevant column.
    QVector<int> candidates;
    for (int i = 0; i < m_rects.size(); ++i) {
        const RectData& r = m_rects.at(i);
        const qreal leftPx = r.leftN * width();
        const qreal rightPx = r.rightN * width();
        if (posPx.x() >= leftPx && posPx.x() <= rightPx) {
            candidates.push_back(i);
        }
    }

    if (candidates.isEmpty()) {
        return -1;
    }

    // candidates preserve the original back-to-front order - scanning in reverse visits the
    // frontmost (lowest-pitched) bar first, exactly matching what's actually visible on screen.
    qreal minTopSoFarPx = std::numeric_limits<qreal>::max();
    for (auto it = candidates.rbegin(); it != candidates.rend(); ++it) {
        const RectData& r = m_rects.at(*it);
        const qreal topPx = r.yTopN * height();
        const qreal basePx = r.y0N * height();
        const qreal exposedBottomPx = std::min(basePx, minTopSoFarPx);

        if (posPx.y() >= topPx - EDGE_HIT_MARGIN_PX && posPx.y() <= exposedBottomPx) {
            return *it;
        }

        minTopSoFarPx = std::min(minTopSoFarPx, topPx);
    }

    return -1;
}

void NoteVelocityOverlay::mousePressEvent(QMouseEvent* e)
{
    const int hit = hitTestPx(e->position());
    if (hit < 0) {
        e->ignore();
        return;
    }

    m_pressed = true;
    m_activeRectIndex = hit;
    e->accept();
}

void NoteVelocityOverlay::mouseMoveEvent(QMouseEvent* e)
{
    if (!m_pressed) {
        return;
    }

    const qreal yN = std::clamp(e->position().y() / std::max(1.0, height()), 0.0, 1.0);
    emit barDragged(m_activeRectIndex, yN, false);
}

void NoteVelocityOverlay::mouseReleaseEvent(QMouseEvent* e)
{
    if (!m_pressed) {
        return;
    }

    const qreal yN = std::clamp(e->position().y() / std::max(1.0, height()), 0.0, 1.0);
    emit barDragged(m_activeRectIndex, yN, true);

    m_pressed = false;
    m_activeRectIndex = -1;
}

void NoteVelocityOverlay::mouseUngrabEvent()
{
    // The mouse grab taken in mousePressEvent can be stolen mid-drag (e.g. a popup opening) -
    // without this, mouseReleaseEvent never fires and this item is left thinking a drag is still
    // active. Treat it as a cancel rather than guessing a commit at an unknown final position.
    m_pressed = false;
    m_activeRectIndex = -1;
}
