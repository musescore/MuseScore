/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "editvisibility.h"

#include "global/containers.h"

#include "../dom/accidental.h"
#include "../dom/beam.h"
#include "../dom/chord.h"
#include "../dom/engravingitem.h"
#include "../dom/note.h"
#include "../dom/notedot.h"
#include "../dom/ornament.h"
#include "../dom/rest.h"
#include "../dom/score.h"
#include "../dom/select.h"
#include "../dom/trill.h"

#include "editproperty.h"
#include "transaction/transaction.h"

using namespace mu::engraving;

void EditVisibility::setSelectedElementsVisible(Transaction& tx, Score* score)
{
    for (EngravingItem* e : score->selection().elements()) {
        tx.push(new ChangeProperty(e, Pid::VISIBLE, true));
    }
}

void EditVisibility::setSelectedElementsInvisible(Transaction& tx, Score* score)
{
    for (EngravingItem* e : score->selection().elements()) {
        tx.push(new ChangeProperty(e, Pid::VISIBLE, false));
    }
}

void EditVisibility::toggleVisible(Transaction& tx, Score* score)
{
    bool allVisible = true;

    for (EngravingItem* item : score->selection().elements()) {
        if (!item->getProperty(Pid::VISIBLE).toBool()) {
            allVisible = false;
            break;
        }
    }

    bool newVisible = !allVisible;

    for (EngravingItem* item : score->selection().elements()) {
        undoChangeVisible(tx, item, newVisible);
    }
}

static bool chordHasVisibleNote(const Chord* chord)
{
    for (const Note* note : chord->notes()) {
        if (note->visible()) {
            return true;
        }
    }

    return false;
}

static void undoChangeOrnamentVisibility(Ornament* ornament, bool visible);

static void undoChangeNoteVisibility(Note* note, bool visible)
{
    note->undoChangeProperty(Pid::VISIBLE, visible);

    if (note->bendBack() || note->bendFor()) {
        note->setOverrideBendVisibilityRules(true);
    } else {
        note->setOverrideBendVisibilityRules(false);
    }

    for (NoteDot* dot : note->dots()) {
        dot->undoChangeProperty(Pid::VISIBLE, visible);
    }

    for (EngravingItem* e : note->el()) {
        e->undoChangeProperty(Pid::VISIBLE, visible);
    }

    if (note->accidental()) {
        note->accidental()->undoChangeProperty(Pid::VISIBLE, visible);
    }

    Chord* noteChord = note->chord();
    Beam* beam = noteChord->beam();
    std::vector<Chord*> chords;

    bool chordHasVisibleNote_ = visible || chordHasVisibleNote(noteChord);
    bool beamHasVisibleNote_ = chordHasVisibleNote_;

    if (beam) {
        for (EngravingItem* item : beam->elements()) {
            if (!item->isChord()) {
                continue;
            }

            Chord* chord = toChord(item);
            chords.push_back(chord);

            if (!beamHasVisibleNote_ && chord != noteChord) {
                beamHasVisibleNote_ = chordHasVisibleNote(chord);
            }
        }
    } else {
        chords.push_back(noteChord);
    }

    static const std::unordered_set<ElementType> IGNORED_TYPES {
        ElementType::NOTE,
        ElementType::LYRICS,
        ElementType::SLUR,
        ElementType::HAMMER_ON_PULL_OFF,
        ElementType::CHORD, // grace notes
        ElementType::LEDGER_LINE, // temporary objects, impossible to change visibility
    };

    for (const Chord* chord : chords) {
        for (const EngravingObject* obj : chord->linkList()) {
            const Chord* linkedChord = toChord(obj);
            chordHasVisibleNote_ = chordHasVisibleNote(linkedChord);
            for (EngravingObject* child : linkedChord->getChildren()) {
                const ElementType type = child->type();

                if (muse::contains(IGNORED_TYPES, type)) {
                    continue;
                }

                if (beam) {
                    if (type == ElementType::STEM || type == ElementType::BEAM) {
                        child->undoChangeProperty(Pid::VISIBLE, beamHasVisibleNote_);
                        continue;
                    }
                }
                if (child->isOrnament()) {
                    undoChangeOrnamentVisibility(toOrnament(child), visible);
                } else {
                    child->undoChangeProperty(Pid::VISIBLE, chordHasVisibleNote_);
                }
            }
        }
    }
}

static void undoChangeRestVisibility(Rest* rest, bool visible)
{
    rest->undoChangeProperty(Pid::VISIBLE, visible);

    for (NoteDot* dot : rest->dotList()) {
        dot->undoChangeProperty(Pid::VISIBLE, visible);
    }
}

static void undoChangeOrnamentVisibility(Ornament* ornament, bool visible)
{
    ornament->undoChangeProperty(Pid::VISIBLE, visible);
    Chord* cueNoteChord = ornament->cueNoteChord();
    if (cueNoteChord) {
        undoChangeNoteVisibility(cueNoteChord->upNote(), visible);
    }
    if (ornament->accidentalAbove()) {
        ornament->accidentalAbove()->undoChangeProperty(Pid::VISIBLE, visible);
    }
    if (ornament->accidentalBelow()) {
        ornament->accidentalBelow()->undoChangeProperty(Pid::VISIBLE, visible);
    }
}

void EditVisibility::undoChangeVisible(Transaction&, EngravingItem* item, bool visible)
{
    if (item->isNote()) {
        undoChangeNoteVisibility(toNote(item), visible);
    } else if (item->isRest()) {
        undoChangeRestVisibility(toRest(item), visible);
    } else if (item->isOrnament()) {
        undoChangeOrnamentVisibility(toOrnament(item), visible);
    } else if (item->isTrillSegment()) {
        item->undoChangeProperty(Pid::VISIBLE, visible);
        Ornament* orn = toTrillSegment(item)->trill()->ornament();
        if (orn) {
            undoChangeOrnamentVisibility(orn, visible);
        }
    } else {
        item->undoChangeProperty(Pid::VISIBLE, visible);
    }
}
