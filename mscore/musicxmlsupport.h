//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: musicxmlsupport.h 5595 2012-04-29 15:30:32Z lvinken $
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

#ifndef __MUSICXMLSUPPORT_H__
#define __MUSICXMLSUPPORT_H__

#include "libmscore/mscore.h"

//---------------------------------------------------------
//   NoteList
//---------------------------------------------------------

/**
 List of note start/stop times in a voice in a single staff.
*/

typedef QPair<int, int> StartStop;
typedef QList<StartStop> StartStopList;

//---------------------------------------------------------
//   NoteList
//---------------------------------------------------------

/**
 List of note start/stop times in a voice in all staves.
*/

class NoteList {
   public:
      NoteList();
      void addNote(const int startTick, const int endTick, const int staff);
      void dump(const int voice) const;
      bool stavesOverlap(const int staff1, const int staff2) const;
      bool anyStaffOverlaps() const;
   private:
      QList<StartStopList> _staffNoteLists; ///< The note start/stop times in all staves
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

class VoiceOverlapDetector {
   public:
      VoiceOverlapDetector();
      void addNote(const int startTick, const int endTick, const int voice, const int staff);
      void dump() const;
      void newMeasure();
      bool stavesOverlap(const int voice) const;
   private:
      QMap<int, NoteList> _noteLists; ///< The notelists for all the voices
      };

//---------------------------------------------------------
//   MusicXMLDrumInstrument
//---------------------------------------------------------

/**
 A single instrument in a MusicXML drumset.
 */

struct MusicXMLDrumInstrument {
      int pitch;
      QString name;
      NoteHeadGroup notehead; ///< notehead symbol set
      int line;               ///< place notehead onto this line
      Direction stemDirection;

      QString toString() const;

      MusicXMLDrumInstrument()
         : pitch(-1), name(), notehead(HEAD_INVALID), line(0), stemDirection(AUTO) {}
      MusicXMLDrumInstrument(QString s)
         : pitch(-1), name(s), notehead(HEAD_INVALID), line(0), stemDirection(AUTO) {}
      MusicXMLDrumInstrument(int p, QString s, NoteHeadGroup nh, int l, Direction d)
         : pitch(p), name(s), notehead(nh), line(l), stemDirection(d) {}
      };

/**
 A MusicXML drumset.
 */

typedef QMap<QString, MusicXMLDrumInstrument> MusicXMLDrumset;
typedef QMapIterator<QString, MusicXMLDrumInstrument> MusicXMLDrumsetIterator;

#endif
