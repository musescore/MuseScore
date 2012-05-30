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
class StaffText;
class Measure;

//---------------------------------------------------------
//   Cursor
//---------------------------------------------------------

class Cursor : public QObject {
      Q_OBJECT
      Q_PROPERTY(int track          READ track         WRITE setTrack)
      Q_PROPERTY(int staffIdx       READ staffIdx      WRITE setStaffIdx)
      Q_PROPERTY(int voice          READ voice         WRITE setVoice)
      Q_PROPERTY(bool expandRepeats READ expandRepeats WRITE setExpandRepeats)

      Q_PROPERTY(Element* element READ element)
      Q_PROPERTY(Segment* segment READ segment)

      Q_PROPERTY(int tick         READ tick)
      Q_PROPERTY(double time      READ time)
      Q_PROPERTY(Score* score     READ score    WRITE setScore)

      Score* _score;
      int _track;
      bool _expandRepeats;

      //state
      Segment* _segment;
      RepeatSegment* _curRepeatSegment;
      int _curRepeatSegmentIndex;

   public:
      Cursor(Score* c = 0);
      Cursor(Score*, bool);

      Score* score() const                    { return _score;    }
      void setScore(Score* s)                 { _score = s; }

      int track() const                       { return _track;    }
      void setTrack(int v);

      int staffIdx() const;
      void setStaffIdx(int v);

      int voice() const;
      void setVoice(int v);

      bool expandRepeats() const              { return _expandRepeats; }
      void setExpandRepeats(bool v)           { _expandRepeats = v;    }

      Element* element() const;
      Segment* segment() const                { return _segment;  }

      RepeatSegment* repeatSegment() const    { return _curRepeatSegment;  }
      void setRepeatSegment(RepeatSegment* s) { _curRepeatSegment = s;     }

      int repeatSegmentIndex()                { return _curRepeatSegmentIndex; }
      void setRepeatSegmentIndex(int idx)     { _curRepeatSegmentIndex = idx; }

      int tick();
      double time();

      Q_INVOKABLE void rewind(int);
      Q_INVOKABLE bool next();
      Q_INVOKABLE bool nextMeasure();
      Q_INVOKABLE void add(Element*);
      };

#endif

