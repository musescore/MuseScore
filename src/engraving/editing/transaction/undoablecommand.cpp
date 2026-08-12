/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "undoablecommand.h"

#include "dom/chord.h"
#include "dom/fret.h"
#include "dom/harmony.h"
#include "dom/note.h"

using namespace mu::engraving;

std::vector<EngravingObject*> mu::engraving::compoundObjects(EngravingObject* object)
{
    std::vector<EngravingObject*> objects;

    if (object->isChord()) {
        const Chord* chord = toChord(object);
        for (const Note* note : chord->notes()) {
            for (Note* compoundNote : note->compoundNotes()) {
                objects.push_back(compoundNote);
            }
        }
    } else if (object->isNote()) {
        const Note* note = toNote(object);
        for (Note* compoundNote : note->compoundNotes()) {
            objects.push_back(compoundNote);
        }
    } else if (object->isFretDiagram()) {
        const FretDiagram* fret = toFretDiagram(object);
        if (Harmony* harmony = fret->harmony()) {
            objects.push_back(harmony);
        }
    }

    objects.push_back(object);

    return objects;
}

void UndoableCommand::undo()
{
    flip();
}

void UndoableCommand::redo()
{
    flip();
}
