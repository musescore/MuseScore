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
#include <gtest/gtest.h>

#include "notation/viewtransform.h"

using namespace mu::notation;
using namespace muse;
using namespace muse::draw;

class Notation_ViewTransformTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_transform.setViewSize(SizeF(800, 600));
        m_transform.setContentRect(RectF(0, 0, 1000, 1000));
        m_transform.setScrollAreaLimited(true);
    }

    ViewTransform m_transform;
};

TEST_F(Notation_ViewTransformTests, ToLogicalAndFromLogical_AreInverse)
{
    //! [GIVEN] The view is scaled and moved
    Transform matrix;
    matrix.translate(-100, -50);
    matrix.scale(2.0, 2.0);
    m_transform.setMatrix(matrix);

    //! [WHEN] A point is converted from the logical coordinates and back
    const PointF viewPoint = m_transform.fromLogical(PointF(30, 40));
    const PointF logicPoint = m_transform.toLogical(viewPoint);

    //! [THEN] It is the same point
    EXPECT_NEAR(logicPoint.x(), 30, 1e-9);
    EXPECT_NEAR(logicPoint.y(), 40, 1e-9);
}

TEST_F(Notation_ViewTransformTests, Viewport_IsTheViewInTheLogicalCoordinates)
{
    //! [GIVEN] The view is scaled twice
    Transform matrix;
    matrix.scale(2.0, 2.0);
    m_transform.setMatrix(matrix);

    //! [THEN] The viewport is twice smaller than the view
    EXPECT_NEAR(m_transform.viewport().width(), 400, 1e-9);
    EXPECT_NEAR(m_transform.viewport().height(), 300, 1e-9);
}

TEST_F(Notation_ViewTransformTests, MoveCanvas_ToTheEdge_IsConstrainedByTheContent)
{
    //! [GIVEN] The viewport is at the top left corner of the content, the scroll area is limited

    //! [WHEN] The canvas is moved so that the viewport would leave the content
    const bool moved = m_transform.moveCanvas(500, 0);

    //! [THEN] It is not moved
    EXPECT_FALSE(moved);
    EXPECT_NEAR(m_transform.viewportTopLeft().x(), 0, 1e-9);
}

TEST_F(Notation_ViewTransformTests, MoveCanvas_InsideTheContent_MovesTheViewport)
{
    //! [GIVEN] The viewport is at the top left corner of the content

    //! [WHEN] The canvas is moved to the left
    const bool moved = m_transform.moveCanvas(-100, -50);

    //! [THEN] The viewport is moved to the right and to the bottom
    EXPECT_TRUE(moved);
    EXPECT_NEAR(m_transform.viewportTopLeft().x(), 100, 1e-9);
    EXPECT_NEAR(m_transform.viewportTopLeft().y(), 50, 1e-9);
}

TEST_F(Notation_ViewTransformTests, MoveCanvas_ContentIsSmallerThanTheView_CentersTheContent)
{
    //! [GIVEN] The content is smaller than the view
    m_transform.setContentRect(RectF(0, 0, 400, 300));

    //! [WHEN] The canvas is moved
    m_transform.moveCanvas(10, 10);

    //! [THEN] The content is in the center of the view
    const PointF viewCenter(m_transform.viewSize().width() / 2, m_transform.viewSize().height() / 2);
    const PointF contentCenter = m_transform.fromLogical(m_transform.contentRect().center());
    EXPECT_NEAR(contentCenter.x(), viewCenter.x(), 1e-9);
    EXPECT_NEAR(contentCenter.y(), viewCenter.y(), 1e-9);
}

TEST_F(Notation_ViewTransformTests, Scale_AroundThePoint_KeepsThePointInPlace)
{
    //! [GIVEN] A big content, the view is moved inside it
    m_transform.setContentRect(RectF(0, 0, 10000, 10000));
    m_transform.moveCanvas(-1000, -1000);

    //! [GIVEN] A point of the view and its logical position
    const PointF viewPoint(300, 200);
    const PointF logicPoint = m_transform.toLogical(viewPoint);

    //! [WHEN] Scaling around the point
    const bool scaled = m_transform.scale(2.0, viewPoint);

    //! [THEN] The scale is changed, the point is at the same place of the view
    EXPECT_TRUE(scaled);
    EXPECT_NEAR(m_transform.scaling(), 2.0, 1e-9);
    EXPECT_NEAR(m_transform.fromLogical(logicPoint).x(), viewPoint.x(), 1e-9);
    EXPECT_NEAR(m_transform.fromLogical(logicPoint).y(), viewPoint.y(), 1e-9);
}

TEST_F(Notation_ViewTransformTests, SetScaling_TheSameScaling_DoesNothing)
{
    //! [GIVEN] The current scaling is 1

    //! [WHEN] Setting the same scaling
    const bool changed = m_transform.setScaling(1.0, PointF(100, 100));

    //! [THEN] Nothing is changed
    EXPECT_FALSE(changed);
}

TEST_F(Notation_ViewTransformTests, SetScaling_NewScaling_ChangesTheScaling)
{
    //! [GIVEN] The current scaling is 1

    //! [WHEN] Setting another scaling
    const bool changed = m_transform.setScaling(0.5, PointF(100, 100));

    //! [THEN] The scaling is changed
    EXPECT_TRUE(changed);
    EXPECT_NEAR(m_transform.scaling(), 0.5, 1e-9);
}

TEST_F(Notation_ViewTransformTests, EnsureViewportInsideScrollableArea_EmptyView_DoesNothing)
{
    //! [GIVEN] The view is not sized yet, the viewport is outside the content
    m_transform.setViewSize(SizeF(0, 0));
    Transform matrix;
    matrix.translate(5000, 5000);
    m_transform.setMatrix(matrix);

    //! [WHEN] Ensuring the viewport is inside
    const bool changed = m_transform.ensureViewportInsideScrollableArea();

    //! [THEN] The matrix is not touched
    EXPECT_FALSE(changed);
    EXPECT_EQ(m_transform.matrix(), matrix);
}

TEST_F(Notation_ViewTransformTests, EnsureViewportInsideScrollableArea_ViewportIsOutside_MovesItBack)
{
    //! [GIVEN] The viewport is outside the content
    Transform matrix;
    matrix.translate(5000, 5000);
    m_transform.setMatrix(matrix);

    //! [WHEN] Ensuring the viewport is inside
    const bool changed = m_transform.ensureViewportInsideScrollableArea();

    //! [THEN] The viewport is back at the content
    EXPECT_TRUE(changed);
    EXPECT_GE(m_transform.viewportTopLeft().x(), m_transform.contentRect().left() - 1e-9);
    EXPECT_GE(m_transform.viewportTopLeft().y(), m_transform.contentRect().top() - 1e-9);
}

TEST_F(Notation_ViewTransformTests, Scrollbars_ShowTheViewportInTheContent)
{
    //! [GIVEN] The viewport is moved to the right
    m_transform.moveCanvas(-100, 0);

    //! [THEN] The scrollbar is the size of the viewport relative to the content, and shifted
    EXPECT_NEAR(m_transform.horizontalScrollbarSize(), 800.0 / 1000.0, 1e-9);
    EXPECT_NEAR(m_transform.startHorizontalScrollPosition(), 0.1, 1e-9);
    EXPECT_NEAR(m_transform.startVerticalScrollPosition(), 0.0, 1e-9);
}

TEST_F(Notation_ViewTransformTests, ScrollbarSize_ContentFitsTheViewport_IsZero)
{
    //! [GIVEN] The content is inside the viewport
    m_transform.setContentRect(RectF(100, 100, 400, 300));

    //! [THEN] There is no need in scrollbars
    EXPECT_NEAR(m_transform.horizontalScrollbarSize(), 0.0, 1e-9);
    EXPECT_NEAR(m_transform.verticalScrollbarSize(), 0.0, 1e-9);
}
