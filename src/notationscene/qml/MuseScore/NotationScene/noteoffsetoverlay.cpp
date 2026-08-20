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

#include "noteoffsetoverlay.h"

#include <algorithm>
#include <cmath>

#include <QCursor>
#include <QHoverEvent>
#include <QMouseEvent>
#include <QPainter>

using namespace mu::notation;

constexpr static qreal EDGE_HANDLE_HIT_MARGIN_PX = 6.0;
constexpr static qreal EDGE_HANDLE_WIDTH_PX = 4.0;

NoteOffsetOverlay::NoteOffsetOverlay(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
}

void NoteOffsetOverlay::setRects(const QVector<RectData>& rects)
{
    m_rects = rects;
    update();
}

const QVector<NoteOffsetOverlay::RectData>& NoteOffsetOverlay::rects() const
{
    return m_rects;
}

void NoteOffsetOverlay::updateRect(int index, const RectData& rect)
{
    if (index < 0 || index >= m_rects.size()) {
        return;
    }

    m_rects[index] = rect;
    update();
}

void NoteOffsetOverlay::setFillColor(const QColor& color)
{
    m_fillColor = color;
    update();
}

void NoteOffsetOverlay::setSelectedFillColor(const QColor& color)
{
    m_selectedFillColor = color;
    update();
}

void NoteOffsetOverlay::setModifiedFillColor(const QColor& color)
{
    m_modifiedFillColor = color;
    update();
}

void NoteOffsetOverlay::setBorderColor(const QColor& color)
{
    m_borderColor = color;
    update();
}

void NoteOffsetOverlay::setHandleColor(const QColor& color)
{
    m_handleColor = color;
    update();
}

void NoteOffsetOverlay::setSelectedHandleColor(const QColor& color)
{
    m_selectedHandleColor = color;
    update();
}

void NoteOffsetOverlay::setModifiedHandleColor(const QColor& color)
{
    m_modifiedHandleColor = color;
    update();
}

void NoteOffsetOverlay::paint(QPainter* painter)
{
    if (m_rects.isEmpty()) {
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing);

    for (const RectData& rect : m_rects) {
        const qreal leftPx = rect.leftN * width();
        const qreal rightPx = rect.rightN * width();
        const qreal centerYPx = rect.centerYN * height();
        const qreal halfHeightPx = (rect.heightYN * height()) / 2.0;

        const QRectF bodyRect(leftPx, centerYPx - halfHeightPx, rightPx - leftPx, halfHeightPx * 2.0);

        painter->setPen(QPen(m_borderColor, 1.0));
        painter->setBrush(rect.selected ? m_selectedFillColor : (rect.userModified ? m_modifiedFillColor : m_fillColor));
        painter->drawRect(bodyRect);

        painter->setPen(Qt::NoPen);
        painter->setBrush(rect.selected ? m_selectedHandleColor : (rect.userModified ? m_modifiedHandleColor : m_handleColor));
        if (rect.hasLeftHandle) {
            painter->drawRoundedRect(QRectF(leftPx - EDGE_HANDLE_WIDTH_PX / 2.0, bodyRect.top(), EDGE_HANDLE_WIDTH_PX, bodyRect.height()),
                                     EDGE_HANDLE_WIDTH_PX / 2.0, EDGE_HANDLE_WIDTH_PX / 2.0);
        }
        if (rect.hasRightHandle) {
            painter->drawRoundedRect(QRectF(rightPx - EDGE_HANDLE_WIDTH_PX / 2.0, bodyRect.top(), EDGE_HANDLE_WIDTH_PX, bodyRect.height()),
                                     EDGE_HANDLE_WIDTH_PX / 2.0, EDGE_HANDLE_WIDTH_PX / 2.0);
        }
    }
}

NoteOffsetOverlay::HitResult NoteOffsetOverlay::hitTestPx(const QPointF& posPx) const
{
    for (int i = 0; i < m_rects.size(); ++i) {
        const RectData& rect = m_rects.at(i);
        const qreal centerYPx = rect.centerYN * height();
        const qreal halfHeightPx = (rect.heightYN * height()) / 2.0 + EDGE_HANDLE_HIT_MARGIN_PX;
        if (posPx.y() < centerYPx - halfHeightPx || posPx.y() > centerYPx + halfHeightPx) {
            continue;
        }

        const qreal leftPx = rect.leftN * width();
        const qreal rightPx = rect.rightN * width();

        const bool hitLeft = rect.hasLeftHandle && std::abs(posPx.x() - leftPx) <= EDGE_HANDLE_HIT_MARGIN_PX;
        const bool hitRight = rect.hasRightHandle && std::abs(posPx.x() - rightPx) <= EDGE_HANDLE_HIT_MARGIN_PX;

        if (!hitLeft && !hitRight) {
            continue;
        }

        HitResult hit;
        hit.rectIndex = i;
        hit.isLeftEdge = hitLeft && (!hitRight || std::abs(posPx.x() - leftPx) <= std::abs(posPx.x() - rightPx));
        return hit;
    }

    return HitResult();
}

void NoteOffsetOverlay::updateCursor(bool hoveringEdge)
{
    if (hoveringEdge == m_hoveringEdge) {
        return;
    }
    m_hoveringEdge = hoveringEdge;
    setCursor(hoveringEdge ? Qt::SizeHorCursor : Qt::ArrowCursor);
}

void NoteOffsetOverlay::hoverMoveEvent(QHoverEvent* e)
{
    const HitResult hit = hitTestPx(e->position());
    updateCursor(hit.isValid());
}

void NoteOffsetOverlay::hoverLeaveEvent(QHoverEvent*)
{
    updateCursor(false);
}

void NoteOffsetOverlay::mousePressEvent(QMouseEvent* e)
{
    const HitResult hit = hitTestPx(e->position());
    if (!hit.isValid()) {
        e->ignore();
        return;
    }

    m_pressed = true;
    m_activeRectIndex = hit.rectIndex;
    m_activeIsLeftEdge = hit.isLeftEdge;
    e->accept();
}

void NoteOffsetOverlay::mouseMoveEvent(QMouseEvent* e)
{
    if (!m_pressed) {
        return;
    }

    const qreal xN = std::clamp(e->position().x() / std::max(1.0, width()), 0.0, 1.0);
    emit edgeDragged(m_activeRectIndex, m_activeIsLeftEdge, xN, false);
}

void NoteOffsetOverlay::mouseReleaseEvent(QMouseEvent* e)
{
    if (!m_pressed) {
        return;
    }

    const qreal xN = std::clamp(e->position().x() / std::max(1.0, width()), 0.0, 1.0);
    emit edgeDragged(m_activeRectIndex, m_activeIsLeftEdge, xN, true);

    m_pressed = false;
    m_activeRectIndex = -1;
}

void NoteOffsetOverlay::mouseUngrabEvent()
{
    // The mouse grab taken in mousePressEvent can be stolen mid-drag (e.g. a popup opening) -
    // without this, mouseReleaseEvent never fires and this item is left thinking a drag is still
    // active. Treat it as a cancel rather than guessing a commit at an unknown final position.
    m_pressed = false;
    m_activeRectIndex = -1;
}
