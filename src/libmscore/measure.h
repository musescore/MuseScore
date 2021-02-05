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
class MeasureRepeat;

//---------------------------------------------------------
//   MeasureNumberMode
//---------------------------------------------------------

enum class MeasureNumberMode : char {
    AUTO,         // show measure number depending on style
    SHOW,         // always show measure number
    HIDE          // don’t show measure number
};

//---------------------------------------------------------
//   MStaff
///   Per staff values of measure.
//---------------------------------------------------------

class MStaff
{
public:
    MStaff() {}
    ~MStaff();
    MStaff(const MStaff&);

    void setScore(Score*);
    void setTrack(int);

    MeasureNumber* noText() const { return m_noText; }
    void setNoText(MeasureNumber* t) { m_noText = t; }

    MMRestRange* mmRangeText() const { return m_mmRangeText; }
    void setMMRangeText(MMRestRange* r) { m_mmRangeText = r; }

    StaffLines* lines() const { return m_lines; }
    void setLines(StaffLines* l) { m_lines = l; }

    Spacer* vspacerUp() const { return m_vspacerUp; }
    void setVspacerUp(Spacer* s) { m_vspacerUp = s; }
    Spacer* vspacerDown() const { return m_vspacerDown; }
    void setVspacerDown(Spacer* s) { m_vspacerDown = s; }

    bool hasVoices() const { return m_hasVoices; }
    void setHasVoices(bool val) { m_hasVoices = val; }

    bool visible() const { return m_visible; }
    void setVisible(bool val) { m_visible = val; }

    bool stemless() const { return m_stemless; }
    void setStemless(bool val) { m_stemless = val; }

#ifndef NDEBUG
    bool corrupted() const { return m_corrupted; }
    void setCorrupted(bool val) { m_corrupted = val; }
#endif

    int measureRepeatCount() const { return m_measureRepeatCount; }
    void setMeasureRepeatCount(int n) { m_measureRepeatCount = n; }

private:
    MeasureNumber* m_noText { nullptr };      ///< Measure number text object
    MMRestRange* m_mmRangeText { nullptr };    ///< Multi measure rest range text object
    StaffLines* m_lines     { nullptr };
    Spacer* m_vspacerUp     { nullptr };
    Spacer* m_vspacerDown   { nullptr };
    bool m_hasVoices        { false };  ///< indicates that MStaff contains more than one voice,
                                        ///< this changes some layout rules
    bool m_visible          { true };
    bool m_stemless         { false };
#ifndef NDEBUG
    bool m_corrupted        { false };
#endif
    int m_measureRepeatCount { 0 };
};

//---------------------------------------------------------
//   @@ Measure
///    one measure in a system
//
//   @P firstSegment    Segment       the first segment of the measure (read-only)
//   @P lastSegment     Segment       the last segment of the measure (read-only)
//---------------------------------------------------------

class Measure final : public MeasureBase
{
public:
    Measure(Score* = 0);
    Measure(const Measure&);
    ~Measure();

    Measure* clone() const override { return new Measure(*this); }
    ElementType type() const override { return ElementType::MEASURE; }
    void setScore(Score* s) override;
    Measure* cloneMeasure(Score*, const Fraction& tick, TieMap*);

    // Score Tree functions
    ScoreElement* treeParent() const override;
    ScoreElement* treeChild(int idx) const override;
    int treeChildCount() const override;

    void read(XmlReader&, int idx);
    void read(XmlReader& d) override { read(d, 0); }
    void readAddConnector(ConnectorInfoReader* info, bool pasteMode) override;
    void write(XmlWriter& xml) const override { Element::write(xml); }
    void write(XmlWriter&, int, bool writeSystemElements, bool forceTimeSig) const override;
    void writeBox(XmlWriter&) const;
    void readBox(XmlReader&);
    bool isEditable() const override { return false; }
    void checkMeasure(int idx);

    void add(Element*) override;
    void remove(Element*) override;
    void change(Element* o, Element* n) override;
    void spatiumChanged(qreal oldValue, qreal newValue) override;

    System* system() const { return toSystem(parent()); }
    bool hasVoices(int staffIdx, Fraction stick, Fraction len) const;
    bool hasVoices(int staffIdx) const;
    void setHasVoices(int staffIdx, bool v);

    StaffLines* staffLines(int staffIdx);
    Spacer* vspacerDown(int staffIdx) const;
    Spacer* vspacerUp(int staffIdx) const;
    void setStaffVisible(int staffIdx, bool visible);
    void setStaffStemless(int staffIdx, bool stemless);
#ifndef NDEBUG
    bool corrupted(int staffIdx) const { return m_mstaves[staffIdx]->corrupted(); }
    void setCorrupted(int staffIdx, bool val) { m_mstaves[staffIdx]->setCorrupted(val); }
#endif
    MeasureNumber* noText(int staffIdx) const { return m_mstaves[staffIdx]->noText(); }
    void setNoText(int staffIdx, MeasureNumber* t) { m_mstaves[staffIdx]->setNoText(t); }

    void setMMRangeText(int staffIdx, MMRestRange*);
    MMRestRange* mmRangeText(int staffIdx) const;

    void createStaves(int);

    MeasureNumberMode measureNumberMode() const { return m_noMode; }
    void setMeasureNumberMode(MeasureNumberMode v) { m_noMode = v; }

    Fraction timesig() const { return m_timesig; }
    void setTimesig(const Fraction& f) { m_timesig = f; }

    Fraction stretchedLen(Staff*) const;
    bool isIrregular() const { return m_timesig != _len; }

    int size() const { return m_segments.size(); }
    Ms::Segment* first() const { return m_segments.first(); }
    Segment* first(SegmentType t) const { return m_segments.first(t); }
    Segment* firstEnabled() const { return m_segments.first(ElementFlag::ENABLED); }

    Ms::Segment* last() const { return m_segments.last(); }
    Segment* lastEnabled() const { return m_segments.last(ElementFlag::ENABLED); }
    SegmentList& segments() { return m_segments; }
    const SegmentList& segments() const { return m_segments; }

    qreal userStretch() const;
    void setUserStretch(qreal v) { m_userStretch = v; }

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

    Segment* searchSegment(qreal x, SegmentType st, int strack, int etrack, const Segment* preferredSegment = nullptr,
                           qreal spacingFactor = 0.5) const;

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

    int repeatCount() const { return m_repeatCount; }
    void setRepeatCount(int val) { m_repeatCount = val; }

    Segment* findSegmentR(SegmentType st,    const Fraction&) const;
    Segment* undoGetSegmentR(SegmentType st, const Fraction& f);
    Segment* getSegmentR(SegmentType st,     const Fraction& f);
    Segment* findFirstR(SegmentType st, const Fraction& rtick) const;

    // segment routines with absolute tick values
    Segment* findSegment(SegmentType st,    const Fraction& f) const { return findSegmentR(st, f - tick()); }
    Segment* undoGetSegment(SegmentType st, const Fraction& f) { return undoGetSegmentR(st, f - tick()); }
    Segment* getSegment(SegmentType st,     const Fraction& f) { return getSegmentR(st, f - tick()); }

    void connectTremolo();

    qreal createEndBarLines(bool);
    void barLinesSetSpan(Segment*);
    void setEndBarLineType(BarLineType val, int track, bool visible = true, QColor color = QColor());

    void scanElements(void* data, void (* func)(void*, Element*), bool all=true) override;
    void createVoice(int track);
    void adjustToLen(Fraction, bool appendRestsIfNecessary = true);

    AccidentalVal findAccidental(Note*) const;
    AccidentalVal findAccidental(Segment* s, int staffIdx, int line, bool& error) const;
    void exchangeVoice(int voice1, int voice2, int staffIdx);
    void checkMultiVoices(int staffIdx);
    bool hasVoice(int track) const;
    bool isEmpty(int staffIdx) const;
    bool isCutawayClef(int staffIdx) const;
    bool isFullMeasureRest() const;
    bool visible(int staffIdx) const;
    bool stemless(int staffIdx) const;
    bool isFinalMeasureOfSection() const;
    bool isAnacrusis() const;
    bool isFirstInSystem() const;

    bool breakMultiMeasureRest() const { return m_breakMultiMeasureRest; }
    void setBreakMultiMeasureRest(bool val) { m_breakMultiMeasureRest = val; }

    bool empty() const;
    bool isOnlyRests(int track) const;
    bool isOnlyDeletedRests(int track) const;

    int playbackCount() const { return m_playbackCount; }
    void setPlaybackCount(int val) { m_playbackCount = val; }
    QRectF staffabbox(int staffIdx) const;

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid) const override;

    bool hasMMRest() const { return m_mmRest != 0; }
    bool isMMRest() const { return m_mmRestCount > 0; }
    Measure* mmRest() const { return m_mmRest; }
    const Measure* mmRest1() const;
    void setMMRest(Measure* m) { m_mmRest = m; }
    int mmRestCount() const { return m_mmRestCount; }            // number of measures m_mmRest spans
    void setMMRestCount(int n) { m_mmRestCount = n; }
    Measure* mmRestFirst() const;
    Measure* mmRestLast() const;

    int measureRepeatCount(int staffIdx) const { return m_mstaves[staffIdx]->measureRepeatCount(); }
    void setMeasureRepeatCount(int n, int staffIdx) { m_mstaves[staffIdx]->setMeasureRepeatCount(n); }
    bool isMeasureRepeatGroup(int staffIdx) const { return measureRepeatCount(staffIdx); }   // alias for convenience
    bool isMeasureRepeatGroupWithNextM(int staffIdx) const;
    bool isMeasureRepeatGroupWithPrevM(int staffIdx) const;
    Measure* firstOfMeasureRepeatGroup(int staffIdx) const;     // used to find beginning of group
    MeasureRepeat* measureRepeatElement(int staffIdx) const;    // get measure repeat element from anywhere within group
    int measureRepeatNumMeasures(int staffIdx) const;
    bool isOneMeasureRepeat(int staffIdx) const;
    bool nextIsOneMeasureRepeat(int staffidx) const;
    bool prevIsOneMeasureRepeat(int staffIdx) const;

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
    void computeMinWidth() override;
    void checkHeader();
    void checkTrailer();
    void setStretchedWidth(qreal);
    void layoutStaffLines();

private:
    void push_back(Segment* e);
    void push_front(Segment* e);

    void fillGap(const Fraction& pos, const Fraction& len, int track, const Fraction& stretch);
    void computeMinWidth(Segment* s, qreal x, bool isSystemHeader);

    void readVoice(XmlReader& e, int staffIdx, bool irregular);

    MStaff* mstaff(int staffIndex) const;

    std::vector<MStaff*> m_mstaves;
    SegmentList m_segments;
    Measure* m_mmRest;         // multi measure rest which replaces a measure range

    qreal m_userStretch;

    Fraction m_timesig;

    int m_mmRestCount;         // > 0 if this is a multimeasure rest
                               // 0 if this is the start of am mmrest (m_mmRest != 0)
                               // < 0 if this measure is covered by an mmrest

    int m_playbackCount { 0 };  // temp. value used in RepeatList
                                // counts how many times this measure was already played

    int m_repeatCount;          ///< end repeat marker and repeat count

    MeasureNumberMode m_noMode;
    bool m_breakMultiMeasureRest;
};
}     // namespace Ms
#endif
