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

#include "accidental.h"
#include "chord.h"
#include "engravingitem.h"
#include "factory.h"
#include "key.h"
#include "layout/tlayout.h"
#include "layout/chordlayout.h"
#include "note.h"
#include "ornament.h"
#include "score.h"
#include "shape.h"
#include "staff.h"
#include "utils.h"

namespace mu::engraving {
Ornament::Ornament(ChordRest* parent)
    : Articulation(parent, ElementType::ORNAMENT)
{
    _intervalAbove = OrnamentInterval(IntervalStep::SECOND, IntervalType::AUTO);
    _intervalBelow = OrnamentInterval(IntervalStep::SECOND, IntervalType::AUTO);
    _showAccidental = OrnamentShowAccidental::DEFAULT;
    _startOnUpperNote = false;
}

Ornament::Ornament(const Ornament& o)
    : Articulation(o)
{
    _intervalAbove = o._intervalAbove;
    _intervalBelow = o._intervalBelow;
    _showAccidental = o._showAccidental;
    _startOnUpperNote = o._startOnUpperNote;
}

Ornament::~Ornament()
{
    DeleteAll(_notesAboveAndBelow);
    DeleteAll(_accidentalsAboveAndBelow);
    if (_cueNoteChord && _cueNoteChord->notes().size()) {
        _cueNoteChord->notes().clear();
    }
    delete _cueNoteChord;
}

void Ornament::remove(EngravingItem* e)
{
    if (e->isAccidental()) {
        for (Accidental*& acc : _accidentalsAboveAndBelow) {
            if (e == acc) {
                acc = nullptr;
                e->removed();
            }
        }
    }
}

void Ornament::draw(draw::Painter* painter) const
{
    Articulation::draw(painter);
}

void Ornament::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    func(data, this);
    for (Accidental* accidental : _accidentalsAboveAndBelow) {
        if (accidental) {
            func(data, accidental);
        }
    }
    if (_cueNoteChord) {
        _cueNoteChord->scanElements(data, func, all);
    }
}

PropertyValue Ornament::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::INTERVAL_ABOVE:
        return _intervalAbove;
    case Pid::INTERVAL_BELOW:
        return _intervalBelow;
    case Pid::ORNAMENT_SHOW_ACCIDENTAL:
        return _showAccidental;
    case Pid::START_ON_UPPER_NOTE:
        return _startOnUpperNote;
    default:
        return Articulation::getProperty(propertyId);
    }
}

PropertyValue Ornament::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::INTERVAL_ABOVE:
    case Pid::INTERVAL_BELOW:
        return OrnamentInterval(IntervalStep::SECOND, IntervalType::AUTO);
    case Pid::ORNAMENT_SHOW_ACCIDENTAL:
        return OrnamentShowAccidental::DEFAULT;
    case Pid::START_ON_UPPER_NOTE:
        return false;
    default:
        return Articulation::propertyDefault(id);
    }
}

bool Ornament::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::INTERVAL_ABOVE:
        setIntervalAbove(v.value<OrnamentInterval>());
        break;
    case Pid::INTERVAL_BELOW:
        setIntervalBelow(v.value<OrnamentInterval>());
        break;
    case Pid::ORNAMENT_SHOW_ACCIDENTAL:
        setShowAccidental(v.value<OrnamentShowAccidental>());
        break;
    case Pid::START_ON_UPPER_NOTE:
        setStartOnUpperNote(v.toBool());
        break;
    default:
        return Articulation::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

bool Ornament::hasIntervalAbove() const
{
    SymId id = symId();
    return id == SymId::ornamentTurnInverted
           || id == SymId::ornamentTrill
           || id == SymId::ornamentTurnSlash
           || id == SymId::ornamentTurn
           || id == SymId::ornamentShortTrill
           || id == SymId::ornamentTremblement
           || id == SymId::ornamentPrallMordent
           || id == SymId::ornamentUpPrall
           || id == SymId::ornamentPrecompMordentUpperPrefix
           || id == SymId::ornamentUpMordent
           || id == SymId::ornamentPrallUp;
}

bool Ornament::hasIntervalBelow() const
{
    SymId id = symId();
    return id == SymId::ornamentTurnInverted
           || id == SymId::ornamentMordent
           || id == SymId::ornamentTurnSlash
           || id == SymId::ornamentTurn
           || id == SymId::ornamentTremblement
           || id == SymId::ornamentPrallMordent
           || id == SymId::ornamentDownMordent
           || id == SymId::ornamentPrallDown
           || id == SymId::ornamentPrallMordent
           || id == SymId::ornamentPrallMordent;
}

bool Ornament::hasFullIntervalChoice() const
{
    SymId id = symId();
    return id == SymId::ornamentTrill;
}

void Ornament::computeNotesAboveAndBelow(AccidentalState* accState)
{
    Chord* parentChord = explicitParent() ? toChord(parent()) : nullptr;
    Note* mainNote = parentChord ? parentChord->upNote() : nullptr;

    if (!mainNote) {
        return;
    }

    for (int i = 0; i < _notesAboveAndBelow.size(); ++i) {
        bool above = (i == 0);
        bool hasIntAbove = hasIntervalAbove();
        bool hasIntBelow = hasIntervalBelow();

        if ((above && !hasIntAbove) || (!above && !hasIntBelow)) {
            continue;
        }

        // First: get the right notes above or below as needed
        Note*& note = _notesAboveAndBelow[i];
        if (!note) {
            note = mainNote->clone();
        } else {
            note->setTpc1(mainNote->tpc1());
            note->setTpc2(mainNote->tpc2());
            note->setPitch(mainNote->pitch());
        }
        if ((above && _intervalAbove.type == IntervalType::AUTO)
            || (!above && _intervalBelow.type == IntervalType::AUTO)) {
            // NOTE: In AUTO mode, the ornament note should match not only any alteration from the
            // key signature, but also any alteration present in the measure before this point.
            int intervalSteps = above ? static_cast<int>(_intervalAbove.step) : -static_cast<int>(_intervalBelow.step);
            note->transposeDiatonic(intervalSteps, false, true);
            if (_trillOldCompatAccidental) {
                // Compatibility with trills pre-4.1
                AccidentalVal oldCompatValue = Accidental::subtype2value(_trillOldCompatAccidental->accidentalType());
                AccidentalVal noteAccidentalVal = tpc2alter(note->tpc());
                int accidentalDiff = static_cast<int>(oldCompatValue) - static_cast<int>(noteAccidentalVal);
                score()->transpose(note, Interval(0, accidentalDiff), true);
                int semitones = note->pitch() - mainNote->pitch();
                switch (semitones) {
                case 0:
                    _intervalAbove.type = IntervalType::DIMINISHED;
                    break;
                case 1:
                    _intervalAbove.type = IntervalType::MINOR;
                    break;
                case 2:
                    _intervalAbove.type = IntervalType::MAJOR;
                    break;
                case 3:
                    _intervalAbove.type = IntervalType::AUGMENTED;
                    break;
                default:
                    break;
                }
                _trillOldCompatAccidental = nullptr;
            } else {
                int pitchLine = absStep(note->tpc(), note->epitch() + note->ottaveCapoFret());
                AccidentalVal accidentalVal = accState->accidentalVal(pitchLine);
                AccidentalVal noteAccidentalVal = tpc2alter(note->tpc());
                int accidentalDiff = static_cast<int>(accidentalVal) - static_cast<int>(noteAccidentalVal);
                score()->transpose(note, Interval(0, accidentalDiff), true);
            }
        } else {
            Interval interval = Interval::fromOrnamentInterval(above ? _intervalAbove : _intervalBelow);
            if (!above) {
                interval.flip();
            }
            score()->transpose(note, interval, true);
        }

        note->updateAccidental(accState);

        // Second: if the "Accidental visibility" property is different than default,
        // force the implicit accidentals to be visible when appropriate
        if (_showAccidental == OrnamentShowAccidental::ALWAYS) {
            AccidentalVal accidentalValue = tpc2alter(note->tpc());
            AccidentalType accidentalType = Accidental::value2subtype(accidentalValue);
            if (accidentalType == AccidentalType::NONE) {
                accidentalType = AccidentalType::NATURAL;
            }
            note->setAccidentalType(accidentalType);
            note->accidental()->setRole(AccidentalRole::AUTO);
        } else if (_showAccidental == OrnamentShowAccidental::ANY_ALTERATION) {
            AccidentalVal accidentalValue = tpc2alter(note->tpc());
            Key key = staff()->key(tick());
            AccidentalState unalteredState;
            unalteredState.init(key);
            int pitchLine = absStep(note->tpc(), note->epitch() + note->ottaveCapoFret());
            AccidentalVal unalteredAccidentalVal = unalteredState.accidentalVal(pitchLine);
            if (accidentalValue != unalteredAccidentalVal) {
                note->setAccidentalType(Accidental::value2subtype(accidentalValue));
                note->accidental()->setRole(AccidentalRole::AUTO);
            }
        }
    }

    updateAccidentalsAboveAndBelow();
    updateCueNote();
}

void Ornament::updateAccidentalsAboveAndBelow()
{
    for (int i = 0; i < _notesAboveAndBelow.size(); ++i) {
        Note* note = _notesAboveAndBelow[i];
        Accidental* accidental = note ? note->accidental() : nullptr;
        Accidental*& curAccidental = _accidentalsAboveAndBelow[i];
        if (showCueNote() || !accidental) {
            if (curAccidental) {
                delete curAccidental;
                curAccidental = nullptr;
            }
            continue;
        }
        if (accidental) {
            if (!curAccidental) {
                curAccidental = accidental->clone();
                curAccidental->setParent(this);
                curAccidental->setPlacement(i == 0 ? PlacementV::ABOVE : PlacementV::BELOW);
            } else {
                curAccidental->setAccidentalType(accidental->accidentalType());
                curAccidental->setRole(accidental->role());
            }
        }
    }
}

void Ornament::updateCueNote()
{
    if (!showCueNote()) {
        if (noteAbove() && explicitParent()) {
            noteAbove()->setParent(toChord(parentItem()));
        }
        if (_cueNoteChord) {
            _cueNoteChord->notes().clear();
            delete _cueNoteChord;
            _cueNoteChord = nullptr;
        }
        return;
    }

    if (!explicitParent()) {
        return;
    }

    Chord* parentChord = toChord(parentItem());
    Note* cueNote = noteAbove();
    // If needed, create cue note
    if (!_cueNoteChord) {
        _cueNoteChord = Factory::createChord(parentChord->segment());
        _cueNoteChord->setTrack(track());
        _cueNoteChord->setSmall(true);
        cueNote->setHeadHasParentheses(true);
        cueNote->setHeadType(NoteHeadType::HEAD_QUARTER);
        _cueNoteChord->add(cueNote);
        cueNote->setParent(_cueNoteChord);
    }
    cueNote->setIsTrillCueNote(true);
    _cueNoteChord->setParent(parentChord->segment());
}

Shape Ornament::shape() const
{
    Shape s;
    s.add(bbox(), this);
    for (Accidental* accidental : _accidentalsAboveAndBelow) {
        if (accidental && accidental->visible()) {
            s.add(accidental->shape().translate(accidental->pos()));
        }
    }
    return s;
}

SymId Ornament::fromTrillType(TrillType trillType)
{
    switch (trillType) {
    case TrillType::TRILL_LINE:
        return SymId::ornamentTrill;
    case TrillType::UPPRALL_LINE:
        return SymId::ornamentUpPrall;
    case TrillType::DOWNPRALL_LINE:
        return SymId::ornamentDownMordent;
    case TrillType::PRALLPRALL_LINE:
        return SymId::ornamentTrill;
    default:
        return SymId::noSym;
    }
}
} // namespace mu::engraving
