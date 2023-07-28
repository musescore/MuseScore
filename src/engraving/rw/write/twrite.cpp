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
#include "../../infrastructure/rtti.h"

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
#include "../../libmscore/audio.h"

#include "../../libmscore/bagpembell.h"
#include "../../libmscore/barline.h"
#include "../../libmscore/beam.h"
#include "../../libmscore/bend.h"
#include "../../libmscore/stretchedbend.h"
#include "../../libmscore/box.h"
#include "../../libmscore/textframe.h"
#include "../../libmscore/bracket.h"
#include "../../libmscore/breath.h"

#include "../../libmscore/chord.h"
#include "../../libmscore/chordline.h"
#include "../../libmscore/chordrest.h"
#include "../../libmscore/clef.h"
#include "../../libmscore/capo.h"

#include "../../libmscore/drumset.h"
#include "../../libmscore/dynamic.h"
#include "../../libmscore/expression.h"
#include "../../libmscore/fermata.h"
#include "../../libmscore/figuredbass.h"
#include "../../libmscore/fingering.h"
#include "../../libmscore/fret.h"

#include "../../libmscore/glissando.h"
#include "../../libmscore/gradualtempochange.h"
#include "../../libmscore/groups.h"

#include "../../libmscore/hairpin.h"
#include "../../libmscore/harmony.h"
#include "../../libmscore/harmonicmark.h"
#include "../../libmscore/harppedaldiagram.h"
#include "../../libmscore/hook.h"

#include "../../libmscore/image.h"
#include "../../libmscore/imageStore.h"
#include "../../libmscore/instrument.h"
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
#include "../../libmscore/ornament.h"
#include "../../libmscore/ottava.h"

#include "../../libmscore/page.h"
#include "../../libmscore/palmmute.h"
#include "../../libmscore/part.h"
#include "../../libmscore/pedal.h"
#include "../../libmscore/playtechannotation.h"

#include "../../libmscore/rasgueado.h"
#include "../../libmscore/rehearsalmark.h"
#include "../../libmscore/rest.h"

#include "../../libmscore/sig.h"
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
#include "../../libmscore/stringdata.h"
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
#include "connectorinfowriter.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::write;

using WriteTypes = rtti::TypeList<Accidental, ActionIcon, Ambitus, Arpeggio, Articulation,
                                  BagpipeEmbellishment, BarLine, Beam, Bend, StretchedBend,  HBox, VBox, FBox, TBox, Bracket, Breath,
                                  Chord, ChordLine, Clef, Capo,
                                  Dynamic, Expression,
                                  Fermata, FiguredBass, Fingering, FretDiagram,
                                  Glissando, GradualTempoChange,
                                  Hairpin, Harmony, HarmonicMark, HarpPedalDiagram, Hook,
                                  Image, InstrumentChange,
                                  Jump,
                                  KeySig,
                                  LayoutBreak, LedgerLine, LetRing, Lyrics,
                                  Marker, MeasureNumber, MeasureRepeat, MMRest, MMRestRange,
                                  Note, NoteDot, NoteHead, NoteLine,
                                  Ornament, Ottava,
                                  Page, PalmMute, Pedal, PlayTechAnnotation,
                                  Rasgueado, RehearsalMark, Rest,
                                  Segment, Slur, Spacer, StaffState, StaffText, StaffTypeChange, Stem, StemSlash, Sticking,
                                  Symbol, FSymbol, System, SystemDivider, SystemText,
                                  TempoText, Text, TextLine, Tie, TimeSig, Tremolo, TremoloBar, Trill, Tuplet,
                                  Vibrato, Volta,
                                  WhammyBar>;

class WriteVisitor : public rtti::Visitor<WriteVisitor>
{
public:
    template<typename T>
    static bool doVisit(const EngravingItem* item, XmlWriter& xml, WriteContext& ctx)
    {
        if (T::classof(item)) {
            TWrite::write(static_cast<const T*>(item), xml, ctx);
            return true;
        }
        return false;
    }
};

void TWrite::writeItem(const EngravingItem* item, XmlWriter& xml, WriteContext& ctx)
{
    bool found = WriteVisitor::visit(WriteTypes {}, item, xml, ctx);
    DO_ASSERT(found);
}

void TWrite::writeItems(const ElementList& items, XmlWriter& xml, WriteContext& ctx)
{
    for (const EngravingItem* e : items) {
        writeItem(e, xml, ctx);
    }
}

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
        p = PropertyValue(Spatium::fromMM(f1, item->style().spatium()));
        d = PropertyValue();
    } else if (P_TYPE::POINT == type) {
        PointF p1 = p.value<PointF>();
        if (d.isValid()) {
            PointF p2 = d.value<PointF>();
            if ((std::abs(p1.x() - p2.x()) < 0.0001) && (std::abs(p1.y() - p2.y()) < 0.0001)) {
                return;
            }
        }
        double q = item->offsetIsSpatiumDependent() ? item->style().spatium() : DPMM;
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

void TWrite::writeItemProperties(const EngravingItem* item, XmlWriter& xml, WriteContext& ctx)
{
    bool autoplaceEnabled = item->score()->style().styleB(Sid::autoplaceEnabled);
    if (!autoplaceEnabled) {
        item->score()->style().set(Sid::autoplaceEnabled, true);
        writeProperty(item, xml, Pid::AUTOPLACE);
        item->score()->style().set(Sid::autoplaceEnabled, autoplaceEnabled);
    } else {
        writeProperty(item, xml, Pid::AUTOPLACE);
    }

    // copy paste should not keep links
    if (item->links() && (item->links()->size() > 1) && !ctx.clipboardmode()) {
        if (MScore::debugMode) {
            xml.tag("lid", item->links()->lid());
        }

        EngravingItem* me = static_cast<EngravingItem*>(item->links()->mainElement());
        DO_ASSERT(item->type() == me->type());
        Staff* s = item->staff();
        if (!s) {
            s = item->score()->staff(ctx.curTrack() / VOICES);
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
                write(&mainLoc, xml, ctx);
            }
            const int indexDiff = ctx.lidLocalIndex(item->links()->lid()) - guessedLocalIndex;
            xml.tag("indexDiff", indexDiff, 0);
            xml.endElement();       // </linked>
        }
    }
    if ((ctx.writeTrack() || item->track() != ctx.curTrack())
        && (item->track() != mu::nidx) && !item->isBeam() && !item->isTuplet()) {
        // Writing track number for beams and tuplets is redundant as it is calculated
        // during layout.
        int t = static_cast<int>(item->track()) + ctx.trackDiff();
        xml.tag("track", t);
    }
    if (ctx.writePosition()) {
        xml.tagProperty(Pid::POSITION, item->rtick());
    }

    for (Pid pid : { Pid::OFFSET, Pid::COLOR, Pid::VISIBLE, Pid::Z, Pid::PLACEMENT }) {
        if (item->propertyFlags(pid) == PropertyFlags::NOSTYLE) {
            writeProperty(item, xml, pid);
        }
    }
}

void TWrite::write(const Accidental* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperty(item, xml, Pid::ACCIDENTAL_BRACKET);
    writeProperty(item, xml, Pid::ACCIDENTAL_ROLE);
    writeProperty(item, xml, Pid::SMALL);
    writeProperty(item, xml, Pid::ACCIDENTAL_TYPE);
    writeItemProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::write(const ActionIcon* item, XmlWriter& xml, WriteContext&)
{
    xml.startElement(item);
    xml.tag("subtype", int(item->actionType()));
    if (!item->actionCode().empty()) {
        xml.tag("action", String::fromStdString(item->actionCode()));
    }
    xml.endElement();
}

void TWrite::write(const Ambitus* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    xml.tagProperty(Pid::HEAD_GROUP, int(item->noteHeadGroup()), int(Ambitus::NOTEHEADGROUP_DEFAULT));
    xml.tagProperty(Pid::HEAD_TYPE,  int(item->noteHeadType()),  int(Ambitus::NOTEHEADTYPE_DEFAULT));
    xml.tagProperty(Pid::MIRROR_HEAD, int(item->direction()),    int(Ambitus::DIRECTION_DEFAULT));
    xml.tag("hasLine",    item->hasLine(), true);
    xml.tagProperty(Pid::LINE_WIDTH_SPATIUM, item->lineWidth(), Ambitus::LINEWIDTH_DEFAULT);
    xml.tag("topPitch",   item->topPitch());
    xml.tag("topTpc",     item->topTpc());
    xml.tag("bottomPitch", item->bottomPitch());
    xml.tag("bottomTpc",  item->bottomTpc());
    if (item->topAccidental()->accidentalType() != AccidentalType::NONE) {
        xml.startElement("topAccidental");
        write(item->topAccidental(), xml, ctx);
        xml.endElement();
    }
    if (item->bottomAccidental()->accidentalType() != AccidentalType::NONE) {
        xml.startElement("bottomAccidental");
        write(item->bottomAccidental(), xml, ctx);
        xml.endElement();
    }
    writeItemProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::write(const Arpeggio* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeItemProperties(item, xml, ctx);
    writeProperty(item, xml, Pid::ARPEGGIO_TYPE);
    if (item->userLen1() != 0.0) {
        xml.tag("userLen1", item->userLen1() / item->spatium());
    }
    if (item->userLen2() != 0.0) {
        xml.tag("userLen2", item->userLen2() / item->spatium());
    }
    if (item->span() != 1) {
        xml.tag("span", item->span());
    }
    writeProperty(item, xml, Pid::PLAY);
    writeProperty(item, xml, Pid::TIME_STRETCH);
    xml.endElement();
}

void TWrite::write(const Articulation* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    if (toEngravingItem(item)->isOrnament()) {
        write(static_cast<const Ornament*>(item), xml, ctx);
        return;
    }
    xml.startElement(item);
    writeProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::writeProperties(const Articulation* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!item->channelName().isEmpty()) {
        xml.tag("channe", { { "name", item->channelName() } });
    }

    writeProperty(item, xml, Pid::DIRECTION);
    if (item->textType() != ArticulationTextType::NO_TEXT) {
        xml.tag("subtype", TConv::toXml(item->textType()));
    } else {
        xml.tag("subtype", SymNames::nameForSymId(item->symId()));
    }

    writeProperty(item, xml, Pid::PLAY);
    writeProperty(item, xml, Pid::ORNAMENT_STYLE);
    for (const StyledProperty& spp : *item->styledProperties()) {
        writeProperty(item, xml, spp.pid);
    }
    writeItemProperties(item, xml, ctx);
}

void TWrite::write(const Ornament* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);

    if (item->cueNoteChord()) {
        write(item->cueNoteChord(), xml, ctx);
    } else {
        if (item->accidentalAbove()) {
            write(item->accidentalAbove(), xml, ctx);
        }
        if (item->accidentalBelow()) {
            write(item->accidentalBelow(), xml, ctx);
        }
    }

    writeProperty(item, xml, Pid::INTERVAL_ABOVE);
    writeProperty(item, xml, Pid::INTERVAL_BELOW);
    writeProperty(item, xml, Pid::ORNAMENT_SHOW_ACCIDENTAL);
    writeProperty(item, xml, Pid::START_ON_UPPER_NOTE);
    writeProperties(static_cast<const Articulation*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const Audio* item, XmlWriter& xml, WriteContext&)
{
    xml.startElement("Audio");
    xml.tag("path", item->path());
    xml.endElement();
}

void TWrite::write(const BagpipeEmbellishment* item, XmlWriter& xml, WriteContext&)
{
    xml.startElement(item);
    xml.tag("subtype", TConv::toXml(item->embelType()));
    xml.endElement();
}

void TWrite::write(const BarLine* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);

    writeProperty(item, xml, Pid::BARLINE_TYPE);
    writeProperty(item, xml, Pid::BARLINE_SPAN);
    writeProperty(item, xml, Pid::BARLINE_SPAN_FROM);
    writeProperty(item, xml, Pid::BARLINE_SPAN_TO);

    for (const EngravingItem* e : *item->el()) {
        writeItem(e, xml, ctx);
    }
    writeItemProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::write(const Beam* item, XmlWriter& xml, WriteContext& ctx)
{
    if (item->elements().empty()) {
        return;
    }
    xml.startElement(item);
    writeItemProperties(item, xml, ctx);

    writeProperty(item, xml, Pid::STEM_DIRECTION);
    writeProperty(item, xml, Pid::BEAM_NO_SLOPE);
    writeProperty(item, xml, Pid::GROW_LEFT);
    writeProperty(item, xml, Pid::GROW_RIGHT);

    int idx = (item->beamDirection() == DirectionV::AUTO || item->beamDirection() == DirectionV::DOWN) ? 0 : 1;
    if (item->userModified()) {
        double _spatium = item->spatium();
        for (BeamFragment* f : item->beamFragments()) {
            xml.startElement("Fragment");
            xml.tag("y1", f->py1[idx] / _spatium);
            xml.tag("y2", f->py2[idx] / _spatium);
            xml.endElement();
        }
    }

    // this info is used for regression testing
    // l1/l2 is the beam position of the layout engine
    if (MScore::testMode) {
        double spatium8 = item->spatium() * .125;
        for (BeamFragment* f : item->beamFragments()) {
            xml.tag("l1", int(lrint(f->py1[idx] / spatium8)));
            xml.tag("l2", int(lrint(f->py2[idx] / spatium8)));
        }
    }

    xml.endElement();
}

void TWrite::write(const Bend* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    for (const PitchValue& v : item->points()) {
        xml.tag("point", { { "time", v.time }, { "pitch", v.pitch }, { "vibrato", v.vibrato } });
    }
    writeStyledProperties(item, xml);
    writeProperty(item, xml, Pid::PLAY);
    writeItemProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::write(const StretchedBend* item, XmlWriter& xml, WriteContext& ctx)
{
    write(static_cast<const Bend*>(item), xml, ctx);
}

void TWrite::write(const Box* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeBoxProperties(item, xml, ctx);
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
        writeItem(e, xml, ctx);
    }
}

void TWrite::write(const HBox* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::writeProperties(const HBox* item, XmlWriter& xml, WriteContext& ctx)
{
    writeProperty(item, xml, Pid::CREATE_SYSTEM_HEADER);
    writeProperties(static_cast<const Box*>(item), xml, ctx);
}

void TWrite::write(const VBox* item, XmlWriter& xml, WriteContext& ctx)
{
    write(static_cast<const Box*>(item), xml, ctx);
}

void TWrite::write(const FBox* item, XmlWriter& xml, WriteContext& ctx)
{
    write(static_cast<const Box*>(item), xml, ctx);
}

void TWrite::write(const TBox* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperties(static_cast<const Box*>(item), xml, ctx);
    write(item->text(), xml, ctx);
    xml.endElement();
}

void TWrite::write(const Bracket* item, XmlWriter& xml, WriteContext& ctx)
{
    bool isStartTag = false;
    switch (item->bracketItem()->bracketType()) {
    case BracketType::BRACE:
    case BracketType::SQUARE:
    case BracketType::LINE:
    {
        xml.startElement(item, { { "type", TConv::toXml(item->bracketItem()->bracketType()) } });
        isStartTag = true;
    }
    break;
    case BracketType::NORMAL:
        xml.startElement(item);
        isStartTag = true;
        break;
    case BracketType::NO_BRACKET:
        break;
    }

    if (isStartTag) {
        if (item->bracketItem()->column()) {
            xml.tag("level", static_cast<int>(item->bracketItem()->column()));
        }

        writeItemProperties(item, xml, ctx);

        xml.endElement();
    }
}

void TWrite::write(const Breath* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperty(item, xml, Pid::SYMBOL);
    writeProperty(item, xml, Pid::PAUSE);
    writeItemProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::write(const Chord* item, XmlWriter& xml, WriteContext& ctx)
{
    for (Chord* ch : item->graceNotes()) {
        write(ch, xml, ctx);
    }
    writeChordRestBeam(item, xml, ctx);
    xml.startElement(item);
    writeProperties(static_cast<const ChordRest*>(item), xml, ctx);
    for (const Articulation* a : item->articulations()) {
        write(a, xml, ctx);
    }
    switch (item->noteType()) {
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

    if (item->noStem()) {
        xml.tag("noStem", item->noStem());
    } else if (item->stem() && (item->stem()->isUserModified() || (item->stem()->userLength() != 0.0))) {
        write(item->stem(), xml, ctx);
    }
    if (item->hook() && item->hook()->isUserModified()) {
        write(item->hook(), xml, ctx);
    }
    if (item->stemSlash() && item->stemSlash()->isUserModified()) {
        write(item->stemSlash(), xml, ctx);
    }
    writeProperty(item, xml, Pid::STEM_DIRECTION);
    for (Note* n : item->notes()) {
        write(n, xml, ctx);
    }
    if (item->arpeggio()) {
        write(item->arpeggio(), xml, ctx);
    }
    if (item->tremolo() && item->tremoloChordType() != TremoloChordType::TremoloSecondNote) {
        write(item->tremolo(), xml, ctx);
    }
    for (EngravingItem* e : item->el()) {
        if (e->isChordLine() && toChordLine(e)->note()) { // this is now written by Note
            continue;
        }
        writeItem(e, xml, ctx);
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
        write(lyrics, xml, ctx);
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
            writeSpannerStart(s, xml, ctx, item, item->track());
        } else if (s->endElement() == item) {
            writeSpannerEnd(s, xml, ctx, item, item->track());
        }
    }
}

static Fraction fraction(bool clipboardmode, const EngravingItem* current, const Fraction& t)
{
    Fraction tick(t);
    if (!clipboardmode) {
        const Measure* m = toMeasure(current->findMeasure());
        if (m) {
            tick -= m->tick();
        }
    }
    return tick;
}

void TWrite::writeSpannerStart(Spanner* s, XmlWriter& xml, WriteContext& ctx, const EngravingItem* current, track_idx_t track,
                               Fraction tick)
{
    Fraction frac = fraction(ctx.clipboardmode(), current, tick);
    SpannerWriter w(xml, &ctx, current, s, static_cast<int>(track), frac, true);
    w.write();
}

void TWrite::writeSpannerEnd(Spanner* s, XmlWriter& xml, WriteContext& ctx, const EngravingItem* current, track_idx_t track, Fraction tick)
{
    Fraction frac = fraction(ctx.clipboardmode(), current, tick);
    SpannerWriter w(xml, &ctx, current, s, static_cast<int>(track), frac, false);
    w.write();
}

void TWrite::writeTupletStart(DurationElement* item, XmlWriter& xml, WriteContext& ctx)
{
    if (item->tuplet() && item->tuplet()->elements().front() == item) {
        writeTupletStart(item->tuplet(), xml, ctx);               // recursion
        TWrite::write(item->tuplet(), xml, ctx);
    }
}

void TWrite::writeTupletEnd(DurationElement* item, XmlWriter& xml, WriteContext& ctx)
{
    if (item->tuplet() && item->tuplet()->elements().back() == item) {
        xml.tag("endTuplet");
        writeTupletEnd(item->tuplet(), xml, ctx);               // recursion
    }
}

void TWrite::write(const ChordLine* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperty(item, xml, Pid::CHORD_LINE_TYPE);
    writeProperty(item, xml, Pid::CHORD_LINE_STRAIGHT);
    writeProperty(item, xml, Pid::CHORD_LINE_WAVY);
    xml.tag("lengthX", item->lengthX(), 0.0);
    xml.tag("lengthY", item->lengthY(), 0.0);
    writeItemProperties(item, xml, ctx);
    if (item->modified()) {
        const draw::PainterPath& path = item->path();
        size_t n = path.elementCount();
        xml.startElement("Path");
        for (size_t i = 0; i < n; ++i) {
            const PainterPath::Element& e = path.elementAt(i);
            double spatium = item->spatium();
            double x = e.x / spatium;
            double y = e.y / spatium;
            xml.tag("Element", { { "type", int(e.type) }, { "x", x }, { "y", y } });
        }
        xml.endElement();
    }
    xml.endElement();
}

void TWrite::write(const Clef* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperty(item, xml, Pid::CLEF_TYPE_CONCERT);
    writeProperty(item, xml, Pid::CLEF_TYPE_TRANSPOSING);
    writeProperty(item, xml, Pid::CLEF_TO_BARLINE_POS);
    writeProperty(item, xml, Pid::IS_HEADER);
    if (!item->showCourtesy()) {
        xml.tag("showCourtesyClef", item->showCourtesy());
    }
    if (item->forInstrumentChange()) {
        xml.tag("forInstrumentChange", item->forInstrumentChange());
    }
    writeItemProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::write(const Capo* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperty(item, xml, Pid::ACTIVE);
    writeProperty(item, xml, Pid::CAPO_FRET_POSITION);
    writeProperty(item, xml, Pid::CAPO_GENERATE_TEXT);

    std::set<string_idx_t> orderedStrings;
    for (string_idx_t idx : item->params().ignoredStrings) {
        orderedStrings.insert(idx);
    }

    for (string_idx_t idx : orderedStrings) {
        xml.startElement("string", { { "no", idx } });
        xml.tag("apply", false);
        xml.endElement();
    }

    writeProperties(static_cast<const StaffTextBase*>(item), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const Dynamic* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperty(item, xml, Pid::DYNAMIC_TYPE);
    writeProperty(item, xml, Pid::VELOCITY);
    writeProperty(item, xml, Pid::DYNAMIC_RANGE);
    writeProperty(item, xml, Pid::AVOID_BARLINES);
    writeProperty(item, xml, Pid::DYNAMICS_SIZE);
    writeProperty(item, xml, Pid::CENTER_ON_NOTEHEAD);

    if (item->isVelocityChangeAvailable()) {
        writeProperty(item, xml, Pid::VELO_CHANGE);
        writeProperty(item, xml, Pid::VELO_CHANGE_SPEED);
    }

    writeProperties(static_cast<const TextBase*>(item), xml, ctx, toDynamic(item)->hasCustomText());
    xml.endElement();
}

void TWrite::write(const Expression* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
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

void TWrite::write(const Fermata* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }

    xml.startElement(item);
    xml.tag("subtype", SymNames::nameForSymId(item->symId()));
    writeProperty(item, xml, Pid::TIME_STRETCH);
    writeProperty(item, xml, Pid::PLAY);
    writeProperty(item, xml, Pid::MIN_DISTANCE);
    if (!item->isStyled(Pid::OFFSET)) {
        writeProperty(item, xml, Pid::OFFSET);
    }
    writeItemProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::write(const FiguredBass* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }

    xml.startElement(item);
    if (!item->onNote()) {
        xml.tag("onNote", item->onNote());
    }
    if (item->ticks().isNotZero()) {
        xml.tagFraction("ticks", item->ticks());
    }
    // if unparseable items, write full text data
    if (item->items().size() < 1) {
        writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
    } else {
//            if (textStyleType() != StyledPropertyListIdx::FIGURED_BASS)
//                  // if all items parsed and not unstiled, we simply have a special style: write it
//                  xml.tag("style", textStyle().name());
        for (FiguredBassItem* fBItem : item->items()) {
            write(fBItem, xml, ctx);
        }
        for (const StyledProperty& spp : *item->styledProperties()) {
            writeProperty(item, xml, spp.pid);
        }
        writeItemProperties(item, xml, ctx);
    }
    xml.endElement();
}

void TWrite::write(const FiguredBassItem* item, XmlWriter& xml, WriteContext&)
{
    xml.startElement("FiguredBassItem", item);
    xml.tag("brackets", {
        { "b0", int(item->parenth1()) },
        { "b1", int(item->parenth2()) },
        { "b2", int(item->parenth3()) },
        { "b3", int(item->parenth4()) },
        { "b4", int(item->parenth5()) }
    });

    if (item->prefix() != FiguredBassItem::Modifier::NONE) {
        xml.tag("prefix", int(item->prefix()));
    }
    if (item->digit() != FBIDigitNone) {
        xml.tag("digit", item->digit());
    }
    if (item->suffix() != FiguredBassItem::Modifier::NONE) {
        xml.tag("suffix", int(item->suffix()));
    }
    if (item->contLine() != FiguredBassItem::ContLine::NONE) {
        xml.tag("continuationLine", int(item->contLine()));
    }
    xml.endElement();
}

void TWrite::write(const Fingering* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const FretDiagram* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);

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
        writeProperty(item, xml, p);
    }
    writeItemProperties(item, xml, ctx);

    if (item->harmony()) {
        write(item->harmony(), xml, ctx);
    }

    // Lowercase f indicates new writing format
    // TODO: in the next score format version (4) use only write new + props and discard
    // the compatibility writing.
    xml.startElement("fretDiagram");
    // writeNew (if want to make changes, do it here rather than in writeOld)
    {
        //    This is the important one for 3.1+
        //---------------------------------------------------------
        for (int i = 0; i < item->strings(); ++i) {
            FretItem::Marker m = item->marker(i);
            std::vector<FretItem::Dot> allDots = item->dot(i);

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

        for (int fi = 1; fi <= item->frets(); ++fi) {
            FretItem::Barre b = item->barre(fi);
            if (!b.exists()) {
                continue;
            }

            xml.tag("barre", { { "start", b.startString }, { "end", b.endString } }, fi);
        }
    }
    xml.endElement();

    // writeOld (for compatibility only)
    {
        int lowestDotFret = -1;
        int furthestLeftLowestDot = -1;

        // Do some checks for details needed for checking whether to add barres
        for (int i = 0; i < item->strings(); ++i) {
            std::vector<FretItem::Dot> allDots = item->dot(i);

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
        for (auto const& i : item->barres()) {
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

        for (int i = 0; i < item->strings(); ++i) {
            FretItem::Marker m = item->marker(i);
            std::vector<FretItem::Dot> allDots = item->dot(i);

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

void TWrite::write(const Glissando* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    if (item->showText() && !item->text().isEmpty()) {
        xml.tag("text", item->text());
    }

    for (auto id : { Pid::GLISS_TYPE, Pid::PLAY, Pid::GLISS_STYLE, Pid::GLISS_EASEIN, Pid::GLISS_EASEOUT }) {
        writeProperty(item, xml, id);
    }
    for (const StyledProperty& spp : *item->styledProperties()) {
        writeProperty(item, xml, spp.pid);
    }

    writeProperties(static_cast<const SLine*>(item), xml, ctx);
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
    double _spatium = item->style().spatium();
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

void TWrite::write(const GradualTempoChange* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperty(item, xml, Pid::TEMPO_CHANGE_TYPE);
    writeProperty(item, xml, Pid::TEMPO_EASING_METHOD);
    writeProperty(item, xml, Pid::TEMPO_CHANGE_FACTOR);
    writeProperty(item, xml, Pid::PLACEMENT);
    writeProperties(static_cast<const TextLineBase*>(item), xml, ctx);
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

void TWrite::write(const Groups* item, XmlWriter& xml, WriteContext&)
{
    xml.startElement("Groups");
    for (const GroupNode& n : item->nodes()) {
        xml.tag("Node", { { "pos", n.pos }, { "action", n.action } });
    }
    xml.endElement();
}

void TWrite::write(const Hairpin* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    xml.tag("subtype", int(item->hairpinType()));
    writeProperty(item, xml, Pid::VELO_CHANGE);
    writeProperty(item, xml, Pid::HAIRPIN_CIRCLEDTIP);
    writeProperty(item, xml, Pid::DYNAMIC_RANGE);
//      writeProperty(xml, Pid::BEGIN_TEXT);
    writeProperty(item, xml, Pid::END_TEXT);
//      writeProperty(xml, Pid::CONTINUE_TEXT);
    writeProperty(item, xml, Pid::LINE_VISIBLE);
    writeProperty(item, xml, Pid::SINGLE_NOTE_DYNAMICS);
    writeProperty(item, xml, Pid::VELO_CHANGE_METHOD);

    for (const StyledProperty& spp : *item->styledProperties()) {
        if (!item->isStyled(spp.pid)) {
            writeProperty(item, xml, spp.pid);
        }
    }
    writeProperties(static_cast<const SLine*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const Harmony* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperty(item, xml, Pid::HARMONY_TYPE);
    writeProperty(item, xml, Pid::PLAY);
    if (item->leftParen()) {
        xml.tag("leftParen");
    }
    if (item->rootTpc() != Tpc::TPC_INVALID || item->baseTpc() != Tpc::TPC_INVALID) {
        int rRootTpc = item->rootTpc();
        int rBaseTpc = item->baseTpc();
        if (item->staff()) {
            // parent can be a fret diagram
            Segment* segment = item->getParentSeg();
            Fraction tick = segment ? segment->tick() : Fraction(-1, 1);
            const Interval& interval = item->staff()->transpose(tick);
            if (ctx.clipboardmode() && !item->score()->style().styleB(Sid::concertPitch) && interval.chromatic) {
                rRootTpc = transposeTpc(item->rootTpc(), interval, true);
                rBaseTpc = transposeTpc(item->baseTpc(), interval, true);
            }
        }
        if (rRootTpc != Tpc::TPC_INVALID) {
            xml.tag("root", rRootTpc);
            if (item->rootCase() != NoteCaseType::CAPITAL) {
                xml.tag("rootCase", static_cast<int>(item->rootCase()));
            }
        }
        if (item->id() > 0) {
            xml.tag("extension", item->id());
        }
        // parser uses leading "=" as a hidden specifier for minor
        // this may or may not currently be incorporated into _textName
        String writeName = item->hTextName();
        if (item->parsedForm() && item->parsedForm()->name().startsWith(u'=') && !writeName.startsWith(u'=')) {
            writeName = u"=" + writeName;
        }
        if (!writeName.isEmpty()) {
            xml.tag("name", writeName);
        }

        if (rBaseTpc != Tpc::TPC_INVALID) {
            xml.tag("base", rBaseTpc);
            if (item->baseCase() != NoteCaseType::CAPITAL) {
                xml.tag("baseCase", static_cast<int>(item->baseCase()));
            }
        }
        for (const HDegree& hd : item->degreeList()) {
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
        xml.tag("name", item->hTextName());
    }
    if (!item->hFunction().isEmpty()) {
        xml.tag("function", item->hFunction());
    }
    writeProperties(static_cast<const TextBase*>(item), xml, ctx, false);
    //Pid::HARMONY_VOICE_LITERAL, Pid::HARMONY_VOICING, Pid::HARMONY_DURATION
    //written by the above function call because they are part of element style
    if (item->rightParen()) {
        xml.tag("rightParen");
    }
    xml.endElement();
}

void TWrite::write(const HarmonicMark* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperties(static_cast<const TextLineBase*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const HarpPedalDiagram* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperty(item, xml, Pid::HARP_IS_DIAGRAM);

    // Write vector of harp strings.  Order is always D, C, B, E, F, G, A
    xml.startElement("pedalState");
    for (size_t idx = 0; idx < item->getPedalState().size(); idx++) {
        xml.tag("string", { { "name", idx } }, static_cast<int>(item->getPedalState().at(idx)));
    }
    xml.endElement();

    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const Hook* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    xml.tag("name", SymNames::nameForSymId(item->sym()));
    if (item->scoreFont()) {
        xml.tag("font", item->scoreFont()->name());
    }
    writeProperties(static_cast<const BSymbol*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::writeProperties(const BSymbol* item, XmlWriter& xml, WriteContext& ctx)
{
    for (const EngravingItem* e : item->leafs()) {
        writeItem(e, xml, ctx);
    }
    writeItemProperties(item, xml, ctx);
}

void TWrite::write(const Image* item, XmlWriter& xml, WriteContext& ctx)
{
    // attempt to convert the _linkPath to a path relative to the score
    //
    // TODO : on Save As, score()->fileInfo() still contains the old path and fname
    //          if the Save As path is different, image relative path will be wrong!
    //
    String relativeFilePath;
    if (!item->linkPath().isEmpty() && item->linkIsValid()) {
        io::FileInfo fi(item->linkPath());
        // score()->fileInfo()->canonicalPath() would be better
        // but we are saving under a temp file name and the 'final' file
        // might not exist yet, so canonicalFilePath() may return only "/"
        // OTOH, the score 'final' file name is practically always canonical, at this point
        String scorePath = item->score()->masterScore()->fileInfo()->absoluteDirPath().toString();
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
        relativeFilePath = item->linkPath();
    }

    xml.startElement(item);
    writeProperties(static_cast<const BSymbol*>(item), xml, ctx);
    // keep old "path" tag, for backward compatibility and because it is used elsewhere
    // (for instance by Box:read(), Measure:read(), Note:read(), ...)
    xml.tag("path", item->storeItem() ? item->storeItem()->hashName() : relativeFilePath);
    xml.tag("linkPath", relativeFilePath);

    writeProperty(item, xml, Pid::AUTOSCALE);
    writeProperty(item, xml, Pid::SIZE);
    writeProperty(item, xml, Pid::LOCK_ASPECT_RATIO);
    writeProperty(item, xml, Pid::SIZE_IS_SPATIUM);

    xml.endElement();
}

void TWrite::write(const Instrument* item, XmlWriter& xml, WriteContext&, const Part* part)
{
    if (item->id().isEmpty()) {
        xml.startElement("Instrument");
    } else {
        xml.startElement("Instrument", { { "id", item->id() } });
    }
    write(&item->longNames(), xml, "longName");
    write(&item->shortNames(), xml, "shortName");
//      if (!_trackName.empty())
    xml.tag("trackName", item->trackName());
    if (item->minPitchP() > 0) {
        xml.tag("minPitchP", item->minPitchP());
    }
    if (item->maxPitchP() < 127) {
        xml.tag("maxPitchP", item->maxPitchP());
    }
    if (item->minPitchA() > 0) {
        xml.tag("minPitchA", item->minPitchA());
    }
    if (item->maxPitchA() < 127) {
        xml.tag("maxPitchA", item->maxPitchA());
    }
    if (item->transpose().diatonic) {
        xml.tag("transposeDiatonic", item->transpose().diatonic);
    }
    if (item->transpose().chromatic) {
        xml.tag("transposeChromatic", item->transpose().chromatic);
    }
    if (!item->musicXmlId().isEmpty()) {
        xml.tag("instrumentId", item->musicXmlId());
    }
    if (item->useDrumset()) {
        xml.tag("useDrumset", item->useDrumset());
        item->drumset()->save(xml);
    }
    for (size_t i = 0; i < item->cleffTypeCount(); ++i) {
        ClefTypeList ct = item->clefType(i);
        if (ct._concertClef == ct._transposingClef) {
            if (ct._concertClef != ClefType::G) {
                if (i) {
                    xml.tag("clef", { { "staff", i + 1 } }, TConv::toXml(ct._concertClef));
                } else {
                    xml.tag("clef", TConv::toXml(ct._concertClef));
                }
            }
        } else {
            if (i) {
                xml.tag("concertClef", { { "staff", i + 1 } }, TConv::toXml(ct._concertClef));
                xml.tag("transposingClef", { { "staff", i + 1 } }, TConv::toXml(ct._transposingClef));
            } else {
                xml.tag("concertClef", TConv::toXml(ct._concertClef));
                xml.tag("transposingClef", TConv::toXml(ct._transposingClef));
            }
        }
    }

    if (item->singleNoteDynamics() != item->getSingleNoteDynamicsFromTemplate()) {
        xml.tag("singleNoteDynamics", item->singleNoteDynamics());
    }

    if (!item->stringData()->isNull()) {
        write(item->stringData(), xml);
    }
    for (const NamedEventList& a : item->midiActions()) {
        write(&a, xml, "MidiAction");
    }
    for (const MidiArticulation& a : item->articulation()) {
        write(&a, xml);
    }
    for (const InstrChannel* a : item->channel()) {
        write(a, xml, part);
    }
    xml.endElement();
}

static void midi_event_write(const MidiCoreEvent& e, XmlWriter& xml)
{
    switch (e.type()) {
    case ME_NOTEON:
        xml.tag("note-on", { { "channel", e.channel() }, { "pitch", e.pitch() }, { "velo", e.velo() } });
        break;

    case ME_NOTEOFF:
        xml.tag("note-off", { { "channel", e.channel() }, { "pitch", e.pitch() }, { "velo", e.velo() } });
        break;

    case ME_CONTROLLER:
        if (e.controller() == CTRL_PROGRAM) {
            if (e.channel() == 0) {
                xml.tag("program", { { "value", e.value() } });
            } else {
                xml.tag("program", { { "channel", e.channel() }, { "value", e.value() } });
            }
        } else {
            if (e.channel() == 0) {
                xml.tag("controller", { { "ctrl", e.controller() }, { "value", e.value() } });
            } else {
                xml.tag("controller", { { "channel", e.channel() }, { "ctrl", e.controller() }, { "value", e.value() } });
            }
        }
        break;
    default:
        LOGD("MidiCoreEvent::write: unknown type");
        break;
    }
}

void TWrite::write(const InstrChannel* item, XmlWriter& xml, const Part* part)
{
    if (item->name().isEmpty() || item->name() == InstrChannel::DEFAULT_NAME) {
        xml.startElement("Channel");
    } else {
        xml.startElement("Channel", { { "name", item->name() } });
    }
    if (item->color() != InstrChannel::DEFAULT_COLOR) {
        xml.tag("color", item->color());
    }

    for (const MidiCoreEvent& e : item->initList()) {
        if (e.type() == ME_INVALID) {
            continue;
        }
        if (e.type() == ME_CONTROLLER) {
            // Don't write bank if automatically switched, but always write if switched by the user
            if (e.dataA() == CTRL_HBANK && e.dataB() == 0 && !item->userBankController()) {
                continue;
            }
            if (e.dataA() == CTRL_LBANK && e.dataB() == 0 && !item->userBankController()) {
                continue;
            }
            if (e.dataA() == CTRL_VOLUME && e.dataB() == 100) {
                continue;
            }
            if (e.dataA() == CTRL_PANPOT && e.dataB() == 64) {
                continue;
            }
            if (e.dataA() == CTRL_REVERB_SEND && e.dataB() == 0) {
                continue;
            }
            if (e.dataA() == CTRL_CHORUS_SEND && e.dataB() == 0) {
                continue;
            }
        }

        midi_event_write(e, xml);
    }
    if (!MScore::testMode) {
        // xml.tag("synti", ::synti->name(synti));
        xml.tag("synti", item->synti());
    }

    if (part && part->masterScore()->exportMidiMapping() && part->score() == part->masterScore()) {
        xml.tag("midiPort",    part->masterScore()->midiMapping(item->channel())->port());
        xml.tag("midiChannel", part->masterScore()->midiMapping(item->channel())->channel());
    }
    for (const NamedEventList& a : item->midiActions) {
        write(&a, xml, "MidiAction");
    }
    for (const MidiArticulation& a : item->articulation) {
        write(&a, xml);
    }
    xml.endElement();
}

void TWrite::write(const MidiArticulation* item, XmlWriter& xml)
{
    if (item->name.isEmpty()) {
        xml.startElement("Articulation");
    } else {
        xml.startElement("Articulation", { { "name", item->name } });
    }
    if (!item->descr.isEmpty()) {
        xml.tag("descr", item->descr);
    }
    xml.tag("velocity", item->velocity);
    xml.tag("gateTime", item->gateTime);
    xml.endElement();
}

void TWrite::write(const StaffName* item, XmlWriter& xml, const char* tag)
{
    if (!item->name().isEmpty()) {
        if (item->pos() == 0) {
            xml.writeXml(String::fromUtf8(tag), item->name());
        } else {
            xml.writeXml(String(u"%1 pos=\"%2\"").arg(String::fromUtf8(tag)).arg(item->pos()), item->name());
        }
    }
}

void TWrite::write(const StaffNameList* item, XmlWriter& xml, const char* name)
{
    for (const StaffName& sn : *item) {
        write(&sn, xml, name);
    }
}

void TWrite::write(const NamedEventList* item, XmlWriter& xml, const AsciiStringView& n)
{
    xml.startElement(n, { { "name", item->name } });
    if (!item->descr.empty()) {
        xml.tag("descr", item->descr);
    }
    for (const MidiCoreEvent& e : item->events) {
        midi_event_write(e, xml);
    }
    xml.endElement();
}

void TWrite::write(const InstrumentChange* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    write(item->instrument(), xml, ctx, item->part());
    if (item->init()) {
        xml.tag("init", item->init());
    }
    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const Jump* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
    xml.tag("jumpTo", item->jumpTo());
    xml.tag("playUntil", item->playUntil());
    xml.tag("continueAt", item->continueAt());
    writeProperty(item, xml, Pid::PLAY_REPEATS);
    xml.endElement();
}

void TWrite::write(const KeySig* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeItemProperties(item, xml, ctx);
    if (item->isAtonal()) {
        xml.tag("custom", 1);
    } else {
        xml.tag("concertKey", int(item->concertKey()));
        if (item->concertKey() != item->key()) {
            xml.tag("actualKey", int(item->key()));
        }
        if (item->isCustom()) {
            xml.tag("custom", 1);
            for (const CustDef& cd : item->customKeyDefs()) {
                xml.startElement("CustDef");
                xml.tag("sym", SymNames::nameForSymId(cd.sym));
                xml.tag("def", { { "degree", cd.degree }, { "xAlt", cd.xAlt }, { "octAlt", cd.octAlt } });
                xml.endElement();
            }
        }
    }

    if (item->mode() != KeyMode::UNKNOWN) {
        xml.tag("mode", TConv::toXml(item->mode()));
    }

    if (!item->showCourtesy()) {
        xml.tag("showCourtesySig", item->showCourtesy());
    }
    if (item->forInstrumentChange()) {
        xml.tag("forInstrumentChange", true);
    }
    xml.endElement();
}

void TWrite::write(const LayoutBreak* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeItemProperties(item, xml, ctx);

    for (auto id :
         { Pid::LAYOUT_BREAK, Pid::PAUSE, Pid::START_WITH_LONG_NAMES, Pid::START_WITH_MEASURE_ONE, Pid::FIRST_SYSTEM_INDENTATION }) {
        writeProperty(item, xml, id);
    }

    xml.endElement();
}

void TWrite::write(const LedgerLine* item, XmlWriter& xml, WriteContext&)
{
    xml.startElement(item);
    xml.tag("lineWidth", item->width() / item->spatium());
    xml.tag("lineLen", item->len() / item->spatium());
    if (!item->vertical()) {
        xml.tag("vertical", item->vertical());
    }
    xml.endElement();
}

void TWrite::write(const LetRing* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperties(static_cast<const TextLineBase*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const Location* item, XmlWriter& xml, WriteContext&)
{
    static constexpr Location relDefaults = Location::relative();

    DO_ASSERT(item->isRelative());
    xml.startElement("location");
    xml.tag("staves", item->staff(), relDefaults.staff());
    xml.tag("voices", item->voice(), relDefaults.voice());
    xml.tag("measures", item->measure(), relDefaults.measure());
    xml.tagFraction("fractions", item->frac().reduced(), relDefaults.frac());
    xml.tag("grace", item->graceIndex(), relDefaults.graceIndex());
    xml.tag("notes", item->note(), relDefaults.note());
    xml.endElement();
}

void TWrite::write(const Lyrics* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperty(item, xml, Pid::VERSE);
    if (item->syllabic() != LyricsSyllabic::SINGLE) {
        xml.tag("syllabic", TConv::toXml(item->syllabic()));
    }
    xml.tag("ticks", item->ticks().ticks(), 0);   // pre-3.1 compatibility: write integer ticks under <ticks> tag
    writeProperty(item, xml, Pid::LYRIC_TICKS);

    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const Marker* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
    xml.tag("label", item->label());
    xml.endElement();
}

void TWrite::write(const MeasureNumber* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const MeasureRepeat* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperty(item, xml, Pid::SUBTYPE);
    writeProperties(static_cast<const Rest*>(item), xml, ctx);
    writeItems(item->el(), xml, ctx);
    xml.endElement();
}

void TWrite::writeProperties(const Rest* item, XmlWriter& xml, WriteContext& ctx)
{
    writeProperties(static_cast<const ChordRest*>(item), xml, ctx);
}

void TWrite::write(const MMRest* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement("Rest"); // for compatibility, see also Measure::readVoice()
    writeProperties(static_cast<const ChordRest*>(item), xml, ctx);
    writeProperty(item, xml, Pid::MMREST_NUMBER_POS);
    writeProperty(item, xml, Pid::MMREST_NUMBER_VISIBLE);
    writeItems(item->el(), xml, ctx);
    xml.endElement();
}

void TWrite::write(const MMRestRange* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const Note* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeItemProperties(item, xml, ctx);

    if (item->accidental()) {
        write(item->accidental(), xml, ctx);
    }
    writeItems(item->el(), xml, ctx);
    bool write_dots = false;
    for (NoteDot* dot : item->dots()) {
        if (!dot->offset().isNull() || !dot->visible() || dot->color() != engravingConfiguration()->defaultColor()
            || dot->visible() != item->visible()) {
            write_dots = true;
            break;
        }
    }
    if (write_dots) {
        for (NoteDot* dot : item->dots()) {
            write(dot, xml, ctx);
        }
    }

    if (item->tieFor()) {
        writeSpannerStart(item->tieFor(), xml, ctx, item, item->track());
    }
    if (item->tieBack()) {
        writeSpannerEnd(item->tieBack(), xml, ctx, item, item->track());
    }
    if ((item->chord() == 0 || item->chord()->playEventType() != PlayEventType::Auto) && !item->playEvents().empty()) {
        xml.startElement("Events");
        for (const NoteEvent& e : item->playEvents()) {
            write(&e, xml, ctx);
        }
        xml.endElement();
    }
    for (Pid id : { Pid::PITCH, Pid::TPC1, Pid::TPC2, Pid::SMALL, Pid::MIRROR_HEAD, Pid::DOT_POSITION,
                    Pid::HEAD_SCHEME, Pid::HEAD_GROUP, Pid::USER_VELOCITY, Pid::PLAY, Pid::TUNING, Pid::FRET, Pid::STRING,
                    Pid::GHOST, Pid::DEAD, Pid::HEAD_TYPE, Pid::FIXED, Pid::FIXED_LINE }) {
        writeProperty(item, xml, id);
    }

    for (Spanner* e : item->spannerFor()) {
        writeSpannerStart(e, xml, ctx, item, item->track());
    }
    for (Spanner* e : item->spannerBack()) {
        writeSpannerEnd(e, xml, ctx, item, item->track());
    }

    for (EngravingItem* e : item->chord()->el()) {
        if (e->isChordLine() && toChordLine(e)->note() && toChordLine(e)->note() == item) {
            write(toChordLine(e), xml, ctx);
        }
    }

    xml.endElement();
}

void TWrite::write(const NoteEvent* item, XmlWriter& xml, WriteContext&)
{
    xml.startElement("Event");
    xml.tag("pitch", item->pitch(), 0);
    xml.tag("ontime", item->ontime(), 0);
    xml.tag("len", item->len(), NoteEvent::NOTE_LENGTH);
    xml.endElement();
}

void TWrite::write(const NoteDot* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeItemProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::write(const NoteHead* item, XmlWriter& xml, WriteContext& ctx)
{
    write(static_cast<const Symbol*>(item), xml, ctx);
}

void TWrite::write(const NoteLine* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperties(static_cast<const TextLineBase*>(item), xml, ctx);
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

void TWrite::write(const Page* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    for (System* system : item->systems()) {
        write(system, xml, ctx);
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

void TWrite::write(const Part* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item, { { "id", item->id().toUint64() } });

    for (const Staff* staff : item->staves()) {
        write(staff, xml, ctx);
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

    if (item->preferSharpFlat() != PreferSharpFlat::AUTO) {
        switch (item->preferSharpFlat()) {
        case PreferSharpFlat::AUTO:
            break;
        case PreferSharpFlat::FLATS:
            xml.tag("preferSharpFlat", "flats");
            break;
        case PreferSharpFlat::SHARPS:
            xml.tag("preferSharpFlat", "flats");
            break;
        case PreferSharpFlat::NONE:
            xml.tag("preferSharpFlat", "none");
            break;
        default:
            break;
        }
    }

    write(item->instrument(), xml, ctx, item);

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

void TWrite::write(const Rasgueado* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperties(static_cast<const TextLineBase*>(item), xml, ctx);
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
    writeItems(item->el(), xml, ctx);
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
        writeSlur(((SlurTieSegment*)ss), xml, ctx, idx++);
    }
    writeProperty(item, xml, Pid::SLUR_DIRECTION);
    writeProperty(item, xml, Pid::SLUR_STYLE_TYPE);
}

void TWrite::writeSlur(const SlurTieSegment* seg, XmlWriter& xml, WriteContext& ctx, int no)
{
    if (seg->visible() && seg->autoplace()
        && (seg->color() == engravingConfiguration()->defaultColor())
        && seg->offset().isNull()
        && seg->ups(Grip::START).off.isNull()
        && seg->ups(Grip::BEZIER1).off.isNull()
        && seg->ups(Grip::BEZIER2).off.isNull()
        && seg->ups(Grip::END).off.isNull()
        ) {
        return;
    }

    xml.startElement(seg, { { "no", no } });

    double _spatium = seg->style().spatium();
    if (!seg->ups(Grip::START).off.isNull()) {
        xml.tagPoint("o1", seg->ups(Grip::START).off / _spatium);
    }
    if (!seg->ups(Grip::BEZIER1).off.isNull()) {
        xml.tagPoint("o2", seg->ups(Grip::BEZIER1).off / _spatium);
    }
    if (!seg->ups(Grip::BEZIER2).off.isNull()) {
        xml.tagPoint("o3", seg->ups(Grip::BEZIER2).off / _spatium);
    }
    if (!seg->ups(Grip::END).off.isNull()) {
        xml.tagPoint("o4", seg->ups(Grip::END).off / _spatium);
    }
    writeItemProperties(seg, xml, ctx);
    xml.endElement();
}

void TWrite::write(const Spacer* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    xml.tag("subtype", int(item->spacerType()));
    writeItemProperties(item, xml, ctx);
    xml.tag("space", item->gap().val() / item->spatium());
    xml.endElement();
}

void TWrite::write(const Staff* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item, { { "id", item->idx() + 1 } });

    if (item->links()) {
        Score* s = item->masterScore();
        for (auto le : *item->links()) {
            Staff* staff = toStaff(le);
            if ((staff->score() == s) && (staff != item)) {
                xml.tag("linkedTo", static_cast<int>(staff->idx() + 1));
            }
        }
    }

    // for copy/paste we need to know the actual transposition
    if (ctx.clipboardmode()) {
        Interval v = item->part()->instrument()->transpose();     // TODO: tick?
        if (v.diatonic) {
            xml.tag("transposeDiatonic", v.diatonic);
        }
        if (v.chromatic) {
            xml.tag("transposeChromatic", v.chromatic);
        }
    }

    write(item->staffType(Fraction(0, 1)), xml, ctx);
    ClefTypeList ct = item->defaultClefType();
    if (ct._concertClef == ct._transposingClef) {
        if (ct._concertClef != ClefType::G) {
            xml.tag("defaultClef", TConv::toXml(ct._concertClef));
        }
    } else {
        xml.tag("defaultConcertClef", TConv::toXml(ct._concertClef));
        xml.tag("defaultTransposingClef", TConv::toXml(ct._transposingClef));
    }

    if (item->isLinesInvisible(Fraction(0, 1))) {
        xml.tag("invisible", item->isLinesInvisible(Fraction(0, 1)));
    }
    if (item->hideWhenEmpty() != Staff::HideMode::AUTO) {
        xml.tag("hideWhenEmpty", int(item->hideWhenEmpty()));
    }
    if (item->cutaway()) {
        xml.tag("cutaway", item->cutaway());
    }
    if (item->showIfEmpty()) {
        xml.tag("showIfSystemEmpty", item->showIfEmpty());
    }
    if (item->hideSystemBarLine()) {
        xml.tag("hideSystemBarLine", item->hideSystemBarLine());
    }
    if (item->mergeMatchingRests()) {
        xml.tag("mergeMatchingRests", item->mergeMatchingRests());
    }
    if (!item->visible()) {
        xml.tag("isStaffVisible", item->visible());
    }

    for (const BracketItem* i : item->brackets()) {
        BracketType a = i->bracketType();
        size_t b = i->bracketSpan();
        size_t c = i->column();
        bool v = i->visible();
        if (a != BracketType::NO_BRACKET || b > 0) {
            xml.tag("bracket", { { "type", static_cast<int>(a) }, { "span", b }, { "col", c }, { "visible", v } });
        }
    }

    writeProperty(item, xml, Pid::STAFF_BARLINE_SPAN);
    writeProperty(item, xml, Pid::STAFF_BARLINE_SPAN_FROM);
    writeProperty(item, xml, Pid::STAFF_BARLINE_SPAN_TO);
    writeProperty(item, xml, Pid::STAFF_USERDIST);
    writeProperty(item, xml, Pid::STAFF_COLOR);
    writeProperty(item, xml, Pid::PLAYBACK_VOICE1);
    writeProperty(item, xml, Pid::PLAYBACK_VOICE2);
    writeProperty(item, xml, Pid::PLAYBACK_VOICE3);
    writeProperty(item, xml, Pid::PLAYBACK_VOICE4);

    xml.endElement();
}

void TWrite::write(const StaffState* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    xml.tag("subtype", int(item->staffStateType()));
    if (item->staffStateType() == StaffStateType::INSTRUMENT) {
        write(item->instrument(), xml, ctx, nullptr);
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
        if (item->swingParameters().swingUnit == Constants::DIVISION / 2) {
            swingUnit = DurationType::V_EIGHTH;
        } else if (item->swingParameters().swingUnit == Constants::DIVISION / 4) {
            swingUnit = DurationType::V_16TH;
        } else {
            swingUnit = DurationType::V_ZERO;
        }
        int swingRatio = item->swingParameters().swingRatio;
        xml.tag("swing", { { "unit", TConv::toXml(swingUnit) }, { "ratio", swingRatio } });
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

void TWrite::write(const StringData* item, XmlWriter& xml)
{
    xml.startElement("StringData");
    xml.tag("frets", item->frets());
    for (const instrString& strg : item->stringList()) {
        if (strg.open) {
            xml.tag("string open=\"1\"", strg.pitch);
        } else {
            xml.tag("string", strg.pitch);
        }
    }
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

void TWrite::write(const FSymbol* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    xml.tag("font",     item->font().family());
    xml.tag("fontsize", item->font().pointSizeF());
    xml.tag("code",     item->code());
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

void TWrite::write(const TremoloBar* item, XmlWriter& xml, WriteContext&)
{
    xml.startElement(item);
    writeProperty(item, xml, Pid::MAG);
    writeProperty(item, xml, Pid::LINE_WIDTH);
    writeProperty(item, xml, Pid::PLAY);
    for (const PitchValue& v : item->points()) {
        xml.tag("point", { { "time", v.time }, { "pitch", v.pitch }, { "vibrato", v.vibrato } });
    }
    xml.endElement();
}

void TWrite::write(const Trill* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    xml.tag("subtype", TConv::toXml(item->trillType()));
    writeProperty(item, xml, Pid::PLAY);
    writeProperty(item, xml, Pid::ORNAMENT_STYLE);
    writeProperty(item, xml, Pid::PLACEMENT);
    writeProperties(static_cast<const SLine*>(item), xml, ctx);
    if (item->ornament()) {
        write(item->ornament(), xml, ctx);
    }
    xml.endElement();
}

void TWrite::write(const Tuplet* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeItemProperties(item, xml, ctx);

    writeProperty(item, xml, Pid::NORMAL_NOTES);
    writeProperty(item, xml, Pid::ACTUAL_NOTES);
    writeProperty(item, xml, Pid::P1);
    writeProperty(item, xml, Pid::P2);

    xml.tag("baseNote", TConv::toXml(item->baseLen().type()));
    if (int dots = item->baseLen().dots()) {
        xml.tag("baseDots", dots);
    }

    if (item->number()) {
        xml.startElement("Number", item->number());
        writeProperty(item->number(), xml, Pid::TEXT_STYLE);
        writeProperty(item->number(), xml, Pid::TEXT);
        xml.endElement();
    }

    writeStyledProperties(item, xml);

    xml.endElement();
}

void TWrite::write(const Vibrato* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    xml.tag("subtype", TConv::toXml(item->vibratoType()));
    writeProperty(item, xml, Pid::PLAY);
    for (const StyledProperty& spp : *item->styledProperties()) {
        writeProperty(item, xml, spp.pid);
    }
    writeProperties(static_cast<const SLine*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const Volta* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperties(static_cast<const TextLineBase*>(item), xml, ctx);
    xml.tag("endings", TConv::toXml(item->endings()));
    xml.endElement();
}

void TWrite::write(const WhammyBar* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperties(static_cast<const TextLineBase*>(item), xml, ctx);
    xml.endElement();
}

//---------------------------------------------------------
//   writeVoiceMove
//    write <move> and starting <voice> tags to denote
//    change in position.
//    Returns true if <voice> tag was written.
//---------------------------------------------------------

static bool writeVoiceMove(XmlWriter& xml, WriteContext& ctx, Segment* seg, const Fraction& startTick, track_idx_t track,
                           int* lastTrackWrittenPtr)
{
    bool voiceTagWritten = false;
    int& lastTrackWritten = *lastTrackWrittenPtr;
    if ((lastTrackWritten < static_cast<int>(track)) && !ctx.clipboardmode()) {
        while (lastTrackWritten < (static_cast < int > (track) - 1)) {
            xml.tag("voice");
            ++lastTrackWritten;
        }
        xml.startElement("voice");
        ctx.setCurTick(startTick);
        ctx.setCurTrack(track);
        ++lastTrackWritten;
        voiceTagWritten = true;
    }

    if ((ctx.curTick() != seg->tick()) || (track != ctx.curTrack())) {
        Location curr = Location::absolute();
        Location dest = Location::absolute();
        curr.setFrac(ctx.curTick());
        dest.setFrac(seg->tick());
        curr.setTrack(static_cast<int>(ctx.curTrack()));
        dest.setTrack(static_cast<int>(track));

        dest.toRelative(curr);
        TWrite::write(&dest, xml, ctx);

        ctx.setCurTick(seg->tick());
        ctx.setCurTrack(track);
    }

    return voiceTagWritten;
}

//---------------------------------------------------------
//   writeSegments
//    ls  - write upto this segment (excluding)
//          can be zero
//---------------------------------------------------------

void TWrite::writeSegments(XmlWriter& xml, WriteContext& ctx, track_idx_t strack, track_idx_t etrack,
                           Segment* sseg, Segment* eseg, bool writeSystemElements, bool forceTimeSig)
{
    Score* score = sseg->score();
    Fraction startTick = ctx.curTick();
    Fraction endTick   = eseg ? eseg->tick() : score->lastMeasure()->endTick();
    bool clip          = ctx.clipboardmode();

    // in clipboard mode, ls might be in an mmrest
    // since we are traversing regular measures,
    // force them out of mmRest
    if (clip) {
        Measure* lm = eseg ? eseg->measure() : 0;
        Measure* fm = sseg ? sseg->measure() : 0;
        if (lm && lm->isMMRest()) {
            lm = lm->mmRestLast();
            if (lm) {
                eseg = lm->nextMeasure() ? lm->nextMeasure()->first() : nullptr;
            } else {
                LOGD("writeSegments: no measure for end segment in mmrest");
            }
        }
        if (fm && fm->isMMRest()) {
            fm = fm->mmRestFirst();
            if (fm) {
                sseg = fm->first();
            }
        }
    }

    std::list<Spanner*> spanners;
    auto sl = score->spannerMap().findOverlapping(sseg->tick().ticks(), endTick.ticks());
    for (auto i : sl) {
        Spanner* s = i.value;
        if (s->generated() || !ctx.canWrite(s)) {
            continue;
        }
        // don't write voltas to clipboard
        if (clip && s->isVolta() && s->systemFlag()) {
            continue;
        }
        spanners.push_back(s);
    }

    int lastTrackWritten = static_cast<int>(strack - 1);   // for counting necessary <voice> tags
    for (track_idx_t track = strack; track < etrack; ++track) {
        if (!ctx.canWriteVoice(track)) {
            continue;
        }

        bool voiceTagWritten = false;

        bool timeSigWritten = false;     // for forceTimeSig
        bool crWritten = false;          // for forceTimeSig
        bool keySigWritten = false;      // for forceTimeSig

        for (Segment* segment = sseg; segment && segment != eseg; segment = segment->next1()) {
            if (!segment->enabled()) {
                continue;
            }
            if (track == 0) {
                segment->setWritten(false);
            }
            EngravingItem* e = segment->element(track);

            //
            // special case: - barline span > 1
            //               - part (excerpt) staff starts after
            //                 barline element
            bool needMove = (segment->tick() != ctx.curTick() || (static_cast<int>(track) > lastTrackWritten));
            if ((segment->isEndBarLineType()) && !e && writeSystemElements && ((track % VOICES) == 0)) {
                // search barline:
                for (int idx = static_cast<int>(track - VOICES); idx >= 0; idx -= static_cast<int>(VOICES)) {
                    if (segment->element(idx)) {
                        int oDiff = ctx.trackDiff();
                        ctx.setTrackDiff(idx);                      // staffIdx should be zero
                        TWrite::writeItem(segment->element(idx), xml, ctx);
                        ctx.setTrackDiff(oDiff);
                        break;
                    }
                }
            }
            for (EngravingItem* e1 : segment->annotations()) {
                if (e1->generated()) {
                    continue;
                }
                bool writeSystem = writeSystemElements;
                if (!writeSystem) {
                    ElementType et = e1->type();
                    if ((et == ElementType::REHEARSAL_MARK)
                        || (et == ElementType::SYSTEM_TEXT)
                        || (et == ElementType::TRIPLET_FEEL)
                        || (et == ElementType::PLAYTECH_ANNOTATION)
                        || (et == ElementType::CAPO)
                        || (et == ElementType::JUMP)
                        || (et == ElementType::MARKER)
                        || (et == ElementType::TEMPO_TEXT)
                        || (et == ElementType::VOLTA)
                        || (et == ElementType::GRADUAL_TEMPO_CHANGE)) {
                        writeSystem = (e1->track() == track); // always show these on appropriate staves
                    }
                }
                if (e1->track() != track || (e1->systemFlag() && !writeSystem)) {
                    continue;
                }
                if (needMove) {
                    voiceTagWritten |= writeVoiceMove(xml, ctx, segment, startTick, track, &lastTrackWritten);
                    needMove = false;
                }
                TWrite::writeItem(e1, xml, ctx);
            }
            Measure* m = segment->measure();
            // don't write spanners for multi measure rests

            if ((!(m && m->isMMRest())) && segment->isChordRestType()) {
                for (Spanner* s : spanners) {
                    if (s->track() == track) {
                        bool end = false;
                        if (s->anchor() == Spanner::Anchor::CHORD || s->anchor() == Spanner::Anchor::NOTE) {
                            end = s->tick2() < endTick;
                        } else {
                            end = s->tick2() <= endTick;
                        }
                        if (s->tick() == segment->tick() && (!clip || end) && !s->isSlur()) {
                            if (needMove) {
                                voiceTagWritten |= writeVoiceMove(xml, ctx, segment, startTick, track, &lastTrackWritten);
                                needMove = false;
                            }
                            writeSpannerStart(s, xml, ctx, segment, track);
                        }
                    }
                    if ((s->tick2() == segment->tick())
                        && !s->isSlur()
                        && (s->effectiveTrack2() == track)
                        && (!clip || s->tick() >= sseg->tick())
                        ) {
                        if (needMove) {
                            voiceTagWritten |= writeVoiceMove(xml, ctx, segment, startTick, track, &lastTrackWritten);
                            needMove = false;
                        }
                        writeSpannerEnd(s, xml, ctx, segment, track);
                    }
                }
            }

            if (!e || !ctx.canWrite(e)) {
                continue;
            }
            if (e->generated()) {
                continue;
            }
            if (forceTimeSig && track2voice(track) == 0 && segment->segmentType() == SegmentType::ChordRest && !timeSigWritten
                && !crWritten) {
                // Ensure that <voice> tag is open
                voiceTagWritten |= writeVoiceMove(xml, ctx, segment, startTick, track, &lastTrackWritten);
                // we will miss a key sig!
                if (!keySigWritten) {
                    Key ck = score->staff(track2staff(track))->concertKey(segment->tick());
                    Key tk = score->staff(track2staff(track))->key(segment->tick());
                    KeySig* ks = Factory::createKeySig(score->dummy()->segment());
                    ks->setKey(ck, tk);
                    TWrite::write(ks, xml, ctx);
                    delete ks;
                    keySigWritten = true;
                }
                // we will miss a time sig!
                Fraction tsf = score->sigmap()->timesig(segment->tick()).timesig();
                TimeSig* ts = Factory::createTimeSig(score->dummy()->segment());
                ts->setSig(tsf);
                TWrite::write(ts, xml, ctx);
                delete ts;
                timeSigWritten = true;
            }
            if (needMove) {
                voiceTagWritten |= writeVoiceMove(xml, ctx, segment, startTick, track, &lastTrackWritten);
                // needMove = false; //! NOTE Not necessary, because needMove is currently never read again.
            }
            if (e->isChordRest()) {
                ChordRest* cr = toChordRest(e);
                writeTupletStart(cr, xml, ctx);
            }
            TWrite::writeItem(e, xml, ctx);

            if (e->isChordRest()) {
                ChordRest* cr = toChordRest(e);
                writeTupletEnd(cr, xml, ctx);
            }

            if (!(e->isRest() && toRest(e)->isGap())) {
                TWrite::write(segment, xml, ctx); // write only once
            }
            if (forceTimeSig) {
                if (segment->segmentType() == SegmentType::KeySig) {
                    keySigWritten = true;
                }
                if (segment->segmentType() == SegmentType::TimeSig) {
                    timeSigWritten = true;
                }
                if (segment->segmentType() == SegmentType::ChordRest) {
                    crWritten = true;
                }
            }
        }

        //write spanner ending after the last segment, on the last tick
        if (clip || eseg == 0) {
            for (Spanner* s : spanners) {
                if ((s->tick2() == endTick)
                    && !s->isSlur()
                    && (s->track2() == track || (s->track2() == mu::nidx && s->track() == track))
                    && (!clip || s->tick() >= sseg->tick())
                    ) {
                    writeSpannerEnd(s, xml, ctx, score->lastMeasure(), track, endTick);
                }
            }
        }

        if (voiceTagWritten) {
            xml.endElement();       // </voice>
        }
    }
}
