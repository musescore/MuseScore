//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: musicxmlsupport.cpp 5595 2012-04-29 15:30:32Z lvinken $
//
//  Copyright (C) 2012 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

/**
 MusicXML support.
 */

#include "globals.h"
#include "musicxmlsupport.h"

NoteList::NoteList()
      {
      for (int i = 0; i < MAX_STAVES; ++i)
            _staffNoteLists << StartStopList();
      }

void NoteList::addNote(const int startTick, const int endTick, const int staff)
      {
      if (staff >= 0 && staff < _staffNoteLists.size())
            _staffNoteLists[staff] << StartStop(startTick, endTick);
      }

void NoteList::dump(const int voice) const
      {
      // dump contents
      for (int i = 0; i < MAX_STAVES; ++i) {
            printf("voice %d staff %d:", voice, i);
            for (int j = 0; j < _staffNoteLists.at(i).size(); ++j)
                  printf(" %d-%d", _staffNoteLists.at(i).at(j).first, _staffNoteLists.at(i).at(j).second);
            printf("\n");
            }
      // show overlap
      printf("overlap voice %d:", voice);
      for (int i = 0; i < MAX_STAVES - 1; ++i)
            for (int j = i + 1; j < MAX_STAVES; ++j)
                  stavesOverlap(i, j);
      printf("\n");
      }

/**
 Determine if notes n1 and n2 overlap.
 This is NOT the case if
 - n1 starts when or after n2 stops
 - or n2 starts when or after n1 stops
 */

static bool notesOverlap(const StartStop& n1, const StartStop& n2)
      {
      return !(n1.first >= n2.second || n1.second <= n2.first);
      }

/**
 Determine if any note in staff1 and staff2 overlaps.
 */

bool NoteList::stavesOverlap(const int staff1, const int staff2) const
      {
      for (int i = 0; i < _staffNoteLists.at(staff1).size(); ++i)
            for (int j = 0; j < _staffNoteLists.at(staff2).size(); ++j)
                  if (notesOverlap(_staffNoteLists.at(staff1).at(i), _staffNoteLists.at(staff2).at(j))) {
                        // printf(" %d-%d", staff1, staff2);
                        return true;
                        }
      return false;
      }

/**
 Determine if any note in any staff overlaps.
 */

bool NoteList::anyStaffOverlaps() const
      {
      for (int i = 0; i < MAX_STAVES - 1; ++i)
            for (int j = i + 1; j < MAX_STAVES; ++j)
                  if (stavesOverlap(i, j))
                        return true;
      return false;
      }

VoiceOverlapDetector::VoiceOverlapDetector()
      {
      // printf("VoiceOverlapDetector::VoiceOverlapDetector(staves %d)\n", MAX_STAVES);
      }

void VoiceOverlapDetector::addNote(int startTick, int endTick, int voice, int staff)
      {
      // if necessary, create the note list for voice
      if (!_noteLists.contains(voice))
            _noteLists.insert(voice, NoteList());
      _noteLists[voice].addNote(startTick, endTick, staff);
      }

void VoiceOverlapDetector::dump() const
      {
      // printf("VoiceOverlapDetector::dump()\n");
      QMapIterator<int, NoteList> i(_noteLists);
      while (i.hasNext()) {
            i.next();
            i.value().dump(i.key());
            }
      }

void VoiceOverlapDetector::newMeasure()
      {
      // printf("VoiceOverlapDetector::newMeasure()\n");
      _noteLists.clear();
      }

bool VoiceOverlapDetector::stavesOverlap(const int voice) const
      {
      if (_noteLists.contains(voice))
            return _noteLists.value(voice).anyStaffOverlaps();
      else
            return false;
      }

QString MusicXMLDrumInstrument::toString() const
      {
      return QString("pitch %1 name %2 notehead %3 line %4 stemDirection %5")
             .arg(pitch)
             .arg(name)
             .arg(notehead)
             .arg(line)
             .arg(stemDirection);
      }
