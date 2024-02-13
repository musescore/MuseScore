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

#ifndef __MUSICXMLSUPPORT_H__
#define __MUSICXMLSUPPORT_H__

#include "engraving/types/fraction.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"

namespace mu {
class XmlStreamReader;
}

namespace mu::engraving {
class Chord;

//---------------------------------------------------------
//   NoteList
//---------------------------------------------------------

/**
 List of note start/stop times in a voice in a single staff.
*/

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
};

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

/**
 A MusicXML drumset or set of instruments in a multi-instrument part.
 */

typedef std::map<String, MusicXMLInstrument> MusicXMLInstruments;

//---------------------------------------------------------
//   MxmlSupport -- MusicXML import support functions
//---------------------------------------------------------

class MxmlSupport
{
public:
    static int stringToInt(const String& s, bool* ok);
    static Fraction noteTypeToFraction(const String& type);
    static Fraction calculateFraction(const String& type, int dots, int normalNotes, int actualNotes);
};

extern String accSymId2MxmlString(const SymId id);
extern String accSymId2SmuflMxmlString(const SymId id);
extern String accidentalType2MxmlString(const AccidentalType type);
extern String accidentalType2SmuflMxmlString(const AccidentalType type);
extern AccidentalType mxmlString2accidentalType(const String mxmlName, const String smufl);
extern SymId mxmlString2accSymId(const String mxmlName, const String smufl = {});
extern AccidentalType microtonalGuess(double val);
extern bool isLaissezVibrer(const SymId id);
extern const Articulation* findLaissezVibrer(const Chord* chord);
extern String errorStringWithLocation(int line, int col, const String& error);
extern String checkAtEndElement(const XmlStreamReader& e, const String& expName);
} // namespace Ms
#endif
