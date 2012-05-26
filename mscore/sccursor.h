//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: sccursor.h 2847 2010-03-06 16:52:48Z wschweer $
//
//  Copyright (C) 2009 Werner Schweer and others
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

#ifndef __SCCURSOR_H__
#define __SCCURSOR_H__

class Score;
class Chord;
class Rest;
class Segment;
class RepeatSegment;
class SCursor;
class ChordRest;
class Text;
class Measure;

typedef Measure* MeasurePtr;
typedef Text* TextPtr;

class Measure;
typedef Measure* MeasurePtr;

//---------------------------------------------------------
//   SCursor
//---------------------------------------------------------

class SCursor {
      Score* _score;
      int _staffIdx;
      int _voice;
      bool _expandRepeat;

      //state
      Segment* _segment;
      RepeatSegment* _curRepeatSegment;
      int _curRepeatSegmentIndex;

   public:
      SCursor(Score*);
      SCursor(Score*, bool);
      SCursor() {}
      int staffIdx() const                    { return _staffIdx; }
      int voice() const                       { return _voice;    }
      void setStaffIdx(int v)                 { _staffIdx = v;    }
      void setVoice(int v)                    { _voice = v;       }
      Segment* segment() const                { return _segment;  }
      void setSegment(Segment* s)             { _segment = s;     }
      RepeatSegment* repeatSegment() const    { return _curRepeatSegment;  }
      void setRepeatSegment(RepeatSegment* s) { _curRepeatSegment = s;     }
      int repeatSegmentIndex()                { return _curRepeatSegmentIndex; }
      void setRepeatSegmentIndex(int idx)     { _curRepeatSegmentIndex = idx; }
      bool expandRepeat()                     { return _expandRepeat; }
      Score* score() const                    { return _score;    }
      ChordRest* cr() const;
      void rewind(int);
      bool next();
      bool nextMeasure();
      void putStaffText(Text* s);
      void add(ChordRest* c);
      int tick();
      double time();
      };

#endif

