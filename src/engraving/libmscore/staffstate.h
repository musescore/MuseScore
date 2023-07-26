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
#include "draw/types/painterpath.h"

namespace mu::engraving {
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
    OBJECT_ALLOCATOR(engraving, StaffState)
    DECLARE_CLASSOF(ElementType::STAFF_STATE)

public:

    ~StaffState();

    StaffState* clone() const override { return new StaffState(*this); }

    void setStaffStateType(const String&);
    void setStaffStateType(StaffStateType st) { m_staffStateType = st; }
    StaffStateType staffStateType() const { return m_staffStateType; }
    String staffStateTypeName() const;

    const mu::draw::PainterPath& path() const { return m_path; }
    void setPath(const mu::draw::PainterPath& p) { m_path = p; }

    double lw() const { return m_lw; }
    void setLw(double w) { m_lw = w; }

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    Instrument* instrument() const { return m_instrument; }
    void setInstrument(const Instrument* i) { *m_instrument = *i; }
    void setInstrument(const Instrument&& i) { *m_instrument = i; }
    Segment* segment() { return (Segment*)explicitParent(); }

private:

    friend class Factory;
    StaffState(EngravingItem* parent);
    StaffState(const StaffState&);

    void draw(mu::draw::Painter*) const override;

    StaffStateType m_staffStateType = StaffStateType::INVISIBLE;
    double m_lw = 0.0;
    mu::draw::PainterPath m_path;
    Instrument* m_instrument = nullptr;
};
} // namespace mu::engraving
#endif
