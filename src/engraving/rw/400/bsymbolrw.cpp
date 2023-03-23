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
#include "bsymbolrw.h"

#include "../../libmscore/bsymbol.h"
#include "../../libmscore/mscore.h"
#include "../../libmscore/factory.h"
#include "../../libmscore/image.h"
#include "../../libmscore/symbol.h"

#include "../xmlreader.h"

#include "engravingitemrw.h"
#include "symbolrw.h"
#include "fsymbolrw.h"
#include "imagerw.h"

using namespace mu::engraving::rw400;

bool BSymbolRW::readProperties(BSymbol* sym, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag = e.name();

    if (EngravingItemRW::readProperties(sym, e, ctx)) {
        return true;
    } else if (tag == "systemFlag") {
        sym->setSystemFlag(e.readInt());
    } else if (tag == "Symbol") {
        Symbol* s = Factory::createSymbol(sym);
        SymbolRW::read(s, e, ctx);
        sym->add(s);
    } else if (tag == "FSymbol") {
        FSymbol* s = Factory::createFSymbol(sym);
        FSymbolRW::read(s, e, ctx);
        sym->add(s);
    } else if (tag == "Image") {
        if (MScore::noImages) {
            e.skipCurrentElement();
        } else {
            Image* image = new Image(sym);
            ImageRW::read(image, e, ctx);
            sym->add(image);
        }
    } else {
        return false;
    }
    return true;
}
