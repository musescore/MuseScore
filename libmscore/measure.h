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
 Definition of classes MStaff, Measure.
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
      Text* _noText        { 0 };         ///< Measure number text object
      StaffLines*  lines   { 0 };
      Spacer* _vspacerUp   { 0 };
      Spacer* _vspacerDown { 0 };
      Shape _shape;
      bool hasVoices       { false };     ///< indicates that MStaff contains more than one voice,
                                          ///< this changes some layout rules
      bool _visible        { true };
      bool _slashStyle     { false };
#ifndef NDEBUG
      bool _corrupted      { false };
#endif
      MStaff()  {}
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

      Shape& shape()               { return _shape; }
      const Shape& shape() const   { return _shape; }
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
//
//   @P firstSegment    Segment       the first segment of the measure (read-only)
//   @P lastSegment     Segment       the last segment of the measure (read-only)
//---------------------------------------------------------

class Measure : public MeasureBase {
      Q_OBJECT
      Q_PROPERTY(Ms::Segment* firstSegment READ first)
      Q_PROPERTY(Ms::Segment* lastSegment  READ last)

      std::vector<MStaff*>  _mstaves;
      SegmentList _segments;
      Measure* _mmRest;       // multi measure rest which replaces a measure range

      qreal _userStretch;

      Fraction _timesig;
      Fraction _len;          ///< actual length of measure

      int _mmRestCount;       // > 0 if this is a multi measure rest
                              // 0 if this is the start of a mm rest (_mmRest != 0)
                              // < 0 if this measure is covered by a mm rest

      int _playbackCount;     // temp. value used in RepeatList
                              // counts how many times this measure was already played

      int _repeatCount;       ///< end repeat marker und repeat count

      MeasureNumberMode _noMode;
      bool _breakMultiMeasureRest;

      void push_back(Segment* e);
      void push_front(Segment* e);

   public:
      Measure(Score* = 0);
      Measure(const Measure&);
      ~Measure();
      virtual Measure* clone() const override     { return new Measure(*this); }
      virtual Element::Type type() const override { return Element::Type::MEASURE; }
      virtual void setScore(Score* s) override;
      Measure* cloneMeasure(Score*, TieMap*);

      void read(XmlReader&, int idx);
      void read(XmlReader& d) { read(d, 0); }
      virtual void write(Xml& xml) const override { Element::write(xml); }
      void write(Xml&, int, bool writeSystemElements) const;
      void writeBox(Xml&) const;
      void readBox(XmlReader&);
      virtual bool isEditable() const override { return false; }
      void checkMeasure(int idx);

      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void change(Element* o, Element* n) override;
      virtual void spatiumChanged(qreal oldValue, qreal newValue) override;

      System* system() const                { return (System*)parent(); }
      std::vector<MStaff*>& mstaves()       { return _mstaves;      }
      const std::vector<MStaff*>& mstaves() const { return _mstaves;      }
      MStaff* mstaff(int staffIdx)          { return _mstaves[staffIdx]; }
      const MStaff* mstaff(int staffIdx) const { return _mstaves[staffIdx]; }
      bool hasVoices(int staffIdx) const    { return _mstaves[staffIdx]->hasVoices; }
      StaffLines* staffLines(int staffIdx)  { return _mstaves[staffIdx]->lines; }

      MeasureNumberMode measureNumberMode() const     { return _noMode;      }
      void setMeasureNumberMode(MeasureNumberMode v)  { _noMode = v;         }

      qreal minWidth1() const;

      Fraction timesig() const             { return _timesig;     }
      void setTimesig(const Fraction& f)   { _timesig = f;        }
      Fraction len() const                 { return _len;         }
      Fraction stretchedLen(Staff*) const;
      void setLen(const Fraction& f)       { _len = f;            }
      // actual length of measure in ticks
      virtual int ticks() const override;
      bool isIrregular() const             { return _timesig != _len; }

      int size() const                          { return _segments.size();        }
      Ms::Segment* first() const                { return _segments.first();       }
      Segment* first(Segment::Type t) const     { return _segments.first(t);      }

      Ms::Segment* last() const                 { return _segments.last(); }
      SegmentList& segments()                   { return _segments; }
      const SegmentList& segments() const       { return _segments; }

      qreal userStretch() const;
      void setUserStretch(qreal v)              { _userStretch = v; }

      void stretchMeasure(qreal stretch);
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
      Segment* tick2segment(int tick, Segment::Type st = Segment::Type::ChordRest);

      void sortStaves(QList<int>& dst);

      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&) override;

      int repeatCount() const         { return _repeatCount; }
      void setRepeatCount(int val)    { _repeatCount = val; }

      Segment* undoGetSegment(Segment::Type st, int tick);
      Segment* getSegment(Element* el, int tick);
      Segment* getSegment(Segment::Type st, int tick);
      Segment* findSegment(Segment::Type st, int tick);

      qreal createEndBarLines(bool);
      void setEndBarLineType(BarLineType val, int track, bool visible = true, QColor color = QColor());
      void setStartRepeatBarLine();

      RepeatMeasure* cmdInsertRepeatMeasure(int staffIdx);

      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      void createVoice(int track);
      void adjustToLen(Fraction);

      AccidentalVal findAccidental(Note*) const;
      AccidentalVal findAccidental(Segment* s, int staffIdx, int line, bool &error) const;
      void exchangeVoice(int voice1, int voice2, int staffIdx);
      void checkMultiVoices(int staffIdx);
      bool hasVoice(int track) const;
      bool isMeasureRest(int staffIdx) const;
      bool isFullMeasureRest() const;
      bool isRepeatMeasure(Staff* staff) const;
      bool visible(int staffIdx) const;
      bool slashStyle(int staffIdx) const;
      bool isFinalMeasureOfSection() const;
      bool isAnacrusis() const;

      bool breakMultiMeasureRest() const        { return _breakMultiMeasureRest; }
      void setBreakMultiMeasureRest(bool val)   { _breakMultiMeasureRest = val;  }

      bool empty() const;
      bool isOnlyRests(int track) const;
      bool isOnlyDeletedRests(int track) const;

      int playbackCount() const      { return _playbackCount; }
      void setPlaybackCount(int val) { _playbackCount = val; }
      QRectF staffabbox(int staffIdx) const;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;

      bool hasMMRest() const        { return _mmRest != 0; }
      bool isMMRest() const         { return _mmRestCount > 0; }
      Measure* mmRest() const       { return _mmRest;      }
      const Measure* mmRest1() const;
      void setMMRest(Measure* m)    { _mmRest = m;         }
      int mmRestCount() const       { return _mmRestCount; }    // number of measures _mmRest spans
      void setMMRestCount(int n)    { _mmRestCount = n;    }
      Measure* mmRestFirst() const;
      Measure* mmRestLast() const;

      Element* nextElementStaff(int staff);
      Element* prevElementStaff(int staff);
      virtual QString accessibleInfo() const override;

      void removeSystemHeader();
      void removeSystemTrailer();

      const BarLine* endBarLine() const;
      BarLineType endBarLineType() const;
      bool endBarLineVisible() const;
      Shape& staffShape(int staffIdx) { return mstaff(staffIdx)->shape(); }
      virtual void triggerLayout() const override;
      };

}     // namespace Ms
#endif

