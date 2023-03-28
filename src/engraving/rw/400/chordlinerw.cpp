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
#include "chordlinerw.h"

#include "../../libmscore/chordline.h"
#include "../../libmscore/score.h"

#include "../../types/typesconv.h"

#include "../xmlreader.h"

#include "engravingitemrw.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void ChordLineRW::read(ChordLine* l, XmlReader& e, ReadContext& ctx)
{
    l->setPath(PainterPath());
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Path") {
            PainterPath path;
            PointF curveTo;
            PointF p1;
            int state = 0;
            while (e.readNextStartElement()) {
                const AsciiStringView nextTag(e.name());
                if (nextTag == "Element") {
                    int type = e.intAttribute("type");
                    double x  = e.doubleAttribute("x");
                    double y  = e.doubleAttribute("y");
                    switch (PainterPath::ElementType(type)) {
                    case PainterPath::ElementType::MoveToElement:
                        path.moveTo(x, y);
                        break;
                    case PainterPath::ElementType::LineToElement:
                        path.lineTo(x, y);
                        break;
                    case PainterPath::ElementType::CurveToElement:
                        curveTo.rx() = x;
                        curveTo.ry() = y;
                        state = 1;
                        break;
                    case PainterPath::ElementType::CurveToDataElement:
                        if (state == 1) {
                            p1.rx() = x;
                            p1.ry() = y;
                            state = 2;
                        } else if (state == 2) {
                            path.cubicTo(curveTo, p1, PointF(x, y));
                            state = 0;
                        }
                        break;
                    }
                    e.skipCurrentElement();           //needed to go to next EngravingItem in Path
                } else {
                    e.unknown();
                }
            }
            l->setPath(path);
            l->setModified(true);
        } else if (tag == "subtype") {
            l->setChordLineType(TConv::fromXml(e.readAsciiText(), ChordLineType::NOTYPE));
        } else if (tag == "straight") {
            l->setStraight(e.readInt());
        } else if (tag == "wavy") {
            l->setWavy(e.readInt());
        } else if (tag == "lengthX") {
            l->setLengthX(e.readInt());
        } else if (tag == "lengthY") {
            l->setLengthY(e.readInt());
        } else if (tag == "offset" && l->score()->mscVersion() < 400) { // default positions has changed in 4.0 so ignore previous offset
            e.skipCurrentElement();
        } else if (!EngravingItemRW::readProperties(l, e, ctx)) {
            e.unknown();
        }
    }
}
