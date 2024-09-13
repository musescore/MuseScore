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

#include "musicxmltypes.h"

using namespace mu::engraving;

NoteList::NoteList()
{
    _staffNoteLists.reserve(MAX_STAVES);
    for (int i = 0; i < MAX_STAVES; ++i) {
        _staffNoteLists.push_back(StartStopList());
    }
}

void NoteList::addNote(const int startTick, const int endTick, const size_t staff)
{
    if (staff < _staffNoteLists.size()) {
        _staffNoteLists[staff].push_back(StartStop(startTick, endTick));
    }
}

void NoteList::dump(const int& voice) const
{
    // dump contents
    for (int i = 0; i < MAX_STAVES; ++i) {
        printf("voice %d staff %d:", voice, i);
        for (size_t j = 0; j < _staffNoteLists.at(i).size(); ++j) {
            printf(" %d-%d", _staffNoteLists.at(i).at(j).first, _staffNoteLists.at(i).at(j).second);
        }
        printf("\n");
    }
    // show overlap
    printf("overlap voice %d:", voice);
    for (int i = 0; i < MAX_STAVES - 1; ++i) {
        for (int j = i + 1; j < MAX_STAVES; ++j) {
            stavesOverlap(i, j);
        }
    }
    printf("\n");
}

/**
 Determine if notes n1 and n2 overlap.
 This is NOT the case if
 - n1 starts when or after n2 stops
 - or n2 starts when or after n1 stops
 */

bool NoteList::notesOverlap(const StartStop& n1, const StartStop& n2) const
{
    return !(n1.first >= n2.second || n1.second <= n2.first);
}

/**
 Determine if any note in staff1 and staff2 overlaps.
 */

bool NoteList::stavesOverlap(const int staff1, const int staff2) const
{
    for (size_t i = 0; i < _staffNoteLists.at(staff1).size(); ++i) {
        for (size_t j = 0; j < _staffNoteLists.at(staff2).size(); ++j) {
            if (notesOverlap(_staffNoteLists.at(staff1).at(i), _staffNoteLists.at(staff2).at(j))) {
                //printf(" %d-%d", staff1, staff2);
                return true;
            }
        }
    }
    return false;
}

/**
 Determine if any note in any staff overlaps.
 */

bool NoteList::anyStaffOverlaps() const
{
    for (int i = 0; i < MAX_STAVES - 1; ++i) {
        for (int j = i + 1; j < MAX_STAVES; ++j) {
            if (stavesOverlap(i, j)) {
                return true;
            }
        }
    }
    return false;
}

VoiceOverlapDetector::VoiceOverlapDetector()
{
    // LOGD("VoiceOverlapDetector::VoiceOverlapDetector(staves %d)", MAX_STAVES);
}

void VoiceOverlapDetector::addNote(const int startTick, const int endTick, const int& voice, const int staff)
{
    // if necessary, create the note list for voice
    if (!muse::contains(_noteLists, voice)) {
        _noteLists.insert({ voice, NoteList() });
    }
    _noteLists[voice].addNote(startTick, endTick, staff);
}

void VoiceOverlapDetector::dump() const
{
    // LOGD("VoiceOverlapDetector::dump()");
    for (auto& p : _noteLists) {
        p.second.dump(p.first);
    }
}

void VoiceOverlapDetector::newMeasure()
{
    // LOGD("VoiceOverlapDetector::newMeasure()");
    _noteLists.clear();
}

bool VoiceOverlapDetector::stavesOverlap(const int& voice) const
{
    if (muse::contains(_noteLists, voice)) {
        return _noteLists.at(voice).anyStaffOverlaps();
    } else {
        return false;
    }
}

String MusicXMLInstrument::toString() const
{
    return String(u"chan %1 prog %2 vol %3 pan %4 unpitched %5 name '%6' sound '%7' head %8 line %9 stemDir %10")
           .arg(midiChannel)
           .arg(midiProgram)
           .arg(midiVolume)
           .arg(midiPan)
           .arg(unpitched)
           .arg(name, sound)
           .arg(int(notehead))
           .arg(line)
           .arg(int(stemDirection));
}

//---------------------------------------------------------
//   VoiceDesc
//---------------------------------------------------------

VoiceDesc::VoiceDesc()
    : m_staff(-1), m_voice(-1), m_overlaps(false)
{
    for (int i = 0; i < MAX_STAVES; ++i) {
        m_chordRests[i] =  0;
        m_staffAlloc[i] = -1;
        m_voices[i]     = -1;
    }
}

void VoiceDesc::incrChordRests(int s)
{
    if (0 <= s && s < MAX_STAVES) {
        m_chordRests[s]++;
    }
}

int VoiceDesc::numberChordRests() const
{
    int res = 0;
    for (int i = 0; i < MAX_STAVES; ++i) {
        res += m_chordRests[i];
    }
    return res;
}

int VoiceDesc::preferredStaff() const
{
    int max = 0;
    int res = 0;
    for (int i = 0; i < MAX_STAVES; ++i) {
        if (m_chordRests[i] > max) {
            max = m_chordRests[i];
            res = i;
        }
    }
    return res;
}

String VoiceDesc::toString() const
{
    String res = u"[";
    for (int i = 0; i < MAX_STAVES; ++i) {
        res += String(u" %1").arg(m_chordRests[i]);
    }
    res += String(u" ] overlaps %1").arg(m_overlaps);
    if (m_overlaps) {
        res += u" staffAlloc [";
        for (int i = 0; i < MAX_STAVES; ++i) {
            res += String(u" %1").arg(m_staffAlloc[i]);
        }
        res += u" ] voices [";
        for (int i = 0; i < MAX_STAVES; ++i) {
            res += String(u" %1").arg(m_voices[i]);
        }
        res += u" ]";
    } else {
        res += String(u" staff %1 voice %2").arg(m_staff + 1).arg(m_voice + 1);
    }
    return res;
}
