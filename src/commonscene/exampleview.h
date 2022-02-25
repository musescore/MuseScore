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

#ifndef __EXAMPLEVIEW_H__
#define __EXAMPLEVIEW_H__

#include <QTransform>
#include <QStateMachine>
#include <QPaintEvent>
#include <QFrame>
#include <QEventTransition>

#include "libmscore/mscoreview.h"

#include "modularity/ioc.h"
#include "ui/iuiconfiguration.h"
#include "notation/inotationconfiguration.h"

namespace Ms {
class EngravingItem;
class Score;
class Note;
class Chord;
class ActionIcon;

class ExampleView : public QFrame, public MuseScoreView
{
    Q_OBJECT

    INJECT(commonscene, mu::ui::IUiConfiguration, uiConfiguration)
    INJECT(commonscene, mu::notation::INotationConfiguration, notationConfiguration)

public:
    ExampleView(QWidget* parent = 0);
    ~ExampleView();
    void resetMatrix();
    void layoutChanged() override;
    void dataChanged(const mu::RectF&) override;
    void updateAll() override;
    void adjustCanvasPosition(const EngravingItem* el, bool playBack, int staff = -1) override;
    void setScore(Score*) override;
    void removeScore() override;

    void changeEditElement(EngravingItem*) override;
    void setDropRectangle(const mu::RectF&) override;
    void cmdAddSlur(Note* firstNote, Note* lastNote);
    void drawBackground(mu::draw::Painter*, const mu::RectF&) const override;
    void dragExampleView(QMouseEvent* ev);
    const mu::Rect geometry() const override { return mu::Rect(QFrame::geometry()); }

signals:
    void noteClicked(Note*);
    void beamPropertyDropped(Chord*, ActionIcon*);

private:
    void drawElements(mu::draw::Painter& painter, const QList<EngravingItem*>& el);
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

//---------------------------------------------------------
//   DragTransitionExampleView
//---------------------------------------------------------

class DragTransitionExampleView : public QEventTransition
{
    ExampleView* canvas;

protected:
    virtual void onTransition(QEvent* e);

public:
    DragTransitionExampleView(ExampleView* c)
        : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
};
} // namespace Ms
#endif
