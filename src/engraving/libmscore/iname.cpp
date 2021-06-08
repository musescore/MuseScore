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

#include "score.h"
#include "iname.h"
#include "measure.h"
#include "staff.h"
#include "system.h"
#include "part.h"
#include "undo.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   longInstrumentStyle
//---------------------------------------------------------

static const ElementStyle longInstrumentStyle {
};

//---------------------------------------------------------
//   shortInstrumentStyle
//---------------------------------------------------------

static const ElementStyle shortInstrumentStyle {
};

//---------------------------------------------------------
//   InstrumentName
//---------------------------------------------------------

InstrumentName::InstrumentName(Score* s)
    : TextBase(s, Tid::INSTRUMENT_LONG, ElementFlag::NOTHING)
{
    setFlag(ElementFlag::MOVABLE, false);
    setInstrumentNameType(InstrumentNameType::LONG);
}

//---------------------------------------------------------
//   instrumentNameTypeName
//---------------------------------------------------------

QString InstrumentName::instrumentNameTypeName() const
{
    return instrumentNameType() == InstrumentNameType::SHORT ? "short" : "long";
}

//---------------------------------------------------------
//   setInstrumentNameType
//---------------------------------------------------------

void InstrumentName::setInstrumentNameType(const QString& s)
{
    if (s == "short") {
        setInstrumentNameType(InstrumentNameType::SHORT);
    } else if (s == "long") {
        setInstrumentNameType(InstrumentNameType::LONG);
    } else {
        qDebug("InstrumentName::setSubtype: unknown <%s>", qPrintable(s));
    }
}

//---------------------------------------------------------
//   setInstrumentNameType
//---------------------------------------------------------

void InstrumentName::setInstrumentNameType(InstrumentNameType st)
{
    _instrumentNameType = st;
    if (st == InstrumentNameType::SHORT) {
        setTid(Tid::INSTRUMENT_SHORT);
        initElementStyle(&shortInstrumentStyle);
    } else {
        setTid(Tid::INSTRUMENT_LONG);
        initElementStyle(&longInstrumentStyle);
    }
}

//---------------------------------------------------------
//   playTick
//---------------------------------------------------------

Fraction InstrumentName::playTick() const
{
    // Instrument names always have a tick value of zero, so play from the start of the first measure in the system that the instrument name belongs to.
    const auto sys = system();
    if (sys) {
        const auto firstMeasure = sys->firstMeasure();
        if (firstMeasure) {
            return firstMeasure->tick();
        }
    }

    return tick();
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant InstrumentName::getProperty(Pid id) const
{
    switch (id) {
    case Pid::INAME_LAYOUT_POSITION:
        return _layoutPos;
    default:
        return TextBase::getProperty(id);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool InstrumentName::setProperty(Pid id, const QVariant& v)
{
    bool rv = true;
    switch (id) {
    case Pid::INAME_LAYOUT_POSITION:
        _layoutPos = v.toInt();
        break;
    case Pid::VISIBLE:
    case Pid::COLOR:
        // not supported
        break;
    default:
        rv = TextBase::setProperty(id, v);
        break;
    }
    return rv;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant InstrumentName::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::INAME_LAYOUT_POSITION:
        return 0;
    default:
        return TextBase::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void InstrumentName::scanElements(void* data, void (* func)(void*, Element*), bool all)
{
    if (all || sysStaff()->show()) {
        func(data, this);
    }
}
}
