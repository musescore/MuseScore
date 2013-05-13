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

#include "libmscore/fraction.h"
#include "libmscore/mscore.h"
#include "libmscore/note.h"

namespace Ms {

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
//   VoiceDesc
//---------------------------------------------------------

/**
 The description of a single voice in a MusicXML part.
*/

class VoiceDesc {
public:
      VoiceDesc();
      void incrChordRests(int s);
      int numberChordRests() const;
      int numberChordRests(int s) const { return (s >= 0 && s < MAX_STAVES) ? _chordRests[s] : 0; }
      int preferredStaff() const;       ///< Determine preferred staff for this voice
      void setStaff(int s) { if (s >= 0) _staff = s; }
      int staff() const { return _staff; }
      void setVoice(int v) { if (v >= 0) _voice = v; }
      int voice() const { return _voice; }
      void setVoice(int s, int v) { if (s >= 0 && s < MAX_STAVES) _voices[s] = v; }
      int voice(int s) const { return (s >= 0 && s < MAX_STAVES) ? _voices[s] : -1; }
      void setOverlap(bool b) { _overlaps = b; }
      bool overlaps() const { return _overlaps; }
      void setStaffAlloc(int s, int i) { if (s >= 0 && s < MAX_STAVES) _staffAlloc[s] = i; }
      int staffAlloc(int s) const { return (s >= 0 && s < MAX_STAVES) ? _staffAlloc[s] : -1; }
      QString toString() const;
private:
      int _chordRests[MAX_STAVES];      ///< The number of chordrests on each MusicXML staff
      int _staff;                       ///< The MuseScore staff allocated
      int _voice;                       ///< The MuseScore voice allocated
      bool _overlaps;                   ///< This voice contains active notes in multiple staves at the same time
      int _staffAlloc[MAX_STAVES];      ///< For overlapping voices: voice is allocated on these staves (note: -2=unalloc -1=undef 1=alloc)
      int _voices[MAX_STAVES];          ///< For every voice allocated on the staff, the voice number
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
      Note::NoteHeadGroup notehead; ///< notehead symbol set
      int line;               ///< place notehead onto this line
      MScore::Direction stemDirection;

      QString toString() const;

      MusicXMLDrumInstrument()
            : pitch(-1), name(), notehead(Note::HEAD_INVALID), line(0), stemDirection(MScore::AUTO) {}
      MusicXMLDrumInstrument(QString s)
            : pitch(-1), name(s), notehead(Note::HEAD_INVALID), line(0), stemDirection(MScore::AUTO) {}
      MusicXMLDrumInstrument(int p, QString s, Note::NoteHeadGroup nh, int l, MScore::Direction d)
            : pitch(p), name(s), notehead(nh), line(l), stemDirection(d) {}
      };

/**
 A MusicXML drumset.
 */

typedef QMap<QString, MusicXMLDrumInstrument> MusicXMLDrumset;
typedef QMapIterator<QString, MusicXMLDrumInstrument> MusicXMLDrumsetIterator;


//---------------------------------------------------------
//   MxmlSupport -- MusicXML import support functions
//---------------------------------------------------------

class MxmlSupport {
public:
      static int stringToInt(const QString& s, bool* ok);
      static Fraction durationAsFraction(const int divisions, const QDomElement e);
      static Fraction noteTypeToFraction(QString type);
      static Fraction calculateFraction(QString type, int dots, int normalNotes, int actualNotes);
};

//---------------------------------------------------------
//   ValidatorMessageHandler
//---------------------------------------------------------

/**
 Message handler for the MusicXML schema validator QXmlSchemaValidator.
 */

class ValidatorMessageHandler : public QAbstractMessageHandler
      {
public:
      ValidatorMessageHandler() : QAbstractMessageHandler(0) {}
      QString getErrors() const { return errors; }
protected:
      virtual void handleMessage(QtMsgType type, const QString& description,
                                 const QUrl& identifier, const QSourceLocation& sourceLocation);
private:
      QString errors;
      };

extern void domError(const QDomElement&);
extern void domNotImplemented(const QDomElement&);


} // namespace Ms
#endif
