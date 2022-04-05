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

#include "text.h"

namespace Ms {
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
    InstrumentNameType _instrumentNameType;
    int _layoutPos { 0 };
    SysStaff* _sysStaff { nullptr };

public:
    InstrumentName(System*);

    InstrumentName* clone() const override { return new InstrumentName(*this); }

    int layoutPos() const { return _layoutPos; }
    void setLayoutPos(int val) { _layoutPos = val; }

    QString instrumentNameTypeName() const;
    InstrumentNameType instrumentNameType() const { return _instrumentNameType; }
    void setInstrumentNameType(InstrumentNameType v);
    void setInstrumentNameType(const QString& s);

    System* system() const { return toSystem(explicitParent()); }

    SysStaff* sysStaff() const { return _sysStaff; }
    void setSysStaff(SysStaff* s) { _sysStaff = s; }

    Fraction playTick() const override;
    bool isEditable() const override { return false; }
    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid) const override;
};
}     // namespace Ms
#endif
