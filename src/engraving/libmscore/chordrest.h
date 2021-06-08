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
#include "symbol.h"
#include "duration.h"
#include "beam.h"
#include "shape.h"
#include "measure.h"

namespace Ms {
enum class CrossMeasure : signed char {
    UNKNOWN = -1,
    NONE = 0,
    FIRST,
    SECOND
};

class Score;
class Measure;
class Tuplet;
class Segment;
class Slur;
class Articulation;
class Lyrics;
class TabDurationSymbol;
class Spanner;
enum class SegmentType;

//-------------------------------------------------------------------
//   ChordRest
//    Virtual base class. Chords and rests can be part of a beam
//
//-------------------------------------------------------------------

class ChordRest : public DurationElement
{
    ElementList _el;
    TDuration _durationType;
    int _staffMove;           // -1, 0, +1, used for crossbeaming

    void processSiblings(std::function<void(Element*)> func);

protected:
    std::vector<Lyrics*> _lyrics;
    TabDurationSymbol* _tabDur;           // stores a duration symbol in tablature staves

    Beam* _beam;
    Beam::Mode _beamMode;
    bool _up;                             // actual stem direction
    bool _small;
    bool _melismaEnd;

    // CrossMeasure: combine 2 tied notes if across a bar line and can be combined in a single duration
    CrossMeasure _crossMeasure;           ///< 0: no cross-measure modification; 1: 1st note of a mod.; -1: 2nd note
    TDuration _crossMeasureTDur;          ///< the total Duration type of the combined notes

public:
    ChordRest(Score*);
    ChordRest(const ChordRest&, bool link = false);
    ChordRest& operator=(const ChordRest&) = delete;
    ~ChordRest();

    // Score Tree functions
    virtual ScoreElement* treeParent() const override;
    virtual ScoreElement* treeChild(int idx) const override;
    virtual int treeChildCount() const override;

    virtual Element* drop(EditData&) override;
    virtual void undoUnlink() override;

    virtual Segment* segment() const { return (Segment*)parent(); }

    virtual void writeProperties(XmlWriter& xml) const override;
    virtual bool readProperties(XmlReader&) override;
    virtual void readAddConnector(ConnectorInfoReader* info, bool pasteMode) override;

    void setBeamMode(Beam::Mode m) { _beamMode = m; }
    void undoSetBeamMode(Beam::Mode m);
    Beam::Mode beamMode() const { return _beamMode; }

    void setBeam(Beam* b);
    virtual Beam* beam() const final { return !(measure() && measure()->stemless(staffIdx())) ? _beam : nullptr; }
    int beams() const { return _durationType.hooks(); }
    virtual qreal upPos()   const = 0;
    virtual qreal downPos() const = 0;

    int line(bool up) const { return up ? upLine() : downLine(); }
    int line() const { return _up ? upLine() : downLine(); }
    virtual int upLine() const = 0;
    virtual int downLine() const = 0;
    virtual mu::PointF stemPos() const = 0;
    virtual qreal stemPosX() const = 0;
    virtual mu::PointF stemPosBeam() const = 0;
    virtual qreal rightEdge() const = 0;

    bool up() const { return _up; }
    void setUp(bool val) { _up = val; }

    bool small() const { return _small; }
    void setSmall(bool val);
    void undoSetSmall(bool val);

    int staffMove() const { return _staffMove; }
    void setStaffMove(int val) { _staffMove = val; }
    virtual int vStaffIdx() const override { return staffIdx() + _staffMove; }

    const TDuration durationType() const
    {
        return _crossMeasure == CrossMeasure::FIRST
               ? _crossMeasureTDur : _durationType;
    }

    const TDuration actualDurationType() const { return _durationType; }
    void setDurationType(TDuration::DurationType t);
    void setDurationType(const QString& s);
    void setDurationType(const Fraction& ticks);
    void setDurationType(TDuration v);
    void setDots(int n) { _durationType.setDots(n); }
    int dots() const
    {
        return _crossMeasure == CrossMeasure::FIRST ? _crossMeasureTDur.dots()
               : (_crossMeasure == CrossMeasure::SECOND ? 0 : _durationType.dots());
    }

    int actualDots() const { return _durationType.dots(); }
    Fraction durationTypeTicks()
    {
        return _crossMeasure == CrossMeasure::FIRST ? _crossMeasureTDur.ticks() : _durationType.ticks();
    }

    QString durationUserName() const;

    virtual void setTrack(int val) override;

    const std::vector<Lyrics*>& lyrics() const { return _lyrics; }
    std::vector<Lyrics*>& lyrics() { return _lyrics; }
    Lyrics* lyrics(int verse, Placement) const;
    int lastVerse(Placement) const;
    bool isMelismaEnd() const;
    void setMelismaEnd(bool v);

    virtual void add(Element*) override;
    virtual void remove(Element*) override;
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

    virtual void localSpatiumChanged(qreal oldValue, qreal newValue) override;
    virtual QVariant getProperty(Pid propertyId) const override;
    virtual bool setProperty(Pid propertyId, const QVariant&) override;
    virtual QVariant propertyDefault(Pid) const override;
    bool isGrace() const;
    bool isGraceBefore() const;
    bool isGraceAfter() const;
    Breath* hasBreathMark() const;
    void writeBeam(XmlWriter& xml) const;
    Segment* nextSegmentAfterCR(SegmentType types) const;

    virtual void setScore(Score* s) override;
    Element* nextArticulationOrLyric(Element* e);
    Element* prevArticulationOrLyric(Element* e);
    virtual Element* nextElement() override;
    virtual Element* prevElement() override;
    Element* lastElementBeforeSegment();
    virtual Element* nextSegmentElement() override;
    virtual Element* prevSegmentElement() override;
    virtual QString accessibleExtraInfo() const override;
    virtual Shape shape() const override;
    virtual void layoutStem1() {}
    virtual void computeUp() { _up = true; }

    bool isFullMeasureRest() const { return _durationType == TDuration::DurationType::V_MEASURE; }
    virtual void removeMarkings(bool keepTremolo = false);

    bool isBefore(const ChordRest*) const;

    void undoAddAnnotation(Element*);
};
}     // namespace Ms
#endif
