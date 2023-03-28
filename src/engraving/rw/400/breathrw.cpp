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
#include "breathrw.h"

#include "../../libmscore/breath.h"

#include "../../types/symnames.h"

#include "../xmlreader.h"

#include "engravingitemrw.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void BreathRW::read(Breath* b, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {                 // obsolete
            SymId symId = SymId::noSym;
            switch (e.readInt()) {
            case 0:
            case 1:
                symId = SymId::breathMarkComma;
                break;
            case 2:
                symId = SymId::caesuraCurved;
                break;
            case 3:
                symId = SymId::caesura;
                break;
            }
            b->setSymId(symId);
        } else if (tag == "symbol") {
            b->setSymId(SymNames::symIdByName(e.readAsciiText()));
        } else if (tag == "pause") {
            b->setPause(e.readDouble());
        } else if (!EngravingItemRW::readProperties(b, e, ctx)) {
            e.unknown();
        }
    }
}
