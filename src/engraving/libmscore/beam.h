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

#ifndef __BEAM_H__
#define __BEAM_H__

#include "engravingitem.h"
#include "durationtype.h"
#include "property.h"

namespace mu::engraving {
class Factory;
class ChordRest;
class MuseScoreView;
class Chord;
class System;
class Skyline;

enum class ActionIconType;
enum class SpannerSegmentType;

struct BeamFragment;

struct BeamSegment {
    mu::LineF line;
    int level;
    bool above; // above level 0 or below? (meaningless for level 0)
};

//---------------------------------------------------------
//   @@ Beam
//---------------------------------------------------------

class Beam final : public EngravingItem
{
    OBJECT_ALLOC(engraving, Beam)

    std::vector<ChordRest*> _elements;          // must be sorted by tick
    std::vector<BeamSegment*> _beamSegments;
    DirectionV _direction    { DirectionV::AUTO };

    bool _up                { true };
    bool _distribute        { false };                    // equal spacing of elements

    bool _userModified[2]   { false };                // 0: auto/down  1: up
    bool _isGrace           { false };
    bool _cross             { false };

    double _grow1            { 1.0f };                     // define "feather" beams
    double _grow2            { 1.0f };
    double _beamDist         { 0.0f };
    int _beamSpacing        { 3 }; // how far apart beams are spaced in quarter spaces
    double _beamWidth        { 0.0f }; // how wide each beam is
    mu::PointF _startAnchor;
    mu::PointF _endAnchor;

    // for tabs
    bool _isBesideTabStaff  { false };
    StaffType const* _tab         { nullptr };

    std::vector<BeamFragment*> fragments;       // beam splits across systems

    mutable int _id         { 0 };          // used in read()/write()

    int _minMove             { 0 };                // set in layout1()
    int _maxMove             { 0 };
    TDuration _maxDuration;

    bool _noSlope = false;
    double _slope             { 0.0 };

    std::vector<int> _notes;

    friend class Factory;
    Beam(System* parent);
    Beam(const Beam&);

    int getMiddleStaffLine(ChordRest* startChord, ChordRest* endChord, int staffLines) const;
    int computeDesiredSlant(int startNote, int endNote, int middleLine, int dictator, int pointer) const;
    int isSlopeConstrained(int startNote, int endNote) const;
    int getMaxSlope() const;
    int getBeamCount(std::vector<ChordRest*> chordRests) const;
    void offsetBeamToRemoveCollisions(std::vector<ChordRest*> chordRests, int& dictator, int& pointer, const double startX,
                                      const double endX, bool isFlat, bool isStartDictator) const;
    void offsetBeamWithAnchorShortening(std::vector<ChordRest*> chordRests, int& dictator, int& pointer, int beamCount, int staffLines,
                                        bool isStartDictator, int stemLengthDictator, int stemLengthPointer) const;
    bool isBeamInsideStaff(int yPos, int staffLines, bool isDictator) const;
    int getOuterBeamPosOffset(int innerBeam, int beamCount, int staffLines) const;
    bool isValidBeamPosition(int yPos, bool isStart, bool isAscending, bool isFlat, int staffLines, int beamCount) const;
    bool is64thBeamPositionException(int& yPos, int staffLines) const;
    int findValidBeamOffset(int outer, int beamCount, int staffLines, bool isStart, bool isAscending, bool isFlat) const;
    void setValidBeamPositions(int& dictator, int& pointer, int beamCount, int staffLines, bool isStartDictator, bool isFlat,
                               bool isAscending);
    void addMiddleLineSlant(int& dictator, int& pointer, int beamCount, int middleLine, int interval, int desiredSlant);
    void add8thSpaceSlant(mu::PointF& dictatorAnchor, int dictator, int pointer, int beamCount, int interval, int middleLine, bool Flat);
    void extendStem(Chord* chord, double addition);
    bool calcIsBeamletBefore(Chord* chord, int i, int level, bool isAfter32Break, bool isAfter64Break) const;
    void createBeamSegment(Chord* startChord, Chord* endChord, int level);
    void createBeamletSegment(Chord* chord, bool isBefore, int level);
    void createBeamSegments(const std::vector<ChordRest*>& chordRests);
    void layout2(const std::vector<ChordRest*>& chordRests, SpannerSegmentType, int frag);
    void addChordRest(ChordRest* a);
    void removeChordRest(ChordRest* a);

    const Chord* findChordWithCustomStemDirection() const;

public:
    ~Beam();

    // Score Tree functions
    EngravingObject* scanParent() const override;

    Beam* clone() const override { return new Beam(*this); }
    mu::PointF pagePos() const override;      ///< position in page coordinates
    mu::PointF canvasPos() const override;    ///< position in page coordinates

    bool isEditable() const override { return true; }
    void startEdit(EditData&) override;
    void endEdit(EditData&) override;
    void editDrag(EditData&) override;

    Fraction tick() const override;
    Fraction rtick() const override;
    Fraction ticks() const;

    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;
    void spatiumChanged(double /*oldValue*/, double /*newValue*/) override;

    void reset() override;

    System* system() const { return toSystem(explicitParent()); }

    void layout1();
    void layout() override;

    enum class ChordBeamAnchorType {
        Start, End, Middle
    };

    double chordBeamAnchorX(const Chord* chord, ChordBeamAnchorType anchorType) const;
    double chordBeamAnchorY(const Chord* chord) const;
    PointF chordBeamAnchor(const Chord* chord, ChordBeamAnchorType anchorType) const;

    const std::vector<ChordRest*>& elements() const { return _elements; }
    void clear() { _elements.clear(); }
    bool empty() const { return _elements.empty(); }
    bool contains(const ChordRest* cr) const
    {
        return std::find(_elements.begin(), _elements.end(), cr) != _elements.end();
    }

    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;

    void move(const mu::PointF&) override;
    void draw(mu::draw::Painter*) const override;

    bool up() const { return _up; }
    void setUp(bool v) { _up = v; }
    void setId(int i) const { _id = i; }
    int id() const { return _id; }

    void setBeamDirection(DirectionV d);
    DirectionV beamDirection() const { return _direction; }

    void calcBeamBreaks(const ChordRest* chord, const ChordRest* prevChord, int level, bool& isBroken32, bool& isBroken64) const;

    //!Note Unfortunately we have no FEATHERED_BEAM_MODE for now int BeamMode enum, so we'll handle this locally
    void setAsFeathered(const bool slower);
    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    double growLeft() const { return _grow1; }
    double growRight() const { return _grow2; }
    void setGrowLeft(double val) { _grow1 = val; }
    void setGrowRight(double val) { _grow2 = val; }

    bool distribute() const { return _distribute; }
    void setDistribute(bool val) { _distribute = val; }

    bool userModified() const;
    void setUserModified(bool val);

    PairF beamPos() const;
    void setBeamPos(const PairF& bp);

    double beamDist() const { return _beamDist; }

    bool noSlope() const { return _noSlope; }
    void setNoSlope(bool b);

    inline const mu::PointF startAnchor() const { return _startAnchor; }
    inline const mu::PointF endAnchor() const { return _endAnchor; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    void setIsGrace(bool val) { _isGrace = val; }
    bool cross() const { return _cross; }

    void addSkyline(Skyline&);

    void triggerLayout() const override;

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 3; }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::MIDDLE; }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;

    static ActionIconType actionIconTypeForBeamMode(BeamMode);

    mu::RectF drag(EditData&) override;
    bool isMovable() const override;
    void startDrag(EditData&) override;

private:
    void initBeamEditData(EditData& ed);

    static constexpr std::array _maxSlopes = { 0, 1, 2, 3, 4, 5, 6, 7 };
};
} // namespace mu::engraving
#endif
