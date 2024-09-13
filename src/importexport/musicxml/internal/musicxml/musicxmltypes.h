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
#include "engraving/dom/line.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/fret.h"

namespace mu::engraving {
//---------------------------------------------------------
//   MusicXmlPartGroup
//---------------------------------------------------------

struct MusicXmlPartGroup {
    int span = 0;
    int start = 0;
    BracketType type = BracketType::NO_BRACKET;
    bool barlineSpan = false;
    int column = 0;
};
typedef std::vector<MusicXmlPartGroup*> MusicXmlPartGroupList;
typedef std::map<String, Part*> PartMap;
typedef std::map<int, MusicXmlPartGroup*> MusicXmlPartGroupMap;

//---------------------------------------------------------
//   CreditWords
//    a single parsed MusicXML credit-words element
//---------------------------------------------------------

struct CreditWords {
    int page = 0;
    String type;
    double defaultX = 0.0;
    double defaultY = 0.0;
    double fontSize = 0.0;
    String justify;
    String hAlign;
    String vAlign;
    String words;
    CreditWords(int p, String tp, double dx, double dy, double fs, String j, String ha, String va, String w)
    {
        page = p;
        type = tp;
        defaultX = dx;
        defaultY = dy;
        fontSize = fs;
        justify  = j;
        hAlign   = ha;
        vAlign   = va;
        words    = w;
    }
};
typedef  std::vector<CreditWords*> CreditWordsList;

//---------------------------------------------------------
//   SlurDesc
//---------------------------------------------------------

/**
 The description of Slurs being handled
 */

class SlurDesc
{
public:
    enum class State : char {
        NONE, START, STOP
    };
    SlurDesc()
        : m_slur(0), m_state(State::NONE) {}
    Slur* slur() const { return m_slur; }
    void start(Slur* slur) { m_slur = slur; m_state = State::START; }
    void stop(Slur* slur) { m_slur = slur; m_state = State::STOP; }
    bool isStart() const { return m_state == State::START; }
    bool isStop() const { return m_state == State::STOP; }
private:
    Slur* m_slur = nullptr;
    State m_state;
};
typedef std::map<SLine*, std::pair<int, int> > MusicXmlSpannerMap;

//---------------------------------------------------------
//   NoteList
//---------------------------------------------------------

/**
 List of note start/stop times in a voice in a single staff.
*/

//---------------------------------------------------------
//   MxmlStartStop
//---------------------------------------------------------

enum class MxmlStartStop : char {
    NONE, START, STOP
};

typedef std::pair<int, int> StartStop;
typedef std::vector<StartStop> StartStopList;

//---------------------------------------------------------
//   NoteList
//---------------------------------------------------------

/**
 List of note start/stop times in a voice in all staves.
*/

class NoteList
{
public:
    NoteList();
    void addNote(const int startTick, const int endTick, const size_t staff);
    void dump(const int& voice) const;
    bool stavesOverlap(const int staff1, const int staff2) const;
    bool anyStaffOverlaps() const;
private:
    std::vector<StartStopList> _staffNoteLists;   // The note start/stop times in all staves
    bool notesOverlap(const StartStop& n1, const StartStop& n2) const;
};

struct MusicXmlArpeggioDesc {
    Arpeggio* arp;
    int no;

    MusicXmlArpeggioDesc(Arpeggio* arp, int no)
        : arp(arp), no(no) {}
};
typedef std::multimap<int, MusicXmlArpeggioDesc> ArpeggioMap;

/**
 The description of a chord symbol with or without a fret diagram
 */

struct HarmonyDesc
{
    track_idx_t m_track;
    bool fretDiagramVisible() const { return m_fretDiagram ? m_fretDiagram->visible() : false; }
    Harmony* m_harmony;
    FretDiagram* m_fretDiagram;

    HarmonyDesc(track_idx_t m_track, Harmony* m_harmony, FretDiagram* m_fretDiagram)
        : m_track(m_track), m_harmony(m_harmony),
        m_fretDiagram(m_fretDiagram) {}

    HarmonyDesc()
        : m_track(0), m_harmony(nullptr), m_fretDiagram(nullptr) {}
};
using HarmonyMap = std::multimap<int, HarmonyDesc>;

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

//---------------------------------------------------------
//   VoiceOverlapDetector
//---------------------------------------------------------

/**
 Detect overlap in a voice, which is when a voice has two or more notes
 active at the same time. In theory this should not happen, as voices
 only move forward in time, but Sibelius 7 reuses voice numbers in multi-
 staff parts, which leads to overlap.

 Current implementation does not detect voice overlap within a staff,
 but only between staves.
*/

class VoiceOverlapDetector
{
public:
    VoiceOverlapDetector();
    void addNote(const int startTick, const int endTick, const int& voice, const int staff);
    void dump() const;
    void newMeasure();
    bool stavesOverlap(const int& voice) const;
private:
    std::map<int, NoteList> _noteLists;   // The notelists for all the voices
};

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

struct InferredPercInstr {
    int pitch;
    track_idx_t track;
    String name;
    Fraction tick;

    InferredPercInstr(int pitch, track_idx_t track, String name, Fraction tick)
        : pitch(pitch), track(track), name(name), tick(tick) {}

    InferredPercInstr()
        : pitch(-1), track(muse::nidx), name(u""), tick(Fraction(0, -1)) {}
};
typedef std::vector<InferredPercInstr> InferredPercList;

typedef std::map<String, std::pair<String, DurationType> > MetronomeTextMap;

// Ties are identified by the pitch and track of their first note
typedef std::pair<int, track_idx_t> TieLocation;
}

#endif // MUSICXMLTYPES_H
