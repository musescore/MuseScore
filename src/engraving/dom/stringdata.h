/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <cstddef>
#include <cstdint>
#include <vector>
#include <map>
#include <utility> 
#include <tuple> 

#include "../types/fraction.h"
#include "../types/types.h"

namespace mu::engraving {
class Chord;
class Note;

class Staff;

//---------------------------------------------------------
//   StringData
//---------------------------------------------------------

// defines the string of an instrument
struct instrString {
    instrString(int p = 0, bool o = false, int s = 0)
        : pitch(p), open(o), startFret(s) {}

    int pitch = 0;          // the pitch of the string
    bool open = false;          // true: string is open | false: string is fretted
    int startFret = 0;      // banjo 5th string starts on 5th fret
    bool useFlat = false;

    bool operator==(const instrString& d) const { return d.pitch == pitch && d.open == open; }
};

class StringData
{
public:
    StringData() {}
    StringData(int numFrets, int numStrings, int strings[], bool useFlats = false);
    StringData(int numFrets, std::vector<instrString>& strings);

    bool isNull() const;

    void        set(const StringData& src);
    bool        convertPitch(int pitch, int pitchOffset, int* string, int* fret, const CapoParams& capo = {}) const;
    bool        convertPitch(int pitch, const Staff* staff, int* string, int* fret) const;
    bool        convertPitch(int pitch, const Staff* staff, const Fraction& tick, int* string, int* fret) const;
    int         fret(int pitch, int string, const Staff* staff) const;
    int         fret(int pitch, int string, const Staff* staff, const Fraction& tick) const;
    void        fretChords(Chord* chord) const;
    int         getPitch(int string, int fret, int pitchOffset) const;
    int         getPitch(int string, int fret, const Staff* staff) const;
    int         getPitch(int string, int fret, const Staff* staff, const Fraction& tick) const;
    static int  pitchOffsetAt(const Staff* staff);
    static int  pitchOffsetAt(const Staff* staff, const Fraction& tick);
    static int  pitchOffsetAt(const Staff* staff, const Fraction& tick, int string);
    size_t      strings() const { return m_stringTable.size(); }
    int         frettedStrings() const;
    const std::vector<instrString>& stringList() const { return m_stringTable; }
    std::vector<instrString>& stringList() { return m_stringTable; }
    int         frets() const { return m_frets; }
    void        setFrets(int val) { m_frets = val; }
    bool operator==(const StringData& d) const { return d.m_frets == m_frets && d.m_stringTable == m_stringTable; }
    void        configBanjo5thString();
    int         adjustBanjo5thFret(int fret) const;
    bool        isFiveStringBanjo() const;
    bool        useFlats() const { return m_useFlats; }

private:
    using FrettingCacheKey = std::tuple<int, const Staff*, int64_t>;
    mutable std::map<FrettingCacheKey, std::vector<std::pair<int, int>>> m_candidateFrettingCache;

    int         fret(int pitch, int string, int pitchOffset) const;
    int         scoreFrettingCandidate(const std::pair<int, int>& anchor, const std::pair<int, int>& candidate) const;
    int         resolveForcedString(const Note* note) const; 
    void        assignRemainingNotesAroundBass(const Chord* chord, Note* bassNote, const std::pair<int, int>& bassFretting) const;
    void        sortChordNotes(std::map<int, Note*>& sortedNotes, const Chord* chord, int* count) const;
    void        sortChordNotesUseSameString(const Chord* chord) const;
    bool        hasPendingPitchChange(const Chord* chord) const;
    void        updateFretsOnSameStrings(const Chord* chord) const;
    void        preferBassStringForNegativeFret(const Chord* chord) const;
    void        reassignNegativeFretNotes(const Chord* chord) const;
    void        assignBestFrettingForBassNote(std::pair<int, int> bestFretting, Note* desiredBassNote, Chord* chord) const; 
    bool        tryResolveStringConflictWithOutOfRangeFret(const Note* note, int numStrings, std::vector<int>& bUsed, int& nNewString,
                                                           int& nNewFret) const;
    bool        stringSupportsGlissando(const Note* note, int candidateString) const; 
    
    Note*       glissandoFrom(const Note* note) const; 
    Note*       glissandoTo(const Note* note) const; 
    Note*       getBassNote(const Chord* chord) const;
    Note*       getBassNoteOfVoicings(const Chord* chord) const;
    std::vector<std::pair<int, int>> allCandidateFrettings(int pitch, const Staff* staff, const Fraction& tick) const;
    std::vector<Note*> collectNotesAtSameTick(const Chord* chord) const;
    std::pair<Note*, std::pair<int, int>> getBestFrettingForBassNote(const std::pair<int, int>& prevFretting, Chord* chord) const; 
    std::pair<int, int> defaultFretboardAnchor() const; 

    std::vector<instrString> m_stringTable;                      // no strings by default

    mutable std::pair<int, int> m_lastNonOpenFretting = { 0, 7 }; // this allows us to deal with open chords more efficiently 
    
    static constexpr int DEFAULT_ANCHOR_FRET = 7; 
    // approximate middle of the fretboard for most fretted instruments 
    // this is technically a magic number, however it functions as an incredibly powerful heuristic

    int m_frets = 0;

    static bool bFretting;
    bool m_useFlats = false;
};
}
