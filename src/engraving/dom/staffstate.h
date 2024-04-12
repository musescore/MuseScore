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

#ifndef MU_ENGRAVING_STAFFSTATE_H
#define MU_ENGRAVING_STAFFSTATE_H

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

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    Instrument* instrument() const { return m_instrument; }
    void setInstrument(const Instrument* i) { *m_instrument = *i; }
    void setInstrument(const Instrument&& i) { *m_instrument = i; }
    Segment* segment() { return (Segment*)explicitParent(); }

    struct LayoutData : public EngravingItem::LayoutData {
        double lw = 0.0;
        muse::draw::PainterPath path;
    };
    DECLARE_LAYOUTDATA_METHODS(StaffState)

private:

    friend class Factory;
    StaffState(EngravingItem* parent);
    StaffState(const StaffState&);

    StaffStateType m_staffStateType = StaffStateType::INVISIBLE;

    Instrument* m_instrument = nullptr;
};
} // namespace mu::engraving
#endif
