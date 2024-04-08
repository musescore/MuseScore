/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
#include <gtest/gtest.h>

#include <QPainter>
#include <QImage>

#include "draw/painter.h"

#include "draw/internal/qpainterprovider.h"

using namespace muse;
using namespace muse::draw;

class Draw_PainterTests : public ::testing::Test
{
public:
};

static QPainter* getUnderlyingQPainter(const Painter* painter)
{
    return dynamic_cast<QPainterProvider*>(painter->provider().get())->qpainter();
}

TEST_F(Draw_PainterTests, Painter_FromNewQPainter)
{
    //! GIVEN New QPainter
    QImage pd(100, 100, QImage::Format_ARGB32_Premultiplied);
    QPainter qp(&pd);

    //! DO Create Painter from it
    Painter painter(&qp, "test");

    //! CHECK
    EXPECT_EQ(getUnderlyingQPainter(&painter), &qp);

    EXPECT_EQ(painter.worldTransform(), Transform());
    EXPECT_EQ(painter.provider()->transform(), Transform());

    //! DO Modify Painter
    painter.setWorldTransform(Transform(1, 2, 3, 4, 5, 6));

    //! CHECK The changes should have been propagated to QPainter
    EXPECT_EQ(painter.worldTransform(), Transform(1, 2, 3, 4, 5, 6));
    EXPECT_EQ(qp.transform(), QTransform(1, 2, 3, 4, 5, 6));
}

TEST_F(Draw_PainterTests, Painter_FromExistingQPainter)
{
    //! GIVEN QPainter that is already modified
    QImage pd(100, 100, QImage::Format_ARGB32_Premultiplied);
    QPainter qp(&pd);
    qp.setTransform(QTransform(1, 2, 3, 4, 5, 6));

    //! DO Create Painter from it
    Painter painter(&qp, "test");

    //! CHECK
    EXPECT_EQ(getUnderlyingQPainter(&painter), &qp);

    //! CHECK Painter should have inherited these changes
    EXPECT_EQ(painter.worldTransform(), Transform(1, 2, 3, 4, 5, 6));
    EXPECT_EQ(painter.provider()->transform(), Transform(1, 2, 3, 4, 5, 6));

    //! DO Modify Painter
    painter.setWorldTransform(Transform(4, 5, 6, 7, 8, 9));

    //! CHECK The changes should have been propagated to QPainter
    EXPECT_EQ(painter.worldTransform(), Transform(4, 5, 6, 7, 8, 9));
    EXPECT_EQ(painter.provider()->transform(), Transform(4, 5, 6, 7, 8, 9));
}

TEST_F(Draw_PainterTests, Painter_ViewportAndWindow)
{
    //! GIVEN Painter
    QImage pd(100, 100, QImage::Format_ARGB32_Premultiplied);
    QPainter qp(&pd);
    Painter painter(&qp, "test");

    //! DO Set viewport and window
    painter.setWindow(RectF(0.0, 0.0, 4.0, 4.0));
    painter.setViewport(RectF(0.0, 0.0, 2.0, 1.0));

    //! CHECK The transformation of the QPainter should be updated accordingly
    Transform expectedViewTransform(0.5, 0.0, 0.0, 0.25, 0.0, 0.0);
    EXPECT_EQ(painter.provider()->transform(), expectedViewTransform);

    //! GIVEN Transform
    Transform worldTransform(1, 2, 3, 4, 5, 6);

    //! DO Modify the world transform too
    painter.setWorldTransform(worldTransform);

    //! CHECK The transformation of the QPainter should be updated accordingly
    EXPECT_EQ(painter.provider()->transform(), worldTransform * expectedViewTransform);
}

TEST_F(Draw_PainterTests, Painter_SaveRestore)
{
    //! GIVEN Painter
    QImage pd(100, 100, QImage::Format_ARGB32_Premultiplied);
    QPainter qp(&pd);
    Painter painter(&qp, "test");

    //! DO Set properties
    painter.setWindow(RectF(0.0, 0.0, 4.0, 4.0));
    painter.setViewport(RectF(0.0, 0.0, 2.0, 1.0));

    Transform worldTransform(1, 2, 3, 4, 5, 6);
    painter.setWorldTransform(worldTransform);

    //! CHECK Properties should be set correctly
    Transform expectedViewTransform(0.5, 0.0, 0.0, 0.25, 0.0, 0.0);
    EXPECT_EQ(painter.provider()->transform(), worldTransform * expectedViewTransform);

    //! DO Save
    painter.save();

    //! DO Set new properties
    painter.setWindow(RectF(0.0, 0.0, 16.0, 15.0));
    painter.setViewport(RectF(0.0, 0.0, 4.0, 3.0));

    Transform newWorldTransform(1, 2, 3, 4, 5, 6);
    painter.setWorldTransform(newWorldTransform);

    //! CHECK Properties should be set correctly
    Transform newExpectedViewTransform(0.25, 0.0, 0.0, 0.2, 0.0, 0.0);
    EXPECT_EQ(painter.provider()->transform(), newWorldTransform * newExpectedViewTransform);

    //! DO Restore
    painter.restore();

    //! CHECK Properties should be restored correctly
    EXPECT_EQ(painter.window(), RectF(0.0, 0.0, 4.0, 4.0));
    EXPECT_EQ(painter.viewport(), RectF(0.0, 0.0, 2.0, 1.0));
    EXPECT_EQ(painter.worldTransform(), worldTransform);

    EXPECT_EQ(painter.provider()->transform(), worldTransform * expectedViewTransform);
}
