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

#ifndef MU_ENGRAVING_SPACER_H
#define MU_ENGRAVING_SPACER_H

#include "engravingitem.h"
#include "draw/types/painterpath.h"

namespace mu::engraving {
class Factory;

//---------------------------------------------------------
//   SpacerType
//---------------------------------------------------------

enum class SpacerType : char {
    UP, DOWN, FIXED
};

//-------------------------------------------------------------------
//   @@ Spacer
///    Vertical spacer element to adjust the distance of staves.
//-------------------------------------------------------------------

class Spacer final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Spacer)
    DECLARE_CLASSOF(ElementType::SPACER)

public:

    Spacer* clone() const override { return new Spacer(*this); }
    Measure* measure() const { return toMeasure(explicitParent()); }

    SpacerType spacerType() const { return m_spacerType; }
    void setSpacerType(SpacerType t) { m_spacerType = t; }

    int subtype() const override { return int(m_spacerType); }
    TranslatableString subtypeUserName() const override;

    bool isEditable() const override { return true; }
    void startEditDrag(EditData&) override;
    void editDrag(EditData&) override;

    void setGap(Spatium sp);
    Spatium gap() const { return m_gap; }

    double absoluteGap() const { return m_gap.toMM(spatium()).val(); }

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 1; }
    Grip initialEditModeGrip() const override { return Grip::START; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<PointF> gripsPositions(const EditData&) const override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    struct LayoutData : public EngravingItem::LayoutData {
        PainterPath path;
    };
    DECLARE_LAYOUTDATA_METHODS(Spacer)

private:

    friend class Factory;
    Spacer(Measure* parent);
    Spacer(const Spacer&);

    SpacerType m_spacerType = SpacerType::UP;
    Spatium m_gap;
};
} // namespace mu::engraving
#endif
