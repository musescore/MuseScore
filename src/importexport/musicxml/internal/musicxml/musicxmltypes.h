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

#ifndef MUSICXMLTYPES_H
#define MUSICXMLTYPES_H
#include "dom/mscore.h"
#include "dom/arpeggio.h"
#include "dom/interval.h"

namespace mu::engraving {
//---------------------------------------------------------
//   MxmlStartStop
//---------------------------------------------------------

enum class MxmlStartStop : char {
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
//   VoiceDesc
//---------------------------------------------------------

/**
 The description of a single voice in a MusicXML part.
*/

class VoiceDesc
{
public:
    VoiceDesc();
    void incrChordRests(int s);
    int numberChordRests() const;
    int numberChordRests(int s) const { return (s >= 0 && s < MAX_STAVES) ? m_chordRests[s] : 0; }
    int preferredStaff() const;         // Determine preferred staff for this voice
    void setStaff(int s)
    {
        if (s >= 0) {
            m_staff = s;
        }
    }

    int staff() const { return m_staff; }
    void setVoice(int v)
    {
        if (v >= 0) {
            m_voice = v;
        }
    }

    int voice() const { return m_voice; }
    void setVoice(int s, int v)
    {
        if (s >= 0 && s < MAX_STAVES) {
            m_voices[s] = v;
        }
    }

    int voice(int s) const { return (s >= 0 && s < MAX_STAVES) ? m_voices[s] : -1; }
    void setOverlap(bool b) { m_overlaps = b; }
    bool overlaps() const { return m_overlaps; }
    void setStaffAlloc(int s, int i)
    {
        if (s >= 0 && s < MAX_STAVES) {
            m_staffAlloc[s] = i;
        }
    }

    int staffAlloc(int s) const { return (s >= 0 && s < MAX_STAVES) ? m_staffAlloc[s] : -1; }
    String toString() const;
private:
    int m_chordRests[MAX_STAVES];        // The number of chordrests on each MusicXML staff
    int m_staff;                         // The MuseScore staff allocated
    int m_voice;                         // The MuseScore voice allocated
    bool m_overlaps;                     // This voice contains active notes in multiple staves at the same time
    int m_staffAlloc[MAX_STAVES];        // For overlapping voices: voice is allocated on these staves (note: -2=unalloc -1=undef 1=alloc)
    int m_voices[MAX_STAVES];            // For every voice allocated on the staff, the voice number
};
typedef std::map<int, VoiceDesc> VoiceList;

//---------------------------------------------------------
//   MusicXMLInstrument
//---------------------------------------------------------

/**
 A single instrument in a MusicXML part.
 Used for both a drum part and a (non-drum) multi-instrument part
 */

struct MusicXMLInstrument {
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

    MusicXMLInstrument()        // required by std::map
        : unpitched(-1), name(), midiChannel(-1), midiPort(-1), midiProgram(-1), midiVolume(100), midiPan(63),
        notehead(NoteHeadGroup::HEAD_INVALID), line(0), stemDirection(DirectionV::AUTO) {}
    MusicXMLInstrument(String s)
        : unpitched(-1), name(s), midiChannel(-1), midiPort(-1), midiProgram(-1), midiVolume(100), midiPan(63),
        notehead(NoteHeadGroup::HEAD_NORMAL), line(0), stemDirection(DirectionV::AUTO) {}
    /*
    MusicXMLInstrument(int p, String s, NoteHead::Group nh, int l, Direction d)
          : unpitched(p), name(s), midiChannel(-1), midiPort(-1), midiProgram(-1), midiVolume(100), midiPan(63),
          notehead(nh), line(l), stemDirection(d) {}
     */
};
typedef std::map<String, MusicXMLInstrument> MusicXMLInstruments;

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

#endif // MUSICXMLTYPES_H
