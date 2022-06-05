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

#include <QTransform>
#include <QStateMachine>
#include <QPaintEvent>
#include <QFrame>
#include <QEventTransition>

#include "libmscore/mscoreview.h"

#include "modularity/ioc.h"
#include "ui/iuiconfiguration.h"
#include "notation/inotationconfiguration.h"

namespace mu::engraving {
class EngravingItem;
class Score;
class Note;
class Chord;
class ActionIcon;
}

namespace mu::notation {
class ExampleView : public QFrame, public engraving::MuseScoreView
{
    Q_OBJECT

    INJECT(notation, ui::IUiConfiguration, uiConfiguration)
    INJECT(notation, notation::INotationConfiguration, notationConfiguration)

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

signals:
    void noteClicked(Note*);
    void beamPropertyDropped(Chord*, engraving::ActionIcon*);

private:
    void drawElements(mu::draw::Painter& painter, const std::vector<EngravingItem*>& el);
    void setDropTarget(const EngravingItem* el) override;

    void paintEvent(QPaintEvent*) override;
    void dragEnterEvent(QDragEnterEvent*) override;
    void dragLeaveEvent(QDragLeaveEvent*) override;
    void dragMoveEvent(QDragMoveEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void dropEvent(QDropEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void constraintCanvas(int* dxx);
    QSize sizeHint() const override;

    mu::Transform m_matrix;
    QColor m_backgroundColor;
    QPixmap* m_backgroundPixmap;
    EngravingItem* m_dragElement = 0;
    const EngravingItem* m_dropTarget = 0; ///< current drop target during dragMove
    QRectF m_dropRectangle;                ///< current drop rectangle during dragMove
    QLineF m_dropAnchor;                   ///< line to current anchor point during dragMove

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
