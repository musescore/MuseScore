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

#ifndef __INAME_H__
#define __INAME_H__

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
    InstrumentName(Score*);

    InstrumentName* clone() const override { return new InstrumentName(*this); }
    ElementType type() const override { return ElementType::INSTRUMENT_NAME; }

    int layoutPos() const { return _layoutPos; }
    void setLayoutPos(int val) { _layoutPos = val; }

    QString instrumentNameTypeName() const;
    InstrumentNameType instrumentNameType() const { return _instrumentNameType; }
    void setInstrumentNameType(InstrumentNameType v);
    void setInstrumentNameType(const QString& s);

    System* system() const { return toSystem(parent()); }

    SysStaff* sysStaff() const { return _sysStaff; }
    void setSysStaff(SysStaff* s) { _sysStaff = s; }

    void scanElements(void* data, void (* func)(void*, Element*), bool all=true) override;

    Fraction playTick() const override;
    bool isEditable() const override { return false; }
    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid) const override;
};
}     // namespace Ms
#endif
