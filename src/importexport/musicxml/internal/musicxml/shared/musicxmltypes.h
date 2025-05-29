/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "types/types.h"
#include "types/string.h"
#include "dom/interval.h"

namespace mu::engraving {
class Arpeggio;
}

namespace mu::iex::musicxml {
const int MAX_NUMBER_LEVEL = 16; // maximum number of overlapping MusicXML objects

enum class MusicXmlExporterSoftware : char {
    DOLET6,
    DOLET8,
    DORICO,
    FINALE,
    NOTEFLIGHT,
    SIBELIUS,
    OTHER
};

//---------------------------------------------------------
//   MusicXmlStartStop
//---------------------------------------------------------

enum class MusicXmlStartStop : char {
    NONE, START, STOP
};

struct MusicXmlArpeggioDesc {
    engraving::Arpeggio* arp;
    int no;

    MusicXmlArpeggioDesc(engraving::Arpeggio* arp, int no)
        : arp(arp), no(no) {}
};
typedef std::multimap<int, MusicXmlArpeggioDesc> ArpeggioMap;

//---------------------------------------------------------
//   MusicXmlInstrument
//---------------------------------------------------------

/**
 A single instrument in a MusicXML part.
 Used for both a drum part and a (non-drum) multi-instrument part
 */

struct MusicXmlInstrument {
    int unpitched;                     // midi-unpitched read from MusicXML
    muse::String name;                       // instrument-name read from MusicXML
    muse::String sound;                      // instrument-sound read from MusicXML
    muse::String virtLib;                    // virtual-library read from MusicXML
    muse::String virtName;                   // virtual-name read from MusicXML
    int midiChannel;                   // midi-channel read from MusicXML
    int midiPort;                      // port read from MusicXML
    int midiProgram;                   // midi-program read from MusicXML
    int midiVolume;                    // volume read from MusicXML
    int midiPan;                       // pan value read from MusicXML
    engraving::NoteHeadGroup notehead;            // notehead symbol set
    int line = 0;                      // place notehead onto this line
    engraving::DirectionV stemDirection;

    muse::String toString() const;

    MusicXmlInstrument()        // required by std::map
        : unpitched(-1), name(), midiChannel(-1), midiPort(-1), midiProgram(-1), midiVolume(100), midiPan(63),
        notehead(engraving::NoteHeadGroup::HEAD_INVALID), line(0), stemDirection(engraving::DirectionV::AUTO) {}
    MusicXmlInstrument(muse::String s)
        : unpitched(-1), name(s), midiChannel(-1), midiPort(-1), midiProgram(-1), midiVolume(100), midiPan(63),
        notehead(engraving::NoteHeadGroup::HEAD_NORMAL), line(0), stemDirection(engraving::DirectionV::AUTO) {}
    /*
    MusicXmlInstrument(int p, String s, NoteHead::Group nh, int l, Direction d)
          : unpitched(p), name(s), midiChannel(-1), midiPort(-1), midiProgram(-1), midiVolume(100), midiPan(63),
          notehead(nh), line(l), stemDirection(d) {}
     */
};
typedef std::map<muse::String, MusicXmlInstrument> MusicXmlInstruments;

class MusicXmlIntervalList : public std::map<engraving::Fraction, engraving::Interval>
{
public:
    MusicXmlIntervalList() {}
    engraving::Interval interval(const engraving::Fraction f) const;
};

class MusicXmlInstrList : public std::map<engraving::Fraction, muse::String>
{
public:
    MusicXmlInstrList() {}
    const muse::String instrument(const engraving::Fraction f) const;
    void setInstrument(const muse::String instr, const engraving::Fraction f);
};

class LyricNumberHandler
{
public:
    LyricNumberHandler() {}
    void addNumber(const muse::String& number);
    muse::String toString() const;
    int getLyricNo(const muse::String& number) const;
    void determineLyricNos();
private:
    std::map<muse::String, int> m_numberToNo;
};
}
