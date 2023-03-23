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
#include "barlinerw.h"

#include "../../libmscore/barline.h"
#include "../../libmscore/factory.h"
#include "../../libmscore/score.h"
#include "../../libmscore/chord.h"
#include "../../libmscore/articulation.h"
#include "../../libmscore/symbol.h"
#include "../../libmscore/image.h"

#include "../../types/typesconv.h"

#include "../xmlreader.h"

#include "readcontext.h"
#include "articulationrw.h"
#include "engravingitemrw.h"
#include "symbolrw.h"
#include "imagerw.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void BarLineRW::read(BarLine* b, XmlReader& e, ReadContext& ctx)
{
    b->resetProperty(Pid::BARLINE_SPAN);
    b->resetProperty(Pid::BARLINE_SPAN_FROM);
    b->resetProperty(Pid::BARLINE_SPAN_TO);

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            b->setBarLineType(TConv::fromXml(e.readAsciiText(), BarLineType::NORMAL));
        } else if (tag == "span") {
            b->setSpanStaff(e.readBool());
        } else if (tag == "spanFromOffset") {
            b->setSpanFrom(e.readInt());
        } else if (tag == "spanToOffset") {
            b->setSpanTo(e.readInt());
        } else if (tag == "Articulation") {
            Articulation* a = Factory::createArticulation(ctx.score()->dummy()->chord());
            ArticulationRW::read(a, e, ctx);
            b->add(a);
        } else if (tag == "Symbol") {
            Symbol* s = Factory::createSymbol(b);
            s->setTrack(b->track());
            SymbolRW::read(s, e, ctx);
            b->add(s);
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                Image* image = Factory::createImage(b);
                image->setTrack(b->track());
                ImageRW::read(image, e, ctx);
                b->add(image);
            }
        } else if (!EngravingItemRW::readProperties(b, e, ctx)) {
            e.unknown();
        }
    }
}
