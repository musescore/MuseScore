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

#include "part.h"

// api
#include "instrument.h"

using namespace mu::engraving::apiv1;

InstrumentListProperty::InstrumentListProperty(Part* p)
    : QQmlListProperty<Instrument>(p, p, &count, &at) {}

//---------------------------------------------------------
//   InstrumentListProperty::count
//---------------------------------------------------------

qsizetype InstrumentListProperty::count(QQmlListProperty<Instrument>* l)
{
    return static_cast<qsizetype>(static_cast<Part*>(l->data)->part()->instruments().size());
}

//---------------------------------------------------------
//   InstrumentListProperty::at
//---------------------------------------------------------

Instrument* InstrumentListProperty::at(QQmlListProperty<Instrument>* l, qsizetype i)
{
    Part* part = static_cast<Part*>(l->data);
    const mu::engraving::InstrumentList& il = part->part()->instruments();

    if (i < 0 || i >= int(il.size())) {
        return nullptr;
    }

    mu::engraving::Instrument* instr = std::next(il.begin(), i)->second;

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
    return customWrap<Instrument>(part()->instrument(mu::engraving::Fraction::fromTicks(tick)), part());
}
