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
#include "twrite.h"

#include "../../types/typesconv.h"
#include "../../types/symnames.h"

#include "../../libmscore/score.h"
#include "../../libmscore/masterscore.h"
#include "../../libmscore/factory.h"
#include "../../libmscore/linkedobjects.h"
#include "../../libmscore/mscore.h"
#include "../../libmscore/staff.h"

#include "../../libmscore/accidental.h"
#include "../../libmscore/actionicon.h"
#include "../../libmscore/ambitus.h"
#include "../../libmscore/arpeggio.h"
#include "../../libmscore/articulation.h"

#include "../../libmscore/bagpembell.h"
#include "../../libmscore/barline.h"
#include "../../libmscore/beam.h"
#include "../../libmscore/bend.h"
#include "../../libmscore/box.h"
#include "../../libmscore/bracket.h"
#include "../../libmscore/breath.h"

#include "../xmlwriter.h"
#include "writecontext.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void TWrite::writeProperty(EngravingItem* item, XmlWriter& xml, Pid pid)
{
    if (item->isStyled(pid)) {
        return;
    }
    PropertyValue p = item->getProperty(pid);
    if (!p.isValid()) {
        LOGD("%s invalid property %d <%s>", item->typeName(), int(pid), propertyName(pid));
        return;
    }
    PropertyFlags f = item->propertyFlags(pid);
    PropertyValue d = (f != PropertyFlags::STYLED) ? item->propertyDefault(pid) : PropertyValue();

    if (pid == Pid::FONT_STYLE) {
        FontStyle ds = FontStyle(d.isValid() ? d.toInt() : 0);
        FontStyle fs = FontStyle(p.toInt());
        if ((fs& FontStyle::Bold) != (ds & FontStyle::Bold)) {
            xml.tag("bold", fs & FontStyle::Bold);
        }
        if ((fs& FontStyle::Italic) != (ds & FontStyle::Italic)) {
            xml.tag("italic", fs & FontStyle::Italic);
        }
        if ((fs& FontStyle::Underline) != (ds & FontStyle::Underline)) {
            xml.tag("underline", fs & FontStyle::Underline);
        }
        if ((fs& FontStyle::Strike) != (ds & FontStyle::Strike)) {
            xml.tag("strike", fs & FontStyle::Strike);
        }
        return;
    }

    P_TYPE type = propertyType(pid);
    if (P_TYPE::MILLIMETRE == type) {
        double f1 = p.toReal();
        if (d.isValid() && std::abs(f1 - d.toReal()) < 0.0001) {            // fuzzy compare
            return;
        }
        p = PropertyValue(Spatium::fromMM(f1, item->score()->spatium()));
        d = PropertyValue();
    } else if (P_TYPE::POINT == type) {
        PointF p1 = p.value<PointF>();
        if (d.isValid()) {
            PointF p2 = d.value<PointF>();
            if ((std::abs(p1.x() - p2.x()) < 0.0001) && (std::abs(p1.y() - p2.y()) < 0.0001)) {
                return;
            }
        }
        double q = item->offsetIsSpatiumDependent() ? item->score()->spatium() : DPMM;
        p = PropertyValue(p1 / q);
        d = PropertyValue();
    }
    xml.tagProperty(pid, p, d);
}

void TWrite::writeItemProperties(EngravingItem* item, XmlWriter& xml, WriteContext&)
{
    WriteContext& ctx = *xml.context();

    bool autoplaceEnabled = item->score()->styleB(Sid::autoplaceEnabled);
    if (!autoplaceEnabled) {
        item->score()->setStyleValue(Sid::autoplaceEnabled, true);
        writeProperty(item, xml, Pid::AUTOPLACE);
        item->score()->setStyleValue(Sid::autoplaceEnabled, autoplaceEnabled);
    } else {
        writeProperty(item, xml, Pid::AUTOPLACE);
    }

    // copy paste should not keep links
    if (item->links() && (item->links()->size() > 1) && !xml.context()->clipboardmode()) {
        if (MScore::debugMode) {
            xml.tag("lid", item->links()->lid());
        }

        EngravingItem* me = static_cast<EngravingItem*>(item->links()->mainElement());
        assert(item->type() == me->type());
        Staff* s = item->staff();
        if (!s) {
            s = item->score()->staff(xml.context()->curTrack() / VOICES);
            if (!s) {
                LOGW("EngravingItem::writeProperties: linked element's staff not found (%s)", item->typeName());
            }
        }
        Location loc = Location::positionForElement(item);
        if (me == item) {
            xml.tag("linkedMain");
            int index = ctx.assignLocalIndex(loc);
            ctx.setLidLocalIndex(item->links()->lid(), index);
        } else {
            if (s && s->links()) {
                Staff* linkedStaff = toStaff(s->links()->mainElement());
                loc.setStaff(static_cast<int>(linkedStaff->idx()));
            }
            xml.startElement("linked");
            if (!me->score()->isMaster()) {
                if (me->score() == item->score()) {
                    xml.tag("score", "same");
                } else {
                    LOGW(
                        "EngravingItem::writeProperties: linked elements belong to different scores but none of them is master score: (%s lid=%d)",
                        item->typeName(), item->links()->lid());
                }
            }

            Location mainLoc = Location::positionForElement(me);
            const int guessedLocalIndex = ctx.assignLocalIndex(mainLoc);
            if (loc != mainLoc) {
                mainLoc.toRelative(loc);
                mainLoc.write(xml);
            }
            const int indexDiff = ctx.lidLocalIndex(item->links()->lid()) - guessedLocalIndex;
            xml.tag("indexDiff", indexDiff, 0);
            xml.endElement();       // </linked>
        }
    }
    if ((ctx.writeTrack() || item->track() != ctx.curTrack())
        && (item->track() != mu::nidx) && !item->isBeam()) {
        // Writing track number for beams is redundant as it is calculated
        // during layout.
        int t = static_cast<int>(item->track()) + ctx.trackDiff();
        xml.tag("track", t);
    }
    if (ctx.writePosition()) {
        xml.tagProperty(Pid::POSITION, item->rtick());
    }
    if (item->tag() != 0x1) {
        for (int i = 1; i < MAX_TAGS; i++) {
            if (item->tag() == ((unsigned)1 << i)) {
                xml.tag("tag", item->score()->layerTags()[i]);
                break;
            }
        }
    }
    for (Pid pid : { Pid::OFFSET, Pid::COLOR, Pid::VISIBLE, Pid::Z, Pid::PLACEMENT }) {
        if (item->propertyFlags(pid) == PropertyFlags::NOSTYLE) {
            writeProperty(item, xml, pid);
        }
    }
}

void TWrite::write(Accidental* a, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(a);
    writeProperty(a, xml, Pid::ACCIDENTAL_BRACKET);
    writeProperty(a, xml, Pid::ACCIDENTAL_ROLE);
    writeProperty(a, xml, Pid::SMALL);
    writeProperty(a, xml, Pid::ACCIDENTAL_TYPE);
    writeItemProperties(a, xml, ctx);
    xml.endElement();
}

void TWrite::write(ActionIcon* a, XmlWriter& xml, WriteContext&)
{
    xml.startElement(a);
    xml.tag("subtype", int(a->actionType()));
    if (!a->actionCode().empty()) {
        xml.tag("action", String::fromStdString(a->actionCode()));
    }
    xml.endElement();
}

void TWrite::write(Ambitus* a, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(a);
    xml.tagProperty(Pid::HEAD_GROUP, int(a->noteHeadGroup()), int(Ambitus::NOTEHEADGROUP_DEFAULT));
    xml.tagProperty(Pid::HEAD_TYPE,  int(a->noteHeadType()),  int(Ambitus::NOTEHEADTYPE_DEFAULT));
    xml.tagProperty(Pid::MIRROR_HEAD, int(a->direction()),    int(Ambitus::DIR_DEFAULT));
    xml.tag("hasLine",    a->hasLine(), true);
    xml.tagProperty(Pid::LINE_WIDTH_SPATIUM, a->lineWidth(), Ambitus::LINEWIDTH_DEFAULT);
    xml.tag("topPitch",   a->topPitch());
    xml.tag("topTpc",     a->topTpc());
    xml.tag("bottomPitch", a->bottomPitch());
    xml.tag("bottomTpc",  a->bottomTpc());
    if (a->topAccidental()->accidentalType() != AccidentalType::NONE) {
        xml.startElement("topAccidental");
        a->topAccidental()->write(xml);
        xml.endElement();
    }
    if (a->bottomAccidental()->accidentalType() != AccidentalType::NONE) {
        xml.startElement("bottomAccidental");
        a->bottomAccidental()->write(xml);
        xml.endElement();
    }
    writeItemProperties(a, xml, ctx);
    xml.endElement();
}

void TWrite::write(Arpeggio* a, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(a)) {
        return;
    }
    xml.startElement(a);
    writeItemProperties(a, xml, ctx);
    writeProperty(a, xml, Pid::ARPEGGIO_TYPE);
    if (a->userLen1() != 0.0) {
        xml.tag("userLen1", a->userLen1() / a->spatium());
    }
    if (a->userLen2() != 0.0) {
        xml.tag("userLen2", a->userLen2() / a->spatium());
    }
    if (a->span() != 1) {
        xml.tag("span", a->span());
    }
    writeProperty(a, xml, Pid::PLAY);
    writeProperty(a, xml, Pid::TIME_STRETCH);
    xml.endElement();
}

void TWrite::write(Articulation* a, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(a)) {
        return;
    }
    xml.startElement(a);
    if (!a->channelName().isEmpty()) {
        xml.tag("channe", { { "name", a->channelName() } });
    }

    writeProperty(a, xml, Pid::DIRECTION);
    if (a->textType() != ArticulationTextType::NO_TEXT) {
        xml.tag("subtype", TConv::toXml(a->textType()));
    } else {
        xml.tag("subtype", SymNames::nameForSymId(a->symId()));
    }

    writeProperty(a, xml, Pid::PLAY);
    writeProperty(a, xml, Pid::ORNAMENT_STYLE);
    for (const StyledProperty& spp : *a->styledProperties()) {
        writeProperty(a, xml, spp.pid);
    }
    writeItemProperties(a, xml, ctx);
    xml.endElement();
}

void TWrite::write(BagpipeEmbellishment* b, XmlWriter& xml, WriteContext&)
{
    xml.startElement(b);
    xml.tag("subtype", TConv::toXml(b->embelType()));
    xml.endElement();
}

void TWrite::write(BarLine* b, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(b);

    writeProperty(b, xml, Pid::BARLINE_TYPE);
    writeProperty(b, xml, Pid::BARLINE_SPAN);
    writeProperty(b, xml, Pid::BARLINE_SPAN_FROM);
    writeProperty(b, xml, Pid::BARLINE_SPAN_TO);

    for (const EngravingItem* e : *b->el()) {
        e->write(xml);
    }
    writeItemProperties(b, xml, ctx);
    xml.endElement();
}

void TWrite::write(Beam* b, XmlWriter& xml, WriteContext& ctx)
{
    if (b->elements().empty()) {
        return;
    }
    xml.startElement(b);
    writeItemProperties(b, xml, ctx);

    writeProperty(b, xml, Pid::STEM_DIRECTION);
    writeProperty(b, xml, Pid::BEAM_NO_SLOPE);
    writeProperty(b, xml, Pid::GROW_LEFT);
    writeProperty(b, xml, Pid::GROW_RIGHT);

    int idx = (b->beamDirection() == DirectionV::AUTO || b->beamDirection() == DirectionV::DOWN) ? 0 : 1;
    if (b->userModified()) {
        double _spatium = b->spatium();
        for (BeamFragment* f : b->beamFragments()) {
            xml.startElement("Fragment");
            xml.tag("y1", f->py1[idx] / _spatium);
            xml.tag("y2", f->py2[idx] / _spatium);
            xml.endElement();
        }
    }

    // this info is used for regression testing
    // l1/l2 is the beam position of the layout engine
    if (MScore::testMode) {
        double spatium8 = b->spatium() * .125;
        for (BeamFragment* f : b->beamFragments()) {
            xml.tag("l1", int(lrint(f->py1[idx] / spatium8)));
            xml.tag("l2", int(lrint(f->py2[idx] / spatium8)));
        }
    }

    xml.endElement();
}
