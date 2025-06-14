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

#include "pitchspelling.h"
#include "../types/types.h"

#include <array>

namespace mu::engraving {
class XmlWriter;
class XmlReader;
class Articulation;

struct DrumInstrumentVariant {
    int pitch = INVALID_PITCH;
    TremoloType tremolo = TremoloType::INVALID_TREMOLO;
    String articulationName;

    bool operator==(const DrumInstrumentVariant& other) const
    {
        return pitch == other.pitch
               && tremolo == other.tremolo
               && articulationName == other.articulationName;
    }
};

//---------------------------------------------------------
//   DrumInstrument
//---------------------------------------------------------

struct DrumInstrument {
    String name;

    // if notehead = HEAD_CUSTOM, custom, use noteheads
    NoteHeadGroup notehead = NoteHeadGroup::HEAD_INVALID;   ///< notehead symbol set
    std::array<SymId, int(NoteHeadType::HEAD_TYPES)> noteheads
        = { SymId::noteheadWhole, SymId::noteheadHalf, SymId::noteheadBlack, SymId::noteheadDoubleWhole };

    int line = 0;               ///< place notehead onto this line
    DirectionV stemDirection = DirectionV::AUTO;

    int panelRow = -1;
    int panelColumn = -1;

    int voice = 0;
    String shortcut;
    std::list<DrumInstrumentVariant> variants;

    DrumInstrument() {}
    DrumInstrument(const String& n, NoteHeadGroup nh, int l, DirectionV d,
                   int pr = -1, int pc = -1, int v = 0, String sc = String())
        : name(n), notehead(nh), line(l), stemDirection(d), panelRow(pr), panelColumn(pc), voice(v), shortcut(sc) {}

    void addVariant(DrumInstrumentVariant v) { variants.push_back(v); }

    bool operator==(const DrumInstrument& other) const
    {
        return name == other.name
               && notehead == other.notehead
               && noteheads == other.noteheads
               && line == other.line
               && stemDirection == other.stemDirection
               && voice == other.voice
               && shortcut == other.shortcut
               && variants == other.variants
               && panelRow == other.panelRow
               && panelColumn == other.panelColumn;
    }
};

static const int DRUM_INSTRUMENTS = 128;

//---------------------------------------------------------
//   Drumset
//    defines noteheads and line position for all
//    possible midi notes in a drumset
//---------------------------------------------------------

class Drumset
{
public:
    bool isValid(int pitch) const;
    NoteHeadGroup noteHead(int pitch) const { return m_drums[pitch].notehead; }
    SymId noteHeads(int pitch, NoteHeadType t) const { return m_drums[pitch].noteheads[int(t)]; }
    int line(int pitch) const { return m_drums[pitch].line; }
    int voice(int pitch) const { return m_drums[pitch].voice; }
    DirectionV stemDirection(int pitch) const { return m_drums[pitch].stemDirection; }
    const String& name(int pitch) const { return m_drums[pitch].name; }
    String translatedName(int pitch) const;
    String shortcut(int pitch) const { return m_drums[pitch].shortcut; }
    std::list<DrumInstrumentVariant> variants(int pitch) const { return m_drums[pitch].variants; }
    int panelRow(int pitch) const { return m_drums[pitch].panelRow; }
    int panelColumn(int pitch) const { return m_drums[pitch].panelColumn; }

    size_t percussionPanelColumns() const { return m_percussionPanelColumns; }
    void setPercussionPanelColumns(size_t columns) { m_percussionPanelColumns = columns; }

    int pitchForShortcut(const String& shortcut) const;

    // defaultPitchForLine tries to find the pitch of a normal notehead at "line". If a normal notehead can't be found it will
    // instead return the "first valid pitch" (i.e. the lowest used midi note)
    int defaultPitchForLine(int line) const;

    void save(XmlWriter&) const;
    void loadDrum(XmlReader&);
    bool readDrumProperties(XmlReader&, int);
    void clear();
    int nextPitch(int) const;
    int prevPitch(int) const;
    DrumInstrument& drum(int i) { return m_drums[i]; }
    const DrumInstrument& drum(int i) const { return m_drums[i]; }
    void setDrum(int pitch, const DrumInstrument& di) { m_drums[pitch] = di; }
    DrumInstrumentVariant findVariant(int pitch, const std::vector<Articulation*>& articulations, TremoloType tremType) const;

    static void initDrumset();

    bool operator==(const Drumset& other) const
    {
        for (int pitch = 0; pitch < DRUM_INSTRUMENTS; ++pitch) {
            if (m_drums[pitch] == other.m_drums[pitch]) {
                continue;
            }
            return false;
        }
        return m_percussionPanelColumns == other.m_percussionPanelColumns;
    }

    bool operator!=(const Drumset& other) const
    {
        return !(*this == other);
    }

private:
    DrumInstrument m_drums[DRUM_INSTRUMENTS];
    size_t m_percussionPanelColumns = 8;
};

extern Drumset* smDrumset;
} // namespace mu::engraving
