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

#ifndef __INSTRUMENTNAME_H__
#define __INSTRUMENTNAME_H__

#include "textbase.h"

namespace mu::engraving {
enum class InstrumentNameType : char {
    LONG, SHORT
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

    InstrumentNameType _instrumentNameType;
    int _layoutPos { 0 };
    SysStaff* _sysStaff { nullptr };

public:
    InstrumentName(System*);

    InstrumentName* clone() const override { return new InstrumentName(*this); }

    int layoutPos() const { return _layoutPos; }
    void setLayoutPos(int val) { _layoutPos = val; }

    String instrumentNameTypeName() const;
    InstrumentNameType instrumentNameType() const { return _instrumentNameType; }
    void setInstrumentNameType(InstrumentNameType v);
    void setInstrumentNameType(const String& s);

    System* system() const { return toSystem(explicitParent()); }

    SysStaff* sysStaff() const { return _sysStaff; }
    void setSysStaff(SysStaff* s) { _sysStaff = s; }

    double spatium() const override;

    Fraction playTick() const override;
    bool isEditable() const override { return false; }
    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;
};
} // namespace mu::engraving
#endif
