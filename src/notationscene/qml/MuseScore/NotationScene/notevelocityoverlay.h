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

#pragma once

#include <QColor>
#include <QQuickPaintedItem>
#include <QVector>

class QHoverEvent;

// NOTE: all rectangle coordinates are normalized [0, 1], relative to this item's own width/height,
// mirroring NoteOffsetOverlay's convention.
//
// Bars belonging to the same note column (i.e. sharing the same left/right X span - the notes of a
// chord) are expected to be stored in back-to-front paint order: the highest-pitched note's bar
// first (painted first, furthest back), the lowest-pitched note's bar last (painted last, frontmost
// and fully opaque). Painting each bar's opaque body in that order naturally makes a taller,
// further-back bar's tip peek out above a shorter, more-frontward one - exactly like an overlapping
// velocity lane in a DAW piano roll. hitTestPx() reconstructs the same front-to-back visibility
// order to find which bar is actually clickable at a given pixel.

namespace mu::notation {
class NoteVelocityOverlay : public QQuickPaintedItem
{
    Q_OBJECT

public:
    struct RectData {
        qreal leftN = 0.0;
        qreal rightN = 0.0;
        qreal y0N = 1.0;   // velocity 0 (baseline)
        qreal yTopN = 1.0; // current top edge, i.e. the note's velocity
        bool selected = false;
        bool userModified = false; // has an explicit user-set velocity, vs. the dynamics-derived default
        int velocity = 0;  // current velocity (0-127), shown next to the bar while it's being dragged
    };

    explicit NoteVelocityOverlay(QQuickItem* parent);

    void setRects(const QVector<RectData>& rects);
    const QVector<RectData>& rects() const;

    // Mutates a single rect in place, avoiding a full-vector copy-out/copy-back - used for live
    // preview during a drag and for selection-highlight updates, both of which only ever touch a
    // handful of rects at a time even on a staff with many notes.
    void updateRect(int index, const RectData& rect);

    void setFillColor(const QColor& color);
    void setSelectedFillColor(const QColor& color);
    void setModifiedFillColor(const QColor& color);
    void setBorderColor(const QColor& color);
    void setValueLabelColors(const QColor& background, const QColor& text);

    void paint(QPainter* painter) override;

    bool isDragging() const { return m_pressed; }

signals:
    // deltaYN is the mouse's own vertical displacement (normalized to this item's height) since
    // the press that started this drag, not an absolute position - clicking anywhere on a bar acts
    // as a drag handle for it, nudging its velocity relative to wherever it already was, rather
    // than jumping the value to whatever the click position happens to correspond to.
    void barDragged(int rectIndex, qreal deltaYN, bool completed);

    // Fired instead of a final barDragged() when a drag is cancelled by having its mouse grab
    // stolen mid-gesture (e.g. a popup opening) - unlike barDragged(..., completed=true), this is
    // NOT a commit signal (no score change should follow it); it exists so the controller can both
    // reset its own live-drag-only state (e.g. audition throttling) and snap the bar's displayed
    // height back to the note's actual (uncommitted) velocity - previewBarHeight() calls during
    // the drag mutate the overlay's rect directly, so without this it would keep showing the
    // live-preview height indefinitely, out of sync with the note's real value, until some
    // unrelated rebuild happened to refresh it.
    void dragCancelled(int rectIndex);

protected:
    void hoverMoveEvent(QHoverEvent* e) override;
    void hoverLeaveEvent(QHoverEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseUngrabEvent() override;

private:
    int hitTestPx(const QPointF& posPx) const;
    void paintValueLabel(QPainter* painter, const RectData& rect) const;

    QVector<RectData> m_rects;

    QColor m_fillColor;
    QColor m_selectedFillColor;
    QColor m_modifiedFillColor;
    QColor m_borderColor;
    QColor m_valueLabelBgColor;
    QColor m_valueLabelTextColor;

    bool m_pressed = false;
    int m_activeRectIndex = -1;
    qreal m_dragStartYPx = 0.0;
    bool m_movedPastClickThreshold = false;
    bool m_hoveringBar = false;
};
}
