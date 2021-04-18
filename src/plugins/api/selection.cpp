//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "selection.h"
#include "score.h"

#include "libmscore/undo.h"

namespace Ms {
namespace PluginAPI {
//---------------------------------------------------------
//   QmlPlayEventsListAccess::append
//---------------------------------------------------------

Selection* selectionWrap(Ms::Selection* select)
{
    Selection* w = new Selection(select);
    // All wrapper objects should belong to JavaScript code.
    QQmlEngine::setObjectOwnership(w, QQmlEngine::JavaScriptOwnership);
    return w;
}

//---------------------------------------------------------
//   Selection::checkSelectionIsNotLocked
//---------------------------------------------------------

bool Selection::checkSelectionIsNotLocked() const
{
    if (_select->isLocked()) {
        qWarning("Cannot change selection: %s", qPrintable(_select->lockReason()));
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   Selection::select
///   Selects the given element. At this point only a
///   limited number of element types is supported, like
///   notes, rests and most of text elements.
///   \param e element to select
///   \param add if \p true, appends an element to already
///   existing selection. If \p false (default), deselects
///   all other elements and selects this element.
///   \return \p true on success, \p false if selection
///   cannot be changed, e.g. due to the ongoing operation
///   on a score (like dragging elements) or incorrect
///   arguments to this function.
///   \since MuseScore 3.5
//---------------------------------------------------------

bool Selection::select(Element* elWrapper, bool add)
{
    if (!checkSelectionIsNotLocked()) {
        return false;
    }

    if (!elWrapper) {
        return false;
    }

    Ms::Element* e = elWrapper->element();

    // Check whether it's safe to select this element:
    // use types list from UndoMacro for now
    if (!Ms::UndoMacro::canRecordSelectedElement(e)) {
        qWarning("Cannot select element of type %s", e->name());
        return false;
    }

    if (e->score() != _select->score() || elWrapper->ownership() != Ownership::SCORE) {
        qWarning("Selection::select: element does not belong to score");
        return false;
    }

    const SelectType selType = add ? SelectType::ADD : SelectType::SINGLE;
    e->score()->select(e, selType);

    return true;
}

//---------------------------------------------------------
//   Selection::selectRange
///   Selects a range in a score
///   \param startTick start tick to be included in selection
///   \param endTick end tick of selection, excluded from selection
///   \param startStaff start staff index, included in selection
///   \param endStaff end staff index, excluded from seleciton
///   \return \p true on success, \p false if selection
///   cannot be changed, e.g. due to the ongoing operation
///   on a score (like dragging elements) or incorrect
///   arguments to this function.
///   \since MuseScore 3.5
//---------------------------------------------------------

bool Selection::selectRange(int startTick, int endTick, int startStaff, int endStaff)
{
    if (!checkSelectionIsNotLocked()) {
        return false;
    }

    const int nstaves = _select->score()->nstaves();

    startStaff = qBound(0, startStaff, nstaves - 1);
    endStaff = qBound(1, endStaff, nstaves);

    if (startStaff >= endStaff) {
        return false;
    }

    Ms::Segment* segStart = _select->score()->tick2leftSegmentMM(Ms::Fraction::fromTicks(startTick));
    Ms::Segment* segEnd = _select->score()->tick2leftSegmentMM(Ms::Fraction::fromTicks(endTick));

    if (!segStart || (segEnd && !((*segEnd) > (*segStart)))) {
        return false;
    }

    if (segEnd && _select->score()->undoStack()->active()) {
        _select->setRangeTicks(segStart->tick(), segEnd->tick(), startStaff, endStaff);
    } else {
        _select->setRange(segStart, segEnd, startStaff, endStaff);
    }

    return true;
}

//---------------------------------------------------------
//   Selection::deselect
///   Deselects the given element.
///   \return \p true on success, \p false if selection
///   cannot be changed, e.g. due to the ongoing operation
///   on a score (like dragging elements).
///   \since MuseScore 3.5
//---------------------------------------------------------

bool Selection::deselect(Element* elWrapper)
{
    if (!checkSelectionIsNotLocked()) {
        return false;
    }

    if (!elWrapper) {
        return false;
    }

    _select->score()->deselect(elWrapper->element());
    return true;
}

//---------------------------------------------------------
//   Selection::clear
///   Clears the selection.
///   \return \p true on success, \p false if selection
///   cannot be changed, e.g. due to the ongoing operation
///   on a score (like dragging elements).
///   \since MuseScore 3.5
//---------------------------------------------------------

bool Selection::clear()
{
    if (!checkSelectionIsNotLocked()) {
        return false;
    }

    _select->deselectAll();
    return true;
}
}
}
