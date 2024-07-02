/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_NOTATION_EXAMPLEVIEW_H
#define MU_NOTATION_EXAMPLEVIEW_H

#include <QStateMachine>
#include <QPaintEvent>
#include <QFrame>
#include <QEventTransition>

#include "engraving/dom/mscoreview.h"

#include "modularity/ioc.h"
#include "notation/inotationconfiguration.h"
#include "draw/types/transform.h"

//! NOTE When this class is going to be removed, make sure to
//! remove the StateMachine dependency from FindQt6.cmake too.
//! This is the only class that uses it.

namespace mu::engraving {
class EngravingItem;
class Score;
}

namespace mu::notation {
//! NOTE When this class is going to be removed, make sure to
//! remove the StateMachine dependency from FindQt6.cmake too.
//! This is the only class that uses it.
class ExampleView : public QFrame, public engraving::MuseScoreView, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<notation::INotationConfiguration> notationConfiguration = { this };

public:
    ExampleView(QWidget* parent = 0);
    ~ExampleView();
    void resetMatrix();
    void layoutChanged() override;
    void dataChanged(const muse::RectF&) override;
    void updateAll() override;
    void adjustCanvasPosition(const engraving::EngravingItem* el, int staff = -1) override;
    void setScore(engraving::Score*) override;
    void removeScore() override;

    void changeEditElement(engraving::EngravingItem*) override;
    void setDropRectangle(const muse::RectF&) override;
    void cmdAddSlur(engraving::Note* firstNote, Note* lastNote);
    void drawBackground(muse::draw::Painter*, const muse::RectF&) const override;
    void dragExampleView(QMouseEvent* ev);
    const muse::Rect geometry() const override { return muse::Rect(QFrame::geometry()); }

protected:
    void mousePressEvent(QMouseEvent*) override;

    muse::PointF toLogical(const QPointF& point);

private:
    void drawElements(muse::draw::Painter& painter, const std::vector<EngravingItem*>& el);
    void paintEvent(QPaintEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void constraintCanvas(int* dxx);
    QSize sizeHint() const override;

    muse::draw::Transform m_matrix;
    QColor m_backgroundColor;
    QPixmap* m_backgroundPixmap;

    QStateMachine* m_stateMachine;
    muse::PointF m_moveStartPoint;

    double m_defaultScaling = 0;
};

class DragTransitionExampleView : public QEventTransition
{
    ExampleView* canvas;

protected:
    virtual void onTransition(QEvent* e);

public:
    DragTransitionExampleView(ExampleView* c)
        : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
};
}

#endif // MU_NOTATION_EXAMPLEVIEW_H
