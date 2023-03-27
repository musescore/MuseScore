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
#include "arpeggiorw.h"

#include "../../libmscore/arpeggio.h"

#include "../../types/typesconv.h"

#include "../xmlreader.h"

#include "engravingitemrw.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void ArpeggioRW::read(Arpeggio* a, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            a->setArpeggioType(TConv::fromXml(e.readAsciiText(), ArpeggioType::NORMAL));
        } else if (tag == "userLen1") {
            a->setUserLen1(e.readDouble() * a->spatium());
        } else if (tag == "userLen2") {
            a->setUserLen2(e.readDouble() * a->spatium());
        } else if (tag == "span") {
            a->setSpan(e.readInt());
        } else if (tag == "play") {
            a->setPlayArpeggio(e.readBool());
        } else if (tag == "timeStretch") {
            a->setStretch(e.readDouble());
        } else if (!EngravingItemRW::readProperties(a, e, ctx)) {
            e.unknown();
        }
    }
}
