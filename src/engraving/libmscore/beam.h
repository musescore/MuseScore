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

#include "layout/v0/beamtremololayout.h"
#include "engravingitem.h"
#include "durationtype.h"
#include "property.h"

namespace mu::engraving::layout::v0 {
class BeamLayout;
}

namespace mu::engraving {
class Chord;
class ChordRest;
class Factory;
class Skyline;
class System;
class Beam;
enum class ActionIconType;
enum class SpannerSegmentType;

//---------------------------------------------------------
//   BeamFragment
//    position of primary beam
//    idx 0 - DirectionV::AUTO or DirectionV::DOWN
//        1 - DirectionV::UP
//---------------------------------------------------------

struct BeamFragment {
    double py1[2];
    double py2[2];
};

class BeamSegment
{
    OBJECT_ALLOCATOR(engraving, BeamSegment)
public:
    mu::LineF line;
    int level = 0;
    bool above = false; // above level 0 or below? (meaningless for level 0)
    Fraction startTick;
    Fraction endTick;
    bool isBeamlet = false;
    bool isBefore = false;

    Shape shape() const;
    EngravingItem* parentElement;

    BeamSegment(EngravingItem* b)
        : parentElement(b) {}
};

struct TremAnchor {
    ChordRest* chord1 = nullptr;
    double y1 = 0.;
    double y2 = 0.;
};

//---------------------------------------------------------
//   @@ Beam
//---------------------------------------------------------

class Beam final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Beam)
    DECLARE_CLASSOF(ElementType::BEAM)

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

    void spatiumChanged(double /*oldValue*/, double /*newValue*/) override;

    void reset() override;

    System* system() const { return toSystem(explicitParent()); }

    PointF chordBeamAnchor(const ChordRest* chord, layout::v0::BeamTremoloLayout::ChordBeamAnchorType anchorType) const;
    double chordBeamAnchorY(const ChordRest* chord) const;

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

    bool hasAllRests();

    Shape shape() const override;

    const std::vector<TremAnchor>& tremAnchors() const { return _tremAnchors; }

    const std::vector<BeamFragment*>& beamFragments() const { return fragments; }
    void addBeamFragment(BeamFragment* f) { fragments.push_back(f); }

private:

    friend class layout::v0::BeamLayout;
    friend class Factory;
    friend class BeamSegment;
    Beam(System* parent);
    Beam(const Beam&);

    void initBeamEditData(EditData& ed);

    static constexpr std::array _maxSlopes = { 0, 1, 2, 3, 4, 5, 6, 7 };

    void addChordRest(ChordRest* a);
    void removeChordRest(ChordRest* a);

    const Chord* findChordWithCustomStemDirection() const;
    void setTremAnchors();

    std::vector<ChordRest*> _elements;          // must be sorted by tick
    std::vector<BeamSegment*> _beamSegments;
    DirectionV _direction    { DirectionV::AUTO };

    bool _up                { true };

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
    layout::v0::BeamTremoloLayout _layoutInfo;

    // for tabs
    bool _isBesideTabStaff  { false };
    StaffType const* _tab         { nullptr };

    std::vector<BeamFragment*> fragments;       // beam splits across systems

    mutable int _id         { 0 };          // used in read()/write()

    int _minMove             { 0 };                // set in layout1()
    int _maxMove             { 0 };

    bool _noSlope = false;
    double _slope             { 0.0 };

    std::vector<int> _notes;
    std::vector<TremAnchor> _tremAnchors;
};
} // namespace mu::engraving
#endif
