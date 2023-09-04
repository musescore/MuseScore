/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

namespace mu::engraving {
class EngravingItem;
class Score;
}

namespace mu::notation {
class ExampleView : public QFrame, public engraving::MuseScoreView
{
    Q_OBJECT

    INJECT(notation::INotationConfiguration, notationConfiguration)

public:
    ExampleView(QWidget* parent = 0);
    ~ExampleView();
    void resetMatrix();
    void layoutChanged() override;
    void dataChanged(const RectF&) override;
    void updateAll() override;
    void adjustCanvasPosition(const engraving::EngravingItem* el, int staff = -1) override;
    void setScore(engraving::Score*) override;
    void removeScore() override;

    void changeEditElement(engraving::EngravingItem*) override;
    void setDropRectangle(const mu::RectF&) override;
    void cmdAddSlur(engraving::Note* firstNote, Note* lastNote);
    void drawBackground(draw::Painter*, const RectF&) const override;
    void dragExampleView(QMouseEvent* ev);
    const Rect geometry() const override { return Rect(QFrame::geometry()); }

protected:
    void mousePressEvent(QMouseEvent*) override;

    PointF toLogical(const QPointF& point);

private:
    void drawElements(mu::draw::Painter& painter, const std::vector<EngravingItem*>& el);
    void paintEvent(QPaintEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void constraintCanvas(int* dxx);
    QSize sizeHint() const override;

    mu::draw::Transform m_matrix;
    QColor m_backgroundColor;
    QPixmap* m_backgroundPixmap;

    QStateMachine* m_stateMachine;
    mu::PointF m_moveStartPoint;

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
