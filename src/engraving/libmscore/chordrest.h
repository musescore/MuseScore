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

#ifndef __CHORDREST_H__
#define __CHORDREST_H__

#include <functional>

#include "durationelement.h"
#include "types/types.h"

namespace mu::engraving {
enum class CrossMeasure : signed char {
    UNKNOWN = -1,
    NONE = 0,
    FIRST,
    SECOND
};

class Articulation;
class Lyrics;
class Measure;
class Score;
class Segment;
class Slur;
class TabDurationSymbol;
enum class SegmentType;
class BeamSegment;

//-------------------------------------------------------------------
//   ChordRest
//    Virtual base class. Chords and rests can be part of a beam
//-------------------------------------------------------------------

class ChordRest : public DurationElement
{
    OBJECT_ALLOCATOR(engraving, ChordRest)

    ElementList _el;
    TDuration _durationType;
    int _staffMove; // -1, 0, +1, used for crossbeaming
    int _storedStaffMove = 0; // used to remember and re-apply staff move if needed

    void processSiblings(std::function<void(EngravingItem*)> func);

protected:
    std::vector<Lyrics*> _lyrics;
    TabDurationSymbol* _tabDur;           // stores a duration symbol in tablature staves

    Beam* _beam;
    BeamSegment* _beamlet = nullptr;
    BeamMode _beamMode;
    bool _up;                             // actual stem direction
    bool _usesAutoUp;
    bool m_isSmall;
    bool _melismaEnd;

    // CrossMeasure: combine 2 tied notes if across a bar line and can be combined in a single duration
    CrossMeasure _crossMeasure;           ///< 0: no cross-measure modification; 1: 1st note of a mod.; -1: 2nd note
    TDuration _crossMeasureTDur;          ///< the total Duration type of the combined notes

public:
    ChordRest(const ElementType& type, Segment* parent);
    ChordRest(const ChordRest&, bool link = false);
    ChordRest& operator=(const ChordRest&) = delete;
    ~ChordRest();

    // Score Tree functions
    virtual EngravingObject* scanParent() const override;
    virtual EngravingObjectList scanChildren() const override;
    virtual void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    virtual EngravingItem* drop(EditData&) override;
    virtual void undoUnlink() override;

    virtual Segment* segment() const { return (Segment*)explicitParent(); }

    virtual void readAddConnector(ConnectorInfoReader* info, bool pasteMode) override;

    void setBeamMode(BeamMode m) { _beamMode = m; }
    void undoSetBeamMode(BeamMode m);
    BeamMode beamMode() const { return _beamMode; }

    void setBeam(Beam* b);
    void setBeamlet(BeamSegment* b);
    virtual Beam* beam() const final;
    int beams() const { return _durationType.hooks(); }
    virtual double upPos()   const = 0;
    virtual double downPos() const = 0;

    int line(bool up) const { return up ? upLine() : downLine(); }
    int line() const { return _up ? upLine() : downLine(); }
    virtual int upLine() const = 0;
    virtual int downLine() const = 0;
    virtual mu::PointF stemPos() const = 0;
    virtual double stemPosX() const = 0;
    virtual mu::PointF stemPosBeam() const = 0;
    virtual double rightEdge() const = 0;

    void setUp(bool val) { _up = val; }
    bool up() const { return _up; }
    bool usesAutoUp() const { return _usesAutoUp; }

    bool isSmall() const { return m_isSmall; }
    void setSmall(bool val) { m_isSmall = val; }
    void undoSetSmall(bool val);

    int staffMove() const { return _staffMove; }
    int storedStaffMove() const { return _storedStaffMove; }
    void setStaffMove(int val) { _staffMove = val; }
    staff_idx_t vStaffIdx() const override { return staffIdx() + _staffMove; }
    void checkStaffMoveValidity();

    const TDuration durationType() const
    {
        return _crossMeasure == CrossMeasure::FIRST
               ? _crossMeasureTDur : _durationType;
    }

    const TDuration actualDurationType() const { return _durationType; }
    void setDurationType(DurationType t);
    void setDurationType(const Fraction& ticks);
    void setDurationType(TDuration v);
    void setDots(int n) { _durationType.setDots(n); }
    int dots() const
    {
        return _crossMeasure == CrossMeasure::FIRST ? _crossMeasureTDur.dots()
               : (_crossMeasure == CrossMeasure::SECOND ? 0 : _durationType.dots());
    }

    int actualDots() const { return _durationType.dots(); }
    Fraction durationTypeTicks() const
    {
        return _crossMeasure == CrossMeasure::FIRST ? _crossMeasureTDur.ticks() : _durationType.ticks();
    }

    String durationUserName() const;

    void setTrack(track_idx_t val) override;

    const std::vector<Lyrics*>& lyrics() const { return _lyrics; }
    std::vector<Lyrics*>& lyrics() { return _lyrics; }
    Lyrics* lyrics(int verse, PlacementV) const;
    int lastVerse(PlacementV) const;
    bool isMelismaEnd() const;
    void setMelismaEnd(bool v);

    virtual void add(EngravingItem*) override;
    virtual void remove(EngravingItem*) override;
    void removeDeleteBeam(bool beamed);
    void replaceBeam(Beam* newBeam);

    ElementList& el() { return _el; }
    const ElementList& el() const { return _el; }

    Slur* slur(const ChordRest* secondChordRest = nullptr) const;

    CrossMeasure crossMeasure() const { return _crossMeasure; }
    void setCrossMeasure(CrossMeasure val) { _crossMeasure = val; }
    virtual void crossMeasureSetup(bool /*on*/) { }
    // the following two functions should not be used, unless absolutely necessary;
    // the cross-measure duration is best managed through setDuration() and crossMeasureSetup()
    TDuration crossMeasureDurationType() const { return _crossMeasureTDur; }
    void setCrossMeasureDurationType(TDuration v) { _crossMeasureTDur = v; }

    void localSpatiumChanged(double oldValue, double newValue) override;
    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;
    bool isGrace() const;
    bool isGraceBefore() const;
    bool isGraceAfter() const;
    Breath* hasBreathMark() const;

    Segment* nextSegmentAfterCR(SegmentType types) const;

    void setScore(Score* s) override;
    EngravingItem* nextArticulationOrLyric(EngravingItem* e);
    EngravingItem* prevArticulationOrLyric(EngravingItem* e);
    virtual EngravingItem* nextElement() override;
    virtual EngravingItem* prevElement() override;
    EngravingItem* lastElementBeforeSegment();
    virtual EngravingItem* nextSegmentElement() override;
    virtual EngravingItem* prevSegmentElement() override;
    virtual String accessibleExtraInfo() const override;
    virtual Shape shape() const override;
    virtual void computeUp() { _usesAutoUp = false; _up = true; }

    bool isFullMeasureRest() const { return _durationType == DurationType::V_MEASURE; }
    virtual void removeMarkings(bool keepTremolo = false);

    bool isBefore(const ChordRest*) const;

    void undoAddAnnotation(EngravingItem*);

    virtual double intrinsicMag() const = 0;
};
} // namespace mu::engraving
#endif
