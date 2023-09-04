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

#include "location.h"

#include "chord.h"
#include "engravingitem.h"
#include "measure.h"
#include "mscore.h"
#include "note.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
static constexpr Location absDefaults = Location::absolute();

//---------------------------------------------------------
//   Location::track
//---------------------------------------------------------

int Location::track() const
{
    if ((_staff == absDefaults._staff) || (_voice == absDefaults._voice)) {
        return INT_MIN;
    }
    return static_cast<int>(VOICES) * _staff + _voice;
}

//---------------------------------------------------------
//   Location::setTrack
//---------------------------------------------------------

void Location::setTrack(int track)
{
    _staff = track / VOICES;
    _voice = track % VOICES;
}

//---------------------------------------------------------
//   Location::toAbsolute
//---------------------------------------------------------

void Location::toAbsolute(const Location& ref)
{
    if (isAbsolute()) {
        return;
    }
    _staff += ref._staff;
    _voice += ref._voice;
    _measure += ref._measure;
    _frac += ref._frac;
    _note += ref._note;
    _rel = false;
}

//---------------------------------------------------------
//   Location::toRelative
//---------------------------------------------------------

void Location::toRelative(const Location& ref)
{
    if (isRelative()) {
        return;
    }
    _staff -= ref._staff;
    _voice -= ref._voice;
    _measure -= ref._measure;
    _frac -= ref._frac;
    _note -= ref._note;
    _rel = true;
}

//---------------------------------------------------------
//   Location::fillPositionForElement
//    Fills default fields of Location by values relevant
//    for the given EngravingItem. This function fills only
//    position values, not dealing with parameters specific
//    for Chords and Notes, like grace index.
//---------------------------------------------------------

void Location::fillPositionForElement(const EngravingItem* e, bool absfrac)
{
    assert(isAbsolute());
    if (!e) {
        LOGW("Location::fillPositionForElement: element is nullptr");
        return;
    }
    if (track() == absDefaults.track()) {
        setTrack(track(e));
    }
    if (frac() == absDefaults.frac()) {
        setFrac(absfrac ? e->tick() : e->rtick());
    }
    if (measure() == absDefaults.measure()) {
        setMeasure(absfrac ? 0 : measure(e));
    }
}

//---------------------------------------------------------
//   Location::fillForElement
//    Fills default fields of Location by values relevant
//    for the given EngravingItem, including parameters specific
//    for Chords and Notes.
//---------------------------------------------------------

void Location::fillForElement(const EngravingItem* e, bool absfrac)
{
    assert(isAbsolute());
    if (!e) {
        LOGW("Location::fillForElement: element is nullptr");
        return;
    }

    fillPositionForElement(e, absfrac);
    setGraceIndex(graceIndex(e));
    setNote(note(e));
}

//---------------------------------------------------------
//   Location::forElement
//---------------------------------------------------------

Location Location::forElement(const EngravingItem* e, bool absfrac)
{
    Location i = Location::absolute();
    i.fillForElement(e, absfrac);
    return i;
}

//---------------------------------------------------------
//   Location::positionForElement
//---------------------------------------------------------

Location Location::positionForElement(const EngravingItem* e, bool absfrac)
{
    Location i = Location::absolute();
    i.fillPositionForElement(e, absfrac);
    return i;
}

//---------------------------------------------------------
//   Location::track
//---------------------------------------------------------

int Location::track(const EngravingItem* e)
{
    int track = static_cast<int>(e->track());
    if (track < 0) {
        const MeasureBase* mb = e->findMeasureBase();
        if (mb && !mb->isMeasure()) {
            // Such elements are written in the first staff,
            // see writeMeasure() in scorefile.cpp
            track = 0;
        }
    }
    return track;
}

//---------------------------------------------------------
//   Location::measure
//---------------------------------------------------------

int Location::measure(const EngravingItem* e)
{
    const Measure* m = toMeasure(e->findMeasure());
    if (m) {
        return m->measureIndex();
    }
    LOGW("Location::measure: cannot find element's measure (%s)", e->typeName());
    return 0;
}

//---------------------------------------------------------
//   Location::graceIndex
//---------------------------------------------------------

int Location::graceIndex(const EngravingItem* e)
{
    if (e->isChord() || (e->explicitParent() && e->explicitParent()->isChord())) {
        const Chord* ch = e->isChord() ? toChord(e) : toChord(e->explicitParent());
        if (ch->isGrace()) {
            return static_cast<int>(ch->graceIndex());
        }
    }
    return absDefaults.graceIndex();
}

//---------------------------------------------------------
//   Location::note
//---------------------------------------------------------

int Location::note(const EngravingItem* e)
{
    if (e->isNote()) {
        const Note* n = toNote(e);
        const std::vector<Note*>& notes = n->chord()->notes();
        if (notes.size() == 1) {
            return 0;
        }
        size_t noteIdx;
        for (noteIdx = 0; noteIdx < notes.size(); ++noteIdx) {
            if (n == notes.at(noteIdx)) {
                break;
            }
        }
        return static_cast<int>(noteIdx);
    }
    return absDefaults.note();
}

//---------------------------------------------------------
//   Location::getLocationProperty
//---------------------------------------------------------

PropertyValue Location::getLocationProperty(Pid pid, const EngravingItem* start, const EngravingItem* end)
{
    switch (pid) {
    case Pid::LOCATION_STAVES:
        return (track(start) / VOICES) - (track(end) / VOICES);
    case Pid::LOCATION_VOICES:
        return (track(start) % VOICES) - (track(end) / VOICES);
    case Pid::LOCATION_MEASURES:
        return measure(end) - measure(start);
    case Pid::LOCATION_FRACTIONS:
        return end->rtick() - start->rtick();
    case Pid::LOCATION_GRACE:
        return graceIndex(start) - graceIndex(end);
    case Pid::LOCATION_NOTE:
        return note(start) - note(end);
    default:
        return PropertyValue();
    }
}

//---------------------------------------------------------
//   Location::operator==
//---------------------------------------------------------

bool Location::operator==(const Location& pi2) const
{
    const Location& pi1 = *this;
    return (pi1._frac == pi2._frac)
           && (pi1._measure == pi2._measure)
           && (pi1._voice == pi2._voice)
           && (pi1._staff == pi2._staff)
           && (pi1._graceIndex == pi2._graceIndex)
           && (pi1._note == pi2._note)
    ;
}
}
