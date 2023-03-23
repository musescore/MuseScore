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
#include "symbolrw.h"

#include "../../libmscore/symbol.h"
#include "../../libmscore/mscore.h"
#include "../../libmscore/image.h"

#include "../../types/symnames.h"

#include "../xmlreader.h"

#include "readcontext.h"
#include "bsymbolrw.h"
#include "imagerw.h"

#include "log.h"

using namespace mu::engraving::rw400;

void SymbolRW::read(Symbol* sym, XmlReader& e, ReadContext& ctx)
{
    PointF pos;
    SymId symId = SymId::noSym;
    String fontName;
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "name") {
            String val(e.readText());
            symId = SymNames::symIdByName(val);
            if (val != "noSym" && symId == SymId::noSym) {
                // if symbol name not found, fall back to user names
                // TODO: does it make sense? user names are probably localized
                symId = SymNames::symIdByUserName(val);
                if (symId == SymId::noSym) {
                    LOGD("unknown symbol <%s>, falling back to no symbol", muPrintable(val));
                    // set a default symbol, or layout() will crash
                    symId = SymId::noSym;
                }
            }
            sym->setSym(symId);
        } else if (tag == "font") {
            fontName = e.readText();
        } else if (tag == "Symbol") {
            Symbol* s = new Symbol(sym);
            SymbolRW::read(s, e, ctx);
            sym->add(s);
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                Image* image = new Image(sym);
                ImageRW::read(image, e, ctx);
                sym->add(image);
            }
        } else if (tag == "small" || tag == "subtype") {    // obsolete
            e.skipCurrentElement();
        } else if (!BSymbolRW::readProperties(sym, e, ctx)) {
            e.unknown();
        }
    }

    std::shared_ptr<IEngravingFont> scoreFont = nullptr;
    if (!fontName.empty()) {
        scoreFont = ctx.engravingFonts()->fontByName(fontName.toStdString());
    }

    sym->setPos(PointF());
    sym->setSym(symId, scoreFont);
}
