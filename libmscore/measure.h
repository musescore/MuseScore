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
 Definition of class Measure.
*/

#include "measurebase.h"
#include "fraction.h"
#include "segmentlist.h"

namespace Ms {

class XmlWriter;
class Beam;
class Tuplet;
class Staff;
class Chord;
class MeasureNumber;
class MMRestRange;
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

class MStaff;

//---------------------------------------------------------
//   MeasureNumberMode
//---------------------------------------------------------

enum class MeasureNumberMode : char {
      AUTO,       // show measure number depending on style
      SHOW,       // always show measure number
      HIDE        // don’t show measure number
      };

//---------------------------------------------------------
//   @@ Measure
///    one measure in a system
//
//   @P firstSegment    Segment       the first segment of the measure (read-only)
//   @P lastSegment     Segment       the last segment of the measure (read-only)
//---------------------------------------------------------

class Measure final : public MeasureBase {
      std::vector<MStaff*>  _mstaves;
      SegmentList _segments;
      Measure* _mmRest;       // multi measure rest which replaces a measure range

      qreal _userStretch;

      Fraction _timesig;

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

      void fillGap(const Fraction& pos, const Fraction& len, int track, const Fraction& stretch, bool useGapRests = true);
      void computeMinWidth(Segment* s, qreal x, bool isSystemHeader);

      void readVoice(XmlReader& e, int staffIdx, bool irregular);

   public:
      Measure(Score* = 0);
      Measure(const Measure&);
      ~Measure();

      Measure* clone() const override     { return new Measure(*this); }
      ElementType type() const override { return ElementType::MEASURE; }
      void setScore(Score* s) override;
      Measure* cloneMeasure(Score*, const Fraction& tick, TieMap*);

      void read(XmlReader&, int idx);
      void read(XmlReader& d) { read(d, 0); }
      void readAddConnector(ConnectorInfoReader* info, bool pasteMode) override;
      void write(XmlWriter& xml) const override { Element::write(xml); }
      void write(XmlWriter&, int, bool writeSystemElements, bool forceTimeSig) const;
      void writeBox(XmlWriter&) const;
      void readBox(XmlReader&);
      bool isEditable() const override { return false; }
      void checkMeasure(int idx, bool useGapRests = true);

      void add(Element*) override;
      void remove(Element*) override;
      void change(Element* o, Element* n) override;
      void spatiumChanged(qreal oldValue, qreal newValue) override;

      System* system() const                      { return (System*)parent(); }
      bool hasVoices(int staffIdx, Fraction stick, Fraction len) const;
      bool hasVoices(int staffIdx) const;
      void setHasVoices(int staffIdx, bool v);

      StaffLines* staffLines(int staffIdx);
      Spacer* vspacerDown(int staffIdx) const;
      Spacer* vspacerUp(int staffIdx) const;
      void setStaffVisible(int staffIdx, bool visible);
      void setStaffStemless(int staffIdx, bool stemless);
      bool corrupted(int staffIdx) const;
      void setCorrupted(int staffIdx, bool val);
      void setNoText(int staffIdx, MeasureNumber*);
      MeasureNumber* noText(int staffIdx) const;
      void setMMRangeText(int staffIdx, MMRestRange *);
      MMRestRange *mmRangeText(int staffIdx) const;

      void createStaves(int);

      MeasureNumberMode measureNumberMode() const     { return _noMode;      }
      void setMeasureNumberMode(MeasureNumberMode v)  { _noMode = v;         }

      Fraction timesig() const             { return _timesig;     }
      void setTimesig(const Fraction& f)   { _timesig = f;        }

      Fraction stretchedLen(Staff*) const;
      bool isIrregular() const             { return _timesig != _len; }

      int size() const                     { return _segments.size();        }
      Ms::Segment* first() const           { return _segments.first();       }
      Segment* first(SegmentType t) const  { return _segments.first(t);      }
      Segment* firstEnabled() const        { return _segments.first(ElementFlag::ENABLED); }

      Ms::Segment* last() const            { return _segments.last(); }
      Segment* lastEnabled() const         { return _segments.last(ElementFlag::ENABLED); }
      SegmentList& segments()              { return _segments; }
      const SegmentList& segments() const  { return _segments; }

      qreal userStretch() const;
      void setUserStretch(qreal v)         { _userStretch = v; }

      void stretchMeasure(qreal stretch);
      Fraction computeTicks();
      void layout2();

      bool showsMeasureNumber();
      bool showsMeasureNumberInAutoMode();
      void layoutMeasureNumber();
      void layoutMMRestRange();

      Chord* findChord(Fraction tick, int track);
      ChordRest* findChordRest(Fraction tick, int track);
      Fraction snap(const Fraction& tick, const QPointF p) const;
      Fraction snapNote(const Fraction& tick, const QPointF p, int staff) const;

      Segment* searchSegment(qreal x, SegmentType st, int strack, int etrack, const Segment* preferredSegment = nullptr, qreal spacingFactor = 0.5) const;

      void insertStaff(Staff*, int staff);
      void insertMStaff(MStaff* staff, int idx);
      void removeMStaff(MStaff* staff, int idx);

      void moveTicks(const Fraction& diff) override;

      void cmdRemoveStaves(int s, int e);
      void cmdAddStaves(int s, int e, bool createRest);
      void removeStaves(int s, int e);
      void insertStaves(int s, int e);

      qreal tick2pos(Fraction) const;
      Segment* tick2segment(const Fraction& tick, SegmentType st = SegmentType::ChordRest);

      void sortStaves(QList<int>& dst);

      bool acceptDrop(EditData&) const override;
      Element* drop(EditData&) override;

      int repeatCount() const         { return _repeatCount; }
      void setRepeatCount(int val)    { _repeatCount = val; }

      Segment* findSegmentR(SegmentType st,    const Fraction&) const;
      Segment* undoGetSegmentR(SegmentType st, const Fraction& f);
      Segment* getSegmentR(SegmentType st,     const Fraction& f);
      Segment* findFirstR(SegmentType st, const Fraction& rtick) const;

      // segment routines with absolute tick values
      Segment* findSegment(SegmentType st,    const Fraction& f) const { return findSegmentR(st, f - tick()); }
      Segment* undoGetSegment(SegmentType st, const Fraction& f)       { return undoGetSegmentR(st, f - tick()); }
      Segment* getSegment(SegmentType st,     const Fraction& f)       { return getSegmentR(st, f - tick()); }

      void connectTremolo();

      qreal createEndBarLines(bool);
      void barLinesSetSpan(Segment*);
      void setEndBarLineType(BarLineType val, int track, bool visible = true, QColor color = QColor());

      RepeatMeasure* cmdInsertRepeatMeasure(int staffIdx);

      void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      void createVoice(int track);
      void adjustToLen(Fraction, bool appendRestsIfNecessary = true);

      AccidentalVal findAccidental(Note*) const;
      AccidentalVal findAccidental(Segment* s, int staffIdx, int line, bool &error) const;
      void exchangeVoice(int voice1, int voice2, int staffIdx);
      void checkMultiVoices(int staffIdx);
      bool hasVoice(int track) const;
      bool isEmpty(int staffIdx) const;
      bool isCutawayClef(int staffIdx) const;
      bool isFullMeasureRest() const;
      bool isRepeatMeasure(const Staff* staff) const;
      bool visible(int staffIdx) const;
      bool stemless(int staffIdx) const;
      bool isFinalMeasureOfSection() const;
      bool isAnacrusis() const;
      bool isFirstInSystem() const;
      bool isFirstInSection() const;

      bool breakMultiMeasureRest() const        { return _breakMultiMeasureRest; }
      void setBreakMultiMeasureRest(bool val)   { _breakMultiMeasureRest = val;  }

      bool empty() const;
      bool isOnlyRests(int track) const;
      bool isOnlyDeletedRests(int track) const;

      int playbackCount() const      { return _playbackCount; }
      void setPlaybackCount(int val) { _playbackCount = val; }
      QRectF staffabbox(int staffIdx) const;

      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid) const override;

      void undoChangeProperty(Pid id, const QVariant& newValue);
      void undoChangeProperty(Pid id, const QVariant& newValue, PropertyFlags ps) override;

      bool hasMMRest() const        { return _mmRest != 0; }
      bool isMMRest() const         { return _mmRestCount > 0; }
      Measure* mmRest() const       { return _mmRest;      }
      const Measure* coveringMMRestOrThis() const;
      void setMMRest(Measure* m)    { _mmRest = m;         }
      int mmRestCount() const       { return _mmRestCount; }    // number of measures _mmRest spans
      void setMMRestCount(int n)    { _mmRestCount = n;    }
      Measure* mmRestFirst() const;
      Measure* mmRestLast() const;

      Element* nextElementStaff(int staff);
      Element* prevElementStaff(int staff);
      QString accessibleInfo() const override;

      void addSystemHeader(bool firstSystem);
      void addSystemTrailer(Measure* nm);
      void removeSystemHeader();
      void removeSystemTrailer();

      const BarLine* endBarLine() const;
      BarLineType endBarLineType() const;
      bool endBarLineVisible() const;
      void triggerLayout() const override;
      qreal basicStretch() const;
      qreal basicWidth() const;
      int layoutWeight(int maxMMRestLength = 0) const;
      void computeMinWidth();
      void checkHeader();
      void checkTrailer();
      void setStretchedWidth(qreal);
      void layoutStaffLines();

      qreal computeFirstSegmentXPosition(Segment* segment);

      };

}     // namespace Ms
#endif
