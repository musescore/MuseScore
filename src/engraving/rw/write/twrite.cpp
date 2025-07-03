/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "../../infrastructure/rtti.h"

#include "dom/score.h"
#include "dom/masterscore.h"
#include "dom/factory.h"
#include "dom/linkedobjects.h"
#include "dom/mscore.h"
#include "dom/staff.h"
#include "dom/part.h"
#include "dom/utils.h"

#include "dom/accidental.h"
#include "dom/actionicon.h"
#include "dom/ambitus.h"
#include "dom/arpeggio.h"
#include "dom/articulation.h"
#include "dom/audio.h"

#include "dom/bagpembell.h"
#include "dom/barline.h"
#include "dom/beam.h"
#include "dom/bend.h"
#include "dom/box.h"
#include "dom/bracket.h"
#include "dom/breath.h"

#include "dom/chord.h"
#include "dom/chordline.h"
#include "dom/chordrest.h"
#include "dom/clef.h"
#include "dom/capo.h"

#include "dom/drumset.h"
#include "dom/dynamic.h"
#include "dom/expression.h"
#include "dom/fermata.h"
#include "dom/figuredbass.h"
#include "dom/fingering.h"
#include "dom/fret.h"

#include "dom/glissando.h"
#include "dom/gradualtempochange.h"
#include "dom/groups.h"
#include "dom/guitarbend.h"

#include "dom/hairpin.h"
#include "dom/hammeronpulloff.h"
#include "dom/harmony.h"
#include "dom/harmonicmark.h"
#include "dom/harppedaldiagram.h"
#include "dom/hook.h"

#include "dom/image.h"
#include "dom/imageStore.h"
#include "dom/instrument.h"
#include "dom/instrchange.h"

#include "dom/jump.h"

#include "dom/keysig.h"

#include "dom/laissezvib.h"
#include "dom/layoutbreak.h"
#include "dom/ledgerline.h"
#include "dom/letring.h"
#include "dom/location.h"
#include "dom/lyrics.h"

#include "dom/marker.h"
#include "dom/measurenumber.h"
#include "dom/measurerepeat.h"
#include "dom/mmrest.h"
#include "dom/mmrestrange.h"

#include "dom/note.h"
#include "dom/notedot.h"
#include "dom/noteline.h"
#include "dom/ornament.h"
#include "dom/ottava.h"

#include "dom/page.h"
#include "dom/palmmute.h"
#include "dom/parenthesis.h"
#include "dom/part.h"
#include "dom/partialtie.h"
#include "dom/pedal.h"
#include "dom/pickscrape.h"
#include "dom/playtechannotation.h"

#include "dom/rasgueado.h"
#include "dom/rehearsalmark.h"
#include "dom/rest.h"

#include "dom/sig.h"
#include "dom/segment.h"
#include "dom/slur.h"
#include "dom/spacer.h"
#include "dom/staffstate.h"
#include "dom/stafftext.h"
#include "dom/stafftype.h"
#include "dom/stafftypechange.h"
#include "dom/stem.h"
#include "dom/stemslash.h"
#include "dom/sticking.h"
#include "dom/stringdata.h"
#include "dom/stringtunings.h"
#include "dom/symbol.h"
#include "dom/bsymbol.h"
#include "dom/system.h"
#include "dom/systemdivider.h"
#include "dom/systemtext.h"
#include "dom/soundflag.h"

#include "dom/tapping.h"
#include "dom/tempotext.h"
#include "dom/text.h"
#include "dom/textbase.h"
#include "dom/textline.h"
#include "dom/textlinebase.h"
#include "dom/tie.h"
#include "dom/timesig.h"
#include "dom/tremolosinglechord.h"
#include "dom/tremolotwochord.h"
#include "dom/tremolobar.h"
#include "dom/trill.h"
#include "dom/tuplet.h"

#include "dom/vibrato.h"
#include "dom/volta.h"

#include "dom/whammybar.h"

#include "../xmlwriter.h"
#include "writecontext.h"
#include "connectorinfowriter.h"

#include "log.h"

using namespace muse;
using namespace mu::engraving;
using namespace mu::engraving::write;

void TWrite::writeItem(const EngravingItem* item, XmlWriter& xml, WriteContext& ctx)
{
    switch (item->type()) {
    case ElementType::ACCIDENTAL:   write(item_cast<const Accidental*>(item), xml, ctx);
        break;
    case ElementType::ACTION_ICON:  write(item_cast<const ActionIcon*>(item), xml, ctx);
        break;
    case ElementType::AMBITUS:      write(item_cast<const Ambitus*>(item), xml, ctx);
        break;
    case ElementType::ARPEGGIO:     write(item_cast<const Arpeggio*>(item), xml, ctx);
        break;
    case ElementType::ARTICULATION: write(item_cast<const Articulation*>(item), xml, ctx);
        break;
    case ElementType::BAGPIPE_EMBELLISHMENT: write(item_cast<const BagpipeEmbellishment*>(item), xml, ctx);
        break;
    case ElementType::BAR_LINE:     write(item_cast<const BarLine*>(item), xml, ctx);
        break;
    case ElementType::BEAM:         write(item_cast<const Beam*>(item), xml, ctx);
        break;
    case ElementType::BEND:         write(item_cast<const Bend*>(item), xml, ctx);
        break;
    case ElementType::HBOX:         write(item_cast<const HBox*>(item), xml, ctx);
        break;
    case ElementType::VBOX:         write(item_cast<const VBox*>(item), xml, ctx);
        break;
    case ElementType::FBOX:         write(item_cast<const FBox*>(item), xml, ctx);
        break;
    case ElementType::TBOX:         write(item_cast<const TBox*>(item), xml, ctx);
        break;
    case ElementType::BRACKET:      write(item_cast<const Bracket*>(item), xml, ctx);
        break;
    case ElementType::BREATH:       write(item_cast<const Breath*>(item), xml, ctx);
        break;
    case ElementType::CHORD:        write(item_cast<const Chord*>(item), xml, ctx);
        break;
    case ElementType::CHORDLINE:    write(item_cast<const ChordLine*>(item), xml, ctx);
        break;
    case ElementType::CLEF:         write(item_cast<const Clef*>(item), xml, ctx);
        break;
    case ElementType::CAPO:         write(item_cast<const Capo*>(item), xml, ctx);
        break;
    case ElementType::DYNAMIC:      write(item_cast<const Dynamic*>(item), xml, ctx);
        break;
    case ElementType::EXPRESSION:   write(item_cast<const Expression*>(item), xml, ctx);
        break;
    case ElementType::FERMATA:      write(item_cast<const Fermata*>(item), xml, ctx);
        break;
    case ElementType::FIGURED_BASS: write(item_cast<const FiguredBass*>(item), xml, ctx);
        break;
    case ElementType::FINGERING:    write(item_cast<const Fingering*>(item), xml, ctx);
        break;
    case ElementType::FRET_DIAGRAM: write(item_cast<const FretDiagram*>(item), xml, ctx);
        break;
    case ElementType::GLISSANDO:    write(item_cast<const Glissando*>(item), xml, ctx);
        break;
    case ElementType::GRADUAL_TEMPO_CHANGE: write(item_cast<const GradualTempoChange*>(item), xml, ctx);
        break;
    case ElementType::GUITAR_BEND:  write(item_cast<const GuitarBend*>(item), xml, ctx);
        break;
    case ElementType::HAIRPIN:      write(item_cast<const Hairpin*>(item), xml, ctx);
        break;
    case ElementType::HAMMER_ON_PULL_OFF: write(item_cast<const HammerOnPullOff*>(item), xml, ctx);
        break;
    case ElementType::HARMONY:      write(item_cast<const Harmony*>(item), xml, ctx);
        break;
    case ElementType::HARMONIC_MARK: write(item_cast<const HarmonicMark*>(item), xml, ctx);
        break;
    case ElementType::HARP_DIAGRAM: write(item_cast<const HarpPedalDiagram*>(item), xml, ctx);
        break;
    case ElementType::HOOK:         write(item_cast<const Hook*>(item), xml, ctx);
        break;
    case ElementType::IMAGE:        write(item_cast<const Image*>(item), xml, ctx);
        break;
    case ElementType::INSTRUMENT_CHANGE: write(item_cast<const InstrumentChange*>(item), xml, ctx);
        break;
    case ElementType::JUMP:         write(item_cast<const Jump*>(item), xml, ctx);
        break;
    case ElementType::KEYSIG:       write(item_cast<const KeySig*>(item), xml, ctx);
        break;
    case ElementType::LAISSEZ_VIB:  write(item_cast<const LaissezVib*>(item), xml, ctx);
        break;
    case ElementType::LAYOUT_BREAK: write(item_cast<const LayoutBreak*>(item), xml, ctx);
        break;
    case ElementType::LEDGER_LINE:  write(item_cast<const LedgerLine*>(item), xml, ctx);
        break;
    case ElementType::LET_RING:     write(item_cast<const LetRing*>(item), xml, ctx);
        break;
    case ElementType::LYRICS:       write(item_cast<const Lyrics*>(item), xml, ctx);
        break;
    case ElementType::MARKER:       write(item_cast<const Marker*>(item), xml, ctx);
        break;
    case ElementType::MEASURE_NUMBER: write(item_cast<const MeasureNumber*>(item), xml, ctx);
        break;
    case ElementType::MEASURE_REPEAT: write(item_cast<const MeasureRepeat*>(item), xml, ctx);
        break;
    case ElementType::MMREST:       write(item_cast<const MMRest*>(item), xml, ctx);
        break;
    case ElementType::MMREST_RANGE: write(item_cast<const MMRestRange*>(item), xml, ctx);
        break;
    case ElementType::NOTE:         write(item_cast<const Note*>(item), xml, ctx);
        break;
    case ElementType::NOTEDOT:      write(item_cast<const NoteDot*>(item), xml, ctx);
        break;
    case ElementType::NOTEHEAD:     write(item_cast<const NoteHead*>(item), xml, ctx);
        break;
    case ElementType::NOTELINE:     write(item_cast<const NoteLine*>(item), xml, ctx);
        break;
    case ElementType::ORNAMENT:     write(item_cast<const Ornament*>(item), xml, ctx);
        break;
    case ElementType::OTTAVA:       write(item_cast<const Ottava*>(item), xml, ctx);
        break;
    case ElementType::PAGE:         write(item_cast<const Page*>(item), xml, ctx);
        break;
    case ElementType::PALM_MUTE:    write(item_cast<const PalmMute*>(item), xml, ctx);
        break;
    case ElementType::PARENTHESIS:    write(item_cast<const Parenthesis*>(item), xml, ctx);
        break;
    case ElementType::PARTIAL_LYRICSLINE:  write(item_cast<const PartialLyricsLine*>(item), xml, ctx);
        break;
    case ElementType::PARTIAL_TIE:  write(item_cast<const PartialTie*>(item), xml, ctx);
        break;
    case ElementType::PEDAL:        write(item_cast<const Pedal*>(item), xml, ctx);
        break;
    case ElementType::PICK_SCRAPE:  write(item_cast<const PickScrape*>(item), xml, ctx);
        break;
    case ElementType::PLAYTECH_ANNOTATION: write(item_cast<const PlayTechAnnotation*>(item), xml, ctx);
        break;
    case ElementType::RASGUEADO:    write(item_cast<const Rasgueado*>(item), xml, ctx);
        break;
    case ElementType::REHEARSAL_MARK: write(item_cast<const RehearsalMark*>(item), xml, ctx);
        break;
    case ElementType::REST:         write(item_cast<const Rest*>(item), xml, ctx);
        break;
    case ElementType::SEGMENT:      write(item_cast<const Segment*>(item), xml, ctx);
        break;
    case ElementType::SLUR:         write(item_cast<const Slur*>(item), xml, ctx);
        break;
    case ElementType::SPACER:       write(item_cast<const Spacer*>(item), xml, ctx);
        break;
    case ElementType::STAFF_STATE:  write(item_cast<const StaffState*>(item), xml, ctx);
        break;
    case ElementType::STAFF_TEXT:   write(item_cast<const StaffText*>(item), xml, ctx);
        break;
    case ElementType::STAFFTYPE_CHANGE: write(item_cast<const StaffTypeChange*>(item), xml, ctx);
        break;
    case ElementType::STEM:         write(item_cast<const Stem*>(item), xml, ctx);
        break;
    case ElementType::STEM_SLASH:   write(item_cast<const StemSlash*>(item), xml, ctx);
        break;
    case ElementType::STICKING:     write(item_cast<const Sticking*>(item), xml, ctx);
        break;
    case ElementType::STRING_TUNINGS: write(item_cast<const StringTunings*>(item), xml, ctx);
        break;
    case ElementType::SYMBOL:       write(item_cast<const Symbol*>(item), xml, ctx);
        break;
    case ElementType::FSYMBOL:      write(item_cast<const FSymbol*>(item), xml, ctx);
        break;
    case ElementType::SYSTEM:       write(item_cast<const System*>(item), xml, ctx);
        break;
    case ElementType::SYSTEM_DIVIDER: write(item_cast<const SystemDivider*>(item), xml, ctx);
        break;
    case ElementType::SYSTEM_TEXT:  write(item_cast<const SystemText*>(item), xml, ctx);
        break;
    case ElementType::SOUND_FLAG:   write(item_cast<const SoundFlag*>(item), xml, ctx);
        break;
    case ElementType::TAPPING:      write(item_cast<const Tapping*>(item), xml, ctx);
        break;
    case ElementType::TEMPO_TEXT:   write(item_cast<const TempoText*>(item), xml, ctx);
        break;
    case ElementType::TEXT:         write(item_cast<const Text*>(item), xml, ctx);
        break;
    case ElementType::TEXTLINE:     write(item_cast<const TextLine*>(item), xml, ctx);
        break;
    case ElementType::TIE:          write(item_cast<const Tie*>(item), xml, ctx);
        break;
    case ElementType::TIMESIG:      write(item_cast<const TimeSig*>(item), xml, ctx);
        break;
    case ElementType::TREMOLO_SINGLECHORD: write(item_cast<const TremoloSingleChord*>(item), xml, ctx);
        break;
    case ElementType::TREMOLO_TWOCHORD:    write(item_cast<const TremoloTwoChord*>(item), xml, ctx);
        break;
    case ElementType::TREMOLOBAR:   write(item_cast<const TremoloBar*>(item), xml, ctx);
        break;
    case ElementType::TRILL:        write(item_cast<const Trill*>(item), xml, ctx);
        break;
    case ElementType::TUPLET:       write(item_cast<const Tuplet*>(item), xml, ctx);
        break;
    case ElementType::VIBRATO:      write(item_cast<const Vibrato*>(item), xml, ctx);
        break;
    case ElementType::VOLTA:        write(item_cast<const Volta*>(item), xml, ctx);
        break;
    case ElementType::WHAMMY_BAR:   write(item_cast<const WhammyBar*>(item), xml, ctx);
        break;
    default: {
        UNREACHABLE;
        LOGE() << "not implemented write for type: " << item->typeName();
    }
    }
}

void TWrite::writeItems(const ElementList& items, XmlWriter& xml, WriteContext& ctx)
{
    for (const EngravingItem* e : items) {
        writeItem(e, xml, ctx);
    }
}

void TWrite::writeProperty(const EngravingItem* item, XmlWriter& xml, Pid pid, bool force)
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
    PropertyValue d = !force && (f != PropertyFlags::STYLED) ? item->propertyDefault(pid) : PropertyValue();

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
        p = PropertyValue(Spatium::fromMM(f1, item->spatium()));
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

void TWrite::writeSystemLocks(const Score* score, XmlWriter& xml)
{
    std::vector<const SystemLock*> locks = score->systemLocks()->allLocks();
    if (locks.empty()) {
        return;
    }

    xml.startElement("SystemLocks");
    for (const SystemLock* sl : locks) {
        writeSystemLock(sl, xml);
    }
    xml.endElement();
}

void TWrite::writeItemEid(const EngravingObject* item, XmlWriter& xml, WriteContext& ctx)
{
    if (ctx.configuration()->doNotSaveEIDsForBackCompat() || item->score()->isPaletteScore() || ctx.clipboardmode()) {
        return;
    }

    EID eid = item->eid();
    if (!eid.isValid()) {
        eid = item->assignNewEID();
    }
    xml.tag("eid", eid.toStdString());
}

void TWrite::writeItemLink(const EngravingObject* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!item->links() || item->links()->size() <= 1 || ctx.clipboardmode()) {
        return;
    }

    EngravingItem* mainElement = static_cast<EngravingItem*>(item->links()->mainElement());
    if (mainElement != item) {
        EID eidOfMainElement = mainElement->eid();
        DO_ASSERT(eidOfMainElement.isValid());
        xml.tag("linkedTo", mainElement->eid().toStdString());
    }
}

void TWrite::writeSystemLock(const SystemLock* systemLock, XmlWriter& xml)
{
    xml.startElement("systemLock");

    xml.tag("startMeasure", systemLock->startMB()->eid().toStdString());
    xml.tag("endMeasure", systemLock->endMB()->eid().toStdString());

    xml.endElement();
}

void TWrite::writeStyledProperties(const EngravingItem* item, XmlWriter& xml)
{
    for (const StyledProperty& spp : *item->styledProperties()) {
        writeProperty(item, xml, spp.pid);
    }
}

void TWrite::writeItemProperties(const EngravingItem* item, XmlWriter& xml, WriteContext& ctx)
{
    writeItemEid(item, xml, ctx);

    bool autoplaceEnabled = item->score()->style().styleB(Sid::autoplaceEnabled);
    if (!autoplaceEnabled) {
        item->score()->style().set(Sid::autoplaceEnabled, true);
        writeProperty(item, xml, Pid::AUTOPLACE);
        item->score()->style().set(Sid::autoplaceEnabled, autoplaceEnabled);
    } else {
        writeProperty(item, xml, Pid::AUTOPLACE);
    }

    writeItemLink(item, xml, ctx);

    if ((ctx.writeTrack() || item->track() != ctx.curTrack())
        && (item->track() != muse::nidx) && !item->isBeam() && !item->isTuplet()) {
        // Writing track number for beams and tuplets is redundant as it is calculated
        // during layout.
        int t = static_cast<int>(item->track()) + ctx.trackDiff();
        xml.tag("track", t);
    }

    for (Pid pid : { Pid::OFFSET, Pid::COLOR, Pid::VISIBLE, Pid::Z }) {
        if (item->propertyFlags(pid) == PropertyFlags::NOSTYLE) {
            writeProperty(item, xml, pid);
        }
    }

    if (!item->hasVoiceAssignmentProperties() && item->propertyFlags(Pid::PLACEMENT) == PropertyFlags::NOSTYLE) {
        writeProperty(item, xml, Pid::PLACEMENT);
    }

    writeProperty(item, xml, Pid::POSITION_LINKED_TO_MASTER);
    writeProperty(item, xml, Pid::APPEARANCE_LINKED_TO_MASTER);
    writeProperty(item, xml, Pid::EXCLUDE_FROM_OTHER_PARTS);
}

void TWrite::write(const Accidental* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperty(item, xml, Pid::ACCIDENTAL_BRACKET);
    writeProperty(item, xml, Pid::ACCIDENTAL_ROLE);
    writeProperty(item, xml, Pid::SMALL);
    writeProperty(item, xml, Pid::ACCIDENTAL_TYPE);
    writeProperty(item, xml, Pid::ACCIDENTAL_STACKING_ORDER_OFFSET);
    writeItemProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::write(const ActionIcon* item, XmlWriter& xml, WriteContext&)
{
    xml.startElement(item);
    xml.tag("subtype", int(item->actionType()));
    xml.tag("action", String::fromStdString(item->actionCode()));
    xml.endElement();
}

void TWrite::write(const Ambitus* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    xml.tagProperty(Pid::HEAD_GROUP, int(item->noteHeadGroup()), int(Ambitus::NOTEHEADGROUP_DEFAULT));
    xml.tagProperty(Pid::HEAD_TYPE,  int(item->noteHeadType()),  int(Ambitus::NOTEHEADTYPE_DEFAULT));
    xml.tagProperty(Pid::MIRROR_HEAD, int(item->direction()),    int(Ambitus::DIRECTION_DEFAULT));
    xml.tag("hasLine",    item->hasLine(), true);
    xml.tagProperty(Pid::LINE_WIDTH, item->lineWidth(), Ambitus::LINEWIDTH_DEFAULT);
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
    if (!RealIsNull(item->userLen1())) {
        xml.tag("userLen1", item->userLen1() / item->spatium());
    }
    if (!RealIsNull(item->userLen2())) {
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
    } else if (!item->isTapping()) {
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
    writeProperty(item, xml, Pid::ORNAMENT_SHOW_CUE_NOTE);
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
    writeProperty(item, xml, Pid::BEAM_CROSS_STAFF_MOVE);

    int idx = item->directionIdx();
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
        Pid::LEFT_MARGIN, Pid::RIGHT_MARGIN, Pid::TOP_MARGIN, Pid::BOTTOM_MARGIN, Pid::BOX_AUTOSIZE, Pid::SIZE_SPATIUM_DEPENDENT
    }) {
        bool force = ((item->isVBox() || item->isFBox()) && id == Pid::BOX_HEIGHT) || (item->isHBox() && id == Pid::BOX_WIDTH);
        writeProperty(item, xml, id, force);
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
    xml.startElement(item);

    writeProperty(item, xml, Pid::FRET_FRAME_TEXT_SCALE);
    writeProperty(item, xml, Pid::FRET_FRAME_DIAGRAM_SCALE);
    writeProperty(item, xml, Pid::FRET_FRAME_COLUMN_GAP);
    writeProperty(item, xml, Pid::FRET_FRAME_ROW_GAP);
    writeProperty(item, xml, Pid::FRET_FRAME_CHORDS_PER_ROW);
    writeProperty(item, xml, Pid::FRET_FRAME_H_ALIGN);

    writeProperties(static_cast<const Box*>(item), xml, ctx);

    xml.endElement();
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
    // HACK: foundNotes is a workaround introduced with the "notes in chords" selection filter. A substantial overhaul of our
    // copy/paste logic would be required to make this fully compatible with de-selected chords - for now we'll simply replace
    // these chords with a rest of the same duration...
    bool foundNotes = false;
    const size_t noteCount = item->notes().size();
    for (size_t noteIdx = 0; noteIdx < noteCount; ++noteIdx) {
        if (ctx.canWriteNoteIdx(noteIdx, noteCount)) {
            foundNotes = true;
            break;
        }
    }
    if (!foundNotes) {
        Rest* dummyRest = Factory::createRest(item->segment());
        dummyRest->setDurationType(item->durationType());
        dummyRest->setTuplet(item->tuplet());
        dummyRest->setTicks(item->ticks());
        dummyRest->setTrack(item->track());
        write(dummyRest, xml, ctx);
        dummyRest->deleteLater();
        return;
    }

    for (Chord* ch : item->graceNotes()) {
        write(ch, xml, ctx);
    }
    writeChordRestBeam(item, xml, ctx);
    xml.startElement(item);
    writeProperties(static_cast<const ChordRest*>(item), xml, ctx);
    for (const Articulation* a : item->articulations()) {
        writeItem(a, xml, ctx);
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
    } else if (item->stem() && (item->stem()->isUserModified() || !item->stem()->userLength().isZero())) {
        write(item->stem(), xml, ctx);
    }
    if (item->hook() && item->hook()->isUserModified()) {
        write(item->hook(), xml, ctx);
    }
    if (item->showStemSlash() && item->isUserModified()) {
        xml.tag("showStemSlash", item->showStemSlash());
    }
    if (item->stemSlash() && item->stemSlash()->isUserModified()) {
        write(item->stemSlash(), xml, ctx);
    }
    writeProperty(item, xml, Pid::STEM_DIRECTION);

    for (size_t noteIdx = 0; noteIdx < noteCount; ++noteIdx) {
        if (!ctx.canWriteNoteIdx(noteIdx, noteCount)) {
            continue;
        }
        const Note* note = item->notes().at(noteIdx);
        write(note, xml, ctx);
    }

    if (item->arpeggio()) {
        write(item->arpeggio(), xml, ctx);
    }

    if (item->tremoloSingleChord()) {
        write(item->tremoloSingleChord(), xml, ctx);
    } else if (item->tremoloTwoChord() && item->tremoloChordType() != TremoloChordType::TremoloSecondChord) {
        write(item->tremoloTwoChord(), xml, ctx);
    }

    writeProperty(item, xml, Pid::COMBINE_VOICE);

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

        const bool isPartialSlur = toSlur(s)->partialSpannerDirection() != PartialSpannerDirection::NONE;
        const bool writeStart = s->startElement() == item && (s->endElement() != item || isPartialSlur);
        const bool writeEnd = s->endElement() == item && (s->startElement() != item || isPartialSlur);

        if (writeStart) {
            writeSpannerStart(s, xml, ctx, item, item->track());
        }

        if (writeEnd) {
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
    if (frac == s->score()->endTick()) {
        // Write a location tag if the spanner ends on the last tick of the score
        Location spannerEndLoc = Location::absolute();
        spannerEndLoc.setFrac(frac);
        spannerEndLoc.setMeasure(0);
        spannerEndLoc.setTrack(static_cast<int>(track));
        spannerEndLoc.setVoice(static_cast<int>(track2voice(track)));
        spannerEndLoc.setStaff(static_cast<int>(s->staffIdx()));

        Location prevLoc = Location::absolute();
        prevLoc.setFrac(ctx.curTick());
        prevLoc.setMeasure(0);
        prevLoc.setTrack(static_cast<int>(track));
        prevLoc.setVoice(static_cast<int>(track2voice(track)));
        prevLoc.setStaff(static_cast<int>(s->staffIdx()));

        spannerEndLoc.toRelative(prevLoc);
        if (spannerEndLoc.frac() != Fraction(0, 1)) {
            write(&spannerEndLoc, xml, ctx);
        }
    }
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
    writeProperty(item, xml, Pid::PLAY);
    xml.tag("lengthX", item->lengthX(), 0.0);
    xml.tag("lengthY", item->lengthY(), 0.0);
    writeItemProperties(item, xml, ctx);
    if (item->modified()) {
        //! NOTE Need separated "given" data and layout data
        const ChordLine::LayoutData* ldata = item->ldata();
        const PainterPath& path = ldata->path;
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
    writeProperty(item, xml, Pid::IS_COURTESY);
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
    writeProperty(item, xml, Pid::AVOID_BARLINES);
    writeProperty(item, xml, Pid::DYNAMICS_SIZE);
    writeProperty(item, xml, Pid::CENTER_ON_NOTEHEAD);
    writeProperty(item, xml, Pid::PLAY);
    writeProperty(item, xml, Pid::ANCHOR_TO_END_OF_PREVIOUS);

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
    if (item->hasVoiceAssignmentProperties()) {
        writeProperty(item, xml, Pid::VOICE_ASSIGNMENT);
        writeProperty(item, xml, Pid::DIRECTION);
        writeProperty(item, xml, Pid::CENTER_BETWEEN_STAVES);
    }

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

    writeProperty(item, xml, Pid::TEXT_LINKED_TO_MASTER);
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

    static const std::array<Pid, 10> pids { {
        Pid::MIN_DISTANCE,
        Pid::FRET_OFFSET,
        Pid::FRET_FRETS,
        Pid::FRET_STRINGS,
        Pid::FRET_NUT,
        Pid::MAG,
        Pid::ORIENTATION,
        Pid::FRET_SHOW_FINGERINGS,
        Pid::FRET_FINGERING,
        Pid::EXCLUDE_VERTICAL_ALIGN
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

    xml.tagProperty("text", item->text(), item->propertyDefault(Pid::GLISS_TEXT));

    if (ctx.clipboardmode() && item->isHarpGliss().has_value()) {
        xml.tagProperty("isHarpGliss", PropertyValue(item->isHarpGliss().value()));
    }

    for (auto id : { Pid::GLISS_SHIFT, Pid::GLISS_EASEIN, Pid::GLISS_EASEOUT }) {
        writeProperty(item, xml, id);
    }
    for (const StyledProperty& spp : *item->styledProperties()) {
        writeProperty(item, xml, spp.pid);
    }

    writeProperties(static_cast<const SLine*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const GuitarBend* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    xml.tag("guitarBendType", static_cast<int>(item->type()));
    xml.tag("bendStartTimeFactor", item->startTimeFactor());
    xml.tag("bendEndTimeFactor", item->endTimeFactor());
    writeProperty(item, xml, Pid::DIRECTION);
    writeProperty(item, xml, Pid::BEND_SHOW_HOLD_LINE);
    writeProperties(static_cast<const SLine*>(item), xml, ctx);

    GuitarBendHold* hold = item->holdLine();
    if (hold) {
        xml.startElement(hold);
        writeProperties(static_cast<const SLine*>(hold), xml, ctx);
        xml.endElement();
    }

    xml.endElement();
}

void TWrite::writeProperties(const GuitarBendSegment* item, XmlWriter& xml, WriteContext& ctx)
{
    writeProperty(item, xml, Pid::BEND_VERTEX_OFF);

    GuitarBendText* text = item->bendText();
    if (text && text->isUserModified()) {
        xml.startElement(text);
        writeProperties(toTextBase(text), xml, ctx, false);
        xml.endElement();
    }
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

    if (!item->isUserModified()) {
        return;
    }

    double _spatium = item->style().spatium();
    for (const SpannerSegment* seg : item->spannerSegments()) {
        xml.startElement("Segment", seg);
        xml.tag("subtype", int(seg->spannerSegmentType()));
        xml.tagPoint("offset", seg->offset() / _spatium);
        xml.tagPoint("off2", seg->userOff2() / _spatium);
        writeProperty(seg, xml, Pid::MIN_DISTANCE);
        if (seg->isGuitarBendSegment()) {
            writeProperties(static_cast<const GuitarBendSegment*>(seg), xml, ctx);
        }
        writeItemProperties(seg, xml, ctx);
        xml.endElement();
    }
}

void TWrite::writeProperties(const Spanner* item, XmlWriter& xml, WriteContext& ctx)
{
    if (ctx.clipboardmode()) {
        xml.tagFraction("ticks_f", item->ticks());
    }
    writeProperty(item, xml, Pid::PLAY);
    writeItemProperties(item, xml, ctx);
}

void TWrite::write(const GradualTempoChange* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperty(item, xml, Pid::TEMPO_CHANGE_TYPE);
    writeProperty(item, xml, Pid::TEMPO_EASING_METHOD);
    writeProperty(item, xml, Pid::TEMPO_CHANGE_FACTOR);
    writeProperty(item, xml, Pid::PLACEMENT);
    writeProperty(item, xml, Pid::SNAP_AFTER);
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
    writeProperty(item, xml, Pid::SINGLE_NOTE_DYNAMICS);
    writeProperty(item, xml, Pid::VELO_CHANGE_METHOD);

    writeProperty(item, xml, Pid::VOICE_ASSIGNMENT);
    writeProperty(item, xml, Pid::DIRECTION);
    writeProperty(item, xml, Pid::CENTER_BETWEEN_STAVES);

    writeProperty(item, xml, Pid::SNAP_BEFORE);
    writeProperty(item, xml, Pid::SNAP_AFTER);

    writeProperty(item, xml, Pid::HAIRPIN_HEIGHT);
    writeProperty(item, xml, Pid::HAIRPIN_CONT_HEIGHT);

    writeProperties(static_cast<const TextLineBase*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const HammerOnPullOff* item, XmlWriter& xml, WriteContext& ctx)
{
    if (item->broken()) {
        return;
    }
    if (!ctx.canWrite(item)) {
        return;
    }

    xml.startElement(item);

    writeProperty(item, xml, Pid::PARTIAL_SPANNER_DIRECTION);

    writeProperties(static_cast<const Slur*>(item), xml, ctx);

    xml.endElement();
}

void TWrite::writeProperties(const HammerOnPullOffSegment* seg, XmlWriter& xml, WriteContext& ctx)
{
    for (size_t i = 0; i < seg->hopoText().size(); ++i) {
        HammerOnPullOffText* hopoText = seg->hopoText()[i];
        if (!hopoText->isUserModified()) {
            continue;
        }
        write(hopoText, xml, ctx, i);
    }
}

void TWrite::write(const HammerOnPullOffText* item, XmlWriter& xml, WriteContext& ctx, size_t idx)
{
    xml.startElement(item, { { "idx", idx } });

    writeProperties(toTextBase(item), xml, ctx, /*writeText*/ false);
    xml.endElement();
}

static void writeHarmonyInfo(const HarmonyInfo* item, const Harmony* h, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement("harmonyInfo");
    if (item->rootTpc() != Tpc::TPC_INVALID || item->bassTpc() != Tpc::TPC_INVALID) {
        int rRootTpc = item->rootTpc();
        int rBassTpc = item->bassTpc();
        if (h->staff()) {
            // parent can be a fret diagram
            const Segment* segment = h->getParentSeg();
            Fraction tick = segment ? segment->tick() : Fraction(-1, 1);
            const Interval& interval = h->staff()->transpose(tick);
            if (ctx.clipboardmode() && !h->score()->style().styleB(Sid::concertPitch) && interval.chromatic) {
                rRootTpc = transposeTpc(item->rootTpc(), interval, true);
                rBassTpc = transposeTpc(item->bassTpc(), interval, true);
            }
        }

        if (item->id() > 0) {
            xml.tag("extension", item->id());
        }
        // parser uses leading "=" as a hidden specifier for minor
        // this may or may not currently be incorporated into _textName
        String writeName = item->textName();
        if (item->parsedChord() && item->parsedChord()->name().startsWith(u'=') && !writeName.startsWith(u'=')) {
            writeName = u"=" + writeName;
        }
        if (!writeName.isEmpty()) {
            xml.tag("name", writeName);
        }
        if (rRootTpc != Tpc::TPC_INVALID) {
            xml.tag("root", rRootTpc);
        }
        if (rBassTpc != Tpc::TPC_INVALID) {
            xml.tag("bass", rBassTpc);
        }
    } else {
        xml.tag("name", item->textName());
    }

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

    // check tpcs valid?
    if (item->rootCase() != NoteCaseType::CAPITAL) {
        xml.tag("rootCase", static_cast<int>(item->rootCase()));
    }

    if (item->bassCase() != NoteCaseType::CAPITAL) {
        xml.tag("bassCase", static_cast<int>(item->bassCase()));
    }

    for (const HarmonyInfo* info : item->chords()) {
        writeHarmonyInfo(info, item, xml, ctx);
    }

    for (const HDegree& hd : item->degreeList()) { // Do we really still need this?
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

    writeProperty(item, xml, Pid::HARMONY_DO_NOT_STACK_MODIFIERS);
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
        muse::io::FileInfo fi(item->linkPath());
        // score()->fileInfo()->canonicalPath() would be better
        // but we are saving under a temp file name and the 'final' file
        // might not exist yet, so canonicalFilePath() may return only "/"
        // OTOH, the score 'final' file name is practically always canonical, at this point
        String scorePath = item->score()->masterScore()->fileInfo()->absoluteDirPath().toString();
        String imgFPath  = fi.canonicalFilePath();
        // if imgFPath is in (or below) the directory of scorePath
        if (imgFPath.startsWith(scorePath, muse::CaseSensitive)) {
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
            fi = muse::io::FileInfo(scorePath);
            scorePath = fi.path();
            // if imgFPath is in (or below) the directory up the score directory
            if (imgFPath.startsWith(scorePath, muse::CaseSensitive)) {
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

    if (!item->soundId().empty()) {
        xml.tag("soundId", item->soundId());
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
        if (ct.concertClef == ct.transposingClef) {
            if (ct.concertClef != ClefType::G) {
                if (i) {
                    xml.tag("clef", { { "staff", i + 1 } }, TConv::toXml(ct.concertClef));
                } else {
                    xml.tag("clef", TConv::toXml(ct.concertClef));
                }
            }
        } else {
            if (i) {
                xml.tag("concertClef", { { "staff", i + 1 } }, TConv::toXml(ct.concertClef));
                xml.tag("transposingClef", { { "staff", i + 1 } }, TConv::toXml(ct.transposingClef));
            } else {
                xml.tag("concertClef", TConv::toXml(ct.concertClef));
                xml.tag("transposingClef", TConv::toXml(ct.transposingClef));
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

    if (item->isCourtesy()) {
        xml.tag("isCourtesy", item->isCourtesy());
    }

    if (!item->showCourtesy()) {
        xml.tag("showCourtesySig", item->showCourtesy());
    }
    writeProperty(item, xml, Pid::IS_COURTESY);
    if (item->forInstrumentChange()) {
        xml.tag("forInstrumentChange", true);
    }
    xml.endElement();
}

void TWrite::write(const LaissezVib* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperty(item, xml, Pid::MIN_LENGTH);
    writeProperty(item, xml, Pid::TIE_PLACEMENT);
    writeProperties(static_cast<const SlurTie*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const LayoutBreak* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeItemProperties(item, xml, ctx);

    for (auto id :
         { Pid::LAYOUT_BREAK, Pid::PAUSE, Pid::START_WITH_LONG_NAMES, Pid::START_WITH_MEASURE_ONE, Pid::FIRST_SYSTEM_INDENTATION,
           Pid::SHOW_COURTESY }) {
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
    xml.tag("timeTick", item->isTimeTick(), false);
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
    writeProperty(item, xml, Pid::LYRIC_TICKS);

    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const Marker* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperties(static_cast<const TextBase*>(item), xml, ctx, true);
    xml.tag("label", item->label());
    writeProperty(item, xml, Pid::MARKER_CENTER_ON_SYMBOL);
    writeProperty(item, xml, Pid::MARKER_SYMBOL_SIZE);
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
    writeProperty(item, xml, Pid::MMREST_NUMBER_OFFSET);
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
        if (!dot->offset().isNull() || !dot->visible() || dot->color() != ctx.configuration()->defaultColor()
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

    if (item->laissezVib()) {
        write(item->laissezVib(), xml, ctx);
    }

    if (item->incomingPartialTie()) {
        write(item->incomingPartialTie(), xml, ctx);
    }

    if (item->outgoingPartialTie()) {
        write(item->outgoingPartialTie(), xml, ctx);
    }

    if (item->tieForNonPartial()) {
        writeSpannerStart(item->tieFor(), xml, ctx, item, item->track());
    }

    if (item->tieBackNonPartial()) {
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

void TWrite::write(const Parenthesis* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }

    xml.startElement(item);
    writeProperty(item, xml, Pid::HORIZONTAL_DIRECTION);
    writeItemProperties(item, xml, ctx);
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

void TWrite::write(const PartialTie* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperty(item, xml, Pid::TIE_PLACEMENT);
    writeProperty(item, xml, Pid::PARTIAL_SPANNER_DIRECTION);
    writeProperties(static_cast<const SlurTie*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const PartialLyricsLine* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperty(item, xml, Pid::VERSE);
    xml.tag("isEndMelisma", item->isEndMelisma());
    writeItemProperties(item, xml, ctx);
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
        Pid::BEGIN_HOOK_TYPE,
        Pid::BEGIN_TEXT_OFFSET,
        Pid::CONTINUE_TEXT_OFFSET,
        Pid::END_TEXT_OFFSET
    }) {
        writeProperty(item, xml, i);
    }
    for (const StyledProperty& spp : *item->styledProperties()) {
        writeProperty(item, xml, spp.pid);
    }

    writeProperties(static_cast<const SLine*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const PickScrape* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    writeProperties(static_cast<const TextLineBase*>(item), xml, ctx);
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
        if (!dot->offset().isNull() || !dot->visible() || dot->color() != ctx.configuration()->defaultColor()
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

    writeProperty(item, xml, Pid::PARTIAL_SPANNER_DIRECTION);

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
    if (!seg->isUserModified()) {
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

    if (seg->isHammerOnPullOffSegment()) {
        writeProperties(toHammerOnPullOffSegment(seg), xml, ctx);
    }

    writeItemProperties(seg, xml, ctx);
    xml.endElement();
}

void TWrite::write(const Spacer* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    xml.tag("subtype", int(item->spacerType()));
    writeItemProperties(item, xml, ctx);
    xml.tag("space", item->gap().val());
    xml.endElement();
}

void TWrite::write(const Staff* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);

    writeItemEid(item, xml, ctx);
    writeItemLink(item, xml, ctx);

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
    if (ct.concertClef == ct.transposingClef) {
        if (ct.concertClef != ClefType::G) {
            xml.tag("defaultClef", TConv::toXml(ct.concertClef));
        }
    } else {
        xml.tag("defaultConcertClef", TConv::toXml(ct.concertClef));
        xml.tag("defaultTransposingClef", TConv::toXml(ct.transposingClef));
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
    if (item->mergeMatchingRests() != AutoOnOff::AUTO) {
        xml.tag("mergeMatchingRests", TConv::toXml(item->mergeMatchingRests()));
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
    if (!ctx.canWrite(item)) {
        return;
    }

    xml.startElement(item);

    writeProperties(static_cast<const StaffTextBase*>(item), xml, ctx);

    if (const SoundFlag* flag = item->soundFlag()) {
        writeItem(flag, xml, ctx);
    }

    xml.endElement();
}

void TWrite::write(const StaffTextBase* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }

    xml.startElement(item);
    writeProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::writeProperties(const StaffTextBase* item, XmlWriter& xml, WriteContext& ctx)
{
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
}

void TWrite::write(const StaffType* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement("StaffType", { { "group", TConv::toXml(item->group()) } });
    if (!item->xmlName().isEmpty()) {
        xml.tag("name", item->xmlName());
    }
    if (item->lines() != 5) {
        xml.tag("lines", item->lines());
    }
    if (!RealIsEqual(item->lineDistance().val(), 1.0)) {
        xml.tag("lineDistance", item->lineDistance().val());
    }
    if (!RealIsNull(item->yoffset().val())) {
        xml.tag("yoffset", item->yoffset().val());
    }
    if (!RealIsEqual(item->userMag(), 1.0)) {
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
    if (item->color() != ctx.configuration()->defaultColor()) {
        xml.tagProperty(Pid::COLOR, item->color());
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
        if (item->symRepeat() != TablatureSymbolRepeat::NEVER) {
            xml.tag("symbolRepeat", int(item->symRepeat()));
        }
        xml.tag("fretUseTextStyle", item->fretUseTextStyle());
        if (item->fretUseTextStyle()) {
            xml.tag("fretTextStyle", int(item->fretTextStyle()));
        } else {
            xml.tag("fretPresetIdx", item->fretPresetIdx());
            xml.tag("fretFontSize",  item->fretFontSize());
            xml.tag("fretFontY",     item->fretFontUserY());
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
        XmlWriter::Attributes attrs;

        if (strg.open) {
            attrs.push_back({ "open", "1" });
        }

        if (strg.useFlat) {
            attrs.push_back({ "useFlat", "1" });
        }

        xml.tag("string", attrs, strg.pitch);
    }
    xml.endElement();
}

void TWrite::write(const StringTunings* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);

    writeProperty(item, xml, Pid::STRINGTUNINGS_PRESET);

    xml.tag("visibleStrings", TConv::toXml(item->visibleStrings()));

    if (!item->stringData()->isNull()) {
        write(item->stringData(), xml);
    }

    writeProperties(static_cast<const StaffTextBase*>(item), xml, ctx, true);
    xml.endElement();
}

void TWrite::write(const Symbol* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    xml.tag("name", SymNames::nameForSymId(item->sym()));
    if (item->scoreFont()) {
        xml.tag("font", item->scoreFont()->name());
        writeProperty(item, xml, Pid::SYMBOLS_SIZE);
        writeProperty(item, xml, Pid::SYMBOL_ANGLE);
    }
    writeProperties(static_cast<const BSymbol*>(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const FSymbol* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    xml.tag("font",     item->font().family().id());
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

void TWrite::write(const SoundFlag* item, XmlWriter& xml, WriteContext&)
{
    if (item->soundPresets().empty() && item->playingTechnique().empty()) {
        return;
    }

    xml.startElement(item);

    writeProperty(item, xml, Pid::PLAY);

    if (!item->soundPresets().empty()) {
        xml.tag("presets", item->soundPresets().join(u","));
    }

    if (!item->playingTechnique().empty()) {
        xml.tag("playingTechnique", item->playingTechnique());
    }

    writeProperty(item, xml, Pid::APPLY_TO_ALL_STAVES);

    xml.endElement();
}

void TWrite::write(const Tapping* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);

    xml.tag("hand", TConv::toXml(item->hand()));

    if (item->halfSlurAbove() && item->halfSlurAbove()->isUserModified()) {
        write(item->halfSlurAbove(), xml, ctx);
    }
    if (item->halfSlurBelow() && item->halfSlurBelow()->isUserModified()) {
        write(item->halfSlurBelow(), xml, ctx);
    }

    writeProperties(toArticulation(item), xml, ctx);
    xml.endElement();
}

void TWrite::write(const TappingHalfSlur* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);

    xml.tag("isHalfSlurAbove", item->isHalfSlurAbove());
    writeProperties(static_cast<const SlurTie*>(item), xml, ctx);

    xml.endElement();
}

void TWrite::write(const TempoText* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperty(item, xml, Pid::PLAY);
    xml.tag("tempo", TConv::toXml(item->tempo()));
    if (item->followText()) {
        xml.tag("followText", item->followText());
    }
    switch (item->tempoTextType()) {
    case TempoTextType::NORMAL:
        break;
    case TempoTextType::A_TEMPO:
        xml.tag("type", "aTempo");
        break;
    case TempoTextType::TEMPO_PRIMO:
        xml.tag("type", "tempoPrimo");
        break;
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
    writeProperty(item, xml, Pid::TIE_PLACEMENT);
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
    writeProperty(item, xml, Pid::IS_COURTESY);
    writeProperty(item, xml, Pid::SCALE);

    xml.endElement();
}

void TWrite::write(const TremoloSingleChord* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }

    xml.startElement(item);

    writeProperty(item, xml, Pid::TREMOLO_TYPE);
    writeProperty(item, xml, Pid::PLAY);
    writeItemProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::write(const TremoloTwoChord* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }

    xml.startElement(item);

    writeProperty(item, xml, Pid::TREMOLO_TYPE);
    writeProperty(item, xml, Pid::TREMOLO_STYLE);
    writeProperty(item, xml, Pid::PLAY);
    writeItemProperties(item, xml, ctx);

    // write manual adjustments to file
    int idx = item->directionIdx();
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

void TWrite::write(const TremoloBar* item, XmlWriter& xml, WriteContext& ctx)
{
    xml.startElement(item);
    writeProperty(item, xml, Pid::MAG);
    writeProperty(item, xml, Pid::LINE_WIDTH);
    writeProperty(item, xml, Pid::PLAY);
    for (const PitchValue& v : item->points()) {
        xml.tag("point", { { "time", v.time }, { "pitch", v.pitch }, { "vibrato", v.vibrato } });
    }
    writeItemProperties(item, xml, ctx);
    xml.endElement();
}

void TWrite::write(const Trill* item, XmlWriter& xml, WriteContext& ctx)
{
    if (!ctx.canWrite(item)) {
        return;
    }
    xml.startElement(item);
    xml.tag("subtype", TConv::toXml(item->trillType()));
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
        dest.setIsTimeTick(seg->isTimeTickType());
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
                        || (et == ElementType::STRING_TUNINGS)
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

            if ((!(m && m->isMMRest()))) {
                for (Spanner* s : spanners) {
                    if (!segment->canWriteSpannerStartEnd(track, s)) {
                        continue;
                    }
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
                Fraction tsf = score->sigmap()->timesig(segment->tick()).nominal();
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

        // write spanners whose end tick lies outside the clip region
        if (clip) {
            for (Spanner* s : spanners) {
                Fraction spannerEndTick = s->tick2();
                bool spannerEndingAtEdgeOfClipZone = spannerEndTick == endTick && !s->isSlur() && s->effectiveTrack2() == track
                                                     && s->tick() >= sseg->tick();
                if (!spannerEndingAtEdgeOfClipZone) {
                    continue;
                }
                bool needMove = spannerEndTick != ctx.curTick();
                if (needMove) {
                    // If spanner started on a timeTick and there was no other segment in between there and here,
                    // ctx.curTick hasn't been moved forward, so we must move it forward here.
                    Location curr = Location::absolute();
                    Location dest = Location::absolute();
                    curr.setFrac(ctx.curTick());
                    dest.setFrac(spannerEndTick);
                    dest.toRelative(curr);
                    TWrite::write(&dest, xml, ctx);
                    ctx.setCurTick(spannerEndTick);
                }
                writeSpannerEnd(s, xml, ctx, score->lastMeasure(), track, endTick);
            }
        }

        if (voiceTagWritten) {
            xml.endElement();       // </voice>
        }
    }
}
