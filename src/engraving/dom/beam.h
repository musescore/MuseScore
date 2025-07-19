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

#pragma once

#include "beambase.h"
#include "engravingitem.h"
#include "property.h"

namespace mu::engraving {
class Chord;
class ChordRest;
class Factory;
class Skyline;
class System;
class Beam;
enum class ActionIconType : signed char;
enum class SpannerSegmentType : unsigned char;

struct TremAnchor {
    ChordRest* chord1 = nullptr;
    double y1 = 0.;
    double y2 = 0.;
};

//---------------------------------------------------------
//   @@ Beam
//---------------------------------------------------------

class Beam final : public BeamBase
{
    OBJECT_ALLOCATOR(engraving, Beam)
    DECLARE_CLASSOF(ElementType::BEAM)

public:
    ~Beam();

    // Score Tree functions
    EngravingObject* scanParent() const override;

    Beam* clone() const override { return new Beam(*this); }
    PointF pagePos() const override;      ///< position in page coordinates
    PointF canvasPos() const override;    ///< position in page coordinates

    bool isEditable() const override { return true; }
    void startEdit(EditData&) override;
    void editDrag(EditData&) override;

    Fraction tick() const override;
    Fraction rtick() const override;
    Fraction ticks() const;

    void spatiumChanged(double /*oldValue*/, double /*newValue*/) override;

    void reset() override;

    System* system() const { return toSystem(explicitParent()); }

    const std::vector<ChordRest*>& elements() const { return m_elements; }
    std::vector<ChordRest*>& elements() { return m_elements; }
    void clear() { m_elements.clear(); }
    bool empty() const { return m_elements.empty(); }
    bool contains(const ChordRest* cr) const
    {
        return std::find(m_elements.begin(), m_elements.end(), cr) != m_elements.end();
    }

    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;

    void move(const PointF&) override;

    void setId(int i) const { m_id = i; }
    int id() const { return m_id; }

    void setDirection(DirectionV d) override;

    void calcBeamBreaks(const ChordRest* chord, const ChordRest* prevChord, int level, bool& isBroken32, bool& isBroken64) const;

    //!Note Unfortunately we have no FEATHERED_BEAM_MODE for now int BeamMode enum, so we'll handle this locally
    void setAsFeathered(const bool slower);
    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    double growLeft() const { return m_growLeft; }
    double growRight() const { return m_growRight; }
    void setGrowLeft(double val) { m_growLeft = val; }
    void setGrowRight(double val) { m_growRight = val; }

    PairF beamPos() const;
    void setBeamPos(const PairF& bp);
    double beamDist() const { return m_beamDist; }
    void setBeamDist(double d) { m_beamDist = d; }
    double beamWidth() const { return m_beamWidth; }
    void setBeamWidth(double w) { m_beamWidth = w; }
    int beamSpacing() const { return m_beamSpacing; }
    void setBeamSpacing(int val) { m_beamSpacing = val; }

    bool noSlope() const { return m_noSlope; }
    void setNoSlope(bool b);

    double slope() const { return m_slope; }
    void computeAndSetSlope();
    void setSlope(double val) { m_slope = val; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    void setIsGrace(bool val) { m_isGrace = val; }
    bool isGrace() const { return m_isGrace; }

    bool cross() const { return m_cross; }
    void setCross(bool val) { m_cross = val; }

    bool fullCross() const { return m_fullCross; }
    void setFullCross(bool v) { m_fullCross = v; }

    int minCRMove() const override { return m_minCRMove; }
    void setMinMove(int val) { m_minCRMove = val; }
    int maxCRMove() const override { return m_maxCRMove; }
    void setMaxMove(int val) { m_maxCRMove = val; }

    void addSkyline(Skyline&);

    void triggerLayout() const override;

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 3; }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::MIDDLE; }
    std::vector<PointF> gripsPositions(const EditData&) const override;

    static ActionIconType actionIconTypeForBeamMode(BeamMode);

    RectF drag(EditData&) override;
    bool isMovable() const override;
    void startDrag(EditData&) override;

    bool hasAllRests();

    const std::vector<TremAnchor>& tremAnchors() const { return m_tremAnchors; }
    std::vector<TremAnchor>& tremAnchors() { return m_tremAnchors; }

    const std::vector<BeamFragment*>& beamFragments() const { return m_fragments; }
    std::vector<BeamFragment*>& beamFragments() { return m_fragments; }
    void addBeamFragment(BeamFragment* f) { m_fragments.push_back(f); }

    void clearBeamSegments() override;

    const StaffType* tab() const { return m_tab; }
    void setTab(const StaffType* t) { m_tab = t; }
    bool isBesideTabStaff() const { return m_isBesideTabStaff; }
    void setIsBesideTabStaff(bool val) { m_isBesideTabStaff = val; }

    const std::vector<NotePosition>& notePositions() const { return m_notePositions; }
    std::vector<NotePosition>& notePositions() { return m_notePositions; }

    const Chord* findChordWithCustomStemDirection() const;

    const BeamSegment* topLevelSegmentForElement(const ChordRest* element) const;

private:

    friend class Factory;
    friend class BeamSegment;
    Beam(System* parent);
    Beam(const Beam&);

    void initBeamEditData(EditData& ed);

    static constexpr std::array MAX_SLOPES = { 0, 1, 2, 3, 4, 5, 6, 7 };

    void addChordRest(ChordRest* a);
    void removeChordRest(ChordRest* a);

    std::vector<ChordRest*> m_elements;          // must be sorted by tick

    bool m_isGrace = false;
    bool m_cross = false;
    bool m_fullCross = false;

    double m_growLeft = 1.0;               // define "feather" beams
    double m_growRight = 1.0;
    double m_beamDist = 0.0;
    int m_beamSpacing = 3;              // how far apart beams are spaced in quarter spaces
    double m_beamWidth = 0.0;           // how wide each beam is

    // for tabs
    bool m_isBesideTabStaff = false;
    const StaffType* m_tab = nullptr;

    std::vector<BeamFragment*> m_fragments; // beam splits across systems

    mutable int m_id = 0;                // used in read()/write()

    int m_minCRMove = 0;                   // set in layout1()
    int m_maxCRMove = 0;
    int m_crossBeamPos = 0;

    bool m_noSlope = false;
    real_t m_slope = 0.0;

    std::vector<NotePosition> m_notePositions;
    std::vector<TremAnchor> m_tremAnchors;
};
} // namespace mu::engraving
