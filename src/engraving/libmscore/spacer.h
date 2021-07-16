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

#include "element.h"
#include "draw/painterpath.h"

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

class Spacer final : public Element
{
    SpacerType _spacerType;
    qreal _gap;

    mu::PainterPath path;

    void layout0();

public:
    Spacer(Score*);
    Spacer(const Spacer&);

    Spacer* clone() const override { return new Spacer(*this); }
    ElementType type() const override { return ElementType::SPACER; }
    Measure* measure() const { return toMeasure(parent()); }

    SpacerType spacerType() const { return _spacerType; }
    void setSpacerType(SpacerType t) { _spacerType = t; }

    void write(XmlWriter&) const override;
    void read(XmlReader&) override;

    void draw(mu::draw::Painter*) const override;

    void scanElements(void* data, void (* func)(void*, Element*), bool all=true) override;

    bool isEditable() const override { return true; }
    void startEditDrag(EditData&) override;
    void editDrag(EditData&) override;
    void spatiumChanged(qreal, qreal) override;

    void setGap(qreal sp);
    qreal gap() const { return _gap; }

    EditBehavior normalModeEditBehavior() const override { return EditBehavior::Edit; }
    int gripsCount() const override { return 1; }
    Grip initialEditModeGrip() const override { return Grip::START; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid id) const override;
};
}     // namespace Ms
#endif
