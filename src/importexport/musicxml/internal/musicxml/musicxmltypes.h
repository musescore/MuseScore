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
#include "dom/arpeggio.h"
#include "dom/interval.h"

namespace mu::engraving {
const int MAX_NUMBER_LEVEL = 16; // maximum number of overlapping MusicXML objects

enum class MusicXmlExporterSoftware : char {
    SIBELIUS,
    DOLET6,
    DOLET8,
    FINALE,
    NOTEFLIGHT,
    OTHER
};

//---------------------------------------------------------
//   MusicXmlStartStop
//---------------------------------------------------------

enum class MusicXmlStartStop : char {
    NONE, START, STOP
};

struct MusicXmlArpeggioDesc {
    Arpeggio* arp;
    int no;

    MusicXmlArpeggioDesc(Arpeggio* arp, int no)
        : arp(arp), no(no) {}
};
typedef std::multimap<int, MusicXmlArpeggioDesc> ArpeggioMap;

//---------------------------------------------------------
//   MusicXmlInstrument
//---------------------------------------------------------

/**
 A single instrument in a MusicXml part.
 Used for both a drum part and a (non-drum) multi-instrument part
 */

struct MusicXmlInstrument {
    int unpitched;                     // midi-unpitched read from MusicXML
    String name;                       // instrument-name read from MusicXML
    String sound;                      // instrument-sound read from MusicXML
    String virtLib;                    // virtual-library read from MusicXML
    String virtName;                   // virtual-name read from MusicXML
    int midiChannel;                   // midi-channel read from MusicXML
    int midiPort;                      // port read from MusicXML
    int midiProgram;                   // midi-program read from MusicXML
    int midiVolume;                    // volume read from MusicXML
    int midiPan;                       // pan value read from MusicXML
    NoteHeadGroup notehead;            // notehead symbol set
    int line = 0;                      // place notehead onto this line
    DirectionV stemDirection;

    String toString() const;

    MusicXmlInstrument()        // required by std::map
        : unpitched(-1), name(), midiChannel(-1), midiPort(-1), midiProgram(-1), midiVolume(100), midiPan(63),
        notehead(NoteHeadGroup::HEAD_INVALID), line(0), stemDirection(DirectionV::AUTO) {}
    MusicXmlInstrument(String s)
        : unpitched(-1), name(s), midiChannel(-1), midiPort(-1), midiProgram(-1), midiVolume(100), midiPan(63),
        notehead(NoteHeadGroup::HEAD_NORMAL), line(0), stemDirection(DirectionV::AUTO) {}
    /*
    MusicXmlInstrument(int p, String s, NoteHead::Group nh, int l, Direction d)
          : unpitched(p), name(s), midiChannel(-1), midiPort(-1), midiProgram(-1), midiVolume(100), midiPan(63),
          notehead(nh), line(l), stemDirection(d) {}
     */
};
typedef std::map<String, MusicXmlInstrument> MusicXmlInstruments;

class MusicXmlIntervalList : public std::map<Fraction, Interval>
{
public:
    MusicXmlIntervalList() {}
    Interval interval(const Fraction f) const;
};

class MusicXmlInstrList : public std::map<Fraction, String>
{
public:
    MusicXmlInstrList() {}
    const String instrument(const Fraction f) const;
    void setInstrument(const String instr, const Fraction f);
};

class LyricNumberHandler
{
public:
    LyricNumberHandler() {}
    void addNumber(const String& number);
    String toString() const;
    int getLyricNo(const String& number) const;
    void determineLyricNos();
private:
    std::map<String, int> m_numberToNo;
};
}
