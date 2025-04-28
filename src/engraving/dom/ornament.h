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
#pragma once

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

    TranslatableString typeUserName() const override;

    static SymId fromTrillType(TrillType trillType);

    PropertyValue getProperty(Pid propertyId) const override;
    Sid getPropertyStyle(Pid propertyId) const override;
    PropertyValue propertyDefault(Pid id) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;

    bool hasIntervalAbove() const;
    bool hasIntervalBelow() const;
    bool hasFullIntervalChoice() const;
    bool showCueNote();

    void computeNotesAboveAndBelow(AccidentalState* accState);

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    void remove(EngravingItem* e) override;

    Chord* cueNoteChord() const { return m_cueNoteChord; }
    void setCueNoteChord(Chord* c) { m_cueNoteChord = c; }
    void setNoteAbove(Note* n) { m_notesAboveAndBelow[0] = n; }

    const auto& accidentalsAboveAndBelow() const { return m_accidentalsAboveAndBelow; }
    Accidental* accidentalAbove() const { return m_accidentalsAboveAndBelow[0]; }
    Accidental* accidentalBelow() const { return m_accidentalsAboveAndBelow[1]; }
    void setAccidentalAbove(Accidental* a) { m_accidentalsAboveAndBelow[0] = a; }
    void setAccidentalBelow(Accidental* a) { m_accidentalsAboveAndBelow[1] = a; }

    void setTrillOldCompatAccidental(Accidental* a) { m_trillOldCompatAccidental = a; }

    Note* noteAbove() const { return m_notesAboveAndBelow[0]; }
    Note* noteBelow() const { return m_notesAboveAndBelow[1]; }

    void setTrack(track_idx_t val) override;

    void setShowCueNote(AutoOnOff show) { m_showCueNote = show; }

private:
    void updateAccidentalsAboveAndBelow();
    void updateCueNote();
    void mapOldTrillAccidental(Note* note, const Note* mainNote);
    void manageAccidentalVisibilityRules(Note* note);

private:
    M_PROPERTY(OrnamentInterval, intervalAbove, setIntervalAbove)
    M_PROPERTY(OrnamentInterval, intervalBelow, setIntervalBelow)
    M_PROPERTY(OrnamentShowAccidental, showAccidental, setShowAccidental)
    M_PROPERTY(bool, startOnUpperNote, setStartOnUpperNote)

    std::array<Note*, 2> m_notesAboveAndBelow { nullptr, nullptr }; // [0] = above, [1] = below

    std::array<Accidental*, 2> m_accidentalsAboveAndBelow { nullptr, nullptr }; // [0] = above, [1] = below

    Chord* m_cueNoteChord = nullptr;
    AutoOnOff m_showCueNote = AutoOnOff::AUTO;

    Accidental* m_trillOldCompatAccidental = nullptr; // used temporarily to map old (i.e. pre-4.1) trill accidentals
};
}
