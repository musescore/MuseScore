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

#include "part.h"
#include "instrument.h"

namespace Ms {
namespace PluginAPI {
//---------------------------------------------------------
//   InstrumentListProperty
//---------------------------------------------------------

InstrumentListProperty::InstrumentListProperty(Part* p)
    : QQmlListProperty<Instrument>(p, p, &count, &at) {}

//---------------------------------------------------------
//   InstrumentListProperty::count
//---------------------------------------------------------

int InstrumentListProperty::count(QQmlListProperty<Instrument>* l)
{
    return static_cast<int>(static_cast<Part*>(l->data)->part()->instruments().size());
}

//---------------------------------------------------------
//   InstrumentListProperty::at
//---------------------------------------------------------

Instrument* InstrumentListProperty::at(QQmlListProperty<Instrument>* l, int i)
{
    Part* part = static_cast<Part*>(l->data);
    const Ms::InstrumentList& il = part->part()->instruments();

    if (i < 0 || i >= int(il.size())) {
        return nullptr;
    }

    Ms::Instrument* instr = std::next(il.begin(), i)->second;

    return customWrap<Instrument>(instr, part->part());
}

//---------------------------------------------------------
//   Part::instruments
//---------------------------------------------------------

InstrumentListProperty Part::instruments()
{
    return InstrumentListProperty(this);
}

//---------------------------------------------------------
//   Part::instrumentAtTick
//---------------------------------------------------------

Instrument* Part::instrumentAtTick(int tick)
{
    return customWrap<Instrument>(part()->instrument(Ms::Fraction::fromTicks(tick)), part());
}
} // namespace PluginAPI
} // namespace Ms
