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
#include "restrw.h"

#include "../../libmscore/rest.h"
#include "../../libmscore/symbol.h"
#include "../../libmscore/mscore.h"
#include "../../libmscore/image.h"
#include "../../libmscore/factory.h"

#include "../xmlreader.h"

#include "symbolrw.h"
#include "imagerw.h"
#include "notedotrw.h"
#include "propertyrw.h"
#include "chordrestrw.h"
#include "engravingitemrw.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void RestRW::read(Rest* r, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Symbol") {
            Symbol* s = new Symbol(r);
            s->setTrack(r->track());
            SymbolRW::read(s, e, ctx);
            r->add(s);
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                Image* image = new Image(r);
                image->setTrack(r->track());
                ImageRW::read(image, e, ctx);
                r->add(image);
            }
        } else if (tag == "NoteDot") {
            NoteDot* dot = Factory::createNoteDot(r);
            NoteDotRW::read(dot, e, ctx);
            r->add(dot);
        } else if (PropertyRW::readStyledProperty(r, tag, e, ctx)) {
        } else if (ChordRestRW::readProperties(r, e, ctx)) {
        } else {
            e.unknown();
        }
    }
}

bool RestRW::readProperties(Rest* r, XmlReader& xml, ReadContext& ctx)
{
    return ChordRestRW::readProperties(r, xml, ctx);
}
