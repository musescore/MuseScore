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
#include "fsymbolrw.h"

#include "../../libmscore/symbol.h"

#include "../xmlreader.h"
#include "bsymbolrw.h"

using namespace mu::engraving::rw400;

void FSymbolRW::read(FSymbol* sym, XmlReader& e, ReadContext& ctx)
{
    mu::draw::Font font = sym->font();
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "font") {
            font.setFamily(e.readText(), draw::Font::Type::Unknown);
        } else if (tag == "fontsize") {
            font.setPointSizeF(e.readDouble());
        } else if (tag == "code") {
            sym->setCode(e.readInt());
        } else if (!BSymbolRW::readProperties(sym, e, ctx)) {
            e.unknown();
        }
    }

    sym->setPos(PointF());
    sym->setFont(font);
}
