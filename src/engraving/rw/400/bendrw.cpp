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
#include "bendrw.h"

#include "../../libmscore/bend.h"

#include "../xmlreader.h"

#include "engravingitemrw.h"
#include "propertyrw.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void BendRW::read(Bend* b, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (PropertyRW::readStyledProperty(b, tag, e, ctx)) {
        } else if (tag == "point") {
            PitchValue pv;
            pv.time    = e.intAttribute("time");
            pv.pitch   = e.intAttribute("pitch");
            pv.vibrato = e.intAttribute("vibrato");
            b->addPoint(pv);
            e.readNext();
        } else if (tag == "play") {
            b->setPlayBend(e.readBool());
        } else if (!EngravingItemRW::readProperties(b, e, ctx)) {
            e.unknown();
        }
    }
}
