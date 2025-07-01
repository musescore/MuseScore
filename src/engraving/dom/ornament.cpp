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

#include "accidental.h"
#include "chord.h"
#include "engravingitem.h"
#include "factory.h"
#include "key.h"
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
    _intervalAbove = DEFAULT_ORNAMENT_INTERVAL;
    _intervalBelow = DEFAULT_ORNAMENT_INTERVAL;
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
    m_showCueNote = o.m_showCueNote;

    if (o.m_cueNoteChord) {
        m_cueNoteChord = o.m_cueNoteChord->clone();
    }

    for (size_t i = 0; i < m_accidentalsAboveAndBelow.size(); ++i) {
        Accidental* oldAccidental = o.m_accidentalsAboveAndBelow[i];
        if (!oldAccidental) {
            continue;
        }
        Accidental* newAccidental = oldAccidental->clone();
        newAccidental->setParent(this);
        m_accidentalsAboveAndBelow[i] = newAccidental;
    }
}

Ornament::~Ornament()
{
    std::fill(std::begin(m_notesAboveAndBelow), std::end(m_notesAboveAndBelow), nullptr);
    std::fill(std::begin(m_accidentalsAboveAndBelow), std::end(m_accidentalsAboveAndBelow), nullptr);

    if (m_cueNoteChord && m_cueNoteChord->notes().size()) {
        m_cueNoteChord->notes().clear();
    }

    m_cueNoteChord = nullptr;
}

void Ornament::remove(EngravingItem* e)
{
    if (e->isAccidental()) {
        for (Accidental*& acc : m_accidentalsAboveAndBelow) {
            if (e == acc) {
                acc = nullptr;
                e->removed();
            }
        }
    }
}

muse::TranslatableString Ornament::typeUserName() const
{
    if (textType() != ArticulationTextType::NO_TEXT) {
        return TranslatableString("engraving", "Ornament text");
    }

    return TranslatableString("engraving", "Ornament");
}

void Ornament::setTrack(track_idx_t val)
{
    for (Note* note : m_notesAboveAndBelow) {
        if (note) {
            note->setTrack(val);
        }
    }
    if (m_cueNoteChord) {
        m_cueNoteChord->setTrack(val);
    }
    m_track = val;
}

void Ornament::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    func(data, this);
    for (Accidental* accidental : m_accidentalsAboveAndBelow) {
        if (accidental) {
            func(data, accidental);
        }
    }
    if (m_cueNoteChord) {
        m_cueNoteChord->scanElements(data, func, all);
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
    case Pid::ORNAMENT_SHOW_CUE_NOTE:
        return m_showCueNote;
    case Pid::START_ON_UPPER_NOTE:
        return _startOnUpperNote;
    default:
        return Articulation::getProperty(propertyId);
    }
}

Sid Ornament::getPropertyStyle(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::ARTICULATION_ANCHOR:
        return Sid::articulationAnchorDefault;
    default:
        return Articulation::getPropertyStyle(propertyId);
    }
}

PropertyValue Ornament::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::INTERVAL_ABOVE:
    case Pid::INTERVAL_BELOW:
        return DEFAULT_ORNAMENT_INTERVAL;
    case Pid::ORNAMENT_SHOW_ACCIDENTAL:
        return OrnamentShowAccidental::DEFAULT;
    case Pid::ORNAMENT_SHOW_CUE_NOTE:
        return AutoOnOff::AUTO;
    case Pid::START_ON_UPPER_NOTE:
        return false;
    case Pid::ARTICULATION_ANCHOR:
        return static_cast<int>(ArticulationAnchor::AUTO);
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
    case Pid::ORNAMENT_SHOW_CUE_NOTE:
        setShowCueNote(v.value<AutoOnOff>());
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

bool Ornament::showCueNote()
{
    if (m_showCueNote == AutoOnOff::AUTO) {
        return (hasFullIntervalChoice() && style().styleB(Sid::trillAlwaysShowCueNote)) || _intervalAbove.step != IntervalStep::SECOND;
    }

    return m_showCueNote == AutoOnOff::ON;
}

void Ornament::computeNotesAboveAndBelow(AccidentalState* accState)
{
    Chord* parentChord = explicitParent() ? toChord(parent()) : nullptr;
    const Note* mainNote = parentChord ? parentChord->upNote() : nullptr;

    if (!mainNote) {
        return;
    }

    if (m_cueNoteChord && !m_cueNoteChord->explicitParent()) {
        m_cueNoteChord->setParent(toSegment(parentChord->segment()));
    }

    for (size_t i = 0; i < m_notesAboveAndBelow.size(); ++i) {
        bool above = (i == 0);
        bool hasIntAbove = hasIntervalAbove();
        bool hasIntBelow = hasIntervalBelow();

        if ((above && !hasIntAbove) || (!above && !hasIntBelow)) {
            continue;
        }

        Note*& note = m_notesAboveAndBelow.at(i);
        if (!note && above && m_cueNoteChord) {
            note = m_cueNoteChord->upNote();
        }

        if (!note) {
            note = mainNote->clone();
            Tie* tie = note->tieFor();
            if (tie) {
                score()->undoRemoveElement((EngravingItem*)tie);
            }
        } else {
            note->setTpc1(mainNote->tpc1());
            note->setTpc2(mainNote->tpc2());
            note->setPitch(mainNote->pitch());
        }
        note->setTrack(track());

        if (Accidental::isMicrotonal(note->accidentalType())) {
            // If mainNote has microtonal accidental, don't clone it to the ornament note because microtonal intervals are not supported.
            note->setAccidentalType(Accidental::value2subtype(tpc2alter(note->tpc())));
        }

        bool autoMode = (above && _intervalAbove.type == IntervalType::AUTO) || (!above && _intervalBelow.type == IntervalType::AUTO);
        if (autoMode) {
            // NOTE: In AUTO mode, the ornament note should match not only any alteration from the
            // key signature, but also any alteration present in the measure before this point.
            int intervalSteps = above ? static_cast<int>(_intervalAbove.step) : -static_cast<int>(_intervalBelow.step);
            note->transposeDiatonic(intervalSteps, false, true);
            if (m_trillOldCompatAccidental) {
                mapOldTrillAccidental(note, mainNote);
                m_trillOldCompatAccidental = nullptr;
            } else {
                int pitchLine = absStep(note->tpc(), note->epitch());
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

        AccidentalState copyOfAccState = *accState;
        note->updateAccidental(&copyOfAccState);
        if (note->accidental()) {
            int pitchLine = absStep(note->tpc(), note->epitch());
            accState->setForceRestateAccidental(pitchLine, true);
        }

        manageAccidentalVisibilityRules(note);
    }

    updateAccidentalsAboveAndBelow();
    updateCueNote();
}

void Ornament::manageAccidentalVisibilityRules(Note* note)
{
    if (_showAccidental == OrnamentShowAccidental::DEFAULT) {
        return;
    }

    AccidentalVal accidentalValue = tpc2alter(note->tpc());
    AccidentalType accidentalType = Accidental::value2subtype(accidentalValue);
    if (accidentalType == AccidentalType::NONE) {
        accidentalType = AccidentalType::NATURAL;
    }
    if (note->accidentalType() == accidentalType) {
        return;
    }

    bool show = false;
    if (_showAccidental == OrnamentShowAccidental::ALWAYS) {
        show = true;
    } else if (_showAccidental == OrnamentShowAccidental::ANY_ALTERATION) {
        Key key = staff()->key(tick());
        AccidentalState unalteredState;
        unalteredState.init(key);
        int pitchLine = absStep(note->tpc(), note->epitch() + note->ottaveCapoFret());
        AccidentalVal unalteredAccidentalVal = unalteredState.accidentalVal(pitchLine);
        show = accidentalValue != unalteredAccidentalVal;
    }

    if (show) {
        note->setAccidentalType(accidentalType);
        note->accidental()->setRole(AccidentalRole::AUTO);
    }
}

void Ornament::updateAccidentalsAboveAndBelow()
{
    for (size_t i = 0; i < m_notesAboveAndBelow.size(); ++i) {
        Note* note = m_notesAboveAndBelow[i];
        Accidental* accidental = note ? note->accidental() : nullptr;
        Accidental*& curAccidental = m_accidentalsAboveAndBelow[i];
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
        if (m_cueNoteChord) {
            m_cueNoteChord->notes().clear();
            delete m_cueNoteChord;
            m_cueNoteChord = nullptr;
        }
        return;
    }

    if (!explicitParent()) {
        return;
    }

    Chord* parentChord = toChord(parentItem());
    Note* cueNote = noteAbove();
    // If needed, create cue note
    if (!m_cueNoteChord) {
        m_cueNoteChord = Factory::createChord(parentChord->segment());
        m_cueNoteChord->setSmall(true);
        cueNote->setHeadHasParentheses(true);
        cueNote->setHeadType(NoteHeadType::HEAD_QUARTER);
        m_cueNoteChord->add(cueNote);
        cueNote->setParent(m_cueNoteChord);
    }
    m_cueNoteChord->setTrack(track());
    m_cueNoteChord->setParent(parentChord->segment());
    m_cueNoteChord->setStaffMove(parentChord->staffMove());
    cueNote->updateLine();
    cueNote->setIsTrillCueNote(true);
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

void Ornament::mapOldTrillAccidental(Note* note, const Note* mainNote)
{
    // Compatibility with trills pre-4.1
    AccidentalVal oldCompatValue = Accidental::subtype2value(m_trillOldCompatAccidental->accidentalType());
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
}
} // namespace mu::engraving
