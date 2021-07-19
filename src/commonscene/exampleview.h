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
class Element;
class Score;
class Note;
class Chord;
class ActionIcon;
enum class Grip : int;

//---------------------------------------------------------
//   ExampleView
//---------------------------------------------------------

class ExampleView : public QFrame, public MuseScoreView
{
    Q_OBJECT

    INJECT(commonscene, mu::ui::IUiConfiguration, uiConfiguration)
    INJECT(commonscene, mu::notation::INotationConfiguration, notationConfiguration)

    QTransform _matrix, imatrix;
    QColor _fgColor;
    QPixmap* _fgPixmap;
    Element* dragElement = 0;
    const Element* dropTarget = 0;        ///< current drop target during dragMove
    QRectF dropRectangle;                 ///< current drop rectangle during dragMove
    QLineF dropAnchor;                    ///< line to current anchor point during dragMove

    QStateMachine* sm;
    QPointF startMove;

    double m_defaultScaling = 0;

    void drawElements(mu::draw::Painter& painter, const QList<Element*>& el);
    void setDropTarget(const Element* el) override;

    virtual void paintEvent(QPaintEvent*) override;
    virtual void dragEnterEvent(QDragEnterEvent*) override;
    virtual void dragLeaveEvent(QDragLeaveEvent*) override;
    virtual void dragMoveEvent(QDragMoveEvent*) override;
    virtual void wheelEvent(QWheelEvent*) override;
    virtual void dropEvent(QDropEvent*) override;
    virtual void mousePressEvent(QMouseEvent*) override;
    void constraintCanvas(int* dxx);
    virtual QSize sizeHint() const override;

signals:
    void noteClicked(Note*);
    void beamPropertyDropped(Chord*, ActionIcon*);

public:
    ExampleView(QWidget* parent = 0);
    ~ExampleView();
    void resetMatrix();
    virtual void layoutChanged() override;
    virtual void dataChanged(const mu::RectF&) override;
    virtual void updateAll() override;
    virtual void adjustCanvasPosition(const Element* el, bool playBack, int staff = -1) override;
    virtual void setScore(Score*) override;
    virtual void removeScore() override;

    virtual void changeEditElement(Element*) override;
    virtual QCursor cursor() const override;
    virtual void setCursor(const QCursor&) override;
    virtual void setDropRectangle(const mu::RectF&) override;
    virtual void cmdAddSlur(Note* firstNote, Note* lastNote);
    virtual Element* elementNear(mu::PointF) override;
    virtual void drawBackground(mu::draw::Painter*, const mu::RectF&) const override;
    void dragExampleView(QMouseEvent* ev);
    const mu::Rect geometry() const override { return mu::Rect(QFrame::geometry()); }
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
