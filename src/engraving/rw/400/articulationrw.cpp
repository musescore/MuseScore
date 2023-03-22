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
#include "articulationrw.h"

#include "../../libmscore/articulation.h"

#include "../../types/typesconv.h"
#include "../../types/symnames.h"

#include "../xmlreader.h"
#include "../206/read206.h"

#include "readcontext.h"
#include "propertyrw.h"
#include "engravingitemrw.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void ArticulationRW::read(Articulation* a, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!readProperties(a, e, ctx)) {
            e.unknown();
        }
    }
}

bool ArticulationRW::readProperties(Articulation* a, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "subtype") {
        AsciiStringView s = e.readAsciiText();
        ArticulationTextType textType = TConv::fromXml(s, ArticulationTextType::NO_TEXT);
        if (textType != ArticulationTextType::NO_TEXT) {
            a->setTextType(textType);
        } else {
            SymId id = SymNames::symIdByName(s);
            if (id == SymId::noSym) {
                id = compat::Read206::articulationNames2SymId206(s); // compatibility hack for "old" 3.0 scores
            }
            if (id == SymId::noSym || s == "ornamentMordentInverted") {   // SMuFL < 1.30
                id = SymId::ornamentMordent;
            }

            String programVersion = ctx.mscoreVersion();
            if (!programVersion.isEmpty() && programVersion < u"3.6") {
                if (id == SymId::noSym || s == "ornamentMordent") {   // SMuFL < 1.30 and MuseScore < 3.6
                    id = SymId::ornamentShortTrill;
                }
            }
            a->setSymId(id);
        }
    } else if (tag == "channel") {
        a->setChannelName(e.attribute("name"));
        e.readNext();
    } else if (PropertyRW::readProperty(a, tag, e, ctx, Pid::ARTICULATION_ANCHOR)) {
    } else if (tag == "direction") {
        PropertyRW::readProperty(a, e, ctx, Pid::DIRECTION);
    } else if (tag == "ornamentStyle") {
        PropertyRW::readProperty(a, e, ctx, Pid::ORNAMENT_STYLE);
    } else if (tag == "play") {
        a->setPlayArticulation(e.readBool());
    } else if (tag == "offset") {
        if (ctx.mscVersion() >= 400) {
            EngravingItemRW::readProperties(a, e, ctx);
        } else {
            e.skipCurrentElement();       // ignore manual layout in older scores
        }
    } else if (EngravingItemRW::readProperties(a, e, ctx)) {
    } else {
        return false;
    }
    return true;
}
