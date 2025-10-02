/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "editfretboarddiagram.h"

#include "../dom/box.h"
#include "../dom/fret.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   updateStored
//---------------------------------------------------------

FretUndoData::FretUndoData(FretDiagram* fd)
{
    // We need to store the old barres and markers, since predicting how
    // adding dots, markers, barres etc. will change things is too difficult.
    // Update linked fret diagrams:
    m_diagram = fd;
    m_dots = m_diagram->m_dots;
    m_markers = m_diagram->m_markers;
    m_barres = m_diagram->m_barres;

    m_strings = m_diagram->m_strings;
    m_frets = m_diagram->m_frets;
    m_fretOffset = m_diagram->m_fretOffset;
    m_maxFrets = m_diagram->m_maxFrets;
    m_showNut = m_diagram->m_showNut;
    m_orientation = m_diagram->m_orientation;

    m_userMag = m_diagram->m_userMag;

    m_showFingering = m_diagram->m_showFingering;
}

//---------------------------------------------------------
//   updateDiagram
//---------------------------------------------------------

void FretUndoData::updateDiagram()
{
    if (!m_diagram) {
        ASSERT_X("Tried to undo fret diagram change without ever setting diagram!");
        return;
    }

    // Reset every fret diagram property of the changed diagram
    // FretUndoData is a friend of FretDiagram so has access to these private members
    m_diagram->m_barres = m_barres;
    m_diagram->m_markers = m_markers;
    m_diagram->m_dots = m_dots;

    m_diagram->m_strings = m_strings;
    m_diagram->m_frets = m_frets;
    m_diagram->m_fretOffset = m_fretOffset;
    m_diagram->m_maxFrets = m_maxFrets;
    m_diagram->m_showNut = m_showNut;
    m_diagram->m_orientation = m_orientation;

    m_diagram->m_userMag = m_userMag;

    m_diagram->m_showFingering = m_showFingering;
}

//---------------------------------------------------------
//   FretDataChange
//---------------------------------------------------------

void FretDataChange::redo(EditData*)
{
    m_undoData = FretUndoData(m_diagram);

    m_diagram->updateDiagram(m_harmonyName);
}

void FretDataChange::undo(EditData*)
{
    m_undoData.updateDiagram();
}

//---------------------------------------------------------
//   FretDot
//---------------------------------------------------------

void FretDot::redo(EditData*)
{
    undoData = FretUndoData(diagram);

    diagram->setDot(string, fret, add, dtype);
    diagram->triggerLayout();
}

void FretDot::undo(EditData*)
{
    undoData.updateDiagram();
    diagram->triggerLayout();
}

//---------------------------------------------------------
//   FretMarker
//---------------------------------------------------------

void FretMarker::redo(EditData*)
{
    undoData = FretUndoData(diagram);

    diagram->setMarker(string, mtype);
    diagram->triggerLayout();
}

void FretMarker::undo(EditData*)
{
    undoData.updateDiagram();
    diagram->triggerLayout();
}

//---------------------------------------------------------
//   FretBarre
//---------------------------------------------------------

void FretBarre::redo(EditData*)
{
    undoData = FretUndoData(diagram);

    diagram->setBarre(string, fret, add);
    diagram->triggerLayout();
}

void FretBarre::undo(EditData*)
{
    undoData.updateDiagram();
    diagram->triggerLayout();
}

//---------------------------------------------------------
//   FretClear
//---------------------------------------------------------

void FretClear::redo(EditData*)
{
    undoData = FretUndoData(diagram);

    diagram->clear();
    diagram->triggerLayout();
}

void FretClear::undo(EditData*)
{
    undoData.updateDiagram();
    diagram->triggerLayout();
}

//---------------------------------------------------------
//   AddFretDiagramToFretBox
//---------------------------------------------------------

AddFretDiagramToFretBox::AddFretDiagramToFretBox(FretDiagram* f, size_t idx)
    : m_fretDiagram(f), m_idx(idx)
{
}

void AddFretDiagramToFretBox::redo(EditData*)
{
    FBox* fbox = toFBox(m_fretDiagram->parent());
    fbox->addAtIdx(m_fretDiagram, m_idx);
}

void AddFretDiagramToFretBox::undo(EditData*)
{
    FBox* fbox = toFBox(m_fretDiagram->parent());
    fbox->remove(m_fretDiagram);
}

//---------------------------------------------------------
//   RemoveFretDiagramFromFretBox
//---------------------------------------------------------

RemoveFretDiagramFromFretBox::RemoveFretDiagramFromFretBox(FretDiagram* f)
    : m_fretDiagram(f)
{
    FBox* fbox = toFBox(f->parent());
    const ElementList& el = fbox->el();
    m_idx = muse::indexOf(el, m_fretDiagram);
}

void RemoveFretDiagramFromFretBox::redo(EditData*)
{
    FBox* fbox = toFBox(m_fretDiagram->parent());
    fbox->remove(m_fretDiagram);
}

void RemoveFretDiagramFromFretBox::undo(EditData*)
{
    FBox* fbox = toFBox(m_fretDiagram->parent());
    fbox->addAtIdx(m_fretDiagram, m_idx);
}
