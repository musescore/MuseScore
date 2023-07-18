/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __MEASURE_H__
#define __MEASURE_H__

/**
 \file
 Definition of class Measure.
*/

#include "measurebase.h"

#include "segmentlist.h"

namespace mu::engraving::read400 {
class MeasureRead;
}

namespace mu::engraving::read410 {
class MeasureRead;
}

namespace mu::engraving::write {
class MeasureWrite;
}

namespace mu::engraving {
class AccidentalState;
class Chord;
class ChordRest;
class MMRestRange;
class MeasureNumber;
class MeasureRepeat;
class Note;
class Part;
class Score;
class Spacer;
class Staff;
class System;
class TieMap;

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
    void setTrack(track_idx_t);

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
    MeasureNumber* m_noText = nullptr;      // Measure number text object
    MMRestRange* m_mmRangeText = nullptr;   // Multi measure rest range text object
    StaffLines* m_lines = nullptr;
    Spacer* m_vspacerUp = nullptr;
    Spacer* m_vspacerDown = nullptr;
    bool m_hasVoices = false;               // indicates that MStaff contains more than one voice,
                                            // this changes some layout rules
    bool m_visible = true;
    bool m_stemless = false;
#ifndef NDEBUG
    bool m_corrupted = false;
#endif
    int m_measureRepeatCount = 0;
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
    OBJECT_ALLOCATOR(engraving, Measure)
    DECLARE_CLASSOF(ElementType::MEASURE)

public:

    ~Measure();

    void setParent(System* s);

    Measure* clone() const override { return new Measure(*this); }
    void setScore(Score* s) override;
    Measure* cloneMeasure(Score*, const Fraction& tick, TieMap*);

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;

    bool isEditable() const override { return false; }
    void checkMeasure(staff_idx_t idx, bool useGapRests = true);

    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;
    void change(EngravingItem* o, EngravingItem* n) override;
    void spatiumChanged(double oldValue, double newValue) override;

    System* system() const { return toSystem(explicitParent()); }
    bool hasVoices(staff_idx_t staffIdx, Fraction stick, Fraction len, bool considerInvisible = false) const;
    bool hasVoices(staff_idx_t staffIdx) const;
    void setHasVoices(staff_idx_t staffIdx, bool v);

    StaffLines* staffLines(staff_idx_t staffIdx);
    Spacer* vspacerDown(staff_idx_t staffIdx) const;
    Spacer* vspacerUp(staff_idx_t staffIdx) const;
    void setStaffVisible(staff_idx_t staffIdx, bool visible);
    void setStaffStemless(staff_idx_t staffIdx, bool stemless);
#ifndef NDEBUG
    bool corrupted(staff_idx_t staffIdx) const { return m_mstaves[staffIdx]->corrupted(); }
    void setCorrupted(staff_idx_t staffIdx, bool val) { m_mstaves[staffIdx]->setCorrupted(val); }
#endif
    MeasureNumber* noText(staff_idx_t staffIdx) const { return m_mstaves[staffIdx]->noText(); }
    void setNoText(staff_idx_t staffIdx, MeasureNumber* t) { m_mstaves[staffIdx]->setNoText(t); }

    const std::vector<MStaff*>& mstaves() const { return m_mstaves; }
    std::vector<MStaff*>& mstaves() { return m_mstaves; }

    void setMMRangeText(staff_idx_t staffIdx, MMRestRange*);
    MMRestRange* mmRangeText(staff_idx_t staffIdx) const;

    void createStaves(staff_idx_t);

    MeasureNumberMode measureNumberMode() const { return m_noMode; }
    void setMeasureNumberMode(MeasureNumberMode v) { m_noMode = v; }

    Fraction timesig() const { return m_timesig; }
    void setTimesig(const Fraction& f) { m_timesig = f; }

    Fraction stretchedLen(Staff*) const;
    bool isIrregular() const { return m_timesig != m_len; }

    int size() const { return m_segments.size(); }
    Segment* first() const { return m_segments.first(); }
    Segment* first(SegmentType t) const { return m_segments.first(t); }
    Segment* firstEnabled() const { return m_segments.first(ElementFlag::ENABLED); }

    Segment* last() const { return m_segments.last(); }
    Segment* lastEnabled() const { return m_segments.last(ElementFlag::ENABLED); }
    SegmentList& segments() { return m_segments; }
    const SegmentList& segments() const { return m_segments; }

    double userStretch() const;
    void setUserStretch(double v) { m_userStretch = v; }

    void setLayoutStretch(double stretchCoeff) { m_layoutStretch = stretchCoeff; }
    double layoutStretch() const { return m_layoutStretch; }

    Fraction computeTicks();
    Fraction shortestChordRest() const;
    Fraction maxTicks() const;

    bool showsMeasureNumber();
    bool showsMeasureNumberInAutoMode();

    Chord* findChord(Fraction tick, track_idx_t track);
    ChordRest* findChordRest(Fraction tick, track_idx_t track);
    Fraction snap(const Fraction& tick, const mu::PointF p) const;
    Fraction snapNote(const Fraction& tick, const mu::PointF p, int staff) const;

    Segment* searchSegment(double x, SegmentType st, track_idx_t strack, track_idx_t etrack, const Segment* preferredSegment = nullptr,
                           double spacingFactor = 0.5) const;

    void insertStaff(Staff*, staff_idx_t staff);
    void insertMStaff(MStaff* staff, staff_idx_t idx);
    void removeMStaff(MStaff* staff, staff_idx_t idx);

    void moveTicks(const Fraction& diff) override;

    void cmdRemoveStaves(staff_idx_t s, staff_idx_t e);
    void cmdAddStaves(staff_idx_t s, staff_idx_t e, bool createRest);
    void removeStaves(staff_idx_t s, staff_idx_t e);
    void insertStaves(staff_idx_t s, staff_idx_t e);

    double tick2pos(Fraction) const;
    Segment* tick2segment(const Fraction& tick, SegmentType st = SegmentType::ChordRest);

    void sortStaves(std::vector<staff_idx_t>& dst);

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

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

    void setEndBarLineType(BarLineType val, track_idx_t track, bool visible = true, mu::draw::Color color = mu::draw::Color());

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;
    void createVoice(int track);
    void adjustToLen(Fraction, bool appendRestsIfNecessary = true);

    AccidentalVal findAccidental(Note*) const;
    AccidentalVal findAccidental(Segment* s, staff_idx_t staffIdx, int line, bool& error) const;
    void exchangeVoice(track_idx_t voice1, track_idx_t voice2, staff_idx_t staffIdx);
    void checkMultiVoices(staff_idx_t staffIdx);
    bool hasVoice(track_idx_t track) const;
    bool isEmpty(staff_idx_t staffIdx) const;
    bool isCutawayClef(staff_idx_t staffIdx) const;
    bool isFullMeasureRest() const;
    bool visible(staff_idx_t staffIdx) const;
    bool stemless(staff_idx_t staffIdx) const;
    bool isFinalMeasureOfSection() const;
    bool isAnacrusis() const;
    bool isFirstInSystem() const;

    bool breakMultiMeasureRest() const { return m_breakMultiMeasureRest; }
    void setBreakMultiMeasureRest(bool val) { m_breakMultiMeasureRest = val; }

    bool empty() const;
    bool isOnlyRests(track_idx_t track) const;
    bool isOnlyDeletedRests(track_idx_t track) const;

    int playbackCount() const { return m_playbackCount; }
    void setPlaybackCount(int val) { m_playbackCount = val; }
    mu::RectF staffabbox(staff_idx_t staffIdx) const;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    bool hasMMRest() const { return m_mmRest != 0; }
    bool isMMRest() const { return m_mmRestCount > 0; }
    Measure* mmRest() const { return m_mmRest; }
    const Measure* coveringMMRestOrThis() const;
    void setMMRest(Measure* m) { m_mmRest = m; }
    int mmRestCount() const { return m_mmRestCount; }            // number of measures m_mmRest spans
    void setMMRestCount(int n) { m_mmRestCount = n; }
    Measure* mmRestFirst() const;
    Measure* mmRestLast() const;

    int measureRepeatCount(staff_idx_t staffIdx) const;
    bool containsMeasureRepeat(const staff_idx_t staffIdxFrom, const staff_idx_t staffIdxTo) const;
    void setMeasureRepeatCount(int n, staff_idx_t staffIdx);
    bool isMeasureRepeatGroup(staff_idx_t staffIdx) const;
    bool isMeasureRepeatGroupWithNextM(staff_idx_t staffIdx) const;
    bool isMeasureRepeatGroupWithPrevM(staff_idx_t staffIdx) const;
    Measure* firstOfMeasureRepeatGroup(staff_idx_t staffIdx) const;     // used to find beginning of group
    MeasureRepeat* measureRepeatElement(staff_idx_t staffIdx) const;    // get measure repeat element from anywhere within group
    int measureRepeatNumMeasures(staff_idx_t staffIdx) const;
    bool isOneMeasureRepeat(staff_idx_t staffIdx) const;
    bool nextIsOneMeasureRepeat(staff_idx_t staffidx) const;
    bool prevIsOneMeasureRepeat(staff_idx_t staffIdx) const;

    EngravingItem* nextElementStaff(staff_idx_t staff);
    EngravingItem* prevElementStaff(staff_idx_t staff);

    String accessibleInfo() const override;

#ifndef ENGRAVING_NO_ACCESSIBILITY
    AccessibleItemPtr createAccessible() override;
#endif

    const BarLine* endBarLine() const;
    BarLineType endBarLineType() const;
    bool endBarLineVisible() const;
    void triggerLayout() const override;
    double basicStretch() const;
    double basicWidth() const;
    void stretchToTargetWidth(double targetWidth);
    void checkHeader();
    void checkTrailer();

    bool isWidthLocked() const { return m_isWidthLocked; }
    // A measure is widthLocked if its width has been locked by the minMeasureWidth (or minMMRestWidth)
    // parameter, meaning it can't be any narrower than it currently is.
    void setWidthLocked(bool b) { m_isWidthLocked = b; }

    //! puts segments on the positions according to their length
    void layoutSegmentsInPracticeMode(const std::vector<int>& visibleParts);

    double computeFirstSegmentXPosition(Segment* segment);

    void layoutSegmentsWithDuration(const std::vector<int>& visibleParts);

    void calculateQuantumCell(const std::vector<int>& visibleParts);

    Fraction quantumOfSegmentCell() const;

    double squeezableSpace() const { return m_squeezableSpace; }
    void setSqueezableSpace(double val) { m_squeezableSpace = val; }

    void respaceSegments();

    void spaceRightAlignedSegments();

private:

    friend class Factory;
    friend class read400::MeasureRead;
    friend class read410::MeasureRead;
    friend class write::MeasureWrite;

    Measure(System* parent = 0);
    Measure(const Measure&);

    void push_back(Segment* e);
    void push_front(Segment* e);

    void fillGap(const Fraction& pos, const Fraction& len, track_idx_t track, const Fraction& stretch, bool useGapRests = true);

    MStaff* mstaff(staff_idx_t staffIndex) const;

    double m_squeezableSpace = 0.0;

    std::vector<MStaff*> m_mstaves;
    SegmentList m_segments;
    Measure* m_mmRest = nullptr; // multi measure rest which replaces a measure range

    double m_userStretch = 0.0;

    Fraction m_timesig;

    int m_mmRestCount = 0;      // > 0 if this is a multimeasure rest
                                // 0 if this is the start of am mmrest (m_mmRest != 0)
                                // < 0 if this measure is covered by an mmrest

    int m_playbackCount = 0;    // temp. value used in RepeatList
                                // counts how many times this measure was already played

    int m_repeatCount = 0;      // end repeat marker and repeat count

    MeasureNumberMode m_noMode = MeasureNumberMode::AUTO;
    bool m_breakMultiMeasureRest = false;

    Fraction m_quantumOfSegmentCell = { 1, 16 };

    double m_layoutStretch = 1.0;
    bool m_isWidthLocked = false;
};
} // namespace mu::engraving
#endif
