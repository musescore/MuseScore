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

#ifndef MU_ENGRAVING_INSTRUMENTNAME_H
#define MU_ENGRAVING_INSTRUMENTNAME_H

#include "textbase.h"

namespace mu::engraving {
enum class InstrumentNameType : char {
    LONG, SHORT
};
enum class InstrumentNameRole : char {
    STAFF, PART, GROUP
};

class System;
class SysStaff;

//---------------------------------------------------------
//   InstrumentName
//---------------------------------------------------------

class InstrumentName final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, InstrumentName)
    DECLARE_CLASSOF(ElementType::INSTRUMENT_NAME)

public:
    InstrumentName(System*);

    InstrumentName* clone() const override { return new InstrumentName(*this); }

    InstrumentNameType instrumentNameType() const { return m_instrumentNameType; }
    void setInstrumentNameType(InstrumentNameType v);

    InstrumentNameRole instrumentNameRole() const { return m_instrumentNameRole; }
    void setInstrumentNameRole(InstrumentNameRole v) { m_instrumentNameRole = v; }

    System* system() const { return toSystem(explicitParent()); }

    SysStaff* sysStaff() const { return m_sysStaff; }
    void setSysStaff(SysStaff* s) { m_sysStaff = s; }

    double largestStaffSpatium() const;

    bool isEditable() const override { return false; }

    bool setProperty(Pid propertyId, const PropertyValue&) override;

    bool positionRelativeToNoteheadRest() const override { return false; }

    staff_idx_t effectiveStaffIdx() const override;

    struct LayoutData : public TextBase::LayoutData {
    public:
        int column() const { return m_column; }
        void setColumn(int v) { m_column = v; }
        staff_idx_t endIdxOfGroup() const { return m_endIdxOfGroup; }
        void setEndIdxOfGroup(staff_idx_t v) { m_endIdxOfGroup = v; }

    private:
        int m_column = 0;
        staff_idx_t m_endIdxOfGroup = muse::nidx; // one-after last spanned staff (for GROUP types)
    };
    DECLARE_LAYOUTDATA_METHODS(InstrumentName)

private:

    InstrumentNameType m_instrumentNameType = InstrumentNameType::LONG;
    InstrumentNameRole m_instrumentNameRole = InstrumentNameRole::PART;
    SysStaff* m_sysStaff = nullptr;
};
} // namespace mu::engraving
#endif
