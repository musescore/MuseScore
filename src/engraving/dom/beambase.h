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
public:

    struct LayoutData : public EngravingItem::LayoutData {
        BeamType m_beamType = BeamType::INVALID;
        EngravingItem* m_element = nullptr;
        Beam* m_beam = nullptr;
        TremoloTwoChord* m_trem = nullptr;
        bool m_up = false;
        Fraction m_tick = Fraction(0, 1);
        double m_spatium = 0.;
        PointF m_startAnchor;
        PointF m_endAnchor;
        double m_slope = 0.;
        bool m_isGrace = false;
        int m_beamSpacing = 0;
        double m_beamDist = 0.0;
        double m_beamWidth = 0.0;
        std::vector<ChordRest*> m_elements;
        std::vector<int> m_notes;
        StaffType const* m_tab = nullptr;
        bool m_isBesideTabStaff = false;

        double beamDist() const { return m_beamDist; }
        double beamWidth() const { return m_beamWidth; }
        PointF startAnchor() const { return m_startAnchor; }
        PointF endAnchor() const { return m_endAnchor; }
        void setAnchors(PointF startAnchor, PointF endAnchor) { m_startAnchor = startAnchor; m_endAnchor = endAnchor; }
        bool isValid() const { return !(m_beamType == BeamType::INVALID); }
    };
    DECLARE_LAYOUTDATA_METHODS(BeamBase)

protected:
    BeamBase(const ElementType& type, EngravingItem* parent, ElementFlags flags = ElementFlag::NOTHING);
    BeamBase(const BeamBase&);
};
}

#endif // MU_ENGRAVING_BEAMBASE_H
