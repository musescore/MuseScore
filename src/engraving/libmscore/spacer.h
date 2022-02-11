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

#ifndef __SPACER_H__
#define __SPACER_H__

#include "engravingitem.h"
#include "infrastructure/draw/painterpath.h"

namespace mu::engraving {
class Factory;
}

namespace Ms {
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
    SpacerType _spacerType;
    Millimetre _gap;

    mu::PainterPath path;

    friend class mu::engraving::Factory;
    Spacer(Measure* parent);
    Spacer(const Spacer&);

    void layout0();

public:

    Spacer* clone() const override { return new Spacer(*this); }
    Measure* measure() const { return toMeasure(explicitParent()); }

    SpacerType spacerType() const { return _spacerType; }
    void setSpacerType(SpacerType t) { _spacerType = t; }

    void write(XmlWriter&) const override;
    void read(XmlReader&) override;

    void draw(mu::draw::Painter*) const override;

    bool isEditable() const override { return true; }
    void startEditDrag(EditData&) override;
    void editDrag(EditData&) override;
    void spatiumChanged(qreal, qreal) override;

    void setGap(Millimetre sp);
    Millimetre gap() const { return _gap; }

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 1; }
    Grip initialEditModeGrip() const override { return Grip::START; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid id) const override;
};
}     // namespace Ms
#endif
