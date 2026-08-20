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

// NOTE: all rectangle coordinates are normalized [0, 1], relative to this item's own width/height,
// mirroring the approach used by muse::uicomponents::PolylinePlot for the automation overlay - this
// keeps stored positions valid regardless of the live view/zoom transform applied to the item itself.

namespace mu::notation {
class NoteOffsetOverlay : public QQuickPaintedItem
{
    Q_OBJECT

public:
    struct RectData {
        qreal leftN = 0.0;
        qreal rightN = 0.0;
        qreal centerYN = 0.5;
        qreal heightYN = 1.0;
        bool selected = false;
        bool userModified = false; // either playback offset is non-zero

        // A tie-chain fragment only offers the handle for the edge it actually owns: the start
        // handle on the chain's first note, the duration handle on its last - an intermediate
        // fragment (or one whose own chain-end lives in a different system) has neither.
        bool hasLeftHandle = true;
        bool hasRightHandle = true;
    };

    explicit NoteOffsetOverlay(QQuickItem* parent);

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
    void setHandleColor(const QColor& color);
    void setSelectedHandleColor(const QColor& color);
    void setModifiedHandleColor(const QColor& color);

    void paint(QPainter* painter) override;

    bool isDragging() const { return m_pressed; }

signals:
    void edgeDragged(int rectIndex, bool isLeftEdge, qreal newXN, bool completed);

protected:
    void hoverMoveEvent(QHoverEvent* e) override;
    void hoverLeaveEvent(QHoverEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseUngrabEvent() override;

private:
    struct HitResult {
        int rectIndex = -1;
        bool isLeftEdge = false;

        bool isValid() const { return rectIndex >= 0; }
    };

    HitResult hitTestPx(const QPointF& posPx) const;
    void updateCursor(bool hoveringEdge);

    QVector<RectData> m_rects;

    QColor m_fillColor;
    QColor m_selectedFillColor;
    QColor m_modifiedFillColor;
    QColor m_borderColor;
    QColor m_handleColor;
    QColor m_selectedHandleColor;
    QColor m_modifiedHandleColor;

    bool m_pressed = false;
    int m_activeRectIndex = -1;
    bool m_activeIsLeftEdge = false;
    bool m_hoveringEdge = false;
};
}
