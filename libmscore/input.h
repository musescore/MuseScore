//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __INPUT_H__
#define __INPUT_H__

#include "mscore.h"
#include "durationtype.h"

namespace Ms {

class Element;
class Slur;
class ChordRest;
class Drumset;
class Segment;
class Score;

//---------------------------------------------------------
//   InputState
//---------------------------------------------------------

class InputState {
      Score*      _score;
      TDuration   _duration = TDuration::DurationType::V_INVALID;        // currently duration
      int         _drumNote = -1;
#if 0 // yet(?) unused
      Drumset*    _drumset = 0;
#endif
      int         _track = 0;
      Segment*    _lastSegment = 0;
      Segment*    _segment = 0;         // current segment
      int         _string = VISUAL_STRING_NONE;          // visual string selected for input (TAB staves only)
      bool        _repitchMode = false;

      bool _rest = false;              // rest mode
      NoteType _noteType = NoteType::NORMAL;
      BeamMode _beamMode = BeamMode::AUTO;
      bool _noteEntryMode = false;
      Slur* _slur = 0;

      Segment* nextInputPos() const;

   public:
      InputState(Score* s) : _score(s) {}

      ChordRest* cr() const;

      int tick() const;

      void setDuration(const TDuration& d) { _duration = d;          }
      TDuration duration() const           { return _duration;       }
      void setDots(int n)                  { _duration.setDots(n);   }
      int ticks() const                    { return _duration.ticks(); }

      Segment* segment() const            { return _segment;        }
      void setSegment(Segment* s);

      Segment* lastSegment() const        { return _lastSegment;        }
      void setLastSegment(Segment* s)     { _lastSegment = s;           }

      Drumset* drumset() const;

      int drumNote() const                { return _drumNote;       }
      void setDrumNote(int v)             { _drumNote = v;          }

      int voice() const                   { return _track % VOICES; }
      int track() const                   { return _track;          }
      void setTrack(int v)                { _track = v;             }

      int string() const                  { return _string;             }
      void setString(int val)             { _string = val;              }

      bool repitchMode() const            { return _repitchMode;    }
      void setRepitchMode(bool val)       { _repitchMode = val;     }

      StaffGroup staffGroup() const;

      bool rest() const                   { return _rest; }
      void setRest(bool v)                { _rest = v; }

      NoteType noteType() const           { return _noteType; }
      void setNoteType(NoteType t)        { _noteType = t; }

      BeamMode beamMode() const           { return _beamMode; }
      void setBeamMode(BeamMode m)        { _beamMode = m; }

      bool noteEntryMode() const          { return _noteEntryMode; }
      void setNoteEntryMode(bool v)       { _noteEntryMode = v; }

      Slur* slur() const                  { return _slur; }
      void setSlur(Slur* s)               { _slur = s; }

      void update(Element* e);
      void moveInputPos(Element* e);
      void moveToNextInputPos();
      bool endOfScore() const;
      };


}     // namespace Ms
#endif

