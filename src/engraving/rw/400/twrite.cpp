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

#include "global/io/fileinfo.h"

#include "../../iengravingfont.h"
#include "../../types/typesconv.h"
#include "../../types/symnames.h"
#include "../../style/textstyle.h"
#include "../../infrastructure/ifileinfoprovider.h"

#include "../../libmscore/score.h"
#include "../../libmscore/masterscore.h"
#include "../../libmscore/factory.h"
#include "../../libmscore/linkedobjects.h"
#include "../../libmscore/mscore.h"
#include "../../libmscore/staff.h"
#include "../../libmscore/part.h"
#include "../../libmscore/utils.h"

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
#include "../../libmscore/textframe.h"
#include "../../libmscore/bracket.h"
#include "../../libmscore/breath.h"

#include "../../libmscore/chord.h"
#include "../../libmscore/chordline.h"
#include "../../libmscore/chordrest.h"
#include "../../libmscore/clef.h"

#include "../../libmscore/dynamic.h"

#include "../../libmscore/fermata.h"
#include "../../libmscore/figuredbass.h"
#include "../../libmscore/fingering.h"
#include "../../libmscore/fret.h"

#include "../../libmscore/glissando.h"
#include "../../libmscore/gradualtempochange.h"
#include "../../libmscore/groups.h"

#include "../../libmscore/hairpin.h"
#include "../../libmscore/harmony.h"
#include "../../libmscore/hook.h"

#include "../../libmscore/image.h"
#include "../../libmscore/imageStore.h"
#include "../../libmscore/instrchange.h"

#include "../../libmscore/jump.h"

#include "../../libmscore/keysig.h"

#include "../../libmscore/layoutbreak.h"
#include "../../libmscore/ledgerline.h"
#include "../../libmscore/letring.h"
#include "../../libmscore/location.h"
#include "../../libmscore/lyrics.h"

#include "../../libmscore/marker.h"
#include "../../libmscore/measurenumber.h"
#include "../../libmscore/measurerepeat.h"
#include "../../libmscore/mmrest.h"
#include "../../libmscore/mmrestrange.h"

#include "../../libmscore/note.h"
#include "../../libmscore/notedot.h"
#include "../../libmscore/noteline.h"

#include "../../libmscore/ottava.h"

#include "../../libmscore/page.h"
#include "../../libmscore/palmmute.h"
#include "../../libmscore/part.h"
#include "../../libmscore/pedal.h"
#include "../../libmscore/playtechannotation.h"

#include "../../libmscore/rehearsalmark.h"
#include "../../libmscore/rest.h"

#include "../../libmscore/segment.h"
#include "../../libmscore/slur.h"
#include "../../libmscore/spacer.h"
#include "../../libmscore/staffstate.h"
#include "../../libmscore/stafftext.h"
#include "../../libmscore/stafftype.h"
#include "../../libmscore/stafftypechange.h"
#include "../../libmscore/stem.h"
#include "../../libmscore/stemslash.h"
#include "../../libmscore/sticking.h"
#include "../../libmscore/symbol.h"
#include "../../libmscore/bsymbol.h"
#include "../../libmscore/system.h"
#include "../../libmscore/systemdivider.h"
#include "../../libmscore/systemtext.h"

#include "../../libmscore/tempotext.h"
#include "../../libmscore/text.h"
#include "../../libmscore/textbase.h"
#include "../../libmscore/textline.h"
#include "../../libmscore/textlinebase.h"
#include "../../libmscore/chordtextlinebase.h"
#include "../../libmscore/tie.h"
#include "../../libmscore/timesig.h"
#include "../../libmscore/tremolo.h"
#include "../../libmscore/tremolobar.h"
#include "../../libmscore/trill.h"
#include "../../libmscore/tuplet.h"

#include "../../libmscore/vibrato.h"
#include "../../libmscore/volta.h"

#include "../../libmscore/whammybar.h"

#include "../xmlwriter.h"
#include "writecontext.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void TWrite::writeProperty(const EngravingItem* item, XmlWriter& xml, Pid pid)
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

void TWrite::writeStyledProperties(const EngravingItem* item, XmlWriter& xml)
{
    for (const StyledProperty& spp : *item->styledProperties()) {
        writeProperty(item, xml, spp.pid);
    }
}

void TWrite::writeItemProperties(const EngravingItem* item, XmlWriter& xml, WriteContext&)
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

void TWrite::write(const Accidental* a, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(a);
    writeProperty(a, xml, Pid::ACCIDENTAL_BRACKET);
    writeProperty(a, xml, Pid::ACCIDENTAL_ROLE);
    writeProperty(a, xml, Pid::SMALL);
    writeProperty(a, xml, Pid::ACCIDENTAL_TYPE);
    writeItemProperties(a, xml, ctx);
    xml.endElement();
}

void TWrite::write(const ActionIcon* a, XmlWriter& xml, WriteContext&)
{
    xml.startElement(a);
    xml.tag("subtype", int(a->actionType()));
    if (!a->actionCode().empty()) {
        xml.tag("action", String::fromStdString(a->actionCode()));
    }
    xml.endElement();
}

void TWrite::write(const Ambitus* a, XmlWriter& xml, WriteContext& ctx)
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

void TWrite::write(const Arpeggio* a, XmlWriter& xml, WriteContext& ctx)
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

void TWrite::write(const Articulation* a, XmlWriter& xml, WriteContext& ctx)
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

void TWrite::write(const BagpipeEmbellishment* b, XmlWriter& xml, WriteContext&)
{
    xml.startElement(b);
    xml.tag("subtype", TConv::toXml(b->embelType()));
    xml.endElement();
}

void TWrite::write(const BarLine* b, XmlWriter& xml, WriteContext& ctx)
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

void TWrite::write(const Beam* b, XmlWriter& xml, WriteContext& ctx)
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

void TWrite::write(const Bend* b, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(b);
    for (const PitchValue& v : b->points()) {
        xml.tag("point", { { "time", v.time }, { "pitch", v.pitch }, { "vibrato", v.vibrato } });
    }
    writeStyledProperties(b, xml);
    writeProperty(b, xml, Pid::PLAY);
    writeItemProperties(b, xml, ctx);
    xml.endElement();
}

void TWrite::write(const Box* b, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(b);
    writeBoxProperties(b, xml, ctx);
    xml.endElement();
}

void TWrite::writeBoxProperties(const Box* item, XmlWriter& xml, WriteContext& ctx)
{
    if (item->isHBox()) {
        return writeProperties(dynamic_cast<const HBox*>(item), xml, ctx);
    }
    return writeProperties(item, xml, ctx);
}

void TWrite::writeProperties(const Box* item, XmlWriter& xml, WriteContext& ctx)
{
    for (Pid id : {
        Pid::BOX_HEIGHT, Pid::BOX_WIDTH, Pid::TOP_GAP, Pid::BOTTOM_GAP,
        Pid::LEFT_MARGIN, Pid::RIGHT_MARGIN, Pid::TOP_MARGIN, Pid::BOTTOM_MARGIN, Pid::BOX_AUTOSIZE }) {
        writeProperty(item, xml, id);
    }
    writeItemProperties(item, xml, ctx);
    for (const EngravingItem* e : item->el()) {
        e->write(xml);
    }
}

void TWrite::write(const HBox* b, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(b);
    writeProperties(b, xml, ctx);
    xml.endElement();
}

void TWrite::writeProperties(const HBox* item, XmlWriter& xml, WriteContext& ctx)
{
    writeProperty(item, xml, Pid::CREATE_SYSTEM_HEADER);
    writeProperties(static_cast<const Box*>(item), xml, ctx);
}

void TWrite::write(const VBox* b, XmlWriter& xml, WriteContext& ctx)
{
    write(static_cast<const Box*>(b), xml, ctx);
}

void TWrite::write(const FBox* b, XmlWriter& xml, WriteContext& ctx)
{
    write(static_cast<const Box*>(b), xml, ctx);
}

void TWrite::write(const TBox* b, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(b);
    writeProperties(static_cast<const Box*>(b), xml, ctx);
    b->text()->write(xml);
    xml.endElement();
}

void TWrite::write(const Bracket* b, XmlWriter& xml, WriteContext& ctx)
{
    bool isStartTag = false;
    switch (b->bracketItem()->bracketType()) {
    case BracketType::BRACE:
    case BracketType::SQUARE:
    case BracketType::LINE:
    {
        xml.startElement(b, { { "type", TConv::toXml(b->bracketItem()->bracketType()) } });
        isStartTag = true;
    }
    break;
    case BracketType::NORMAL:
        xml.startElement(b);
        isStartTag = true;
        break;
    case BracketType::NO_BRACKET:
        break;
    }

    if (isStartTag) {
        if (b->bracketItem()->column()) {
            xml.tag("level", static_cast<int>(b->bracketItem()->column()));
        }

        writeItemProperties(b, xml, ctx);

        xml.endElement();
    }
}

void TWrite::write(const Breath* b, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(b)) {
        return;
    }
    xml.startElement(b);
    writeProperty(b, xml, Pid::SYMBOL);
    writeProperty(b, xml, Pid::PAUSE);
    writeItemProperties(b, xml, ctx);
    xml.endElement();
}

void TWrite::write(const Chord* c, XmlWriter& xml, WriteContext& ctx)
{
    for (Chord* ch : c->graceNotes()) {
        write(ch, xml, ctx);
    }
    writeChordRestBeam(c, xml, ctx);
    xml.startElement(c);
    writeProperties(static_cast<const ChordRest*>(c), xml, ctx);
    for (const Articulation* a : c->articulations()) {
        write(a, xml, ctx);
    }
    switch (c->noteType()) {
    case NoteType::NORMAL:
        break;
    case NoteType::ACCIACCATURA:
        xml.tag("acciaccatura");
        break;
    case NoteType::APPOGGIATURA:
        xml.tag("appoggiatura");
        break;
    case NoteType::GRACE4:
        xml.tag("grace4");
        break;
    case NoteType::GRACE16:
        xml.tag("grace16");
        break;
    case NoteType::GRACE32:
        xml.tag("grace32");
        break;
    case NoteType::GRACE8_AFTER:
        xml.tag("grace8after");
        break;
    case NoteType::GRACE16_AFTER:
        xml.tag("grace16after");
        break;
    case NoteType::GRACE32_AFTER:
        xml.tag("grace32after");
        break;
    default:
        break;
    }

    if (c->noStem()) {
        xml.tag("noStem", c->noStem());
    } else if (c->stem() && (c->stem()->isUserModified() || (c->stem()->userLength() != 0.0))) {
        c->stem()->write(xml);
    }
    if (c->hook() && c->hook()->isUserModified()) {
        c->hook()->write(xml);
    }
    if (c->stemSlash() && c->stemSlash()->isUserModified()) {
        c->stemSlash()->write(xml);
    }
    writeProperty(c, xml, Pid::STEM_DIRECTION);
    for (Note* n : c->notes()) {
        n->write(xml);
    }
    if (c->arpeggio()) {
        write(c->arpeggio(), xml, ctx);
    }
    if (c->tremolo() && c->tremoloChordType() != TremoloChordType::TremoloSecondNote) {
        c->tremolo()->write(xml);
    }
    for (EngravingItem* e : c->el()) {
        if (e->isChordLine() && toChordLine(e)->note()) { // this is now written by Note
            continue;
        }
        e->write(xml);
    }
    xml.endElement();
}

void TWrite::writeChordRestBeam(const ChordRest* item, XmlWriter& xml, WriteContext& ctx)
{
    Beam* b = item->beam();
    if (b && b->elements().front() == item && (MScore::testMode || !b->generated())) {
        write(b, xml, ctx);
    }
}

void TWrite::writeProperties(const ChordRest* item, XmlWriter& xml, WriteContext& ctx)
{
    writeItemProperties(item, xml, ctx);

    //
    // BeamMode default:
    //    REST  - BeamMode::NONE
    //    CHORD - BeamMode::AUTO
    //
    if ((item->isRest() && item->beamMode() != BeamMode::NONE) || (item->isChord() && item->beamMode() != BeamMode::AUTO)) {
        xml.tag("BeamMode", TConv::toXml(item->beamMode()));
    }
    writeProperty(item, xml, Pid::SMALL);
    if (item->actualDurationType().dots()) {
        xml.tag("dots", item->actualDurationType().dots());
    }
    writeProperty(item, xml, Pid::STAFF_MOVE);

    if (item->actualDurationType().isValid()) {
        xml.tag("durationType", TConv::toXml(item->actualDurationType().type()));
    }

    if (!item->ticks().isZero() && (!item->actualDurationType().fraction().isValid()
                                    || (item->actualDurationType().fraction() != item->ticks()))) {
        xml.tagFraction("duration", item->ticks());
        //xml.tagE("duration z=\"%d\" n=\"%d\"", ticks().numerator(), ticks().denominator());
    }

    for (Lyrics* lyrics : item->lyrics()) {
        lyrics->write(xml);
    }

    const int curTick = ctx.curTick().ticks();

    if (!item->isGrace()) {
        Fraction t(item->globalTicks());
        if (item->staff()) {
            t /= item->staff()->timeStretch(ctx.curTick());
        }
        ctx.incCurTick(t);
    }

    for (auto i : item->score()->spannerMap().findOverlapping(curTick - 1, curTick + 1)) {
        Spanner* s = i.value;
        if (s->generated() || !s->isSlur() || toSlur(s)->broken() || !ctx.canWrite(s)) {
            continue;
        }

        if (s->startElement() == item) {
            s->writeSpannerStart(xml, item, item->track());
        } else if (s->endElement() == item) {
            s->writeSpannerEnd(xml, item, item->track());
        }
    }
}

void TWrite::write(const ChordLine* c, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(c);
    writeProperty(c, xml, Pid::CHORD_LINE_TYPE);
    writeProperty(c, xml, Pid::CHORD_LINE_STRAIGHT);
    writeProperty(c, xml, Pid::CHORD_LINE_WAVY);
    xml.tag("lengthX", c->lengthX(), 0.0);
    xml.tag("lengthY", c->lengthY(), 0.0);
    writeItemProperties(c, xml, ctx);
    if (c->modified()) {
        const draw::PainterPath& path = c->path();
        size_t n = path.elementCount();
        xml.startElement("Path");
        for (size_t i = 0; i < n; ++i) {
            const PainterPath::Element& e = path.elementAt(i);
            xml.tag("Element", { { "type", int(e.type) }, { "x", e.x }, { "y", e.y } });
        }
        xml.endElement();
    }
    xml.endElement();
}

void TWrite::write(const Clef* c, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(c);
    writeProperty(c, xml, Pid::CLEF_TYPE_CONCERT);
    writeProperty(c, xml, Pid::CLEF_TYPE_TRANSPOSING);
    if (!c->showCourtesy()) {
        xml.tag("showCourtesyClef", c->showCourtesy());
    }
    if (c->forInstrumentChange()) {
        xml.tag("forInstrumentChange", c->forInstrumentChange());
    }
    writeItemProperties(c, xml, ctx);
    xml.endElement();
}

void TWrite::write(const Dynamic* d, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(d)) {
        return;
    }
    xml.startElement(d);
    writeProperty(d, xml, Pid::DYNAMIC_TYPE);
    writeProperty(d, xml, Pid::VELOCITY);
    writeProperty(d, xml, Pid::DYNAMIC_RANGE);

    if (d->isVelocityChangeAvailable()) {
        writeProperty(d, xml, Pid::VELO_CHANGE);
        writeProperty(d, xml, Pid::VELO_CHANGE_SPEED);
    }

    writeProperties(static_cast<const TextBase*>(d), xml, ctx, d->dynamicType() == DynamicType::OTHER);
    xml.endElement();
}

void TWrite::writeProperties(const TextBase* item, XmlWriter& xml, WriteContext& ctx, bool writeText)
{
    writeItemProperties(item, xml, ctx);
    writeProperty(item, xml, Pid::TEXT_STYLE);

    for (const StyledProperty& spp : *item->styledProperties()) {
        if (!item->isStyled(spp.pid)) {
            writeProperty(item, xml, spp.pid);
        }
    }
    for (const auto& spp : *textStyle(item->textStyleType())) {
        if (item->isStyled(spp.pid)
            || (spp.pid == Pid::FONT_SIZE && item->getProperty(spp.pid).toDouble() == TextBase::UNDEFINED_FONT_SIZE)
            || (spp.pid == Pid::FONT_FACE && item->getProperty(spp.pid).value<String>() == TextBase::UNDEFINED_FONT_FAMILY)) {
            continue;
        }
        writeProperty(item, xml, spp.pid);
    }
    if (writeText) {
        xml.writeXml(u"text", item->xmlText());
    }
}

void TWrite::write(const Fermata* f, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(f)) {
        return;
    }

    xml.startElement(f);
    xml.tag("subtype", SymNames::nameForSymId(f->symId()));
    writeProperty(f, xml, Pid::TIME_STRETCH);
    writeProperty(f, xml, Pid::PLAY);
    writeProperty(f, xml, Pid::MIN_DISTANCE);
    if (!f->isStyled(Pid::OFFSET)) {
        writeProperty(f, xml, Pid::OFFSET);
    }
    writeItemProperties(f, xml, ctx);
    xml.endElement();
}

void TWrite::write(const FiguredBass* f, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(f)) {
        return;
    }

    xml.startElement(f);
    if (!f->onNote()) {
        xml.tag("onNote", f->onNote());
    }
    if (f->ticks().isNotZero()) {
        xml.tagFraction("ticks", f->ticks());
    }
    // if unparseable items, write full text data
    if (f->items().size() < 1) {
        writeProperties(static_cast<const TextBase*>(f), xml, ctx, true);
    } else {
//            if (textStyleType() != StyledPropertyListIdx::FIGURED_BASS)
//                  // if all items parsed and not unstiled, we simply have a special style: write it
//                  xml.tag("style", textStyle().name());
        for (FiguredBassItem* item : f->items()) {
            write(item, xml, ctx);
        }
        for (const StyledProperty& spp : *f->styledProperties()) {
            writeProperty(f, xml, spp.pid);
        }
        writeItemProperties(f, xml, ctx);
    }
    xml.endElement();
}

void TWrite::write(const FiguredBassItem* f, XmlWriter& xml, WriteContext&)
{
    xml.startElement("FiguredBassItem", f);
    xml.tag("brackets", {
        { "b0", int(f->parenth1()) },
        { "b1", int(f->parenth2()) },
        { "b2", int(f->parenth3()) },
        { "b3", int(f->parenth4()) },
        { "b4", int(f->parenth5()) }
    });

    if (f->prefix() != FiguredBassItem::Modifier::NONE) {
        xml.tag("prefix", int(f->prefix()));
    }
    if (f->digit() != FBIDigitNone) {
        xml.tag("digit", f->digit());
    }
    if (f->suffix() != FiguredBassItem::Modifier::NONE) {
        xml.tag("suffix", int(f->suffix()));
    }
    if (f->contLine() != FiguredBassItem::ContLine::NONE) {
        xml.tag("continuationLine", int(f->contLine()));
    }
    xml.endElement();
}

void TWrite::write(const Fingering* f, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(f)) {
        return;
    }
    xml.startElement(f);
    writeProperties(static_cast<const TextBase*>(f), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const FretDiagram* f, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(f)) {
        return;
    }
    xml.startElement(f);

    static const std::array<Pid, 8> pids { {
        Pid::MIN_DISTANCE,
        Pid::FRET_OFFSET,
        Pid::FRET_FRETS,
        Pid::FRET_STRINGS,
        Pid::FRET_NUT,
        Pid::MAG,
        Pid::FRET_NUM_POS,
        Pid::ORIENTATION
    } };

    // Write properties first and only once
    for (Pid p : pids) {
        writeProperty(f, xml, p);
    }
    writeItemProperties(f, xml, ctx);

    if (f->harmony()) {
        f->harmony()->write(xml);
    }

    // Lowercase f indicates new writing format
    // TODO: in the next score format version (4) use only write new + props and discard
    // the compatibility writing.
    xml.startElement("fretDiagram");
    // writeNew
    {
        //    This is the important one for 3.1+
        //---------------------------------------------------------
        for (int i = 0; i < f->strings(); ++i) {
            FretItem::Marker m = f->marker(i);
            std::vector<FretItem::Dot> allDots = f->dot(i);

            bool dotExists = false;
            for (auto const& d : allDots) {
                if (d.exists()) {
                    dotExists = true;
                    break;
                }
            }

            // Only write a string if we have anything to write
            if (!dotExists && !m.exists()) {
                continue;
            }

            // Start the string writing
            xml.startElement("string", { { "no", i } });

            // Write marker
            if (m.exists()) {
                xml.tag("marker", FretItem::markerTypeToName(m.mtype));
            }

            // Write any dots
            for (auto const& d : allDots) {
                if (d.exists()) {
                    // TODO: write fingering
                    xml.tag("dot", { { "fret", d.fret } }, FretItem::dotTypeToName(d.dtype));
                }
            }

            xml.endElement();
        }

        for (int fi = 1; fi <= f->frets(); ++fi) {
            FretItem::Barre b = f->barre(fi);
            if (!b.exists()) {
                continue;
            }

            xml.tag("barre", { { "start", b.startString }, { "end", b.endString } }, fi);
        }
    }
    xml.endElement();

    // writeOld
    {
        int lowestDotFret = -1;
        int furthestLeftLowestDot = -1;

        // Do some checks for details needed for checking whether to add barres
        for (int i = 0; i < f->strings(); ++i) {
            std::vector<FretItem::Dot> allDots = f->dot(i);

            bool dotExists = false;
            for (auto const& d : allDots) {
                if (d.exists()) {
                    dotExists = true;
                    break;
                }
            }

            if (!dotExists) {
                continue;
            }

            for (auto const& d : allDots) {
                if (d.exists()) {
                    if (d.fret < lowestDotFret || lowestDotFret == -1) {
                        lowestDotFret = d.fret;
                        furthestLeftLowestDot = i;
                    } else if (d.fret == lowestDotFret && (i < furthestLeftLowestDot || furthestLeftLowestDot == -1)) {
                        furthestLeftLowestDot = i;
                    }
                }
            }
        }

        // The old system writes a barre as a bool, which causes no problems in any way, not at all.
        // So, only write that if the barre is on the lowest fret with a dot,
        // and there are no other dots on its fret, and it goes all the way to the right.
        int barreStartString = -1;
        int barreFret = -1;
        for (auto const& i : f->barres()) {
            FretItem::Barre b = i.second;
            if (b.exists()) {
                int fret = i.first;
                if (fret <= lowestDotFret && b.endString == -1 && !(fret == lowestDotFret && b.startString > furthestLeftLowestDot)) {
                    barreStartString = b.startString;
                    barreFret = fret;
                    break;
                }
            }
        }

        for (int i = 0; i < f->strings(); ++i) {
            FretItem::Marker m = f->marker(i);
            std::vector<FretItem::Dot> allDots = f->dot(i);

            bool dotExists = false;
            for (auto const& d : allDots) {
                if (d.exists()) {
                    dotExists = true;
                    break;
                }
            }

            if (!dotExists && !m.exists() && i != barreStartString) {
                continue;
            }

            xml.startElement("string", { { "no", i } });

            if (m.exists()) {
                xml.tag("marker", FretItem::markerToChar(m.mtype).unicode());
            }

            for (auto const& d : allDots) {
                if (d.exists() && !(i == barreStartString && d.fret == barreFret)) {
                    xml.tag("dot", d.fret);
                }
            }

            // Add dot so barre will display in pre-3.1
            if (barreStartString == i) {
                xml.tag("dot", barreFret);
            }

            xml.endElement();
        }

        if (barreFret > 0) {
            xml.tag("barre", 1);
        }
    }
    xml.endElement();
}

void TWrite::write(const Glissando* g, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(g)) {
        return;
    }
    xml.startElement(g);
    if (g->showText() && !g->text().isEmpty()) {
        xml.tag("text", g->text());
    }

    for (auto id : { Pid::GLISS_TYPE, Pid::PLAY, Pid::GLISS_STYLE, Pid::GLISS_EASEIN, Pid::GLISS_EASEOUT }) {
        writeProperty(g, xml, id);
    }
    for (const StyledProperty& spp : *g->styledProperties()) {
        writeProperty(g, xml, spp.pid);
    }

    writeProperties(static_cast<const SLine*>(g), xml, ctx);
    xml.endElement();
}

void TWrite::writeProperties(const SLine* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!item->endElement()) {
        ((Spanner*)item)->computeEndElement();                    // HACK
        if (!item->endElement()) {
            xml.tagFraction("ticks", item->ticks());
        }
    }
    writeProperties(static_cast<const Spanner*>(item), xml, ctx);
    if (item->diagonal()) {
        xml.tag("diagonal", item->diagonal());
    }
    writeProperty(item, xml, Pid::LINE_WIDTH);
    writeProperty(item, xml, Pid::LINE_STYLE);
    writeProperty(item, xml, Pid::COLOR);
    writeProperty(item, xml, Pid::ANCHOR);
    writeProperty(item, xml, Pid::DASH_LINE_LEN);
    writeProperty(item, xml, Pid::DASH_GAP_LEN);

    if (item->score()->isPaletteScore()) {
        // when used as icon
        if (!item->spannerSegments().empty()) {
            const LineSegment* s = item->frontSegment();
            xml.tag("length", s->pos2().x());
        } else {
            xml.tag("length", item->spatium() * 4);
        }
        return;
    }
    //
    // check if user has modified the default layout
    //
    bool modified = false;
    for (const SpannerSegment* seg : item->spannerSegments()) {
        if (!seg->autoplace() || !seg->visible()
            || (seg->propertyFlags(Pid::MIN_DISTANCE) == PropertyFlags::UNSTYLED
                || seg->getProperty(Pid::MIN_DISTANCE) != seg->propertyDefault(Pid::MIN_DISTANCE))
            || (!seg->isStyled(Pid::OFFSET) && (!seg->offset().isNull() || !seg->userOff2().isNull()))) {
            modified = true;
            break;
        }
    }
    if (!modified) {
        return;
    }

    //
    // write user modified layout and other segment properties
    //
    double _spatium = item->score()->spatium();
    for (const SpannerSegment* seg : item->spannerSegments()) {
        xml.startElement("Segment", seg);
        xml.tag("subtype", int(seg->spannerSegmentType()));
        // TODO:
        // NOSTYLE offset written in EngravingItem::writeProperties,
        // so we probably don't need to duplicate it here
        // see https://musescore.org/en/node/286848
        //if (seg->propertyFlags(Pid::OFFSET) & PropertyFlags::UNSTYLED)
        xml.tagPoint("offset", seg->offset() / _spatium);
        xml.tagPoint("off2", seg->userOff2() / _spatium);
        writeProperty(seg, xml, Pid::MIN_DISTANCE);
        writeItemProperties(seg, xml, ctx);
        xml.endElement();
    }
}

void TWrite::writeProperties(const Spanner* item, XmlWriter& xml, WriteContext& ctx)
{
    if (ctx.clipboardmode()) {
        xml.tagFraction("ticks_f", item->ticks());
    }
    writeItemProperties(item, xml, ctx);
}

void TWrite::write(const GradualTempoChange* g, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(g);
    writeProperty(g, xml, Pid::TEMPO_CHANGE_TYPE);
    writeProperty(g, xml, Pid::TEMPO_EASING_METHOD);
    writeProperty(g, xml, Pid::TEMPO_CHANGE_FACTOR);
    writeProperty(g, xml, Pid::PLACEMENT);
    writeProperties(static_cast<const TextLineBase*>(g), xml, ctx);
    xml.endElement();
}

void TWrite::writeProperties(const TextLineBase* item, XmlWriter& xml, WriteContext& ctx)
{
    for (Pid pid : TextLineBase::textLineBasePropertyIds()) {
        if (!item->isStyled(pid)) {
            writeProperty(item, xml, pid);
        }
    }
    writeProperties(static_cast<const SLine*>(item), xml, ctx);
}

void TWrite::write(const Groups* g, XmlWriter& xml, WriteContext&)
{
    xml.startElement("Groups");
    for (const GroupNode& n : g->nodes()) {
        xml.tag("Node", { { "pos", n.pos }, { "action", n.action } });
    }
    xml.endElement();
}

void TWrite::write(const Hairpin* h, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(h)) {
        return;
    }
    xml.startElement(h);
    xml.tag("subtype", int(h->hairpinType()));
    writeProperty(h, xml, Pid::VELO_CHANGE);
    writeProperty(h, xml, Pid::HAIRPIN_CIRCLEDTIP);
    writeProperty(h, xml, Pid::DYNAMIC_RANGE);
//      writeProperty(xml, Pid::BEGIN_TEXT);
    writeProperty(h, xml, Pid::END_TEXT);
//      writeProperty(xml, Pid::CONTINUE_TEXT);
    writeProperty(h, xml, Pid::LINE_VISIBLE);
    writeProperty(h, xml, Pid::SINGLE_NOTE_DYNAMICS);
    writeProperty(h, xml, Pid::VELO_CHANGE_METHOD);

    for (const StyledProperty& spp : *h->styledProperties()) {
        if (!h->isStyled(spp.pid)) {
            writeProperty(h, xml, spp.pid);
        }
    }
    writeProperties(static_cast<const SLine*>(h), xml, ctx);
    xml.endElement();
}

void TWrite::write(const Harmony* h, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(h)) {
        return;
    }
    xml.startElement(h);
    writeProperty(h, xml, Pid::HARMONY_TYPE);
    writeProperty(h, xml, Pid::PLAY);
    if (h->leftParen()) {
        xml.tag("leftParen");
    }
    if (h->rootTpc() != Tpc::TPC_INVALID || h->baseTpc() != Tpc::TPC_INVALID) {
        int rRootTpc = h->rootTpc();
        int rBaseTpc = h->baseTpc();
        if (h->staff()) {
            // parent can be a fret diagram
            Segment* segment = h->getParentSeg();
            Fraction tick = segment ? segment->tick() : Fraction(-1, 1);
            const Interval& interval = h->part()->instrument(tick)->transpose();
            if (ctx.clipboardmode() && !h->score()->styleB(Sid::concertPitch) && interval.chromatic) {
                rRootTpc = transposeTpc(h->rootTpc(), interval, true);
                rBaseTpc = transposeTpc(h->baseTpc(), interval, true);
            }
        }
        if (rRootTpc != Tpc::TPC_INVALID) {
            xml.tag("root", rRootTpc);
            if (h->rootCase() != NoteCaseType::CAPITAL) {
                xml.tag("rootCase", static_cast<int>(h->rootCase()));
            }
        }
        if (h->id() > 0) {
            xml.tag("extension", h->id());
        }
        // parser uses leading "=" as a hidden specifier for minor
        // this may or may not currently be incorporated into _textName
        String writeName = h->hTextName();
        if (h->parsedForm() && h->parsedForm()->name().startsWith(u'=') && !writeName.startsWith(u'=')) {
            writeName = u"=" + writeName;
        }
        if (!writeName.isEmpty()) {
            xml.tag("name", writeName);
        }

        if (rBaseTpc != Tpc::TPC_INVALID) {
            xml.tag("base", rBaseTpc);
            if (h->baseCase() != NoteCaseType::CAPITAL) {
                xml.tag("baseCase", static_cast<int>(h->baseCase()));
            }
        }
        for (const HDegree& hd : h->degreeList()) {
            HDegreeType tp = hd.type();
            if (tp == HDegreeType::ADD || tp == HDegreeType::ALTER || tp == HDegreeType::SUBTRACT) {
                xml.startElement("degree");
                xml.tag("degree-value", hd.value());
                xml.tag("degree-alter", hd.alter());
                switch (tp) {
                case HDegreeType::ADD:
                    xml.tag("degree-type", "add");
                    break;
                case HDegreeType::ALTER:
                    xml.tag("degree-type", "alter");
                    break;
                case HDegreeType::SUBTRACT:
                    xml.tag("degree-type", "subtract");
                    break;
                default:
                    break;
                }
                xml.endElement();
            }
        }
    } else {
        xml.tag("name", h->hTextName());
    }
    if (!h->hFunction().isEmpty()) {
        xml.tag("function", h->hFunction());
    }
    writeProperties(static_cast<const TextBase*>(h), xml, ctx, false);
    //Pid::HARMONY_VOICE_LITERAL, Pid::HARMONY_VOICING, Pid::HARMONY_DURATION
    //written by the above function call because they are part of element style
    if (h->rightParen()) {
        xml.tag("rightParen");
    }
    xml.endElement();
}

void TWrite::write(const Hook* h, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(h);
    xml.tag("name", SymNames::nameForSymId(h->sym()));
    if (h->scoreFont()) {
        xml.tag("font", h->scoreFont()->name());
    }
    writeProperties(static_cast<const BSymbol*>(h), xml, ctx);
    xml.endElement();
}

void TWrite::writeProperties(const BSymbol* item, XmlWriter& xml, WriteContext& ctx)
{
    for (const EngravingItem* e : item->leafs()) {
        e->write(xml);
    }
    writeItemProperties(item, xml, ctx);
}

void TWrite::write(const Image* img, XmlWriter& xml, WriteContext& ctx)
{
    // attempt to convert the _linkPath to a path relative to the score
    //
    // TODO : on Save As, score()->fileInfo() still contains the old path and fname
    //          if the Save As path is different, image relative path will be wrong!
    //
    String relativeFilePath;
    if (!img->linkPath().isEmpty() && img->linkIsValid()) {
        io::FileInfo fi(img->linkPath());
        // score()->fileInfo()->canonicalPath() would be better
        // but we are saving under a temp file name and the 'final' file
        // might not exist yet, so canonicalFilePath() may return only "/"
        // OTOH, the score 'final' file name is practically always canonical, at this point
        String scorePath = img->score()->masterScore()->fileInfo()->absoluteDirPath().toString();
        String imgFPath  = fi.canonicalFilePath();
        // if imgFPath is in (or below) the directory of scorePath
        if (imgFPath.startsWith(scorePath, mu::CaseSensitive)) {
            // relative img path is the part exceeding scorePath
            imgFPath.remove(0, scorePath.size());
            if (imgFPath.startsWith(u'/')) {
                imgFPath.remove(0, 1);
            }
            relativeFilePath = imgFPath;
        }
        // try 1 level up
        else {
            // reduce scorePath by one path level
            fi = io::FileInfo(scorePath);
            scorePath = fi.path();
            // if imgFPath is in (or below) the directory up the score directory
            if (imgFPath.startsWith(scorePath, mu::CaseSensitive)) {
                // relative img path is the part exceeding new scorePath plus "../"
                imgFPath.remove(0, scorePath.size());
                if (!imgFPath.startsWith(u'/')) {
                    imgFPath.prepend(u'/');
                }
                imgFPath.prepend(u"..");
                relativeFilePath = imgFPath;
            }
        }
    }
    // if no match, use full _linkPath
    if (relativeFilePath.isEmpty()) {
        relativeFilePath = img->linkPath();
    }

    xml.startElement(img);
    writeProperties(static_cast<const BSymbol*>(img), xml, ctx);
    // keep old "path" tag, for backward compatibility and because it is used elsewhere
    // (for instance by Box:read(), Measure:read(), Note:read(), ...)
    xml.tag("path", img->storeItem() ? img->storeItem()->hashName() : relativeFilePath);
    xml.tag("linkPath", relativeFilePath);

    writeProperty(img, xml, Pid::AUTOSCALE);
    writeProperty(img, xml, Pid::SIZE);
    writeProperty(img, xml, Pid::LOCK_ASPECT_RATIO);
    writeProperty(img, xml, Pid::SIZE_IS_SPATIUM);

    xml.endElement();
}

void TWrite::write(const InstrumentChange* ich, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(ich);
    ich->instrument()->write(xml, ich->part());
    if (ich->init()) {
        xml.tag("init", ich->init());
    }
    writeProperties(static_cast<const TextBase*>(ich), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const Jump* j, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(j);
    writeProperties(static_cast<const TextBase*>(j), xml, ctx, true);
    xml.tag("jumpTo", j->jumpTo());
    xml.tag("playUntil", j->playUntil());
    xml.tag("continueAt", j->continueAt());
    writeProperty(j, xml, Pid::PLAY_REPEATS);
    xml.endElement();
}

void TWrite::write(const KeySig* k, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(k);
    writeItemProperties(k, xml, ctx);
    if (k->isAtonal()) {
        xml.tag("custom", 1);
    } else if (k->isCustom()) {
        xml.tag("accidental", int(k->key()));
        xml.tag("custom", 1);
        for (const CustDef& cd : k->customKeyDefs()) {
            xml.startElement("CustDef");
            xml.tag("sym", SymNames::nameForSymId(cd.sym));
            xml.tag("def", { { "degree", cd.degree }, { "xAlt", cd.xAlt }, { "octAlt", cd.octAlt } });
            xml.endElement();
        }
    } else {
        xml.tag("accidental", int(k->key()));
    }

    if (k->mode() != KeyMode::UNKNOWN) {
        xml.tag("mode", TConv::toXml(k->mode()));
    }

    if (!k->showCourtesy()) {
        xml.tag("showCourtesySig", k->showCourtesy());
    }
    if (k->forInstrumentChange()) {
        xml.tag("forInstrumentChange", true);
    }
    xml.endElement();
}

void TWrite::write(const LayoutBreak* l, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(l);
    writeItemProperties(l, xml, ctx);

    for (auto id :
         { Pid::LAYOUT_BREAK, Pid::PAUSE, Pid::START_WITH_LONG_NAMES, Pid::START_WITH_MEASURE_ONE, Pid::FIRST_SYSTEM_INDENTATION }) {
        writeProperty(l, xml, id);
    }

    xml.endElement();
}

void TWrite::write(const LedgerLine* l, XmlWriter& xml, WriteContext&)
{
    xml.startElement(l);
    xml.tag("lineWidth", l->width() / l->spatium());
    xml.tag("lineLen", l->len() / l->spatium());
    if (!l->vertical()) {
        xml.tag("vertical", l->vertical());
    }
    xml.endElement();
}

void TWrite::write(const LetRing* l, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(l)) {
        return;
    }
    xml.startElement(l);
    writeProperties(static_cast<const TextLineBase*>(l), xml, ctx);
    xml.endElement();
}

void TWrite::write(const Location* l, XmlWriter& xml, WriteContext&)
{
    static constexpr Location relDefaults = Location::relative();

    assert(l->isRelative());
    xml.startElement("location");
    xml.tag("staves", l->staff(), relDefaults.staff());
    xml.tag("voices", l->voice(), relDefaults.voice());
    xml.tag("measures", l->measure(), relDefaults.measure());
    xml.tagFraction("fractions", l->frac().reduced(), relDefaults.frac());
    xml.tag("grace", l->graceIndex(), relDefaults.graceIndex());
    xml.tag("notes", l->note(), relDefaults.note());
    xml.endElement();
}

void TWrite::write(const Lyrics* l, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(l)) {
        return;
    }
    xml.startElement(l);
    writeProperty(l, xml, Pid::VERSE);
    if (l->syllabic() != LyricsSyllabic::SINGLE) {
        xml.tag("syllabic", TConv::toXml(l->syllabic()));
    }
    xml.tag("ticks", l->ticks().ticks(), 0);   // pre-3.1 compatibility: write integer ticks under <ticks> tag
    writeProperty(l, xml, Pid::LYRIC_TICKS);

    writeProperties(static_cast<const TextBase*>(l), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const Marker* m, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(m);
    writeProperties(static_cast<const TextBase*>(m), xml, ctx, true);
    xml.tag("label", m->label());
    xml.endElement();
}

void TWrite::write(const MeasureNumber* m, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(m)) {
        return;
    }
    xml.startElement(m);
    writeProperties(static_cast<const TextBase*>(m), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const MeasureRepeat* m, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(m);
    writeProperty(m, xml, Pid::SUBTYPE);
    writeProperties(static_cast<const Rest*>(m), xml, ctx);
    m->el().write(xml);
    xml.endElement();
}

void TWrite::writeProperties(const Rest* item, XmlWriter& xml, WriteContext& ctx)
{
    writeProperties(static_cast<const ChordRest*>(item), xml, ctx);
}

void TWrite::write(const MMRest* m, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement("Rest"); // for compatibility, see also Measure::readVoice()
    writeProperties(static_cast<const ChordRest*>(m), xml, ctx);
    writeProperty(m, xml, Pid::MMREST_NUMBER_POS);
    writeProperty(m, xml, Pid::MMREST_NUMBER_VISIBLE);
    m->el().write(xml);
    xml.endElement();
}

void TWrite::write(const MMRestRange* m, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(m)) {
        return;
    }
    xml.startElement(m);
    writeProperties(static_cast<const TextBase*>(m), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const Note* n, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(n);
    writeItemProperties(n, xml, ctx);

    if (n->accidental()) {
        write(n->accidental(), xml, ctx);
    }
    n->el().write(xml);
    bool write_dots = false;
    for (NoteDot* dot : n->dots()) {
        if (!dot->offset().isNull() || !dot->visible() || dot->color() != engravingConfiguration()->defaultColor()
            || dot->visible() != n->visible()) {
            write_dots = true;
            break;
        }
    }
    if (write_dots) {
        for (NoteDot* dot : n->dots()) {
            dot->write(xml);
        }
    }
    if (n->tieFor()) {
        n->tieFor()->writeSpannerStart(xml, n, n->track());
    }
    if (n->tieBack()) {
        n->tieBack()->writeSpannerEnd(xml, n, n->track());
    }
    if ((n->chord() == 0 || n->chord()->playEventType() != PlayEventType::Auto) && !n->playEvents().empty()) {
        xml.startElement("Events");
        for (const NoteEvent& e : n->playEvents()) {
            write(&e, xml, ctx);
        }
        xml.endElement();
    }
    for (Pid id : { Pid::PITCH, Pid::TPC1, Pid::TPC2, Pid::SMALL, Pid::MIRROR_HEAD, Pid::DOT_POSITION,
                    Pid::HEAD_SCHEME, Pid::HEAD_GROUP, Pid::USER_VELOCITY, Pid::PLAY, Pid::TUNING, Pid::FRET, Pid::STRING,
                    Pid::GHOST, Pid::DEAD, Pid::HEAD_TYPE, Pid::FIXED, Pid::FIXED_LINE }) {
        writeProperty(n, xml, id);
    }

    for (Spanner* e : n->spannerFor()) {
        e->writeSpannerStart(xml, n, n->track());
    }
    for (Spanner* e : n->spannerBack()) {
        e->writeSpannerEnd(xml, n, n->track());
    }

    for (EngravingItem* e : n->chord()->el()) {
        if (e->isChordLine() && toChordLine(e)->note() && toChordLine(e)->note() == n) {
            toChordLine(e)->write(xml);
        }
    }

    xml.endElement();
}

void TWrite::write(const NoteEvent* n, XmlWriter& xml, WriteContext&)
{
    xml.startElement("Event");
    xml.tag("pitch", n->pitch(), 0);
    xml.tag("ontime", n->ontime(), 0);
    xml.tag("len", n->len(), NoteEvent::NOTE_LENGTH);
    xml.endElement();
}

void TWrite::write(const NoteDot* n, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(n);
    writeItemProperties(n, xml, ctx);
    xml.endElement();
}

void TWrite::write(const NoteLine* n, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(n)) {
        return;
    }
    xml.startElement(n);
    writeProperties(static_cast<const TextLineBase*>(n), xml, ctx);
    xml.endElement();
}

void TWrite::write(const Ottava* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperty(item, xml, Pid::OTTAVA_TYPE);
    writeProperty(item, xml, Pid::PLACEMENT);
    writeProperty(item, xml, Pid::NUMBERS_ONLY);
//      for (const StyledProperty& spp : *styledProperties())
//            writeProperty(xml, spp.pid);
    writeProperties(static_cast<const TextLineBase*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const Page* item, XmlWriter& xml, WriteContext&)
{
    xml.startElement(item);
    for (System* system : item->systems()) {
        system->write(xml);
    }
    xml.endElement();
}

void TWrite::write(const PalmMute* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperties(static_cast<const TextLineBase*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const Part* item, XmlWriter& xml, WriteContext&)
{
    xml.startElement(item, { { "id", item->id().toUint64() } });

    for (const Staff* staff : item->staves()) {
        staff->write(xml);
    }

    if (!item->show()) {
        xml.tag("show", item->show());
    }

    if (item->soloist()) {
        xml.tag("soloist", item->soloist());
    }

    xml.tag("trackName", item->partName());

    if (item->color() != Part::DEFAULT_COLOR) {
        xml.tag("color", item->color());
    }

    if (item->preferSharpFlat() != PreferSharpFlat::DEFAULT) {
        xml.tag("preferSharpFlat", item->preferSharpFlat() == PreferSharpFlat::SHARPS ? "sharps" : "flats");
    }

    item->instrument()->write(xml, item);

    xml.endElement();
}

void TWrite::write(const Pedal* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);

    for (auto i : {
        Pid::END_HOOK_TYPE,
        Pid::BEGIN_TEXT,
        Pid::CONTINUE_TEXT,
        Pid::END_TEXT,
        Pid::LINE_VISIBLE,
        Pid::BEGIN_HOOK_TYPE
    }) {
        writeProperty(item, xml, i);
    }
    for (const StyledProperty& spp : *item->styledProperties()) {
        writeProperty(item, xml, spp.pid);
    }

    writeProperties(static_cast<const SLine*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const PlayTechAnnotation* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperty(item, xml, Pid::PLAY_TECH_TYPE);
    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const RehearsalMark* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const Rest* item, XmlWriter& xml, WriteContext& ctx)
{
    if (item->isGap()) {
        return;
    }
    writeChordRestBeam(item, xml, ctx);
    xml.startElement(item);
    writeStyledProperties(item, xml);
    writeProperties(static_cast<const ChordRest*>(item), xml, ctx);
    item->el().write(xml);
    bool write_dots = false;
    for (NoteDot* dot : item->dotList()) {
        if (!dot->offset().isNull() || !dot->visible() || dot->color() != engravingConfiguration()->defaultColor()
            || dot->visible() != item->visible()) {
            write_dots = true;
            break;
        }
    }
    if (write_dots) {
        for (NoteDot* dot: item->dotList()) {
            write(dot, xml, ctx);
        }
    }
    xml.endElement();
}

void TWrite::write(const Segment* item, XmlWriter& xml, WriteContext&)
{
    if (item->written()) {
        return;
    }
    item->setWritten(true);
    if (item->extraLeadingSpace().isZero()) {
        return;
    }
    xml.startElement(item);
    xml.tag("leadingSpace", item->extraLeadingSpace().val());
    xml.endElement();
}

void TWrite::write(const Slur* item, XmlWriter& xml, WriteContext& ctx)
{
    if (item->broken()) {
        LOGD("broken slur not written");
        return;
    }
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    if (ctx.clipboardmode()) {
        xml.tag("stemArr", Slur::calcStemArrangement(item->startElement(), item->endElement()));
    }
    writeProperties(static_cast<const SlurTie*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::writeProperties(const SlurTie* item, XmlWriter& xml, WriteContext& ctx)
{
    writeProperties(static_cast<const Spanner*>(item), xml, ctx);
    int idx = 0;
    for (const SpannerSegment* ss : item->spannerSegments()) {
        ((SlurTieSegment*)ss)->writeSlur(xml, idx++);
    }
    writeProperty(item, xml, Pid::SLUR_DIRECTION);
    writeProperty(item, xml, Pid::SLUR_STYLE_TYPE);
}

void TWrite::write(const Spacer* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    xml.tag("subtype", int(item->spacerType()));
    writeItemProperties(item, xml, ctx);
    xml.tag("space", item->gap().val() / item->spatium());
    xml.endElement();
}

void TWrite::write(const StaffState* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    xml.tag("subtype", int(item->staffStateType()));
    if (item->staffStateType() == StaffStateType::INSTRUMENT) {
        item->instrument()->write(xml, nullptr);
    }
    writeItemProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::write(const StaffText* item, XmlWriter& xml, WriteContext& ctx)
{
    write(static_cast<const StaffTextBase*>(item), xml, ctx);
}

void TWrite::write(const StaffTextBase* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);

    for (const ChannelActions& s : item->channelActions()) {
        int channel = s.channel;
        for (const String& name : s.midiActionNames) {
            xml.tag("MidiAction", { { "channel", channel }, { "name", name } });
        }
    }
    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
        if (!item->channelName(voice).isEmpty()) {
            xml.tag("channelSwitch", { { "voice", voice }, { "name", item->channelName(voice) } });
        }
    }
    if (item->setAeolusStops()) {
        for (int i = 0; i < 4; ++i) {
            xml.tag("aeolus", { { "group", i } }, item->aeolusStop(i));
        }
    }
    if (item->swing()) {
        DurationType swingUnit;
        if (item->swingParameters().swingUnit == Constants::division / 2) {
            swingUnit = DurationType::V_EIGHTH;
        } else if (item->swingParameters().swingUnit == Constants::division / 4) {
            swingUnit = DurationType::V_16TH;
        } else {
            swingUnit = DurationType::V_ZERO;
        }
        int swingRatio = item->swingParameters().swingRatio;
        xml.tag("swing", { { "unit", TConv::toXml(swingUnit) }, { "ratio", swingRatio } });
    }
    if (item->capo() != 0) {
        xml.tag("capo", { { "fretId", item->capo() } });
    }
    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);

    xml.endElement();
}

void TWrite::write(const StaffType* item, XmlWriter& xml, WriteContext&)
{
    xml.startElement("StaffType", { { "group", TConv::toXml(item->group()) } });
    if (!item->xmlName().isEmpty()) {
        xml.tag("name", item->xmlName());
    }
    if (item->lines() != 5) {
        xml.tag("lines", item->lines());
    }
    if (item->lineDistance().val() != 1.0) {
        xml.tag("lineDistance", item->lineDistance().val());
    }
    if (item->yoffset().val() != 0.0) {
        xml.tag("yoffset", item->yoffset().val());
    }
    if (item->userMag() != 1.0) {
        xml.tag("mag", item->userMag());
    }
    if (item->isSmall()) {
        xml.tag("small", item->isSmall());
    }
    if (item->stepOffset()) {
        xml.tag("stepOffset", item->stepOffset());
    }
    if (!item->genClef()) {
        xml.tag("clef", item->genClef());
    }
    if (item->stemless()) {
        xml.tag("slashStyle", item->stemless());     // for backwards compatibility
        xml.tag("stemless", item->stemless());
    }
    if (!item->showBarlines()) {
        xml.tag("barlines", item->showBarlines());
    }
    if (!item->genTimesig()) {
        xml.tag("timesig", item->genTimesig());
    }
    if (item->invisible()) {
        xml.tag("invisible", item->invisible());
    }
    if (item->color() != engravingConfiguration()->defaultColor()) {
        xml.tag("color", item->color().toString().c_str());
    }
    if (item->group() == StaffGroup::STANDARD) {
        xml.tag("noteheadScheme", TConv::toXml(item->noteHeadScheme()), TConv::toXml(NoteHeadScheme::HEAD_NORMAL));
    }
    if (item->group() == StaffGroup::STANDARD || item->group() == StaffGroup::PERCUSSION) {
        if (!item->genKeysig()) {
            xml.tag("keysig", item->genKeysig());
        }
        if (!item->showLedgerLines()) {
            xml.tag("ledgerlines", item->showLedgerLines());
        }
    } else {
        xml.tag("durations",        item->genDurations());
        xml.tag("durationFontName", item->durationFontName());     // write font names anyway for backward compatibility
        xml.tag("durationFontSize", item->durationFontSize());
        xml.tag("durationFontY",    item->durationFontUserY());
        xml.tag("fretFontName",     item->fretFontName());
        xml.tag("fretFontSize",     item->fretFontSize());
        xml.tag("fretFontY",        item->fretFontUserY());
        if (item->symRepeat() != TablatureSymbolRepeat::NEVER) {
            xml.tag("symbolRepeat", int(item->symRepeat()));
        }
        xml.tag("linesThrough",     item->linesThrough());
        xml.tag("minimStyle",       int(item->minimStyle()));
        xml.tag("onLines",          item->onLines());
        xml.tag("showRests",        item->showRests());
        xml.tag("stemsDown",        item->stemsDown());
        xml.tag("stemsThrough",     item->stemThrough());
        xml.tag("upsideDown",       item->upsideDown());
        xml.tag("showTabFingering", item->showTabFingering(), false);
        xml.tag("useNumbers",       item->useNumbers());
        // only output "showBackTied" if different from !"stemless"
        // to match the behaviour in 2.0.2 scores (or older)
        if (item->showBackTied() != !item->stemless()) {
            xml.tag("showBackTied",  item->showBackTied());
        }
    }
    xml.endElement();
}

void TWrite::write(const StaffTypeChange* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    if (item->staffType()) {
        write(item->staffType(), xml, ctx);
    }
    writeItemProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::write(const Stem* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeItemProperties(item, xml, ctx);
    writeProperty(item, xml, Pid::USER_LEN);
    writeProperty(item, xml, Pid::LINE_WIDTH);
    xml.endElement();
}

void TWrite::write(const StemSlash* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeItemProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::write(const Sticking* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const Symbol* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    xml.tag("name", SymNames::nameForSymId(item->sym()));
    if (item->scoreFont()) {
        xml.tag("font", item->scoreFont()->name());
    }
    writeProperties(static_cast<const BSymbol*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const System* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    if (item->systemDividerLeft() && item->systemDividerLeft()->isUserModified()) {
        write(item->systemDividerLeft(), xml, ctx);
    }
    if (item->systemDividerRight() && item->systemDividerRight()->isUserModified()) {
        write(item->systemDividerRight(), xml, ctx);
    }
    xml.endElement();
}

void TWrite::write(const SystemDivider* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item, { { "type", (item->dividerType() == SystemDivider::Type::LEFT ? "left" : "right") } });
    writeProperties(static_cast<const BSymbol*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const SystemText* item, XmlWriter& xml, WriteContext& ctx)
{
    write(static_cast<const StaffTextBase*>(item), xml, ctx);
}

void TWrite::write(const TempoText* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    xml.tag("tempo", TConv::toXml(item->tempo()));
    if (item->followText()) {
        xml.tag("followText", item->followText());
    }
    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const Text* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const TextLine* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    if (item->systemFlag()) {
        xml.startElement(item, { { "system", "1" } });
    } else {
        xml.startElement(item);
    }
    // other styled properties are included in TextLineBase pids list
    writeProperty(item, xml, Pid::PLACEMENT);
    writeProperty(item, xml, Pid::OFFSET);
    writeProperties(static_cast<const TextLineBase*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const Tie* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperties(static_cast<const SlurTie*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const TimeSig* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperty(item, xml, Pid::TIMESIG_TYPE);
    writeItemProperties(item, xml, ctx);

    xml.tag("sigN", item->numerator());
    xml.tag("sigD", item->denominator());
    if (item->stretch() != Fraction(1, 1)) {
        xml.tag("stretchN", item->stretch().numerator());
        xml.tag("stretchD", item->stretch().denominator());
    }
    writeProperty(item, xml, Pid::NUMERATOR_STRING);
    writeProperty(item, xml, Pid::DENOMINATOR_STRING);
    if (!item->groups().empty()) {
        write(&item->groups(), xml, ctx);
    }
    writeProperty(item, xml, Pid::SHOW_COURTESY);
    writeProperty(item, xml, Pid::SCALE);

    xml.endElement();
}

void TWrite::write(const Tremolo* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperty(item, xml, Pid::TREMOLO_TYPE);
    writeProperty(item, xml, Pid::TREMOLO_STYLE);
    writeItemProperties(item, xml, ctx);
    if (!item->twoNotes()) {
        xml.endElement();
        return;
    }
    // write manual adjustments to file
    int idx = (item->direction() == DirectionV::AUTO || item->direction() == DirectionV::DOWN) ? 0 : 1;
    if (item->userModified()) {
        double _spatium = item->spatium();

        xml.startElement("Fragment");
        xml.tag("y1", item->beamFragment().py1[idx] / _spatium);
        xml.tag("y2", item->beamFragment().py2[idx] / _spatium);
        xml.endElement();
    }

    // this info is used for regression testing
    // l1/l2 is the beam position of the layout engine
    if (MScore::testMode) {
        double spatium8 = item->spatium() * .125;
        xml.tag("l1", int(lrint(item->beamFragment().py1[idx] / spatium8)));
        xml.tag("l2", int(lrint(item->beamFragment().py2[idx] / spatium8)));
    }
    xml.endElement();
}
