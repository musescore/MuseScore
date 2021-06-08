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

#ifndef __LASSO_H__
#define __LASSO_H__

#include "element.h"

namespace Ms {
//---------------------------------------------------------
//   Lasso
//---------------------------------------------------------

class Lasso : public Element
{
public:
    Lasso(Score*);
    virtual Lasso* clone() const override { return new Lasso(*this); }
    ElementType type() const final { return ElementType::LASSO; }
    virtual void draw(mu::draw::Painter*) const override;
    virtual bool isEditable() const override { return true; }
    virtual void editDrag(EditData&) override;
    virtual void endDrag(EditData&) override {}

    virtual QVariant getProperty(Pid propertyId) const override;
    virtual bool setProperty(Pid propertyId, const QVariant&) override;

    int gripsCount() const override { return 8; }
    Grip initialEditModeGrip() const override { return Grip(7); }
    Grip defaultGrip() const override { return Grip(7); }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;
};
}     // namespace Ms
#endif
