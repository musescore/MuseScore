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
#include "../../style/textstyle.h"

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

#include "../../libmscore/hook.h"

#include "../../libmscore/lyrics.h"

#include "../../libmscore/note.h"

#include "../../libmscore/slur.h"
#include "../../libmscore/stem.h"
#include "../../libmscore/stemslash.h"

#include "../../libmscore/text.h"
#include "../../libmscore/textbase.h"
#include "../../libmscore/chordtextlinebase.h"
#include "../../libmscore/tremolo.h"

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

void TWrite::write(Bend* b, XmlWriter& xml, WriteContext& ctx)
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

void TWrite::write(Box* b, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(b);
    writeBoxProperties(b, xml, ctx);
    xml.endElement();
}

void TWrite::writeBoxProperties(Box* b, XmlWriter& xml, WriteContext& ctx)
{
    if (b->isHBox()) {
        return writeProperties(dynamic_cast<HBox*>(b), xml, ctx);
    }
    return writeProperties(b, xml, ctx);
}

void TWrite::writeProperties(Box* b, XmlWriter& xml, WriteContext& ctx)
{
    for (Pid id : {
        Pid::BOX_HEIGHT, Pid::BOX_WIDTH, Pid::TOP_GAP, Pid::BOTTOM_GAP,
        Pid::LEFT_MARGIN, Pid::RIGHT_MARGIN, Pid::TOP_MARGIN, Pid::BOTTOM_MARGIN, Pid::BOX_AUTOSIZE }) {
        writeProperty(b, xml, id);
    }
    writeItemProperties(b, xml, ctx);
    for (const EngravingItem* e : b->el()) {
        e->write(xml);
    }
}

void TWrite::write(HBox* b, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(b);
    writeProperties(b, xml, ctx);
    xml.endElement();
}

void TWrite::writeProperties(HBox* b, XmlWriter& xml, WriteContext& ctx)
{
    writeProperty(b, xml, Pid::CREATE_SYSTEM_HEADER);
    writeProperties(static_cast<Box*>(b), xml, ctx);
}

void TWrite::write(VBox* b, XmlWriter& xml, WriteContext& ctx)
{
    write(static_cast<Box*>(b), xml, ctx);
}

void TWrite::write(FBox* b, XmlWriter& xml, WriteContext& ctx)
{
    write(static_cast<Box*>(b), xml, ctx);
}

void TWrite::write(TBox* b, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(b);
    writeProperties(static_cast<Box*>(b), xml, ctx);
    b->text()->write(xml);
    xml.endElement();
}

void TWrite::write(Bracket* b, XmlWriter& xml, WriteContext& ctx)
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

void TWrite::write(Breath* b, XmlWriter& xml, WriteContext& ctx)
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
    for (Chord* c : c->graceNotes()) {
        write(c, xml, ctx);
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

void TWrite::writeChordRestBeam(const ChordRest* c, XmlWriter& xml, WriteContext& ctx)
{
    Beam* b = c->beam();
    if (b && b->elements().front() == c && (MScore::testMode || !b->generated())) {
        write(b, xml, ctx);
    }
}

void TWrite::writeProperties(const ChordRest* c, XmlWriter& xml, WriteContext& ctx)
{
    writeItemProperties(c, xml, ctx);

    //
    // BeamMode default:
    //    REST  - BeamMode::NONE
    //    CHORD - BeamMode::AUTO
    //
    if ((c->isRest() && c->beamMode() != BeamMode::NONE) || (c->isChord() && c->beamMode() != BeamMode::AUTO)) {
        xml.tag("BeamMode", TConv::toXml(c->beamMode()));
    }
    writeProperty(c, xml, Pid::SMALL);
    if (c->actualDurationType().dots()) {
        xml.tag("dots", c->actualDurationType().dots());
    }
    writeProperty(c, xml, Pid::STAFF_MOVE);

    if (c->actualDurationType().isValid()) {
        xml.tag("durationType", TConv::toXml(c->actualDurationType().type()));
    }

    if (!c->ticks().isZero() && (!c->actualDurationType().fraction().isValid()
                                 || (c->actualDurationType().fraction() != c->ticks()))) {
        xml.tagFraction("duration", c->ticks());
        //xml.tagE("duration z=\"%d\" n=\"%d\"", ticks().numerator(), ticks().denominator());
    }

    for (Lyrics* lyrics : c->lyrics()) {
        lyrics->write(xml);
    }

    const int curTick = ctx.curTick().ticks();

    if (!c->isGrace()) {
        Fraction t(c->globalTicks());
        if (c->staff()) {
            t /= c->staff()->timeStretch(ctx.curTick());
        }
        ctx.incCurTick(t);
    }

    for (auto i : c->score()->spannerMap().findOverlapping(curTick - 1, curTick + 1)) {
        Spanner* s = i.value;
        if (s->generated() || !s->isSlur() || toSlur(s)->broken() || !ctx.canWrite(s)) {
            continue;
        }

        if (s->startElement() == c) {
            s->writeSpannerStart(xml, c, c->track());
        } else if (s->endElement() == c) {
            s->writeSpannerEnd(xml, c, c->track());
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

void TWrite::writeProperties(const TextBase* t, XmlWriter& xml, WriteContext& ctx, bool writeText)
{
    writeItemProperties(t, xml, ctx);
    writeProperty(t, xml, Pid::TEXT_STYLE);

    for (const StyledProperty& spp : *t->styledProperties()) {
        if (!t->isStyled(spp.pid)) {
            writeProperty(t, xml, spp.pid);
        }
    }
    for (const auto& spp : *textStyle(t->textStyleType())) {
        if (t->isStyled(spp.pid)
            || (spp.pid == Pid::FONT_SIZE && t->getProperty(spp.pid).toDouble() == TextBase::UNDEFINED_FONT_SIZE)
            || (spp.pid == Pid::FONT_FACE && t->getProperty(spp.pid).value<String>() == TextBase::UNDEFINED_FONT_FAMILY)) {
            continue;
        }
        writeProperty(t, xml, spp.pid);
    }
    if (writeText) {
        xml.writeXml(u"text", t->xmlText());
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

void TWrite::write(const FiguredBassItem* f, XmlWriter& xml, WriteContext& ctx)
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
