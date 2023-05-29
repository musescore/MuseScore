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

#ifndef MU_ENGRAVING_ORNAMENT_H
#define MU_ENGRAVING_ORNAMENT_H

#include "articulation.h"

namespace mu::engraving {
class AccidentalState;

class Ornament final : public Articulation
{
    OBJECT_ALLOCATOR(engraving, Ornament)
    DECLARE_CLASSOF(ElementType::ORNAMENT)

public:
    Ornament(ChordRest* parent);
    Ornament* clone() const override { return new Ornament(*this); }
    Ornament(const Ornament& o);
    ~Ornament();

    static SymId fromTrillType(TrillType trillType);

    void draw(mu::draw::Painter* painter) const override;

    PropertyValue getProperty(Pid propertyId) const override;
    Sid getPropertyStyle(Pid propertyId) const override;
    PropertyValue propertyDefault(Pid id) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;

    bool hasIntervalAbove() const;
    bool hasIntervalBelow() const;
    bool hasFullIntervalChoice() const;
    bool showCueNote() { return _intervalAbove.step != IntervalStep::SECOND; }

    void computeNotesAboveAndBelow(AccidentalState* accState);

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    Shape shape() const override;

    void remove(EngravingItem* e) override;

    Chord* cueNoteChord() const { return _cueNoteChord; }
    void setCueNoteChord(Chord* c) { _cueNoteChord = c; }
    void setNoteAbove(Note* n) { _notesAboveAndBelow[0] = n; }

    const auto& accidentalsAboveAndBelow() const { return _accidentalsAboveAndBelow; }
    Accidental* accidentalAbove() const { return _accidentalsAboveAndBelow[0]; }
    Accidental* accidentalBelow() const { return _accidentalsAboveAndBelow[1]; }
    void setAccidentalAbove(Accidental* a) { _accidentalsAboveAndBelow[0] = a; }
    void setAccidentalBelow(Accidental* a) { _accidentalsAboveAndBelow[1] = a; }

    void setTrillOldCompatAccidental(Accidental* a) { _trillOldCompatAccidental = a; }

private:
    void updateAccidentalsAboveAndBelow();
    void updateCueNote();
    Note* noteAbove() const { return _notesAboveAndBelow[0]; }
    Note* noteBelow() const { return _notesAboveAndBelow[1]; }

private:
    M_PROPERTY(OrnamentInterval, intervalAbove, setIntervalAbove)
    M_PROPERTY(OrnamentInterval, intervalBelow, setIntervalBelow)
    M_PROPERTY(OrnamentShowAccidental, showAccidental, setShowAccidental)
    M_PROPERTY(bool, startOnUpperNote, setStartOnUpperNote)

    std::array<Note*, 2> _notesAboveAndBelow { nullptr, nullptr }; // [0] = above, [1] = below

    std::array<Accidental*, 2> _accidentalsAboveAndBelow { nullptr, nullptr }; // [0] = above, [1] = below

    Chord* _cueNoteChord = nullptr;

    Accidental* _trillOldCompatAccidental = nullptr; // used temporarily to map old (i.e. pre-4.1) trill accidentals
};
} // namespace mu::engraving
#endif // MU_ENGRAVING_ORNAMENT_H
