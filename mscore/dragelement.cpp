//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "log.h"

#include "scoreview.h"
#include "libmscore/score.h"
#include "musescore.h"
#include "libmscore/staff.h"
#include "libmscore/utils.h"
#include "libmscore/undo.h"
#include "libmscore/part.h"
#include "tourhandler.h"

namespace Ms {
//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void ScoreView::startDrag()
{
    editData.grips = 0;
    editData.clearData();
    editData.normalizedStartMove = editData.startMove - editData.element->offset();

    const Selection& sel = _score->selection();
    const bool filterType = sel.isRange();
    const ElementType type = editData.element->type();

    const auto isDragged = [filterType, type](const Element* e) {
                               return e && e->selected() && (!filterType || type == e->type());
                           };

    for (Element* e : sel.elements()) {
        if (!isDragged(e)) {
            continue;
        }

        std::unique_ptr<ElementGroup> g = e->getDragGroup(isDragged);
        if (g && g->enabled()) {
            dragGroups.push_back(std::move(g));
        }
    }

    _score->startCmd();

    for (auto& g : dragGroups) {
        g->startDrag(editData);
    }

    _score->selection().lock("drag");
}

//---------------------------------------------------------
//   doDragElement
//---------------------------------------------------------

void ScoreView::doDragElement(QMouseEvent* ev)
{
    const QPointF logicalPos = toLogical(ev->pos());
    QPointF delta = logicalPos - editData.normalizedStartMove;
    QPointF evtDelta = logicalPos - editData.pos;

    TourHandler::startTour("autoplace-tour");

    QPointF pt(delta);
    if (qApp->keyboardModifiers() == Qt::ShiftModifier) {
        pt.setX(editData.element->offset().x());
        evtDelta.setX(0.0);
    } else if (qApp->keyboardModifiers() == Qt::ControlModifier) {
        pt.setY(editData.element->offset().y());
        evtDelta.setY(0.0);
    }

    editData.lastPos = editData.pos;
    editData.hRaster = mscore->hRaster();
    editData.vRaster = mscore->vRaster();
    editData.delta   = pt;
    editData.moveDelta = pt + (editData.normalizedStartMove - editData.startMove);   // TODO: restructure
    editData.evtDelta = evtDelta;
    editData.pos     = logicalPos;

    const Selection& sel = _score->selection();

    for (auto& g : dragGroups) {
        _score->addRefresh(g->drag(editData));
    }

    _score->update();
    QVector<QLineF> anchorLines;

    for (Element* e : sel.elements()) {
        QVector<QLineF> elAnchorLines = e->dragAnchorLines();
        if (!elAnchorLines.isEmpty()) {
            anchorLines.append(elAnchorLines);
        }
    }

    if (anchorLines.isEmpty()) {
        setDropTarget(0);     // this also resets dropAnchor
    } else {
        setDropAnchorLines(anchorLines);
    }

    Element* e = _score->getSelectedElement();
    if (e) {
        if (_score->playNote()) {
            mscore->play(e);
            _score->setPlayNote(false);
        }
    }
    updateGrips();
    _score->update();
}

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void ScoreView::endDrag()
{
    for (auto& g : dragGroups) {
        g->endDrag(editData);
    }

    dragGroups.clear();
    _score->selection().unlock("drag");
    setDropTarget(0);   // this also resets dropAnchor
    _score->endCmd();
    updateGrips();
    if (editData.element->normalModeEditBehavior() == Element::EditBehavior::Edit
        && _score->selection().element() == editData.element) {
        startEdit(/* editMode */ false);
    }
}
}
