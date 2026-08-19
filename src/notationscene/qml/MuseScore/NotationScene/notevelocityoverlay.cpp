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

#include <QHoverEvent>
#include <QMouseEvent>
#include <QPainter>

using namespace mu::notation;

constexpr static qreal EDGE_HIT_MARGIN_PX = 4.0;
constexpr static qreal BAR_HALF_WIDTH_MARGIN_PX = 1.0; // keeps adjacent chord bars from visually touching

// Below this, a press+release is a plain click (jump straight to that position) rather than a
// drag (nudge relative to wherever the bar already was) - see mouseReleaseEvent().
constexpr static qreal CLICK_MOVE_THRESHOLD_PX = 3.0;

constexpr static qreal VALUE_LABEL_FONT_PX = 11.0;
constexpr static qreal VALUE_LABEL_GAP_PX = 4.0; // horizontal gap between the bar and the label chip
constexpr static qreal VALUE_LABEL_PADDING_X_PX = 4.0;
constexpr static qreal VALUE_LABEL_PADDING_Y_PX = 2.0;
constexpr static qreal VALUE_LABEL_CORNER_RADIUS_PX = 3.0;

NoteVelocityOverlay::NoteVelocityOverlay(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
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

void NoteVelocityOverlay::setValueLabelColors(const QColor& background, const QColor& text)
{
    m_valueLabelBgColor = background;
    m_valueLabelTextColor = text;
    update();
}

void NoteVelocityOverlay::paint(QPainter* painter)
{
    if (m_rects.isEmpty()) {
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(m_borderColor, 1.0));

    const auto drawBar = [&](const RectData& rect) {
        const qreal leftPx = rect.leftN * width() + BAR_HALF_WIDTH_MARGIN_PX;
        const qreal rightPx = rect.rightN * width() - BAR_HALF_WIDTH_MARGIN_PX;
        const qreal topPx = rect.yTopN * height();
        const qreal basePx = rect.y0N * height();

        const QRectF barRect(leftPx, topPx, std::max(0.0, rightPx - leftPx), std::max(0.0, basePx - topPx));

        painter->setBrush(rect.selected ? m_selectedFillColor : (rect.userModified ? m_modifiedFillColor : m_fillColor));
        painter->drawRect(barRect);
    };

    // Bars are stored in back-to-front paint order (see header comment) - simply painting each
    // one's fully opaque body in order reproduces the stacked/overlapping look of a DAW velocity
    // lane, with no extra bookkeeping needed here.
    for (const RectData& rect : m_rects) {
        if (!rect.selected) {
            drawBar(rect);
        }
    }

    // A selected note's bar must stay fully visible (and, per hitTestPx(), clickable) no matter
    // where it sits in the pitch-based stacking order - otherwise selecting a chord note that isn't
    // the pitch-frontmost one leaves its bar hidden behind another note's, with no way to drag it.
    // Redraw selected bars last so they always end up on top.
    for (const RectData& rect : m_rects) {
        if (rect.selected) {
            drawBar(rect);
        }
    }

    // Only the bar actually being dragged gets a live numeric readout, to keep the staff
    // uncluttered the rest of the time (matches Dorico's convention for its velocity lane).
    if (m_pressed && m_activeRectIndex >= 0 && m_activeRectIndex < m_rects.size()) {
        paintValueLabel(painter, m_rects.at(m_activeRectIndex));
    }
}

void NoteVelocityOverlay::paintValueLabel(QPainter* painter, const RectData& rect) const
{
    const QString text = QString::number(rect.velocity);

    QFont font = painter->font();
    font.setPixelSize(static_cast<int>(VALUE_LABEL_FONT_PX));
    painter->setFont(font);

    const QFontMetrics metrics(font);
    const QSize textSize = metrics.size(Qt::TextSingleLine, text);

    const qreal chipWidth = textSize.width() + 2 * VALUE_LABEL_PADDING_X_PX;
    const qreal chipHeight = textSize.height() + 2 * VALUE_LABEL_PADDING_Y_PX;

    const qreal leftPx = rect.leftN * width();
    const qreal rightPx = rect.rightN * width();
    const qreal topPx = rect.yTopN * height();

    // Prefer sitting to the right of the bar; flip to the left if there isn't room, rather than
    // letting the chip run off the edge of the overlay.
    qreal chipLeft = rightPx + VALUE_LABEL_GAP_PX;
    if (chipLeft + chipWidth > width()) {
        chipLeft = leftPx - VALUE_LABEL_GAP_PX - chipWidth;
    }
    chipLeft = std::clamp(chipLeft, 0.0, std::max(0.0, width() - chipWidth));

    const qreal chipTop = std::clamp(topPx - chipHeight / 2.0, 0.0, std::max(0.0, height() - chipHeight));

    const QRectF chipRect(chipLeft, chipTop, chipWidth, chipHeight);

    painter->setPen(Qt::NoPen);
    painter->setBrush(m_valueLabelBgColor);
    painter->drawRoundedRect(chipRect, VALUE_LABEL_CORNER_RADIUS_PX, VALUE_LABEL_CORNER_RADIUS_PX);

    painter->setPen(m_valueLabelTextColor);
    painter->drawText(chipRect, Qt::AlignCenter, text);
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

    // A selected bar is always redrawn on top of every other bar in its column (see paint()), so
    // it must win hit-testing too, regardless of pitch-based stacking order - otherwise a selected
    // chord note that isn't the pitch-frontmost one would be visible but not draggable. Selected
    // bars occlude everything below them, so account for all of them up front...
    qreal minTopSoFarPx = std::numeric_limits<qreal>::max();
    for (int idx : candidates) {
        const RectData& r = m_rects.at(idx);
        if (r.selected) {
            minTopSoFarPx = std::min(minTopSoFarPx, r.yTopN * height());
        }
    }

    // ...then let each selected bar claim any click within its own full body, ignoring occlusion
    // from other selected bars (there's normally at most one per column anyway).
    for (int idx : candidates) {
        const RectData& r = m_rects.at(idx);
        if (!r.selected) {
            continue;
        }
        const qreal topPx = r.yTopN * height();
        const qreal basePx = r.y0N * height();
        if (posPx.y() >= topPx - EDGE_HIT_MARGIN_PX && posPx.y() <= basePx) {
            return idx;
        }
    }

    // candidates preserve the original back-to-front order - scanning in reverse visits the
    // frontmost (lowest-pitched) unselected bar first, exactly matching what's actually visible
    // once any selected bar's on-top redraw (accounted for above) is factored in.
    for (auto it = candidates.rbegin(); it != candidates.rend(); ++it) {
        const RectData& r = m_rects.at(*it);
        if (r.selected) {
            continue;
        }
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

void NoteVelocityOverlay::hoverMoveEvent(QHoverEvent* e)
{
    // Which item's cursor actually gets displayed over an overlap is decided by QQuickWindow from
    // each item's *declared* cursor (whichever topmost item has ever called setCursor()) - it has
    // nothing to do with which item's hoverMoveEvent ignore()s the event. NoteOffsetOverlay
    // unconditionally declares a cursor on every hover move, so unless this item declares (and
    // un-declares) its own right here, Qt falls through to the offset overlay's stale declaration
    // underneath even where a bar - painted on top, and already winning mouse presses via the same
    // hit test - visually covers it.
    const bool hoveringBar = hitTestPx(e->position()) >= 0;
    if (hoveringBar == m_hoveringBar) {
        return;
    }
    m_hoveringBar = hoveringBar;

    if (hoveringBar) {
        setCursor(Qt::ArrowCursor);
    } else {
        unsetCursor();
    }
}

void NoteVelocityOverlay::hoverLeaveEvent(QHoverEvent*)
{
    m_hoveringBar = false;
    unsetCursor();
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
    // Stored as a raw pixel position, not pre-divided by height() - the height a drag started at
    // and the height read on a later move/release event aren't guaranteed to be the same value (a
    // window resize or a view zoom/pan can call setHeight() on this item while the mouse is still
    // held down), so normalizing each endpoint separately before subtracting could mix two
    // different scales into one delta. Dividing the raw pixel delta by a single, current height()
    // below keeps both ends of the subtraction on the same scale.
    m_dragStartYPx = e->position().y();
    m_movedPastClickThreshold = false;
    e->accept();

    // A zero delta - the mouse hasn't moved yet - so the controller hears a plain click on a bar
    // even if it never turns into an actual drag.
    emit barDragged(m_activeRectIndex, 0.0, false);
}

void NoteVelocityOverlay::mouseMoveEvent(QMouseEvent* e)
{
    if (!m_pressed) {
        return;
    }

    if (std::abs(e->position().y() - m_dragStartYPx) > CLICK_MOVE_THRESHOLD_PX) {
        m_movedPastClickThreshold = true;
    }

    // Not clamped to [0, 1] - unlike the drag-start position, which is always a valid in-bounds
    // click on a bar, the mouse can (and, mid-drag, routinely does) move outside this item's own
    // bounds while still grabbed; clamping here would flatten the delta near the edges instead of
    // tracking the mouse's actual displacement all the way through.
    const qreal deltaYN = (e->position().y() - m_dragStartYPx) / std::max(1.0, height());
    emit barDragged(m_activeRectIndex, deltaYN, false);
}

void NoteVelocityOverlay::mouseReleaseEvent(QMouseEvent* e)
{
    if (!m_pressed) {
        return;
    }

    qreal deltaYN;
    if (m_movedPastClickThreshold) {
        // A real drag - unchanged relative behavior, nudging from wherever the bar already was.
        deltaYN = (e->position().y() - m_dragStartYPx) / std::max(1.0, height());
    } else {
        // A plain click, released without ever moving past the threshold - jump straight to the
        // clicked position instead. barDragged()'s delta is always relative to the bar's *current*
        // position (see its own doc comment) rather than an absolute target, so this is expressed
        // as the delta from the bar's current top edge (yTopN) to the click position - the
        // controller's linear canvasY -> velocity mapping means that delta alone, regardless of
        // what it's measured from, resolves to exactly the velocity at the clicked position.
        const qreal clickYN = e->position().y() / std::max(1.0, height());
        deltaYN = clickYN - m_rects.at(m_activeRectIndex).yTopN;
    }
    emit barDragged(m_activeRectIndex, deltaYN, true);

    m_pressed = false;
    m_activeRectIndex = -1;
}

void NoteVelocityOverlay::mouseUngrabEvent()
{
    // The mouse grab taken in mousePressEvent can be stolen mid-drag (e.g. a popup opening) -
    // without this, mouseReleaseEvent never fires and this item is left thinking a drag is still
    // active. Treat it as a cancel rather than guessing a commit at an unknown final position.
    if (m_pressed) {
        emit dragCancelled(m_activeRectIndex);
    }
    m_pressed = false;
    m_activeRectIndex = -1;
}
