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

#ifndef MU_ENGRAVING_TABLATURE_H
#define MU_ENGRAVING_TABLATURE_H

#include <cstddef>
#include <vector>
#include <map>

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
    bool        convertPitch(int pitch, Staff* staff, int* string, int* fret) const;
    int         fret(int pitch, int string, Staff* staff) const;
    void        fretChords(Chord* chord) const;
    int         getPitch(int string, int fret, Staff* staff) const;
    static int  pitchOffsetAt(Staff* staff);
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

    bool        convertPitch(int pitch, int pitchOffset, int* string, int* fret) const;
    int         fret(int pitch, int string, int pitchOffset) const;
    int         getPitch(int string, int fret, int pitchOffset) const;
    void        sortChordNotes(std::map<int, Note*>& sortedNotes, const Chord* chord, int pitchOffset, int* count) const;
    void        sortChordNotesUseSameString(const Chord* chord, int pitchOffset) const;

    //      std::vector<int>  stringTable { 40, 45, 50, 55, 59, 64 };   // guitar is default
    //      int         _frets = 19;
    std::vector<instrString> m_stringTable;                      // no strings by default
    int m_frets = 0;

    static bool bFretting;
    bool m_useFlats = false;
};
} // namespace mu::engraving
#endif
