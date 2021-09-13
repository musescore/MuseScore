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
#include "pianorollcontroller.h"

#include "log.h"

#include <vector>

#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/tuplet.h"
#include "libmscore/segment.h"
#include "libmscore/noteevent.h"
#include "libmscore/undo.h"
#include "libmscore/utils.h"
#include "libmscore/mscore.h"

#include <qdebug.h>

using namespace mu::pianoroll;
using namespace mu::midi;
using namespace mu::notation;
using namespace mu::async;
using namespace mu::audio;
using namespace mu::actions;

void PianorollController::init()
{
    memset(m_pitchHighlight, 0, 128);

    onNotationChanged();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });
}

int PianorollController::getNotes() const
{
    return 49;
}

Ms::Measure* PianorollController::lastMeasure()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    Ms::Score* score = notation->elements()->msScore();
    return score->lastMeasure();
}

Ms::Score* PianorollController::score()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    Ms::Score* score = notation->elements()->msScore();
    return score;
}

Notification PianorollController::noteLayoutChanged() const
{
    return m_noteLayoutChanged;
}

Notification PianorollController::pitchHighlightChanged() const
{
    return m_pitchHighlightChanged;
}

void PianorollController::onSelectionChanged()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }
    std::vector<EngravingItem*> selectedElements = notation->interaction()->selection()->elements();
}

void PianorollController::onNotationChanged()
{
    auto notation = globalContext()->currentNotation();
    if (notation) {
        notation->interaction()->selectionChanged().onNotify(this, [this]() {
            onSelectionChanged();
        });

        notation->undoStack()->stackChanged().onNotify(this, [this]() {
            onUndoStackChanged();
        });

        notation->notationChanged().onNotify(this, [this]() {
            onCurrentNotationChanged();
        });
    }

    buildNoteBlocks();
}

void PianorollController::onUndoStackChanged()
{
    int j = 9;
    qDebug() << "onUndoStackChanged";
}

void PianorollController::onCurrentNotationChanged()
{
    int j = 9;
    qDebug() << "notationChanged";
}

void PianorollController::setXZoom(double value)
{
    if (value == m_xZoom) {
        return;
    }
    m_xZoom = value;
    emit xZoomChanged();
}

void PianorollController::setNoteHeight(int value)
{
    if (value == m_noteHeight) {
        return;
    }
    m_noteHeight = value;
    emit noteHeightChanged();
}

void PianorollController::addChord(Chord* chrd, int voice)
{
    //for (Chord* c : chrd->graceNotes())
    //    addChord(c, voice);

    //for (Note* note : chrd->notes()) {
    //    if (note->tieBack())
    //          continue;
    //    m_notes.push_back(NoteBlock(note));
    //}
}

Ms::Fraction PianorollController::widthInBeats()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return Ms::Fraction();
    }

    Ms::Score* score = notation->elements()->msScore();

    Ms::Measure* lm = score->lastMeasure();
    Ms::Fraction beats = lm->tick() + lm->ticks();
    return beats;
}

void PianorollController::buildNoteBlocks()
{
}

void PianorollController::setPitchHighlight(int pitch, bool value)
{
    if (m_pitchHighlight[pitch] == value) {
        return;
    }

    m_pitchHighlight[pitch] = value;
    m_pitchHighlightChanged.notify();
}
