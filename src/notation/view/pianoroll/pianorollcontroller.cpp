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
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "pianorollcontroller.h"

#include "engraving/dom/score.h"
#include "engraving/dom/part.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/commands.h"
#include "notation/inotationnoteinput.h"
#include "notation/internal/inotationundostack.h"
#include "notation/inotationinteraction.h"


namespace mu::notation {

PianoRollController::PianoRollController(QObject* parent)
    : QObject(parent)
{
}

void PianoRollController::setNotation(INotationPtr notation)
{
    m_notation = notation;
    if (m_notation) {
        connect(m_notation->interaction()->noteInput(), &INotationNoteInput::stateChanged, this, &PianoRollController::drumsetChanged);
    }
}

QVariant PianoRollController::drumset() const
{
    const mu::engraving::Drumset* drumset = currentDrumset();
    if (drumset) {
        return QVariant::fromValue(const_cast<mu::engraving::Drumset*>(drumset));
    }
    return QVariant();
}

QStringList PianoRollController::drumNames() const
{
    QStringList names;
    const mu::engraving::Drumset* drumset = currentDrumset();
    if (drumset) {
        for (int i = 0; i < mu::engraving::DRUM_INSTRUMENTS; ++i) {
            if (drumset->isValid(i)) {
                names.append(drumset->translatedName(i));
            }
        }
    }
    return names;
}

int PianoRollController::pitch(const QString& name) const
{
    const mu::engraving::Drumset* drumset = currentDrumset();
    if (drumset) {
        for (int i = 0; i < mu::engraving::DRUM_INSTRUMENTS; ++i) {
            if (drumset->isValid(i) && drumset->translatedName(i) == name) {
                return i;
            }
        }
    }
    return -1;
}

const mu::engraving::Drumset* PianoRollController::currentDrumset() const
{
    if (!m_notation) {
        return nullptr;
    }

    auto noteInput = m_notation->interaction()->noteInput();
    if (!noteInput) {
        return nullptr;
    }

    return noteInput->state().drumset();
}

void PianoRollController::addNote(int pitch, int tick)
{
    if (!m_notation) {
        return;
    }

    auto score = m_notation->elements()->msScore();
    if (!score) {
        return;
    }

    auto noteInput = m_notation->interaction()->noteInput();
    if (!noteInput) {
        return;
    }

    const auto& state = noteInput->state();
    const auto track = state.track();
    const auto staffId = state.staffIdx();
    auto part = score->part(staffId);
    if (!part) {
        return;
    }
    auto staff = part->staff(staffId);
    if (!staff) {
        return;
    }

    auto segment = staff->segmentAtTick(tick);
    if (!segment) {
        // TODO: create a new segment
        return;
    }

    auto chord = new mu::engraving::Chord(score);
    chord->setTrack(track);

    auto note = new mu::engraving::Note(chord);
    note->setPitch(pitch);

    segment->add(chord);

    m_notation->interaction()->undoStack()->push(new mu::engraving::AddRemoveCommand(chord, true));
}

} // namespace mu::notation
