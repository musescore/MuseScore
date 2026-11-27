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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <utility>

#include "draw/types/geometry.h"
#include "draw/types/transform.h"

namespace mu::notation {
//! NOTE The transformation between the logical (score) and the view coordinates:
//! the matrix of the view, its scaling and moving with the constraints of the scrollable area, the scrollbars.
//! It does not depend on a toolkit, the view only keeps the size and the content up to date.
class ViewTransform
{
public:
    //! The size of the view
    const muse::SizeF& viewSize() const;
    void setViewSize(const muse::SizeF& size);

    //! The content of the notation in the logical coordinates
    const muse::RectF& contentRect() const;
    void setContentRect(const muse::RectF& rect);

    void setScrollAreaLimited(bool limited);

    const muse::draw::Transform& matrix() const;
    void setMatrix(const muse::draw::Transform& matrix);
    void reset();

    double scaling() const;

    muse::PointF toLogical(const muse::PointF& point) const;
    muse::RectF toLogical(const muse::RectF& rect) const;
    muse::PointF fromLogical(const muse::PointF& point) const;
    muse::RectF fromLogical(const muse::RectF& rect) const;

    muse::RectF viewport() const;
    muse::PointF viewportTopLeft() const;
    muse::RectF scrollableArea() const;

    //! The moving of the canvas, it returns true if the matrix is changed
    bool moveCanvas(double dx, double dy);
    bool moveCanvasToPosition(const muse::PointF& logicPos);
    bool moveCanvasToCenter();
    bool ensureViewportInsideScrollableArea();

    //! The scaling around the point of the view, it returns true if the matrix is changed
    bool scale(double factor, const muse::PointF& pos);
    bool setScaling(double scaling, const muse::PointF& pos);

    // Scrollbars
    double horizontalScrollableSize() const;
    double verticalScrollableSize() const;
    double horizontalScrollbarSize() const;
    double verticalScrollbarSize() const;
    double startHorizontalScrollPosition() const;
    double startVerticalScrollPosition() const;

private:
    muse::PointF canvasCenter() const;
    std::pair<double, double> constraintCanvas(double dx, double dy) const;

    muse::SizeF m_viewSize;
    muse::RectF m_contentRect;
    bool m_isScrollAreaLimited = true;

    muse::draw::Transform m_matrix;
};
}
