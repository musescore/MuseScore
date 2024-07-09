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

enum class BeamType {
    INVALID,
    BEAM,
    TREMOLO
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

    void undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps = PropertyFlags::NOSTYLE) override;

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
        std::vector<ChordPosition> notes;
        const StaffType* tab = nullptr;
        bool isBesideTabStaff = false;
        CrossStaffBeamPosition crossStaffBeamPos = CrossStaffBeamPosition::INVALID;

        void setAnchors(PointF startA, PointF endA) { startAnchor = startA; endAnchor = endA; }
        bool isValid() const override { return !(beamType == BeamType::INVALID); }
    };
    DECLARE_LAYOUTDATA_METHODS(BeamBase)

protected:
    BeamBase(const ElementType& type, EngravingItem* parent, ElementFlags flags = ElementFlag::NOTHING);
    BeamBase(const BeamBase&);

    bool m_up = true;
};
}

#endif // MU_ENGRAVING_BEAMBASE_H
