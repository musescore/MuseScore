//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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

#ifndef __CURSOR_H__
#define __CURSOR_H__

class Element;
class Score;
class Chord;
class Rest;
class Segment;
class RepeatSegment;
class ChordRest;
class Text;
class Measure;

//---------------------------------------------------------
//   Cursor
//---------------------------------------------------------

class Cursor : public QObject {
      Q_OBJECT
      Q_PROPERTY(int staffIdx READ staffIdx WRITE setStaffIdx)
      Q_PROPERTY(int voice READ voice WRITE setVoice)

      Score* _score;
      int _staffIdx;
      int _voice;
      bool _expandRepeat;

      //state
      Segment* _segment;
      RepeatSegment* _curRepeatSegment;
      int _curRepeatSegmentIndex;

   public:
      Cursor(Score* c = 0);
      Cursor(Score*, bool);
      int staffIdx() const                    { return _staffIdx; }
      void setStaffIdx(int v)                 { _staffIdx = v;    }
      int voice() const                       { return _voice;    }
      void setVoice(int v)                    { _voice = v;       }
      Segment* segment() const                { return _segment;  }
      void setSegment(Segment* s)             { _segment = s;     }
      RepeatSegment* repeatSegment() const    { return _curRepeatSegment;  }
      void setRepeatSegment(RepeatSegment* s) { _curRepeatSegment = s;     }
      int repeatSegmentIndex()                { return _curRepeatSegmentIndex; }
      void setRepeatSegmentIndex(int idx)     { _curRepeatSegmentIndex = idx; }
      bool expandRepeat()                     { return _expandRepeat; }
      Score* score() const                    { return _score;    }
      void setScore(Score* s)                 { _score = s; }

      Q_INVOKABLE Element* element() const;
      Q_INVOKABLE void rewind(int);
      Q_INVOKABLE bool next();
      Q_INVOKABLE bool nextMeasure();
      void putStaffText(Text* s);
      void add(ChordRest* c);
      Q_INVOKABLE int tick();
      Q_INVOKABLE double time();
      Q_INVOKABLE bool eos() const;
      Q_INVOKABLE bool isChord() const;
      Q_INVOKABLE bool isRest() const;
      };

#endif

