/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_ENGRAVING_CHORDREST_H
#define MU_ENGRAVING_CHORDREST_H

#include <functional>

#include "../types/types.h"

#include "durationelement.h"
#include "fermata.h"

namespace mu::engraving {
enum class CrossMeasure : signed char {
    UNKNOWN = -1,
    NONE = 0,
    FIRST,
    SECOND
};

class Articulation;
class BeamBase;
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
    DECLARE_CLASSOF(ElementType::INVALID) // dummy

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

    void setBeamMode(BeamMode m) { m_beamMode = m; }
    BeamMode beamMode() const { return m_beamMode; }

    void setBeam(Beam* b);
    void setBeamlet(BeamSegment* b);
    BeamSegment* beamlet() const { return m_beamlet; }

    virtual Beam* beam() const final;
    int beams() const { return m_durationType.hooks(); }
    virtual double upPos()   const = 0;
    virtual double downPos() const = 0;

    virtual PointF stemPos() const = 0;
    virtual double stemPosX() const = 0;
    virtual PointF stemPosBeam() const = 0;
    virtual double rightEdge() const = 0;

    bool isSmall() const { return m_isSmall; }
    void setSmall(bool val) { m_isSmall = val; }
    void undoSetSmall(bool val);

    int staffMove() const { return m_staffMove; }
    void setStaffMove(int val) { m_staffMove = val; }
    int storedStaffMove() const { return m_storedStaffMove; }
    staff_idx_t vStaffIdx() const override { return staffIdx() + m_staffMove; }
    void checkStaffMoveValidity();

    const TDuration durationType() const
    {
        return m_crossMeasure == CrossMeasure::FIRST ? m_crossMeasureTDur : m_durationType;
    }

    const TDuration actualDurationType() const { return m_durationType; }
    void setDurationType(DurationType t);
    void setDurationType(const Fraction& ticks);
    void setDurationType(TDuration v);
    void setDots(int n) { m_durationType.setDots(n); }
    int dots() const
    {
        return m_crossMeasure == CrossMeasure::FIRST
               ? m_crossMeasureTDur.dots()
               : (m_crossMeasure == CrossMeasure::SECOND ? 0 : m_durationType.dots());
    }

    int actualDots() const { return m_durationType.dots(); }
    Fraction durationTypeTicks() const
    {
        return m_crossMeasure == CrossMeasure::FIRST ? m_crossMeasureTDur.ticks() : m_durationType.ticks();
    }

    Fraction endTick() const { return tick() + actualTicks(); }

    String durationUserName() const;

    void setTrack(track_idx_t val) override;

    const std::vector<Lyrics*>& lyrics() const { return m_lyrics; }
    std::vector<Lyrics*>& lyrics() { return m_lyrics; }
    Lyrics* lyrics(int verse) const;
    Lyrics* lyrics(int verse, PlacementV) const;
    int lastVerse(PlacementV) const;
    bool isMelismaEnd() const;
    void setMelismaEnd(bool v);

    virtual void add(EngravingItem*) override;
    virtual void remove(EngravingItem*) override;
    void removeDeleteBeam(bool beamed);
    void replaceBeam(Beam* newBeam);

    const ElementList& el() const { return m_el; }

    //! TODO Look like a hack, see using
    void addFermata(Fermata* f) { addEl(f); }
    void removeFermata(Fermata* f) { removeEl(f); }
    //! --------------------------------

    Slur* slur(const ChordRest* secondChordRest = nullptr) const;

    CrossMeasure crossMeasure() const { return m_crossMeasure; }
    void setCrossMeasure(CrossMeasure val) { m_crossMeasure = val; }

    // the following two functions should not be used, unless absolutely necessary;
    // the cross-measure duration is best managed through setDuration() and crossMeasureSetup()
    TDuration crossMeasureDurationType() const { return m_crossMeasureTDur; }
    void setCrossMeasureDurationType(TDuration v) { m_crossMeasureTDur = v; }

    void undoChangeProperty(Pid id, const PropertyValue& newValue, PropertyFlags ps = PropertyFlags::NOSTYLE) override;
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
    virtual void computeUp();

    bool isFullMeasureRest() const { return m_durationType == DurationType::V_MEASURE; }
    virtual void removeMarkings(bool keepTremolo = false);

    bool isBefore(const EngravingItem*) const override;

    void undoAddAnnotation(EngravingItem*);

    virtual double intrinsicMag() const = 0;

    TabDurationSymbol* tabDur() const { return m_tabDur; }
    void setTabDur(TabDurationSymbol* s) { m_tabDur = s; }

    bool isBelowCrossBeam(const BeamBase* beamBase) const;

    bool hasFollowingJumpItem() const;
    bool hasPrecedingJumpItem() const;

    struct LayoutData : public DurationElement::LayoutData {
        ld_field<bool> up = { "[ChordRest] up", true }; // actual stem direction
    };
    DECLARE_LAYOUTDATA_METHODS(ChordRest)

    //! DEPRECATED ------
    void setUp(bool val) { mutldata()->up = val; }
    bool up() const { return ldata()->up; }
    //! -----------------

protected:

    void addEl(EngravingItem* e) { m_el.push_back(e); }
    bool removeEl(EngravingItem* e) { return m_el.remove(e); }
    void clearEls() { m_el.clear(); }

    std::vector<Lyrics*> m_lyrics;
    TabDurationSymbol* m_tabDur = nullptr;  // stores a duration symbol in tablature staves

    Beam* m_beam = nullptr;
    BeamSegment* m_beamlet = nullptr;
    BeamMode m_beamMode = BeamMode::INVALID;
    bool m_isSmall = false;
    bool m_melismaEnd = false;

    // CrossMeasure: combine 2 tied notes if across a bar line and can be combined in a single duration
    CrossMeasure m_crossMeasure = CrossMeasure::UNKNOWN;           ///< 0: no cross-measure modification; 1: 1st note of a mod.; -1: 2nd note
    TDuration m_crossMeasureTDur;          ///< the total Duration type of the combined notes

private:

    void processSiblings(std::function<void(EngravingItem*)> func);

    ElementList m_el;
    TDuration m_durationType;
    int m_staffMove = 0; // -1, 0, +1, used for crossbeaming
    int m_storedStaffMove = 0; // used to remember and re-apply staff move if needed
};
} // namespace mu::engraving
#endif
