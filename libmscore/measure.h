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

#ifndef __MEASURE_H__
#define __MEASURE_H__

/**
 \file
 Definition of classes MStaff, Measure and MeasureList.
*/

#include "measurebase.h"
#include "fraction.h"
#include "segmentlist.h"

namespace Ms {

class Xml;
class Beam;
class Tuplet;
class Staff;
class Chord;
class Text;
class ChordRest;
class Score;
class MuseScoreView;
class System;
class Note;
class Spacer;
class TieMap;
class AccidentalState;
class Spanner;
class Part;
class RepeatMeasure;

//---------------------------------------------------------
//   MStaff
///   Per staff values of measure.
//---------------------------------------------------------

struct MStaff {
      qreal distanceUp;
      qreal distanceDown;
      Text* _noText;          ///< Measure number text object
      StaffLines*  lines;
      Spacer* _vspacerUp;
      Spacer* _vspacerDown;
      bool hasVoices;         ///< indicates that MStaff contains more than one voice,
                              ///< this changes some layout rules
      bool _visible;
      bool _slashStyle;

      MStaff();
      ~MStaff();
      MStaff(const MStaff&);

      bool visible() const         { return _visible;    }
      void setVisible(bool val)    { _visible = val;     }
      bool slashStyle() const      { return _slashStyle; }
      void setSlashStyle(bool val) { _slashStyle = val;  }
      void setScore(Score*);
      void setTrack(int);
      Text* noText() const         { return _noText;     }
      void setNoText(Text* t)      { _noText = t;        }
      };

enum {
      RepeatEnd         = 1,
      RepeatStart       = 2,
      RepeatMeasureFlag = 4,
      RepeatJump        = 8
      };

//---------------------------------------------------------
//   MeasureNumberMode
//---------------------------------------------------------

enum class MeasureNumberMode : char {
      AUTO,       // show measure number depending on style
      SHOW,       // always show measure number
      HIDE        // dont show measure number
      };

//---------------------------------------------------------
//   @@ Measure
///    one measure in a system
//---------------------------------------------------------

class Measure : public MeasureBase {
      Q_OBJECT

      SegmentList _segments;

      Fraction _timesig;
      Fraction _len;          ///< actual length of measure

      int _repeatCount;       ///< end repeat marker und repeat count
      int _repeatFlags;       ///< or'd RepeatType's

      QList<MStaff*>  staves;

      int    _no;             ///< Measure number, counting from zero
      int    _noOffset;       ///< Offset to measure number
      MeasureNumberMode _noMode;

      qreal _userStretch;

      mutable qreal _minWidth1;     ///< minimal measure width without system header
      mutable qreal _minWidth2;     ///< minimal measure width with system header

      bool _irregular;              ///< Irregular measure, do not count
      bool _breakMultiMeasureRest;  ///< set by user
      bool _breakMMRest;            ///< set by layout

      BarLineType _endBarLineType;
      bool        _endBarLineGenerated;
      bool        _endBarLineVisible;
      QColor      _endBarLineColor;

      int _playbackCount;     // temp. value used in RepeatList
                              // counts how many times this measure was already played

      Measure* _mmRest;       // multi measure rest which replaces a measure range
      int _mmRestCount;       // > 0 if this is a multi measure rest
                              // 0 if this is the start of a mm rest (_mmRest != 0)
                              // < 0 if this measure is covered by a mm rest

      void push_back(Segment* e);
      void push_front(Segment* e);
      void layoutCR0(ChordRest* cr, qreal m);

   public:
      Measure(Score* = 0);
      Measure(const Measure&);
      ~Measure();
      virtual Measure* clone() const override   { return new Measure(*this); }
      virtual ElementType type() const override { return MEASURE; }
      virtual void setScore(Score* s) override;
      Measure* cloneMeasure(Score*, TieMap*);

      void read(XmlReader&, int idx);
      void read(XmlReader& d) { read(d, 0); }
      void write(Xml&, int, bool writeSystemElements) const;
      void writeBox(Xml&) const;
      void readBox(XmlReader&);
      virtual bool isEditable() const override { return false; }

      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void change(Element* o, Element* n) override;

      System* system() const               { return (System*)parent(); }
      QList<MStaff*>* staffList()          { return &staves;      }
      MStaff* mstaff(int staffIdx)         { return staves[staffIdx]; }
      bool hasVoices(int staffIdx) const   { return staves[staffIdx]->hasVoices; }
      StaffLines* staffLines(int staffIdx) { return staves[staffIdx]->lines; }
      int no() const                       { return _no;          }
      bool irregular() const               { return _irregular;   }
      void setIrregular(bool val)          { _irregular = val;    }
      int noOffset() const                 { return _noOffset;    }

      MeasureNumberMode measureNumberMode() const     { return _noMode;      }
      void setMeasureNumberMode(MeasureNumberMode v)  { _noMode = v;         }

      void setNo(int n)                    { _no = n;             }
      void setNoOffset(int n)              { _noOffset = n;       }
      virtual qreal distanceUp(int i) const;
      virtual qreal distanceDown(int i) const;

      qreal minWidth1() const;
      qreal minWidth2() const;
      void setMinWidth1(qreal w)           { _minWidth1 = w;      }
      void setMinWidth2(qreal w)           { _minWidth2 = w;      }
      bool systemHeader() const;
      void setDirty();

      Fraction timesig() const             { return _timesig;     }
      void setTimesig(const Fraction& f)   { _timesig = f;        }
      Fraction len() const                 { return _len;         }
      Fraction stretchedLen(Staff*) const;
      void setLen(const Fraction& f)       { _len = f;            }
      // actual length of measure in ticks
      virtual int ticks() const override   { return _len.ticks(); }

      int size() const                     { return _segments.size();        }
      Q_INVOKABLE Ms::Segment* first() const   { return _segments.first();       }
      Segment* first(Segment::SegmentTypes t) const { return _segments.first(t);      }

      Q_INVOKABLE Ms::Segment* last() const    { return _segments.last(); }
      void remove(Segment* s);
      SegmentList* segments()              { return &_segments; }

      qreal userStretch() const            { return _userStretch; }
      void setUserStretch(qreal v)         { _userStretch = v;    }

      void layoutX(qreal stretch);
      void layout(qreal width);
      void layout2();

      Chord* findChord(int tick, int track);
      ChordRest* findChordRest(int tick, int track);
      int snap(int tick, const QPointF p) const;
      int snapNote(int tick, const QPointF p, int staff) const;

      void insertStaff(Staff*, int staff);
      void insertMStaff(MStaff* staff, int idx);
      void removeMStaff(MStaff* staff, int idx);

      virtual void moveTicks(int diff);

      void cmdRemoveStaves(int s, int e);
      void cmdAddStaves(int s, int e, bool createRest);
      void removeStaves(int s, int e);
      void insertStaves(int s, int e);

      qreal tick2pos(int) const;
      Segment* tick2segment(int) const;

      void sortStaves(QList<int>& dst);

      void dump() const;
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const override;
      virtual Element* drop(const DropData&) override;

      int repeatCount() const         { return _repeatCount; }
      void setRepeatCount(int val)    { _repeatCount = val; }

      Segment* undoGetSegment(Segment::SegmentType st, int tick);
      Segment* getSegment(Element* el, int tick);
      Segment* getSegment(Segment::SegmentType st, int tick);
      Segment* findSegment(Segment::SegmentType st, int t);

      bool createEndBarLines();

      void setEndBarLineType(BarLineType val, bool g, bool visible = true, QColor color = QColor());
      BarLineType endBarLineType() const        { return _endBarLineType; }

      bool setStartRepeatBarLine(bool);
      bool endBarLineGenerated() const          { return _endBarLineGenerated; }
      void setEndBarLineGenerated(bool v)       { _endBarLineGenerated = v;    }
      bool endBarLineVisible() const            { return _endBarLineVisible;   }
      QColor endBarLineColor() const            { return _endBarLineColor;     }

      void cmdRemoveEmptySegment(Segment* s);
      RepeatMeasure* cmdInsertRepeatMeasure(int staffIdx);

      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      void createVoice(int track);
      void adjustToLen(Fraction);
      int repeatFlags() const                   { return _repeatFlags; }
      void setRepeatFlags(int val)              { _repeatFlags = val;  }
      AccidentalVal findAccidental(Note*) const;
      AccidentalVal findAccidental(Segment* s, int staffIdx, int line) const;
      void exchangeVoice(int, int, int, int);
      void checkMultiVoices(int staffIdx);
      bool hasVoice(int track) const;
      bool isMeasureRest(int staffIdx);
      bool isFullMeasureRest();
      bool isRepeatMeasure(Part* part);
      bool visible(int staffIdx) const;
      bool slashStyle(int staffIdx) const;

      bool breakMultiMeasureRest() const        { return _breakMultiMeasureRest | _breakMMRest; }
      bool breakMMRest() const                  { return _breakMMRest; }
      void setBreakMMRest(bool v)               { _breakMMRest = v;    }
      bool getBreakMultiMeasureRest() const     { return _breakMultiMeasureRest; }
      void setBreakMultiMeasureRest(bool val)   { _breakMultiMeasureRest = val;  }

      bool isEmpty() const;

      void layoutChords0(Segment* segment, int startTrack);
      void layoutChords10(Segment* segment, int startTrack, AccidentalState*);
      void updateAccidentals(Segment* segment, int staffIdx, AccidentalState*);
      void layoutStage1();
      int playbackCount() const      { return _playbackCount; }
      void setPlaybackCount(int val) { _playbackCount = val; }
      QRectF staffabbox(int staffIdx) const;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;

      bool hasMMRest() const        { return _mmRest != 0; }
      bool isMMRest() const         { return _mmRestCount > 0; }
      Measure* mmRest() const       { return _mmRest;      }
      void setMMRest(Measure* m)    { _mmRest = m;         }
      int mmRestCount() const       { return _mmRestCount; }    // number of measures _mmRest spans
      void setMMRestCount(int n)    { _mmRestCount = n;    }
      };

}     // namespace Ms
#endif

