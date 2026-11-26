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
#include "viewtransform.h"

#include <algorithm>
#include <cmath>

#include "global/realfn.h"

#include "log.h"

using namespace mu::notation;
using namespace muse;
using namespace muse::draw;

static constexpr double SCROLL_LIMIT_OFF_OVERSCROLL_FACTOR = 0.75;

const SizeF& ViewTransform::viewSize() const
{
    return m_viewSize;
}

void ViewTransform::setViewSize(const SizeF& size)
{
    m_viewSize = size;
}

const RectF& ViewTransform::contentRect() const
{
    return m_contentRect;
}

void ViewTransform::setContentRect(const RectF& rect)
{
    m_contentRect = rect;
}

void ViewTransform::setScrollAreaLimited(bool limited)
{
    m_isScrollAreaLimited = limited;
}

const Transform& ViewTransform::matrix() const
{
    return m_matrix;
}

void ViewTransform::setMatrix(const Transform& matrix)
{
    m_matrix = matrix;
}

void ViewTransform::reset()
{
    m_matrix = Transform();
}

double ViewTransform::scaling() const
{
    return m_matrix.m11();
}

PointF ViewTransform::toLogical(const PointF& point) const
{
    return m_matrix.inverted().map(point);
}

RectF ViewTransform::toLogical(const RectF& rect) const
{
    return m_matrix.inverted().map(rect);
}

PointF ViewTransform::fromLogical(const PointF& point) const
{
    return m_matrix.map(point);
}

RectF ViewTransform::fromLogical(const RectF& rect) const
{
    return m_matrix.map(rect);
}

RectF ViewTransform::viewport() const
{
    return toLogical(RectF(0.0, 0.0, m_viewSize.width(), m_viewSize.height()));
}

PointF ViewTransform::viewportTopLeft() const
{
    return toLogical(PointF(0.0, 0.0));
}

RectF ViewTransform::scrollableArea() const
{
    const RectF viewport = this->viewport();
    const double overscrollFactor = m_isScrollAreaLimited ? 0.0 : SCROLL_LIMIT_OFF_OVERSCROLL_FACTOR;

    const double overscrollX = viewport.width() * overscrollFactor;
    const double overscrollY = viewport.height() * overscrollFactor;

    return m_contentRect.adjusted(-overscrollX, -overscrollY, overscrollX, overscrollY);
}

PointF ViewTransform::canvasCenter() const
{
    const RectF canvasRect = m_matrix.map(m_contentRect);

    const double x = (m_viewSize.width() - canvasRect.width()) / 2;
    const double y = (m_viewSize.height() - canvasRect.height()) / 2;

    return toLogical(PointF(x, y));
}

std::pair<double, double> ViewTransform::constraintCanvas(double dx, double dy) const
{
    const RectF scrollableArea = this->scrollableArea();
    const RectF viewport = this->viewport();

    // horizontal
    {
        double newLeft = viewport.left() - dx;
        if (viewport.width() > scrollableArea.width()) {
            newLeft = scrollableArea.center().x() - viewport.width() / 2;
        } else {
            newLeft = std::max(scrollableArea.left(), std::min(scrollableArea.right() - viewport.width(), newLeft));
        }
        dx = viewport.left() - newLeft;
    }

    // vertical
    {
        double newTop = viewport.top() - dy;
        if (viewport.height() > scrollableArea.height()) {
            newTop = scrollableArea.center().y() - viewport.height() / 2;
        } else {
            newTop = std::max(scrollableArea.top(), std::min(scrollableArea.bottom() - viewport.height(), newTop));
        }
        dy = viewport.top() - newTop;
    }

    return { dx, dy };
}

bool ViewTransform::moveCanvas(double dx, double dy)
{
    if (RealIsNull(dx) && RealIsNull(dy)) {
        return false;
    }

    auto [correctedDX, correctedDY] = constraintCanvas(dx, dy);
    if (RealIsNull(correctedDX) && RealIsNull(correctedDY)) {
        return false;
    }

    m_matrix.translate(correctedDX, correctedDY);

    return true;
}

bool ViewTransform::moveCanvasToPosition(const PointF& logicPos)
{
    const PointF viewTopLeft = viewportTopLeft();
    return moveCanvas(viewTopLeft.x() - logicPos.x(), viewTopLeft.y() - logicPos.y());
}

bool ViewTransform::moveCanvasToCenter()
{
    const PointF center = canvasCenter();
    return moveCanvas(center.x(), center.y());
}

bool ViewTransform::ensureViewportInsideScrollableArea()
{
    // When the viewport is empty, which might happen for example when the view
    // is being loaded or unloaded, the scrollable area is computed without any
    // overscroll allowance, resulting in unexpected clamping of the canvas
    // position to (0, 0).
    if (m_viewSize.width() <= 0 || m_viewSize.height() <= 0) {
        return false;
    }

    auto [dx, dy] = constraintCanvas(0, 0);
    if (RealIsNull(dx) && RealIsNull(dy)) {
        return false;
    }

    m_matrix.translate(dx, dy);
    return true;
}

bool ViewTransform::scale(double factor, const PointF& pos)
{
    if (RealIsEqual(factor, 1.0)) {
        return false;
    }

    const PointF pointBeforeScaling = toLogical(pos);

    m_matrix.scale(factor, factor);

    const PointF pointAfterScaling = toLogical(pos);

    const double dx = pointAfterScaling.x() - pointBeforeScaling.x();
    const double dy = pointAfterScaling.y() - pointBeforeScaling.y();

    moveCanvas(dx, dy);

    return true;
}

bool ViewTransform::setScaling(double scaling, const PointF& pos)
{
    double currentScaling = this->scaling();

    IF_ASSERT_FAILED(!RealIsNull(scaling)) {
        return false;
    }

    if (RealIsEqual(currentScaling, scaling)) {
        return false;
    }

    if (RealIsNull(currentScaling)) {
        currentScaling = 1;
    }

    const double deltaScaling = scaling / currentScaling;
    return scale(deltaScaling, pos);
}

double ViewTransform::horizontalScrollableSize() const
{
    const RectF viewport = this->viewport();
    const RectF contentRect = m_contentRect;

    const double left = std::min(contentRect.left(), viewport.left());
    const double right = std::max(contentRect.right(), viewport.right());

    double size = 0;
    if ((left < 0) && (right > 0)) {
        size = std::abs(left) + right;
    } else {
        size = std::abs(right) - std::abs(left);
    }

    return size;
}

double ViewTransform::verticalScrollableSize() const
{
    const RectF viewport = this->viewport();
    const RectF contentRect = m_contentRect;

    const double top = std::min(contentRect.top(), viewport.top());
    const double bottom = std::max(contentRect.bottom(), viewport.bottom());

    double size = 0;
    if ((top < 0) && (bottom > 0)) {
        size = std::abs(top) + bottom;
    } else {
        size = std::abs(bottom) - std::abs(top);
    }

    return size;
}

double ViewTransform::horizontalScrollbarSize() const
{
    const RectF viewport = this->viewport();
    const RectF contentRect = m_contentRect;

    if (viewport.left() < contentRect.left()
        && viewport.right() > contentRect.right()) {
        return 0;
    }

    const double scrollableWidth = horizontalScrollableSize();
    if (RealIsNull(scrollableWidth)) {
        return 0;
    }

    return viewport.width() / scrollableWidth;
}

double ViewTransform::verticalScrollbarSize() const
{
    const RectF viewport = this->viewport();
    const RectF contentRect = m_contentRect;

    if (viewport.top() < contentRect.top()
        && viewport.bottom() > contentRect.bottom()) {
        return 0;
    }

    const double scrollableHeight = verticalScrollableSize();
    if (RealIsNull(scrollableHeight)) {
        return 0;
    }

    return viewport.height() / scrollableHeight;
}

double ViewTransform::startHorizontalScrollPosition() const
{
    const RectF viewport = this->viewport();
    const RectF contentRect = m_contentRect;

    if (!viewport.isValid() || !contentRect.isValid()) {
        return 0.0;
    }

    return (viewport.left() - contentRect.left()) / contentRect.width();
}

double ViewTransform::startVerticalScrollPosition() const
{
    const RectF viewport = this->viewport();
    const RectF contentRect = m_contentRect;

    if (!viewport.isValid() || !contentRect.isValid()) {
        return 0.0;
    }

    return (viewport.top() - contentRect.top()) / contentRect.height();
}
