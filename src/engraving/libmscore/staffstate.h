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

#ifndef __STAFFSTATE_H__
#define __STAFFSTATE_H__

#include "engravingitem.h"
#include "instrument.h"
#include "infrastructure/draw/painterpath.h"

namespace Ms {
enum class StaffStateType : char {
    INSTRUMENT,
    TYPE,
    VISIBLE,
    INVISIBLE
};

//---------------------------------------------------------
//   @@ StaffState
//---------------------------------------------------------

class StaffState final : public EngravingItem
{
    StaffStateType _staffStateType { StaffStateType::INVISIBLE };
    qreal lw { 0.0 };
    mu::PainterPath path;

    Instrument* _instrument { nullptr };

    friend class mu::engraving::Factory;
    StaffState(EngravingItem* parent);
    StaffState(const StaffState&);

    void draw(mu::draw::Painter*) const override;
    void layout() override;

public:

    ~StaffState();

    StaffState* clone() const override { return new StaffState(*this); }

    void setStaffStateType(const QString&);
    void setStaffStateType(StaffStateType st) { _staffStateType = st; }
    StaffStateType staffStateType() const { return _staffStateType; }
    QString staffStateTypeName() const;

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    void write(XmlWriter&) const override;
    void read(XmlReader&) override;

    Instrument* instrument() const { return _instrument; }
    void setInstrument(const Instrument* i) { *_instrument = *i; }
    void setInstrument(const Instrument&& i) { *_instrument = i; }
    Segment* segment() { return (Segment*)explicitParent(); }
};
}     // namespace Ms
#endif
