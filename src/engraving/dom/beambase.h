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
        std::vector<int> notes;
        const StaffType* tab = nullptr;
        bool isBesideTabStaff = false;

        void setAnchors(PointF startA, PointF endA) { startAnchor = startA; endAnchor = endA; }
        bool isValid() const override { return !(beamType == BeamType::INVALID); }
    };
    DECLARE_LAYOUTDATA_METHODS(BeamBase)

protected:
    BeamBase(const ElementType& type, EngravingItem* parent, ElementFlags flags = ElementFlag::NOTHING);
    BeamBase(const BeamBase&);
};
}

#endif // MU_ENGRAVING_BEAMBASE_H
