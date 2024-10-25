/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "laissezvib.h"
#include "chord.h"
#include "dom/measure.h"
#include "dom/mscoreview.h"
#include "dom/score.h"
#include "note.h"
#include "staff.h"
#include "style/style.h"

using namespace mu::engraving;

static const ElementStyle laissezVibStyle {
    { Sid::minLaissezVibLength, Pid::MIN_LENGTH }
};

LaissezVib::LaissezVib(Note* parent)
    : Tie(ElementType::LAISSEZ_VIB, parent)
{
    initElementStyle(&laissezVibStyle);
}

mu::engraving::PropertyValue mu::engraving::LaissezVib::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::MIN_LENGTH:
        return minLength();
    default:
        return Tie::getProperty(propertyId);
    }
}

PropertyValue LaissezVib::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::MIN_LENGTH:
        return Spatium(2.0);
    default:
        return Tie::getProperty(propertyId);
    }
}

bool LaissezVib::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::MIN_LENGTH:
        setMinLength(v.value<Spatium>());
        break;
    default:
        return Tie::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

SymId LaissezVib::symId() const
{
    return up() ? SymId::articLaissezVibrerAbove : SymId::articLaissezVibrerBelow;
}

static int compareNotesPos(const Note* n1, const Note* n2)
{
    if (n1->line() != n2->line() && !(n1->staffType()->isTabStaff())) {
        return n2->line() - n1->line();
    } else if (n1->string() != n2->string()) {
        return n2->string() - n1->string();
    } else {
        return n1->pitch() - n2->pitch();
    }
}

void LaissezVib::calculateDirection()
{
    const Note* note = startNote();
    const Chord* chord = note ? note->chord() : nullptr;
    const Measure* m1 = chord ? chord->measure() : nullptr;
    if (!m1) {
        return;
    }

    if (m_slurDirection == DirectionV::AUTO) {
        std::vector<Note*> notes = chord->notes();
        size_t n = notes.size();
        StaffType* st = staff()->staffType(note ? note->tick() : Fraction(0, 1));
        bool simpleException = st && st->isSimpleTabStaff();
        // if there are multiple voices, the tie direction goes on stem side
        if (m1->hasVoices(chord->staffIdx(), chord->tick(), chord->actualTicks())) {
            m_up = simpleException ? isUpVoice(chord->voice()) : chord->up();
        } else if (n == 1) {
            // single note
            m_up = !chord->up();
        } else {
            // chords
            // first, find pivot point in chord (below which all ties curve down and above which all ties curve up)
            Note* pivotPoint = nullptr;
            bool multiplePivots = false;
            for (size_t i = 0; i < n - 1; ++i) {
                if (!notes[i]->tieFor()) {
                    continue; // don't include notes that don't have ties
                }
                for (size_t j = i + 1; j < n; ++j) {
                    if (!notes[j]->tieFor()) {
                        continue;
                    }
                    int noteDiff = compareNotesPos(notes[i], notes[j]);
                    if (!multiplePivots && std::abs(noteDiff) <= 1) {
                        // TODO: Fix unison ties somehow--if noteDiff == 0 then we need to determine which of the unison is 'lower'
                        if (pivotPoint) {
                            multiplePivots = true;
                            pivotPoint = nullptr;
                        } else {
                            pivotPoint = noteDiff < 0 ? notes[i] : notes[j];
                        }
                    }
                }
            }
            if (!pivotPoint) {
                // if the pivot point was not found (either there are no unisons/seconds or there are more than one),
                // determine if this note is in the lower or upper half of this chord
                int notesAbove = 0, tiesAbove = 0;
                int notesBelow = 0, tiesBelow = 0;
                int unisonTies = 0;
                for (size_t i = 0; i < n; ++i) {
                    if (notes[i] == note) {
                        // skip counting if this note is the current note or if this note doesn't have a tie
                        continue;
                    }
                    int noteDiff = compareNotesPos(note, notes[i]);
                    if (noteDiff == 0) {  // unison
                        if (notes[i]->tieFor()) {
                            unisonTies++;
                        }
                    }
                    if (noteDiff < 0) { // the note is above startNote
                        notesAbove++;
                        if (notes[i]->tieFor()) {
                            tiesAbove++;
                        }
                    }
                    if (noteDiff > 0) { // the note is below startNote
                        notesBelow++;
                        if (notes[i]->tieFor()) {
                            tiesBelow++;
                        }
                    }
                }

                if (tiesAbove == 0 && tiesBelow == 0 && unisonTies == 0) {
                    // this is the only tie in the chord.
                    if (notesAbove == notesBelow) {
                        m_up = !chord->up();
                    } else {
                        m_up = (notesAbove < notesBelow);
                    }
                } else if (tiesAbove == tiesBelow) {
                    // this note is dead center, so its tie should go counter to the stem direction
                    m_up = !chord->up();
                } else {
                    m_up = (tiesAbove < tiesBelow);
                }
            } else if (pivotPoint == note) {
                // the current note is the lower of the only second or unison in the chord; tie goes down.
                m_up = false;
            } else {
                // if lower than the pivot, tie goes down, otherwise up
                int noteDiff = compareNotesPos(note, pivotPoint);
                m_up = (noteDiff >= 0);
            }
        }
    } else {
        m_up = m_slurDirection == DirectionV::UP ? true : false;
    }
}

void LaissezVib::calculateIsInside()
{
    if (_tiePlacement != TiePlacement::AUTO) {
        setIsInside(_tiePlacement == TiePlacement::INSIDE);
        return;
    }

    const Note* note = startNote();
    const Chord* startChord = note ? note->chord() : nullptr;

    if (!startChord) {
        setIsInside(false);
        return;
    }

    const bool startIsSingleNote = startChord->notes().size() <= 1;

    if (startIsSingleNote) {
        setIsInside(style().styleV(Sid::tiePlacementSingleNote).value<TiePlacement>() == TiePlacement::INSIDE);
    } else {
        setIsInside(style().styleV(Sid::tiePlacementChord).value<TiePlacement>() == TiePlacement::INSIDE);
    }
}

bool LaissezVib::isOuterTieOfChord(Grip grip) const
{
    if (grip != Grip::START) {
        return false;
    }

    if (m_isInside) {
        return false;
    }

    const Note* note = startNote();

    if (!note) {
        return false;
    }

    const Chord* chord = note->chord();

    return (note == chord->upNote() && up()) || (note == chord->downNote() && !up());
}

double LaissezVib::scalingFactor() const
{
    const Note* startN = startNote();

    if (!startN) {
        return 1.0;
    }

    if (startN->isGrace()) {
        return style().styleD(Sid::graceNoteMag);
    }

    return startN->chord()->intrinsicMag();
}

void LaissezVib::setEndNote(Note* note)
{
    setEndElement(note);
}

void LaissezVib::setEndElement(EngravingItem* e)
{
    UNUSED(e);
    LOGE() << "Laissez vibrer ties do not have an end note";
}

LaissezVibSegment::LaissezVibSegment(System* parent)
    : TieSegment(ElementType::LAISSEZ_VIB_SEGMENT, parent)
{
}

LaissezVibSegment::LaissezVibSegment(const LaissezVibSegment& s)
    : TieSegment(s)
{
}

void LaissezVibSegment::editDrag(EditData& ed)
{
    consolidateAdjustmentOffsetIntoUserOffset();

    ups(Grip::DRAG).off = PointF();
    roffset() += ed.delta;
}
