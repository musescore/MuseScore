/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "percussionutilities.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/note.h"
#include "engraving/dom/stem.h"

#include "engraving/dom/masterscore.h"

#include "engraving/dom/factory.h"

using namespace mu::notation;
using namespace mu::engraving;

/// Returns a drum note prepared for preview.
std::shared_ptr<Chord> PercussionUtilities::getDrumNoteForPreview(const Drumset* drumset, int pitch)
{
    double _spatium = gpaletteScore->style().spatium(); // TODO: Don't use palette here?

    bool up = false;
    int line = drumset->line(pitch);
    NoteHeadGroup noteHead = drumset->noteHead(pitch);
    int voice = drumset->voice(pitch);
    DirectionV dir = drumset->stemDirection(pitch);

    if (dir == DirectionV::UP) {
        up = true;
    } else if (dir == DirectionV::DOWN) {
        up = false;
    } else {
        up = line > 4;
    }

    auto chord = Factory::makeChord(gpaletteScore->dummy()->segment());
    chord->setDurationType(DurationType::V_QUARTER);
    chord->setStemDirection(dir);
    chord->setIsUiItem(true);
    chord->setTrack(voice);

    Note* note = Factory::createNote(chord.get());
    note->setMark(true);
    note->setParent(chord.get());
    note->setTrack(voice);
    note->setPitch(pitch);
    note->setTpcFromPitch();
    note->setLine(line);
    note->setPos(0.0, _spatium * .5 * line);
    note->setHeadGroup(noteHead);

    SymId noteheadSym = SymId::noteheadBlack;
    if (noteHead == NoteHeadGroup::HEAD_CUSTOM) {
        noteheadSym = drumset->noteHeads(pitch, NoteHeadType::HEAD_QUARTER);
    } else {
        noteheadSym = note->noteHead(true, noteHead, NoteHeadType::HEAD_QUARTER);
    }

    note->mutldata()->cachedNoteheadSym.set_value(noteheadSym); // we use the cached notehead so we don't recompute it at each layout
    chord->add(note);

    Stem* stem = Factory::createStem(chord.get());
    stem->setParent(chord.get());
    stem->setBaseLength(Millimetre((up ? -3.0 : 3.0) * _spatium));
    engravingRender()->layoutItem(stem);
    chord->add(stem);

    return chord;
}
