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
#ifndef MU_ENGRAVING_BEAMBASE_H
#define MU_ENGRAVING_BEAMBASE_H

#include "engravingitem.h"

namespace mu::engraving {
class Beam;
class TremoloTwoChord;

enum class BeamType : unsigned char {
    INVALID,
    BEAM,
    TREMOLO
};

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

enum class ChordBeamAnchorType : unsigned char {
    Start, End, Middle
};

class BeamSegment
{
    OBJECT_ALLOCATOR(engraving, BeamSegment)
public:
    LineF line;
    int level = 0;
    bool above = false; // above level 0 or below? (meaningless for level 0)
    Fraction startTick;
    Fraction endTick;
    bool isBeamlet = false;
    bool isBefore = false;

    Shape shape() const;
    EngravingItem* parentElement = nullptr;

    BeamSegment(EngravingItem* b);
};

class BeamBase : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, BeamBase)
    DECLARE_CLASSOF(ElementType::INVALID) // dummy

    M_PROPERTY2(int, crossStaffMove, setCrossStaffMove, 0)

public:

    virtual int maxCRMove() const = 0;
    virtual int minCRMove() const = 0;

    virtual PropertyValue getProperty(Pid propertyId) const override;
    virtual bool setProperty(Pid propertyId, const PropertyValue&) override;
    virtual PropertyValue propertyDefault(Pid propertyId) const override;

    int crossStaffIdx() const;
    int defaultCrossStaffIdx() const;
    bool acceptCrossStaffMove(int move) const;

    bool up() const { return m_up; }
    void setUp(bool v) { m_up = v; }

    bool userModified() const;
    void setUserModified(bool val);

    DirectionV direction() const { return m_direction; }
    void doSetDirection(DirectionV val) { m_direction = val; }
    virtual void setDirection(DirectionV v) = 0;

    inline int directionIdx() const { return (m_direction == DirectionV::AUTO || m_direction == DirectionV::DOWN) ? 0 : 1; }

    const std::vector<BeamSegment*>& beamSegments() const { return m_beamSegments; }
    std::vector<BeamSegment*>& beamSegments() { return m_beamSegments; }
    virtual void clearBeamSegments();

    const PointF& startAnchor() const { return m_startAnchor; }
    PointF& startAnchor() { return m_startAnchor; }
    void setStartAnchor(const PointF& p) { m_startAnchor = p; }
    const PointF& endAnchor() const { return m_endAnchor; }
    PointF& endAnchor() { return m_endAnchor; }
    void setEndAnchor(const PointF& p) { m_endAnchor = p; }

    void undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps = PropertyFlags::NOSTYLE) override;

    struct NotePosition {
        int line;
        staff_idx_t staff;
        NotePosition(int note, staff_idx_t staff)
            : line(note), staff(staff) {}
        NotePosition()
            : line(0), staff(0) {}

        inline bool operator<(const NotePosition& other) const
        {
            return staff < other.staff || (staff == other.staff && line < other.line);
        }

        inline bool operator>(const NotePosition& other) const
        {
            return other < *this;
        }

        inline bool operator<=(const NotePosition& other) const
        {
            return !(other < *this);
        }

        inline bool operator >=(const NotePosition& other) const
        {
            return !(*this < other);
        }

        inline bool operator==(const NotePosition& other) const
        {
            return line == other.line && staff == other.staff;
        }

        inline bool operator!=(const NotePosition& other) const
        {
            return line != other.line || staff != other.staff;
        }
    };

    enum class CrossStaffBeamPosition : signed char {
        INVALID,
        BETWEEN,
        ABOVE,
        BELOW
    };

    struct LayoutData : public EngravingItem::LayoutData {
        BeamType beamType = BeamType::INVALID;
        const Beam* beam = nullptr;
        const TremoloTwoChord* trem = nullptr;
        bool up = false;
        Fraction tick = Fraction(0, 1);
        double spatium = 0.;
        PointF startAnchor;
        PointF endAnchor;
        double slope = 0.;
        bool isGrace = false;
        int beamSpacing = 0;
        double beamDist = 0.0;
        double beamWidth = 0.0;
        std::vector<ChordRest*> elements;
        std::vector<NotePosition> notePositions;
        const StaffType* tab = nullptr;
        bool isBesideTabStaff = false;
        CrossStaffBeamPosition crossStaffBeamPos = CrossStaffBeamPosition::INVALID;
        const Chord* limitingChordAbove = nullptr; // <-
        const Chord* limitingChordBelow = nullptr; // <- For cross-staff spacing and centering

        void setAnchors(PointF startA, PointF endA) { startAnchor = startA; endAnchor = endA; }
        bool isValid() const override { return !(beamType == BeamType::INVALID); }
    };
    DECLARE_LAYOUTDATA_METHODS(BeamBase)

protected:
    BeamBase(const ElementType& type, EngravingItem* parent, ElementFlags flags = ElementFlag::NOTHING);
    BeamBase(const BeamBase&);
    std::vector<BeamSegment*> m_beamSegments;
    PointF m_startAnchor;
    PointF m_endAnchor;

private:
    bool m_up = true;
    bool m_userModified[2]{ false };    // 0: auto/down  1: up
    DirectionV m_direction = DirectionV::AUTO;
};
}

#endif // MU_ENGRAVING_BEAMBASE_H
