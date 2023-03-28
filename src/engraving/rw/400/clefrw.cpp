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
#include "clefrw.h"

#include "../../libmscore/clef.h"

#include "../../types/typesconv.h"

#include "../xmlreader.h"

#include "engravingitemrw.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void ClefRW::read(Clef* c, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "concertClefType") {
            c->setConcertClef(TConv::fromXml(e.readAsciiText(), ClefType::G));
        } else if (tag == "transposingClefType") {
            c->setTransposingClef(TConv::fromXml(e.readAsciiText(), ClefType::G));
        } else if (tag == "showCourtesyClef") {
            c->setShowCourtesy(e.readInt());
        } else if (tag == "forInstrumentChange") {
            c->setForInstrumentChange(e.readBool());
        } else if (!EngravingItemRW::readProperties(c, e, ctx)) {
            e.unknown();
        }
    }
    if (c->clefType() == ClefType::INVALID) {
        c->setClefType(ClefType::G);
    }
}
