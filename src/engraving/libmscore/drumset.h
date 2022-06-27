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

#ifndef __DRUMSET_H__
#define __DRUMSET_H__

#include "mscore.h"
#include "tremolo.h"
#include "note.h"

namespace mu::engraving {
class XmlWriter;

struct DrumInstrumentVariant {
    int pitch = INVALID_PITCH;
    TremoloType tremolo = TremoloType::INVALID_TREMOLO;
    String articulationName;
};

//---------------------------------------------------------
//   DrumInstrument
//---------------------------------------------------------

struct DrumInstrument {
    String name;

    // if notehead = HEAD_CUSTOM, custom, use noteheads
    NoteHeadGroup notehead = NoteHeadGroup::HEAD_INVALID;   ///< notehead symbol set
    SymId noteheads[int(NoteHeadType::HEAD_TYPES)]
        = { SymId::noteheadWhole, SymId::noteheadHalf, SymId::noteheadBlack, SymId::noteheadDoubleWhole };

    int line = 0;               ///< place notehead onto this line
    DirectionV stemDirection = DirectionV::AUTO;
    int voice = 0;
    char shortcut = '\0';      ///< accelerator key (CDEFGAB)
    std::list<DrumInstrumentVariant> variants;

    DrumInstrument() {}
    DrumInstrument(const char* s, NoteHeadGroup nh, int l, DirectionV d,
                   int v = 0, char sc = 0)
        : name(s), notehead(nh), line(l), stemDirection(d), voice(v), shortcut(sc) {}

    void addVariant(DrumInstrumentVariant v) { variants.push_back(v); }
};

static const int DRUM_INSTRUMENTS = 128;

//---------------------------------------------------------
//   Drumset
//    defines noteheads and line position for all
//    possible midi notes in a drumset
//---------------------------------------------------------

class Drumset
{
    DrumInstrument _drum[DRUM_INSTRUMENTS];

public:
    bool isValid(int pitch) const { return !_drum[pitch].name.empty(); }
    NoteHeadGroup noteHead(int pitch) const { return _drum[pitch].notehead; }
    SymId noteHeads(int pitch, NoteHeadType t) const { return _drum[pitch].noteheads[int(t)]; }
    int line(int pitch) const { return _drum[pitch].line; }
    int voice(int pitch) const { return _drum[pitch].voice; }
    DirectionV stemDirection(int pitch) const { return _drum[pitch].stemDirection; }
    const String& name(int pitch) const { return _drum[pitch].name; }
    int shortcut(int pitch) const { return _drum[pitch].shortcut; }
    std::list<DrumInstrumentVariant> variants(int pitch) const { return _drum[pitch].variants; }

    void save(XmlWriter&) const;
    void load(XmlReader&);
    bool readProperties(XmlReader&, int);
    void clear();
    int nextPitch(int) const;
    int prevPitch(int) const;
    DrumInstrument& drum(int i) { return _drum[i]; }
    const DrumInstrument& drum(int i) const { return _drum[i]; }
    DrumInstrumentVariant findVariant(int pitch, const std::vector<Articulation*> articulations, Tremolo* tremolo) const;
};

extern Drumset* smDrumset;
extern void initDrumset();
} // namespace mu::engraving
#endif
