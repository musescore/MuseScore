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

#include "engraving/dom/part.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/score.h"
#include "engraving/dom/note.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/noteevent.h"
#include "engraving/dom/undo.h"
#include "engraving/dom/utils.h"
#include "engraving/dom/mscore.h"

#include <qdebug.h>

using namespace mu::engraving;
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

Measure* PianorollController::lastMeasure()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    Score* score = notation->elements()->msScore();
    return score->lastMeasure();
}

Score* PianorollController::score()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    Score* score = notation->elements()->msScore();
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
    qDebug() << "onUndoStackChanged";
}

void PianorollController::onCurrentNotationChanged()
{
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

Fraction PianorollController::widthInBeats()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return Fraction();
    }

    Score* score = notation->elements()->msScore();

    Measure* lm = score->lastMeasure();
    Fraction beats = lm->tick() + lm->ticks();
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
