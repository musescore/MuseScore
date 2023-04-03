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
#include "mmrestrw.h"

#include "../../libmscore/mmrest.h"
#include "../../libmscore/symbol.h"
#include "../../libmscore/mscore.h"
#include "../../libmscore/image.h"
#include "../../libmscore/factory.h"

#include "../xmlreader.h"

#include "tread.h"
#include "propertyrw.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void MMRestRW::read(MMRest* r, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Symbol") {
            Symbol* s = new Symbol(r);
            s->setTrack(r->track());
            TRead::read(s, e, ctx);
            r->add(s);
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                Image* image = new Image(r);
                image->setTrack(r->track());
                TRead::read(image, e, ctx);
                r->add(image);
            }
        } else if (tag == "NoteDot") {
            NoteDot* dot = Factory::createNoteDot(r);
            TRead::read(dot, e, ctx);
            r->add(dot);
        } else if (PropertyRW::readStyledProperty(r, tag, e, ctx)) {
        } else if (readProperties(r, e, ctx)) {
        } else {
            e.unknown();
        }
    }
}

bool MMRestRW::readProperties(MMRest* r, XmlReader& xml, ReadContext& ctx)
{
    const AsciiStringView tag(xml.name());
    if (tag == "mmRestNumberVisible") {
        r->setProperty(Pid::MMREST_NUMBER_VISIBLE, xml.readBool());
    } else if (TRead::readProperties(r, xml, ctx)) {
    } else {
        return false;
    }
    return true;
}
