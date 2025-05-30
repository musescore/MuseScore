/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "singlelayout.h"

#include "draw/fontmetrics.h"

#include "types/typesconv.h"
#include "types/symnames.h"

#include "compat/dummyelement.h"

#include "dom/engravingitem.h"
#include "dom/score.h"

#include "dom/accidental.h"
#include "dom/actionicon.h"
#include "dom/ambitus.h"
#include "dom/articulation.h"
#include "dom/bagpembell.h"
#include "dom/barline.h"
#include "dom/bend.h"
#include "dom/bracket.h"
#include "dom/breath.h"
#include "dom/capo.h"
#include "dom/chordline.h"
#include "dom/clef.h"
#include "dom/dynamic.h"
#include "dom/expression.h"
#include "dom/fermata.h"
#include "dom/fingering.h"
#include "dom/fret.h"
#include "dom/glissando.h"
#include "dom/gradualtempochange.h"
#include "dom/guitarbend.h"
#include "dom/hairpin.h"
#include "dom/hammeronpulloff.h"
#include "dom/harppedaldiagram.h"
#include "dom/instrchange.h"
#include "dom/jump.h"
#include "dom/keysig.h"
#include "dom/letring.h"
#include "dom/line.h"
#include "dom/lyrics.h"
#include "dom/marker.h"
#include "dom/measurenumber.h"
#include "dom/measurerepeat.h"
#include "dom/note.h"
#include "dom/noteline.h"
#include "dom/ornament.h"
#include "dom/ottava.h"
#include "dom/palmmute.h"
#include "dom/pedal.h"
#include "dom/playtechannotation.h"
#include "dom/rehearsalmark.h"
#include "dom/slur.h"
#include "dom/soundflag.h"
#include "dom/stafftext.h"
#include "dom/stafftypechange.h"
#include "dom/sticking.h"
#include "dom/stringtunings.h"
#include "dom/symbol.h"
#include "dom/systemtext.h"
#include "dom/tempotext.h"
#include "dom/text.h"
#include "dom/textline.h"
#include "dom/textlinebase.h"
#include "dom/timesig.h"
#include "dom/tremolobar.h"
#include "dom/tremolosinglechord.h"
#include "dom/tremolotwochord.h"
#include "dom/trill.h"
#include "dom/vibrato.h"
#include "dom/volta.h"

#include "dom/utils.h"

#include "rendering/score/tlayout.h"
#include "rendering/score/tremololayout.h"
#include "rendering/score/chordlayout.h"
#include "rendering/score/slurtielayout.h"

#include "log.h"

using namespace muse;
using namespace muse::draw;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;
using namespace mu::engraving::rendering::single;

void SingleLayout::layoutItem(EngravingItem* item)
{
    Context ctx(item->score());

    switch (item->type()) {
    case ElementType::ACCIDENTAL:   layout(toAccidental(item), ctx);
        break;
    case ElementType::ACTION_ICON:  layout(toActionIcon(item), ctx);
        break;
    case ElementType::AMBITUS:      layout(toAmbitus(item), ctx);
        break;
    case ElementType::ARPEGGIO:     layout(toArpeggio(item), ctx);
        break;
    case ElementType::ARTICULATION: layout(toArticulation(item), ctx);
        break;
    case ElementType::BAGPIPE_EMBELLISHMENT: layout(toBagpipeEmbellishment(item), ctx);
        break;
    case ElementType::BAR_LINE:     layout(toBarLine(item), ctx);
        break;
    case ElementType::BEND:         layout(toBend(item), ctx);
        break;
    case ElementType::BRACKET:      layout(toBracket(item), ctx);
        break;
    case ElementType::BREATH:       layout(toBreath(item), ctx);
        break;
    case ElementType::CAPO:         layout(toCapo(item), ctx);
        break;
    case ElementType::CHORDLINE:    layout(toChordLine(item), ctx);
        break;
    case ElementType::CLEF:         layout(toClef(item), ctx);
        break;
    case ElementType::DYNAMIC:      layout(toDynamic(item), ctx);
        break;
    case ElementType::EXPRESSION:   layout(toExpression(item), ctx);
        break;
    case ElementType::FERMATA:      layout(toFermata(item), ctx);
        break;
    case ElementType::FINGERING:    layout(toFingering(item), ctx);
        break;
    case ElementType::FRET_DIAGRAM: layout(toFretDiagram(item), ctx);
        break;
    case ElementType::FSYMBOL:      layout(toFSymbol(item), ctx);
        break;
    case ElementType::GLISSANDO:    layout(toGlissando(item), ctx);
        break;
    case ElementType::GUITAR_BEND:  layout(toGuitarBend(item), ctx);
        break;
    case ElementType::GRADUAL_TEMPO_CHANGE: layout(toGradualTempoChange(item), ctx);
        break;
    case ElementType::HAIRPIN:      layout(toHairpin(item), ctx);
        break;
    case ElementType::HAMMER_ON_PULL_OFF: layout(toHammerOnPullOff(item), ctx);
        break;
    case ElementType::HARP_DIAGRAM: layout(toHarpPedalDiagram(item), ctx);
        break;
    case ElementType::IMAGE:        layout(toImage(item), ctx);
        break;
    case ElementType::INSTRUMENT_CHANGE: layout(toInstrumentChange(item), ctx);
        break;
    case ElementType::JUMP:         layout(toJump(item), ctx);
        break;
    case ElementType::KEYSIG:       layout(toKeySig(item), ctx);
        break;
    case ElementType::LAYOUT_BREAK: layout(toLayoutBreak(item), ctx);
        break;
    case ElementType::LET_RING:     layout(toLetRing(item), ctx);
        break;
    case ElementType::LYRICS:       layout(toLyrics(item), ctx);
        break;
    case ElementType::MARKER:       layout(toMarker(item), ctx);
        break;
    case ElementType::MEASURE_NUMBER: layout(toMeasureNumber(item), ctx);
        break;
    case ElementType::MEASURE_REPEAT: layout(toMeasureRepeat(item), ctx);
        break;
    case ElementType::NOTEHEAD:     layout(toNoteHead(item), ctx);
        break;
    case ElementType::NOTELINE:     layout(toNoteLine(item), ctx);
        break;
    case ElementType::OTTAVA:       layout(toOttava(item), ctx);
        break;
    case ElementType::ORNAMENT:     layout(toOrnament(item), ctx);
        break;
    case ElementType::PALM_MUTE:    layout(toPalmMute(item), ctx);
        break;
    case ElementType::PEDAL:        layout(toPedal(item), ctx);
        break;
    case ElementType::PLAYTECH_ANNOTATION: layout(toPlayTechAnnotation(item), ctx);
        break;
    case ElementType::REHEARSAL_MARK: layout(toRehearsalMark(item), ctx);
        break;
    case ElementType::SLUR:         layout(toSlur(item), ctx);
        break;
    case ElementType::SPACER:       layout(toSpacer(item), ctx);
        break;
    case ElementType::STAFF_TEXT:   layout(toStaffText(item), ctx);
        break;
    case ElementType::STAFFTYPE_CHANGE: layout(toStaffTypeChange(item), ctx);
        break;
    case ElementType::STICKING:     layout(toSticking(item), ctx);
        break;
    case ElementType::STRING_TUNINGS: layout(toStringTunings(item), ctx);
        break;
    case ElementType::SYMBOL:       layout(toSymbol(item), ctx);
        break;
    case ElementType::SYSTEM_TEXT:  layout(toSystemText(item), ctx);
        break;
    case ElementType::SOUND_FLAG:   layout(item_cast<SoundFlag*>(item), ctx);
        break;
    case ElementType::TEMPO_TEXT:   layout(toTempoText(item), ctx);
        break;
    case ElementType::TEXT:         layout(toText(item), ctx);
        break;
    case ElementType::TEXTLINE:     layout(toTextLine(item), ctx);
        break;
    case ElementType::TIMESIG:      layout(toTimeSig(item), ctx);
        break;
    case ElementType::TREMOLO_SINGLECHORD: layout(item_cast<TremoloSingleChord*>(item), ctx);
        break;
    case ElementType::TREMOLO_TWOCHORD:    layout(item_cast<TremoloTwoChord*>(item), ctx);
        break;
    case ElementType::TREMOLOBAR:   layout(toTremoloBar(item), ctx);
        break;
    case ElementType::TRILL:        layout(toTrill(item), ctx);
        break;
    case ElementType::VIBRATO:      layout(toVibrato(item), ctx);
        break;
    case ElementType::VOLTA:        layout(toVolta(item), ctx);
        break;
    // drumset
    case ElementType::CHORD:        layout(toChord(item), ctx);
        break;
    case ElementType::STEM:         layout(toStem(item), ctx);
        break;
    default:
        LOGE() << "Not handled: " << item->typeName();
        IF_ASSERT_FAILED(false) {
            LayoutContext tctx(item->score());
            TLayout::layoutItem(item, tctx);
        }
        break;
    }
}

void SingleLayout::layoutLineSegment(LineSegment* item, const Context& ctx)
{
    switch (item->type()) {
    case ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT: layout(toGradualTempoChangeSegment(item), ctx);
        break;
    case ElementType::HAIRPIN_SEGMENT:   layout(toHairpinSegment(item), ctx);
        break;
    case ElementType::LET_RING_SEGMENT:  layout(toLetRingSegment(item), ctx);
        break;
    case ElementType::OTTAVA_SEGMENT:    layout(toOttavaSegment(item), ctx);
        break;
    case ElementType::PALM_MUTE_SEGMENT: layout(toPalmMuteSegment(item), ctx);
        break;
    case ElementType::PEDAL_SEGMENT:     layout(toPedalSegment(item), ctx);
        break;
    case ElementType::TEXTLINE_SEGMENT:  layout(toTextLineSegment(item), ctx);
        break;
    case ElementType::TRILL_SEGMENT:     layout(toTrillSegment(item), ctx);
        break;
    case ElementType::VIBRATO_SEGMENT:   layout(toVibratoSegment(item), ctx);
        break;
    case ElementType::VOLTA_SEGMENT:     layout(toVoltaSegment(item), ctx);
        break;
    default:
        UNREACHABLE;
        break;
    }
}

const MStyle& SingleLayout::Context::style() const
{
    return m_score->style();
}

std::shared_ptr<IEngravingFont> SingleLayout::Context::engravingFont() const
{
    return m_score->engravingFont();
}

compat::DummyElement* SingleLayout::Context::dummyParent() const
{
    return m_score->dummy();
}

void SingleLayout::layout(Accidental* item, const Context&)
{
    if (!item->ldata() || !item->ldata()->isValid()) {
        Accidental::LayoutData* ldata = item->mutldata();
        SymId symId = item->symId();
        Accidental::LayoutData::Sym s(symId, 0.0, 0.0);
        ldata->syms.push_back(s);
        ldata->setBbox(item->symBbox(symId));
    }
}

void SingleLayout::layout(ActionIcon* item, const Context&)
{
    FontMetrics fontMetrics(item->iconFont());
    item->setbbox(fontMetrics.boundingRect(Char(item->icon())));
}

void SingleLayout::layout(Ambitus* item, const Context& ctx)
{
    Ambitus::LayoutData* ldata = item->mutldata();
    double headWdt = item->headWidth();
    double spatium = item->spatium();

    double lineDist = spatium;
    constexpr int numOfLines = 3;

    //
    // NOTEHEADS Y POS
    //
    // if pitch == INVALID_PITCH or tpc == Tpc::TPC_INVALID, set to some default:
    // for use in palettes and when actual range cannot be calculated (new ambitus or no notes in staff)
    //

    // top notehead
    ldata->topPos.setY(0.0);

    // bottom notehead
    ldata->bottomPos.setY((numOfLines - 1) * lineDist);

    //
    // NOTEHEAD X POS
    //
    // Note: manages colliding accidentals
    //

    layout(item->topAccidental(), ctx);
    layout(item->bottomAccidental(), ctx);

    double accNoteDist = item->absoluteFromSpatium(ctx.style().styleS(Sid::accidentalNoteDistance));
    double xAccidOffTop = item->topAccidental()->width(LD_ACCESS::BAD) + accNoteDist;
    double xAccidOffBottom = item->bottomAccidental()->width(LD_ACCESS::BAD) + accNoteDist;

    switch (item->direction()) {
    case DirectionH::AUTO:                       // noteheads one above the other
        // left align noteheads and right align accidentals 'hanging' on the left
        ldata->topPos.setX(0.0);
        ldata->bottomPos.setX(0.0);
        item->topAccidental()->mutldata()->setPosX(-xAccidOffTop);
        item->bottomAccidental()->mutldata()->setPosX(-xAccidOffBottom);
        break;
    case DirectionH::LEFT:                       // top notehead at the left of bottom notehead
        // place top notehead at left margin; bottom notehead at right of top head;
        // top accid. 'hanging' on left of top head and bottom accid. 'hanging' at left of bottom head
        ldata->topPos.setX(0.0);
        ldata->bottomPos.setX(headWdt);
        item->topAccidental()->mutldata()->setPosX(-xAccidOffTop);
        item->bottomAccidental()->mutldata()->setPosX(headWdt - xAccidOffBottom);
        break;
    case DirectionH::RIGHT:                      // top notehead at the right of bottom notehead
        // bottom notehead at left margin; top notehead at right of bottomnotehead
        // top accid. 'hanging' on left of top head and bottom accid. 'hanging' at left of bottom head
        ldata->bottomPos.setX(0.0);
        ldata->topPos.setX(headWdt);
        item->bottomAccidental()->mutldata()->setPosX(-xAccidOffBottom);
        item->topAccidental()->mutldata()->setPosX(headWdt - xAccidOffTop);
        break;
    }

    // compute line from top note centre to bottom note centre
    LineF fullLine(ldata->topPos.x() + headWdt * 0.5,
                   ldata->topPos.y(),
                   ldata->bottomPos.x() + headWdt * 0.5,
                   ldata->bottomPos.y());
    // shorten line on each side by offsets
    double yDelta = ldata->bottomPos.y() - ldata->topPos.y();
    if (!RealIsNull(yDelta)) {
        double off = spatium * Ambitus::LINEOFFSET_DEFAULT;
        PointF p1 = fullLine.pointAt(off / yDelta);
        PointF p2 = fullLine.pointAt(1 - (off / yDelta));
        ldata->line = LineF(p1, p2);
    } else {
        ldata->line = fullLine;
    }

    RectF headRect(0, -0.5 * spatium, headWdt, 1 * spatium);
    ldata->setBbox(headRect.translated(ldata->topPos).united(headRect.translated(ldata->bottomPos))
                   .united(item->topAccidental()->ldata()->bbox(LD_ACCESS::BAD)
                           .translated(item->topAccidental()->ldata()->pos()))
                   .united(item->bottomAccidental()->ldata()->bbox(LD_ACCESS::BAD)
                           .translated(item->bottomAccidental()->ldata()->pos()))
                   );
}

void SingleLayout::layout(Arpeggio* item, const Context& ctx)
{
    Arpeggio::LayoutData* ldata = item->mutldata();

    auto symbolLine = [](const std::shared_ptr<const IEngravingFont>& f, Arpeggio::LayoutData* data, SymId end, SymId fill)
    {
        data->symbols.clear();

        double w = data->bottom - data->top;
        double w1 = f->advance(end, data->magS);
        double w2 = f->advance(fill, data->magS);
        int n = lrint((w - w1) / w2);
        for (int i = 0; i < n; ++i) {
            data->symbols.push_back(fill);
        }
        data->symbols.push_back(end);
    };

    ldata->arpeggioHeight = item->spatium() * 4;
    ldata->top = 0.0;
    ldata->bottom = ldata->arpeggioHeight;

    ldata->setMag(item->staff() ? item->staff()->staffMag(item->tick()) : item->mag());
    ldata->magS = ldata->mag() * (ctx.style().spatium() / SPATIUM20);

    std::shared_ptr<const IEngravingFont> font = ctx.engravingFont();
    switch (item->arpeggioType()) {
    case ArpeggioType::NORMAL: {
        symbolLine(font, ldata, SymId::wiggleArpeggiatoUp, SymId::wiggleArpeggiatoUp);
        // string is rotated -90 degrees
        ldata->symsBBox = font->bbox(ldata->symbols, ldata->magS);
        ldata->setBbox(RectF(0.0, -ldata->symsBBox.x() + ldata->top, ldata->symsBBox.height(), ldata->symsBBox.width()));
    } break;

    case ArpeggioType::UP: {
        symbolLine(font, ldata, SymId::wiggleArpeggiatoUpArrow, SymId::wiggleArpeggiatoUp);
        // string is rotated -90 degrees
        ldata->symsBBox = font->bbox(ldata->symbols, ldata->magS);
        ldata->setBbox(RectF(0.0, -ldata->symsBBox.x() + ldata->top, ldata->symsBBox.height(), ldata->symsBBox.width()));
    } break;

    case ArpeggioType::DOWN: {
        symbolLine(font, ldata, SymId::wiggleArpeggiatoUpArrow, SymId::wiggleArpeggiatoUp);
        // string is rotated +90 degrees (so that UpArrow turns into a DownArrow)
        ldata->symsBBox = font->bbox(ldata->symbols, ldata->magS);
        ldata->setBbox(RectF(0.0, ldata->symsBBox.x() + ldata->top, ldata->symsBBox.height(), ldata->symsBBox.width()));
    } break;

    case ArpeggioType::UP_STRAIGHT: {
        double x1 = item->spatium() * 0.5;
        ldata->symsBBox = font->bbox(SymId::arrowheadBlackUp, ldata->magS);
        double w = ldata->symsBBox.width();
        ldata->setBbox(RectF(x1 - w * 0.5, ldata->top, w, ldata->bottom));
    } break;

    case ArpeggioType::DOWN_STRAIGHT: {
        double x1 = item->spatium() * 0.5;
        ldata->symsBBox = font->bbox(SymId::arrowheadBlackDown, ldata->magS);
        double w = ldata->symsBBox.width();
        ldata->setBbox(RectF(x1 - w * 0.5, ldata->top, w, ldata->bottom));
    } break;

    case ArpeggioType::BRACKET: {
        double w  = ctx.style().styleS(Sid::arpeggioHookLen).val() * item->spatium();
        ldata->setBbox(RectF(0.0, ldata->top, w, ldata->bottom));
    } break;
    }
}

void SingleLayout::layout(Articulation* item, const Context&)
{
    RectF bbox;

    if (item->textType() != ArticulationTextType::NO_TEXT) {
        Font scaledFont(item->font());
        scaledFont.setPointSizeF(item->font().pointSizeF() * item->magS());
        FontMetrics fm(scaledFont);
        bbox = fm.boundingRect(scaledFont, TConv::text(item->textType()));
    } else {
        bbox = item->symBbox(item->symId());
    }

    item->setbbox(bbox.translated(-0.5 * bbox.width(), 0.0));
}

void SingleLayout::layout(BagpipeEmbellishment* item, const Context& ctx)
{
    BagpipeEmbellishment::LayoutData* ldata = item->mutldata();
    const double mags = item->magS() * 0.75; // grace head magnification
    const double spatium = ctx.style().spatium();

    const BagpipeNoteList nl = item->resolveNoteList();

    ldata->clearBbox();
    ldata->isDrawBeam = nl.size() > 1;
    ldata->isDrawFlag = nl.size() == 1;
    ldata->spatium = spatium;
    ldata->headsym = SymId::noteheadBlack;
    ldata->flagsym = SymId::flag32ndUp;
    ldata->stemLineW = ldata->spatium * 0.1;

    BagpipeEmbellishment::LayoutData::BeamData& dataBeam = ldata->beamData;
    dataBeam.y = (-8 * spatium / 2);
    dataBeam.width = (0.3 * spatium);

    const RectF headBBox = ctx.engravingFont()->bbox(ldata->headsym, mags);
    const RectF flagBBox = ctx.engravingFont()->bbox(ldata->flagsym, mags);
    const double xcorr = spatium * 0.1;     // correction to align flag with top of stem

    double headW = headBBox.width();
    const double headw = 1.2 * headW;       // grace head width
    const double headp = 1.6 * headW;       // horizontal head pitch
    const double xl = (1 - 1.6 * (nl.size() - 1)) * headW / 2; // calc x for stem of leftmost note

    // draw the notes including stem, (optional) flag and (optional) ledger line
    double x = xl;
    for (size_t i = 0; i < nl.size(); ++i) {
        BagpipeEmbellishment::LayoutData::NoteData& noteData = ldata->notesData[i];
        const int line = BagpipeEmbellishment::BAGPIPE_NOTEINFO_LIST[nl.at(i)].line;
        const double y1f = ((line - 6) * spatium / 2);      // top of stem for note with flag
        const double y2 = (line * spatium / 2);             // bottom of stem
        const double ycorr = (0.8 * spatium);               // correction to align flag with top of stem

        // head
        noteData.headXY = PointF(x - headw, y2);
        ldata->addBbox(headBBox.translated(noteData.headXY));

        // stem
        // top of stems actually used
        double y1 = ldata->isDrawFlag ? y1f : dataBeam.y;
        noteData.stemLine = LineF(x - ldata->stemLineW * .5, y1, x - ldata->stemLineW * .5, y2);
        ldata->addBbox(RectF(x - ldata->stemLineW * .5 - headw, y1, ldata->stemLineW, y2 - y1));

        // flag
        if (ldata->isDrawFlag) {
            noteData.flagXY = PointF(x - ldata->stemLineW * .5 + xcorr, y1 + ycorr);
            ldata->addBbox(flagBBox.translated(noteData.flagXY));
        }

        // draw the ledger line for high A
        if (line == -2) {
            noteData.ledgerLine = LineF(x - headw * 1.5 - ldata->stemLineW * .5, y2, x + headw * .5 - ldata->stemLineW * .5, y2);
            ldata->addBbox(RectF(x - headw * 1.5 - ldata->stemLineW * .5, y2 - ldata->stemLineW * 2, headw * 2, ldata->stemLineW));
        }

        // move x to next note x position
        x += headp;
    }

    dataBeam.x1 = xl - ldata->stemLineW * .5;
    dataBeam.x2 = x - headp - ldata->stemLineW * .5;
}

void SingleLayout::layout(BarLine* item, const Context& ctx)
{
    BarLine::LayoutData* ldata = item->mutldata();
    ldata->setPos(PointF());
    ldata->setMag(1.0);

    double spatium = item->spatium();
    ldata->y1 = (spatium * .5 * item->spanFrom());
    if (muse::RealIsEqual(ldata->y2, 0.0)) {
        ldata->y2 = (spatium * .5 * (8.0 + item->spanTo()));
    }

    auto layoutWidth = [](BarLine* item, const Context& ctx) {
        const double dotWidth = item->symWidth(SymId::repeatDot);

        double w = 0.0;
        switch (item->barLineType()) {
        case BarLineType::DOUBLE:
            w = ctx.style().styleMM(Sid::doubleBarWidth) * 2.0 + ctx.style().styleMM(Sid::doubleBarDistance);
            break;
        case BarLineType::DOUBLE_HEAVY:
            w = ctx.style().styleMM(Sid::endBarWidth) * 2.0 + ctx.style().styleMM(Sid::endBarDistance);
            break;
        case BarLineType::END_START_REPEAT:
            w = ctx.style().styleMM(Sid::endBarWidth)
                + ctx.style().styleMM(Sid::barWidth) * 2.0
                + ctx.style().styleMM(Sid::endBarDistance) * 2.0
                + ctx.style().styleMM(Sid::repeatBarlineDotSeparation) * 2.0
                + dotWidth * 2;
            break;
        case BarLineType::START_REPEAT:
        case BarLineType::END_REPEAT:
            w = ctx.style().styleMM(Sid::endBarWidth)
                + ctx.style().styleMM(Sid::barWidth)
                + ctx.style().styleMM(Sid::endBarDistance)
                + ctx.style().styleMM(Sid::repeatBarlineDotSeparation)
                + dotWidth;
            break;
        case BarLineType::END:
        case BarLineType::REVERSE_END:
            w = ctx.style().styleMM(Sid::endBarWidth)
                + ctx.style().styleMM(Sid::barWidth)
                + ctx.style().styleMM(Sid::endBarDistance);
            break;
        case BarLineType::BROKEN:
        case BarLineType::NORMAL:
        case BarLineType::DOTTED:
            w = ctx.style().styleMM(Sid::barWidth);
            break;
        case BarLineType::HEAVY:
            w = ctx.style().styleMM(Sid::endBarWidth);
            break;
        }
        return w;
    };

    double w = layoutWidth(item, ctx) * ldata->mag();
    RectF bbox(0.0, ldata->y1, w, ldata->y2 - ldata->y1);
    ldata->setBbox(bbox);
}

void SingleLayout::layout(Bend* item, const Context&)
{
    Bend::LayoutData* ldata = item->mutldata();
    double spatium = item->spatium();
    double lw = item->absoluteFromSpatium(item->lineWidth());

    ldata->noteWidth = 0.0;
    ldata->notePos = PointF();

    RectF bb;

    FontMetrics fm(item->font(spatium));

    size_t n   = item->points().size();
    double x = ldata->noteWidth;
    double y = -spatium * .8;
    double x2, y2;

    double aw = spatium * .5;
    PolygonF arrowUp;
    arrowUp << PointF(0, 0) << PointF(aw * .5, aw) << PointF(-aw * .5, aw);
    PolygonF arrowDown;
    arrowDown << PointF(0, 0) << PointF(aw * .5, -aw) << PointF(-aw * .5, -aw);

    for (size_t pt = 0; pt < n; ++pt) {
        if (pt == (n - 1)) {
            break;
        }
        int pitch = item->points().at(pt).pitch;
        if (pt == 0 && pitch) {
            y2 = -ldata->notePos.y() - spatium * 2;
            x2 = x;
            bb.unite(RectF(x, y, x2 - x, y2 - y));

            bb.unite(arrowUp.translated(x2, y2 + spatium * .2).boundingRect());

            int idx = (pitch + 12) / 25;
            const char* l = Bend::label[idx];
            bb.unite(fm.boundingRect(RectF(x2, y2, 0, 0),
                                     muse::draw::AlignHCenter | muse::draw::AlignBottom | muse::draw::TextDontClip,
                                     String::fromAscii(l)));
            y = y2;
        }
        if (pitch == item->points().at(pt + 1).pitch) {
            if (pt == (n - 2)) {
                break;
            }
            x2 = x + spatium;
            y2 = y;
            bb.unite(RectF(x, y, x2 - x, y2 - y));
        } else if (pitch < item->points().at(pt + 1).pitch) {
            // up
            x2 = x + spatium * .5;
            y2 = -ldata->notePos.y() - spatium * 2;
            double dx = x2 - x;
            double dy = y2 - y;

            PainterPath path;
            path.moveTo(x, y);
            path.cubicTo(x + dx / 2, y, x2, y + dy / 4, x2, y2);
            bb.unite(path.boundingRect());
            bb.unite(arrowUp.translated(x2, y2 + spatium * .2).boundingRect());

            int idx = (item->points().at(pt + 1).pitch + 12) / 25;
            const char* l = Bend::label[idx];
            bb.unite(fm.boundingRect(RectF(x2, y2, 0, 0),
                                     muse::draw::AlignHCenter | muse::draw::AlignBottom | muse::draw::TextDontClip,
                                     String::fromAscii(l)));
        } else {
            // down
            x2 = x + spatium * .5;
            y2 = y + spatium * 3;
            double dx = x2 - x;
            double dy = y2 - y;

            PainterPath path;
            path.moveTo(x, y);
            path.cubicTo(x + dx / 2, y, x2, y + dy / 4, x2, y2);
            bb.unite(path.boundingRect());

            bb.unite(arrowDown.translated(x2, y2 - spatium * .2).boundingRect());
        }
        x = x2;
        y = y2;
    }
    bb.adjust(-lw, -lw, lw, lw);
    ldata->setBbox(bb);
    ldata->setPos(0.0, 0.0);
}

void SingleLayout::layout(Bracket* item, const Context& ctx)
{
    Bracket::LayoutData* ldata = item->mutldata();

    ldata->bracketHeight = 3.5 * item->spatium() * 2;
    ldata->braceSymbol = item->braceSymbol();

    Shape shape;

    switch (item->bracketType()) {
    case BracketType::BRACE: {
        if (item->braceSymbol() == SymId::noSym) {
            ldata->braceSymbol = SymId::brace;
        }
        double h = ldata->bracketHeight;
        double w = item->symWidth(ldata->braceSymbol) * item->magx();
        ldata->setBbox(RectF(0, 0, w, h));
        ldata->shape.add(ldata->bbox());
        ldata->bracketWidth = w + ctx.style().styleMM(Sid::akkoladeBarDistance);
    }
    break;
    case BracketType::NORMAL: {
        double spatium = item->spatium();
        double w = ctx.style().styleMM(Sid::bracketWidth) * 0.5;
        double x = -w;

        double bd = spatium * 0.5;
        shape.add(RectF(x, -bd, w * 2, 2 * (item->ldata()->h2() + bd)));
        shape.add(item->symBbox(SymId::bracketTop).translated(PointF(-w, -bd)));
        shape.add(item->symBbox(SymId::bracketBottom).translated(PointF(-w, bd + 2 * item->ldata()->h2())));

        w += item->symWidth(SymId::bracketTop);
        double y = -item->symHeight(SymId::bracketTop) - bd;
        double h = (-y + item->ldata()->h2()) * 2;
        ldata->setBbox(x, y, w, h);

        ldata->bracketWidth = ctx.style().styleMM(Sid::bracketWidth) + ctx.style().styleMM(Sid::bracketDistance);
    }
    break;
    case BracketType::SQUARE: {
        double w = ctx.style().styleMM(Sid::staffLineWidth) * .5;
        double x = -w;
        double y = -w;
        double h = (item->ldata()->h2() + w) * 2;
        w += (0.5 * item->spatium() + 3 * w);
        ldata->setBbox(x, y, w, h);
        shape.add(item->ldata()->bbox());

        ldata->bracketWidth = ctx.style().styleMM(Sid::staffLineWidth) / 2 + 0.5 * item->spatium();
    }
    break;
    case BracketType::LINE: {
        double spatium = item->spatium();
        double w = 0.67 * ctx.style().styleMM(Sid::bracketWidth) * 0.5;
        double x = -w;
        double bd = spatium * 0.25;
        double y = -bd;
        double h = (-y + item->ldata()->h2()) * 2;
        ldata->setBbox(x, y, w, h);
        shape.add(item->ldata()->bbox());

        ldata->bracketWidth = 0.67 * ctx.style().styleMM(Sid::bracketWidth) + ctx.style().styleMM(Sid::bracketDistance);
    }
    break;
    case BracketType::NO_BRACKET:
        break;
    }

    ldata->shape = shape;
}

void SingleLayout::layout(Breath* item, const Context&)
{
    item->setbbox(item->symBbox(item->symId()));
}

void SingleLayout::layout(Capo* item, const Context& ctx)
{
    layoutTextBase(item, ctx, item->mutldata());
}

void SingleLayout::layout(Chord* item, const Context& ctx)
{
    LayoutContext tctx(ctx.dontUseScore());
    ChordLayout::computeUp(item, tctx);
    ChordLayout::layout(item, tctx);
    ChordLayout::layoutStem(item, tctx);
    ChordLayout::layoutLedgerLines({ item }, tctx);
}

void SingleLayout::layout(ChordLine* item, const Context& ctx)
{
    ChordLine::LayoutData* ldata = item->mutldata();
    ldata->setMag(1.0);
    if (!item->modified()) {
        double x2 = 0;
        double y2 = 0;
        double baseLength = item->spatium();
        double horBaseLength = 1.2 * baseLength;     // let the symbols extend a bit more horizontally
        x2 += item->isToTheLeft() ? -horBaseLength : horBaseLength;
        y2 += item->isBelow() ? baseLength : -baseLength;
        if (item->chordLineType() != ChordLineType::NOTYPE && !item->isWavy()) {
            PainterPath path;
            if (!item->isToTheLeft()) {
                if (item->isStraight()) {
                    path.lineTo(x2, y2);
                } else {
                    path.cubicTo(x2 / 2, 0.0, x2, y2 / 2, x2, y2);
                }
            } else {
                if (item->isStraight()) {
                    path.lineTo(x2, y2);
                } else {
                    path.cubicTo(0.0, y2 / 2, x2 / 2, y2, x2, y2);
                }
            }
            ldata->path = path;
        }
    }

    ldata->setPos(0.0, 0.0);

    if (!item->isWavy()) {
        RectF r = ldata->path.boundingRect();
        int x1 = 0, y1 = 0, width = 0, height = 0;

        x1 = r.x();
        y1 = r.y();
        width = r.width();
        height = r.height();
        ldata->setBbox(x1, y1, width, height);
    } else {
        RectF r = ctx.engravingFont()->bbox(item->waveSym(), item->magS());

        ldata->setBbox(r);
    }
}

void SingleLayout::layout(Clef* item, const Context& ctx)
{
    Clef::LayoutData* ldata = item->mutldata();
    constexpr int lines = 5;
    constexpr double lineDist = 1.0;
    double spatium = ctx.style().spatium();
    double yoff = 0.0;

    if (item->clefType() != ClefType::INVALID && item->clefType() != ClefType::MAX) {
        ldata->symId = ClefInfo::symId(item->clefType());
        yoff = lineDist * (5 - ClefInfo::line(item->clefType()));
    } else {
        ldata->symId = SymId::noSym;
    }

    switch (item->clefType()) {
    case ClefType::C_19C:                                    // 19th C clef is like a G clef
        yoff = lineDist * 1.5;
        break;
    case ClefType::TAB:         // TAB clef
    case ClefType::TAB4:        // TAB clef 4 strings
    case ClefType::TAB_SERIF:   // TAB clef alternate style
    case ClefType::TAB4_SERIF:  // TAB clef alternate style
        yoff = lineDist * (lines - 1) * 0.5;
        break;
    case ClefType::PERC:        // percussion clefs
    case ClefType::PERC2:
        yoff = lineDist * (lines - 1) * 0.5;
        break;
    default:
        break;
    }

    ldata->setPos(0.0, yoff * spatium);

    RectF bbox = item->symBbox(ldata->symId);
    ldata->setBbox(bbox);
}

void SingleLayout::layout(Expression* item, const Context& ctx)
{
    layoutTextBase(item, ctx, item->mutldata());
}

void SingleLayout::layout(Fermata* item, const Context&)
{
    item->setPos(PointF());
    item->setOffset(PointF());
    RectF b(item->symBbox(item->symId()));
    item->setbbox(b.translated(-0.5 * b.width(), 0.0));
}

void SingleLayout::layout(Fingering* item, const Context& ctx)
{
    layoutTextBase(item, ctx, item->mutldata());
}

void SingleLayout::layout(FretDiagram* item, const Context& ctx)
{
    FretDiagram::LayoutData* ldata = item->mutldata();
    double spatium  = item->spatium();
    ldata->stringLineWidth = (spatium * 0.08);
    ldata->nutLineWidth = ((item->fretOffset() || !item->showNut()) ? ldata->stringLineWidth : spatium * 0.2);
    ldata->stringDist = (ctx.style().styleMM(Sid::fretStringSpacing));
    ldata->fretDist = (ctx.style().styleMM(Sid::fretFretSpacing));
    ldata->markerSize = (ldata->stringDist * 0.8);

    double w = ldata->stringDist * (item->strings() - 1) + ldata->markerSize;
    double h = (item->frets() + 1) * ldata->fretDist + ldata->markerSize;
    double y = -(ldata->markerSize * 0.5 + ldata->fretDist);
    double x = -(ldata->markerSize * 0.5);

    // Allocate space for fret offset number
    if (item->fretOffset() > 0) {
        Font scaledFont(item->fretNumFont());
        scaledFont.setPointSizeF(item->fretNumFont().pointSizeF() * item->userMag());

        FontMetrics fm2(scaledFont);
        double numw = fm2.width(String::number(item->fretOffset() + 1));
        double xdiff = numw + ldata->stringDist * .4;
        w += xdiff;
        x += (item->numPos() == 0) == (item->orientation() == engraving::Orientation::VERTICAL) ? -xdiff : 0;
    }

    if (item->orientation() == engraving::Orientation::HORIZONTAL) {
        std::swap(w, h);
        std::swap(x, y);
    }

    ldata->setBbox(x, y, w, h);
}

void SingleLayout::layout(FSymbol* item, const Context&)
{
    item->setbbox(FontMetrics::boundingRect(item->font(), item->toString()));
    item->setOffset(0.0, 0.0);
    item->setPos(0.0, 0.0);
}

void SingleLayout::layout(Dynamic* item, const Context& ctx)
{
    layoutTextBase(item, ctx, item->mutldata());
}

void SingleLayout::layout(Glissando* item, const Context& ctx)
{
    double spatium = item->spatium();

    if (item->spannerSegments().empty()) {
        item->add(item->createLineSegment(ctx.dummyParent()->system()));
    }
    LineSegment* s = item->frontSegment();
    s->setPos(PointF(-spatium * Glissando::GLISS_PALETTE_WIDTH / 2, spatium * Glissando::GLISS_PALETTE_HEIGHT / 2));
    s->setPos2(PointF(spatium * Glissando::GLISS_PALETTE_WIDTH, -spatium * Glissando::GLISS_PALETTE_HEIGHT));
    layout(static_cast<GlissandoSegment*>(s), ctx);
}

void SingleLayout::layout(GlissandoSegment* item, const Context&)
{
    if (item->pos2().x() <= 0) {
        item->setbbox(RectF());
        return;
    }

    RectF r = RectF(0.0, 0.0, item->pos2().x(), item->pos2().y()).normalized();
    double lw = item->absoluteFromSpatium(item->lineWidth()) * .5;
    item->setbbox(r.adjusted(-lw, -lw, lw, lw));
}

void SingleLayout::layout(GradualTempoChange* item, const Context& ctx)
{
    layoutLine(item, ctx);
}

void SingleLayout::layout(GradualTempoChangeSegment* item, const Context& ctx)
{
    layoutTextLineBaseSegment(item, ctx);
    item->setOffset(PointF());
}

void SingleLayout::layout(GuitarBend*, const Context&)
{
    NOT_IMPLEMENTED;
    //! NOTE: Bends can be removed from disallowed elements in NotationInteraction::isOutgoingDragElementAllowed once this has been implemented
}

void SingleLayout::layout(GuitarBendSegment*, const Context&)
{
    NOT_IMPLEMENTED;
}

void SingleLayout::layout(Hairpin* item, const Context& ctx)
{
    item->setPos(0.0, 0.0);
    layoutLine(item, ctx);
}

void SingleLayout::layout(HammerOnPullOff* item, const Context& ctx)
{
    double spatium = item->spatium();
    HammerOnPullOffSegment* s = nullptr;
    if (item->spannerSegments().empty()) {
        s = new HammerOnPullOffSegment(ctx.dummyParent()->system());
        s->setTrack(item->track());
        item->add(s);
    } else {
        s = toHammerOnPullOffSegment(item->frontSegment());
    }

    s->setSpannerSegmentType(SpannerSegmentType::SINGLE);

    s->setPos(PointF());
    s->ups(Grip::START).p = PointF(0, 0);
    s->ups(Grip::END).p   = PointF(spatium * 6, 0);
    s->setExtraHeight(0.0);

    SlurTieLayout::computeBezier(s);

    layout(s, ctx);

    item->setbbox(s->ldata()->bbox());
}

void SingleLayout::layout(HammerOnPullOffSegment* item, const Context& ctx)
{
    const std::vector<HammerOnPullOffText*>& hopoTexts = item->hopoText();
    if (item->hopoText().empty()) {
        HammerOnPullOffText* hopoText = new HammerOnPullOffText(item);
        hopoText->setParent(item);
        hopoText->setXmlText("H/P");
        item->addHopoText(hopoText);
    }

    HammerOnPullOffText* hopoText = hopoTexts.front();
    Align align;
    align.vertical = AlignV::BASELINE;
    align.horizontal = AlignH::HCENTER;
    hopoText->setAlign(align);
    layoutTextBase(hopoText, ctx, hopoText->mutldata());

    RectF bbox = item->ldata()->bbox();
    double x = 0.5 * (bbox.left() + bbox.right());
    double y = bbox.top() - 0.5 * item->spatium();
    hopoText->mutldata()->setPos(x, y);

    Shape itemShape = item->mutldata()->shape();
    itemShape.add(hopoText->shape().translated(hopoText->pos()));
    item->mutldata()->setShape(itemShape);
}

void SingleLayout::layout(HairpinSegment* item, const Context& ctx)
{
    const double spatium = item->spatium();

    HairpinType type = item->hairpin()->hairpinType();
    if (item->hairpin()->isLineType()) {
        item->setTwoLines(false);
        layoutTextLineBaseSegment(item, ctx);
        item->setDrawCircledTip(false);
        item->setCircledTipRadius(0.0);
    } else {
        item->setTwoLines(true);

        item->hairpin()->setBeginTextAlign({ AlignH::LEFT, AlignV::VCENTER });
        item->hairpin()->setEndTextAlign({ AlignH::RIGHT, AlignV::VCENTER });

        double x1 = 0.0;
        layoutTextLineBaseSegment(item, ctx);
        if (!item->text()->empty()) {
            x1 = item->text()->width() + spatium * .5;
        }

        Transform t;
        double h1 = item->hairpin()->hairpinHeight().val() * spatium * .5;
        double h2 = item->hairpin()->hairpinContHeight().val() * spatium * .5;

        double x = item->pos2().x();
        if (!item->endText()->empty()) {
            x -= (item->endText()->width() + spatium * .5);             // 0.5 spatium distance
        }
        if (x < spatium) {               // minimum size of hairpin
            x = spatium;
        }
        double y = item->pos2().y();
        double len = sqrt(x * x + y * y);
        t.rotateRadians(asin(y / len));

        item->setDrawCircledTip(item->hairpin()->hairpinCircledTip());
        item->setCircledTipRadius(item->drawCircledTip() ? 0.6 * spatium * .5 : 0.0);

        LineF l1, l2;

        switch (type) {
        case HairpinType::CRESC_HAIRPIN: {
            switch (item->spannerSegmentType()) {
            case SpannerSegmentType::SINGLE:
            case SpannerSegmentType::BEGIN: {
                l1.setLine(x1 + item->circledTipRadius() * 2.0, 0.0, len, h1);
                l2.setLine(x1 + item->circledTipRadius() * 2.0, 0.0, len, -h1);
                PointF circledTip;
                circledTip.setX(x1 + item->circledTipRadius());
                circledTip.setY(0.0);
                item->setCircledTip(circledTip);
            } break;

            case SpannerSegmentType::MIDDLE:
            case SpannerSegmentType::END:
                item->setDrawCircledTip(false);
                l1.setLine(x1,  h2, len, h1);
                l2.setLine(x1, -h2, len, -h1);
                break;
            }
        }
        break;
        case HairpinType::DECRESC_HAIRPIN: {
            switch (item->spannerSegmentType()) {
            case SpannerSegmentType::SINGLE:
            case SpannerSegmentType::END: {
                l1.setLine(x1,  h1, len - item->circledTipRadius() * 2, 0.0);
                l2.setLine(x1, -h1, len - item->circledTipRadius() * 2, 0.0);
                PointF circledTip;
                circledTip.setX(len - item->circledTipRadius());
                circledTip.setY(0.0);
                item->setCircledTip(circledTip);
            } break;
            case SpannerSegmentType::BEGIN:
            case SpannerSegmentType::MIDDLE:
                item->setDrawCircledTip(false);
                l1.setLine(x1,  h1, len, +h2);
                l2.setLine(x1, -h1, len, -h2);
                break;
            }
        }
        break;
        default:
            break;
        }

        // Do Coord rotation
        l1 = t.map(l1);
        l2 = t.map(l2);
        if (item->drawCircledTip()) {
            item->setCircledTip(t.map(item->circledTip()));
        }

        item->pointsRef()[0] = l1.p1();
        item->pointsRef()[1] = l1.p2();
        item->pointsRef()[2] = l2.p1();
        item->pointsRef()[3] = l2.p2();
        item->npointsRef()   = 4;

        RectF r = RectF(l1.p1(), l1.p2()).normalized().united(RectF(l2.p1(), l2.p2()).normalized());
        if (!item->text()->empty()) {
            r.unite(item->text()->ldata()->bbox());
        }
        if (!item->endText()->empty()) {
            r.unite(item->endText()->ldata()->bbox().translated(x + item->endText()->ldata()->bbox().width(), 0.0));
        }
        double w = item->absoluteFromSpatium(ctx.style().styleS(Sid::hairpinLineWidth));
        item->setbbox(r.adjusted(-w * .5, -w * .5, w, w));
    }

    item->setPos(PointF());
    item->setOffset(PointF());
}

void SingleLayout::layout(HarpPedalDiagram* item, const Context& ctx)
{
    item->updateDiagramText();
    layoutTextBase(item, ctx, item->mutldata());
}

void SingleLayout::layout(Image* item, const Context&)
{
    item->init();

    SizeF imageSize = item->size();

    Image::LayoutData* ldata = item->mutldata();
    ldata->setPos(PointF());
    ldata->setBbox(RectF(PointF(), item->size2pixel(imageSize)));
}

void SingleLayout::layout(InstrumentChange* item, const Context& ctx)
{
    layoutTextBase(item, ctx, item->mutldata());
}

void SingleLayout::layout(Jump* item, const Context& ctx)
{
    layoutTextBase(item, ctx, item->mutldata());
}

void SingleLayout::layout(KeySig* item, const Context& ctx)
{
    KeySig::LayoutData* ldata = item->mutldata();
    double spatium = item->spatium();
    double step = spatium * 0.5;

    ldata->setBbox(RectF());
    ldata->keySymbols.clear();

    // determine current clef for this staff
    ClefType clef = ClefType::G;

    int key = int(item->key());

    if (item->isCustom() && !item->isAtonal()) {
        double accidentalGap = ctx.style().styleS(Sid::keysigAccidentalDistance).val();
        // add standard key accidentals first, if necessary
        for (int i = 1; i <= std::abs(key) && std::abs(key) <= 7; ++i) {
            bool drop = false;
            for (const CustDef& cd: item->customKeyDefs()) {
                int degree = item->degInKey(cd.degree);
                // if custom keysig accidental takes place, don't create tonal accidental
                if ((degree * 2 + 2) % 7 == (key < 0 ? 8 - i : i) % 7) {
                    drop = true;
                    break;
                }
            }
            if (!drop) {
                KeySym ks;
                int lineIndexOffset = key > 0 ? -1 : 6;
                ks.sym = key > 0 ? SymId::accidentalSharp : SymId::accidentalFlat;
                ks.line = ClefInfo::lines(clef)[lineIndexOffset + i];
                if (ldata->keySymbols.size() > 0) {
                    KeySym& previous = ldata->keySymbols.back();
                    double previousWidth = item->symWidth(previous.sym) / spatium;
                    ks.xPos = previous.xPos + previousWidth + accidentalGap;
                } else {
                    ks.xPos = 0;
                }
                // TODO octave metters?
                ldata->keySymbols.push_back(ks);
            }
        }
        for (const CustDef& cd : item->customKeyDefs()) {
            SymId sym = item->symInKey(cd.sym, cd.degree);
            int degree = item->degInKey(cd.degree);
            bool flat = std::string(SymNames::nameForSymId(sym).ascii()).find("Flat") != std::string::npos;
            int accIdx = (degree * 2 + 1) % 7; // C D E F ... index to F C G D index
            accIdx = flat ? 13 - accIdx : accIdx;
            int line = ClefInfo::lines(clef)[accIdx] + cd.octAlt * 7;
            double xpos = cd.xAlt;
            if (ldata->keySymbols.size() > 0) {
                KeySym& previous = ldata->keySymbols.back();
                double previousWidth = item->symWidth(previous.sym) / spatium;
                xpos += previous.xPos + previousWidth + accidentalGap;
            }
            // if translated symbol if out of range, add key accidental followed by untranslated symbol
            if (sym == SymId::noSym) {
                KeySym ks;
                ks.line = line;
                ks.xPos = xpos;
                // for quadruple sharp use two double sharps
                if (cd.sym == SymId::accidentalTripleSharp) {
                    ks.sym = SymId::accidentalDoubleSharp;
                    sym = SymId::accidentalDoubleSharp;
                } else {
                    ks.sym = key > 0 ? SymId::accidentalSharp : SymId::accidentalFlat;
                    sym = cd.sym;
                }
                ldata->keySymbols.push_back(ks);
                xpos += key < 0 ? 0.7 : 1; // flats closer
            }
            // create symbol; natural only if is user defined
            if (sym != SymId::accidentalNatural || sym == cd.sym) {
                KeySym ks;
                ks.sym = sym;
                ks.line = line;
                ks.xPos = xpos;
                ldata->keySymbols.push_back(ks);
            }
        }
    } else {
        if (std::abs(key) <= 7) {
            const signed char* lines = ClefInfo::lines(clef);
            SymId sym = key > 0 ? SymId::accidentalSharp : SymId::accidentalFlat;
            double accidentalGap = ctx.style().styleS(Sid::keysigAccidentalDistance).val();
            double previousWidth = item->symWidth(sym) / spatium;
            int lineIndexOffset = key > 0 ? 0 : 7;
            for (int i = 0; i < std::abs(key); ++i) {
                int line = lines[lineIndexOffset + i];
                KeySym ks;
                ks.sym = sym;
                double x = 0.0;
                if (ldata->keySymbols.size() > 0) {
                    const KeySym& previous = ldata->keySymbols.back();
                    x = previous.xPos + previousWidth + accidentalGap;
                    bool isAscending = line < previous.line;
                    SmuflAnchorId currentCutout = isAscending ? SmuflAnchorId::cutOutSW : SmuflAnchorId::cutOutNW;
                    SmuflAnchorId previousCutout = isAscending ? SmuflAnchorId::cutOutNE : SmuflAnchorId::cutOutSE;
                    PointF cutout = item->symSmuflAnchor(sym, currentCutout);
                    double currentCutoutY = line * step + cutout.y();
                    double previousCutoutY = previous.line * step + item->symSmuflAnchor(previous.sym, previousCutout).y();
                    if ((isAscending && currentCutoutY < previousCutoutY) || (!isAscending && currentCutoutY > previousCutoutY)) {
                        x -= cutout.x() / spatium;
                    }
                }
                ks.xPos = x;
                ks.line = line;
                ldata->keySymbols.push_back(ks);
            }
        } else {
            LOGD() << "illegal key:" << key;
        }
    }

    // compute bbox
    for (const KeySym& ks : ldata->keySymbols) {
        double x = ks.xPos * spatium;
        double y = ks.line * step;
        ldata->addBbox(item->symBbox(ks.sym).translated(x, y));
    }
}

void SingleLayout::layout(LayoutBreak* item, const Context&)
{
    FontMetrics metrics(item->font());
    RectF bbox = metrics.boundingRect(item->iconCode());
    item->mutldata()->setShape(Shape(bbox, item));
}

void SingleLayout::layout(LetRing* item, const Context& ctx)
{
    layoutLine(item, ctx);
}

void SingleLayout::layout(LetRingSegment* item, const Context& ctx)
{
    layoutTextLineBaseSegment(item, ctx);
}

void SingleLayout::layout(Lyrics* item, const Context& ctx)
{
    layoutTextBase(static_cast<TextBase*>(item), ctx, item->mutldata());
}

void SingleLayout::layout(NoteHead* item, const Context& ctx)
{
    layout(static_cast<Symbol*>(item), ctx);
}

void SingleLayout::layout(NoteLine*, const Context&)
{
    NOT_IMPLEMENTED;
    //! NOTE: NoteLines can be removed from disallowed elements in NotationInteraction::isOutgoingDragElementAllowed once this has been implemented
}

void SingleLayout::layout(Marker* item, const Context& ctx)
{
    layoutTextBase(item, ctx, item->mutldata());
}

void SingleLayout::layout(MeasureNumber* item, const Context& ctx)
{
    item->setPos(PointF());
    item->setOffset(PointF());

    layout1TextBase(item, ctx, item->mutldata());
}

void SingleLayout::layout(MeasureRepeat* item, const Context& ctx)
{
    MeasureRepeat::LayoutData* ldata = item->mutldata();
    switch (item->numMeasures()) {
    case 1:
    {
        ldata->setSymId(SymId::repeat1Bar);
        if (ctx.style().styleB(Sid::oneMeasureRepeatShow1)) {
            ldata->setNumberSym(1);
        } else {
            ldata->clearNumberSym();
        }
        break;
    }
    case 2:
        ldata->setSymId(SymId::repeat2Bars);
        ldata->setNumberSym(item->numMeasures());
        break;
    case 4:
        ldata->setSymId(SymId::repeat4Bars);
        ldata->setNumberSym(item->numMeasures());
        break;
    default:
        ldata->setSymId(SymId::noSym); // should never happen
        ldata->clearNumberSym();
        break;
    }

    RectF bbox = item->symBbox(ldata->symId);
    ldata->setBbox(bbox);
}

void SingleLayout::layout(Ornament* item, const Context& ctx)
{
    double spatium = item->spatium();
    double vertMargin = 0.35 * spatium;
    constexpr double ornamentAccidentalMag = 0.6; // TODO: style?

    if (!item->showCueNote()) {
        for (size_t i = 0; i < item->accidentalsAboveAndBelow().size(); ++i) {
            bool above = (i == 0);
            Accidental* accidental = item->accidentalsAboveAndBelow()[i];
            if (!accidental) {
                continue;
            }
            accidental->computeMag();
            accidental->mutldata()->setMag(accidental->mag() * ornamentAccidentalMag);
            layout(accidental, ctx);
            Shape accidentalShape = accidental->shape();
            double minVertDist = above
                                 ? accidentalShape.minVerticalDistance(item->ldata()->bbox())
                                 : Shape(item->ldata()->bbox()).minVerticalDistance(accidentalShape);
            accidental->setPos(-0.5 * accidental->width(), above ? (-minVertDist - vertMargin) : (minVertDist + vertMargin));
        }
        return;
    }
}

void SingleLayout::layout(Ottava* item, const Context& ctx)
{
    layoutLine(item, ctx);
}

void SingleLayout::layout(OttavaSegment* item, const Context& ctx)
{
    layoutTextLineBaseSegment(item, ctx);
    item->setOffset(PointF());
}

void SingleLayout::layout(PalmMute* item, const Context& ctx)
{
    layoutLine(item, ctx);
}

void SingleLayout::layout(PalmMuteSegment* item, const Context& ctx)
{
    layoutTextLineBaseSegment(item, ctx);
}

void SingleLayout::layout(Pedal* item, const Context& ctx)
{
    layoutLine(item, ctx);
}

void SingleLayout::layout(PedalSegment* item, const Context& ctx)
{
    layoutTextLineBaseSegment(item, ctx);
    item->setOffset(PointF());
}

void SingleLayout::layout(PlayTechAnnotation* item, const Context& ctx)
{
    layoutTextBase(item, ctx, item->mutldata());
}

void SingleLayout::layout(RehearsalMark* item, const Context& ctx)
{
    layoutTextBase(item, ctx, item->mutldata());
}

void SingleLayout::layout(Slur* item, const Context& ctx)
{
    double spatium = item->spatium();
    SlurSegment* s = nullptr;
    if (item->spannerSegments().empty()) {
        s = new SlurSegment(ctx.dummyParent()->system());
        s->setTrack(item->track());
        item->add(s);
    } else {
        s = item->frontSegment();
    }

    s->setSpannerSegmentType(SpannerSegmentType::SINGLE);

    s->setPos(PointF());
    s->ups(Grip::START).p = PointF(0, 0);
    s->ups(Grip::END).p   = PointF(spatium * 6, 0);
    s->setExtraHeight(0.0);

    SlurTieLayout::computeBezier(s);

    item->setbbox(s->ldata()->bbox());
}

void SingleLayout::layout(Spacer* item, const Context&)
{
    UNUSED(item);
}

void SingleLayout::layout(StaffText* item, const Context& ctx)
{
    layoutTextBase(item, ctx, item->mutldata());

    if (item->hasSoundFlag()) {
        RectF bbox = item->ldata()->bbox();
        double iconHeight = bbox.height();
        RectF iconBBox = RectF(bbox.x(), bbox.y(), iconHeight, iconHeight);
        item->soundFlag()->mutldata()->setBbox(iconBBox);

        double xMove = iconBBox.width() + iconBBox.width() / 2.0;
        bbox.setWidth(bbox.width() + xMove);
        item->setbbox(bbox);

        layout(item->soundFlag(), ctx);

        for (TextBlock& block : item->mutldata()->blocks) {
            auto& fragments = block.fragments();
            for (std::list<TextFragment>::iterator it = fragments.begin(); it != fragments.end(); ++it) {
                it->pos.setX(it->pos.x() + xMove);
            }
        }
    }
}

void SingleLayout::layout(StaffTypeChange* item, const Context& ctx)
{
    double spatium = ctx.style().spatium();
    item->setbbox(RectF(-item->lw() * .5, -item->lw() * .5, spatium * 2.5 + item->lw(), spatium * 2.5 + item->lw()));
    item->setPos(0.0, 0.0);
}

void SingleLayout::layout(StringTunings* item, const Context& ctx)
{
    layoutTextBase(item, ctx, item->mutldata());

    for (TextBlock& block : item->mutldata()->blocks) {
        for (TextFragment& fragment : block.fragments()) {
            Font font = fragment.font(item);
            if (font.type() != Font::Type::MusicSymbol) {
                // HACK: the music symbol doesn't have a good baseline
                // to go with text so we correct text here
                const double baselineAdjustment = font.pointSizeF();
                fragment.pos.setY(fragment.pos.y() - baselineAdjustment);
            }
        }
    }
}

void SingleLayout::layout(Symbol* item, const Context&)
{
    item->setbbox(item->scoreFont() ? item->scoreFont()->bbox(item->sym(), item->magS()) : item->symBbox(item->sym()));
    item->setOffset(0.0, 0.0);
    item->setPos(0.0, 0.0);
}

void SingleLayout::layout(SystemText* item, const Context& ctx)
{
    layoutTextBase(item, ctx, item->mutldata());
}

void SingleLayout::layout(SoundFlag* item, const Context& ctx)
{
    UNUSED(item);
    UNUSED(ctx);
}

void SingleLayout::layout(Stem* item, const Context& ctx)
{
    LayoutContext tctx(ctx.dontUseScore());
    TLayout::layoutStem(item, item->mutldata(), tctx.conf());
}

void SingleLayout::layout(Sticking* item, const Context& ctx)
{
    layoutTextBase(static_cast<TextBase*>(item), ctx, item->mutldata());
}

void SingleLayout::layout(TempoText* item, const Context& ctx)
{
    layoutTextBase(item, ctx, item->mutldata());
}

void SingleLayout::layout(TextLine* item, const Context& ctx)
{
    layoutLine(item, ctx);
}

void SingleLayout::layout(TextLineSegment* item, const Context& ctx)
{
    layoutTextLineBaseSegment(item, ctx);
    item->setOffset(PointF());
}

void SingleLayout::layout(TimeSig* item, const Context& ctx)
{
    TimeSig::LayoutData* ldata = item->mutldata();

    double spatium = item->spatium();

    ldata->setPos(0.0, 0.0);
    ldata->setBbox(RectF());                    // prepare for an empty time signature

    constexpr double lineDist = 1.0;
    constexpr int numOfLines = 5;
    TimeSigType sigType = item->timeSigType();

    // if some symbol
    // compute vert. displacement to center in the staff height
    // determine middle staff position:

    double yoff = spatium * (numOfLines - 1) * 0.5 * lineDist;

    // C and Ccut are placed at the middle of the staff: use yoff directly
    IEngravingFontPtr font = ctx.engravingFont();
    SizeF mag(item->magS() * item->scale());

    if (sigType == TimeSigType::FOUR_FOUR) {
        ldata->pz = PointF(0.0, yoff);
        RectF bbox = font->bbox(SymId::timeSigCommon, mag);
        ldata->setBbox(bbox.translated(ldata->pz));
        ldata->ns.clear();
        ldata->ns.push_back(SymId::timeSigCommon);
        ldata->ds.clear();
    } else if (sigType == TimeSigType::ALLA_BREVE) {
        ldata->pz = PointF(0.0, yoff);
        RectF bbox = font->bbox(SymId::timeSigCutCommon, mag);
        ldata->setBbox(bbox.translated(ldata->pz));
        ldata->ns.clear();
        ldata->ns.push_back(SymId::timeSigCutCommon);
        ldata->ds.clear();
    } else if (sigType == TimeSigType::CUT_BACH) {
        ldata->pz = PointF(0.0, yoff);
        RectF bbox = font->bbox(SymId::timeSigCut2, mag);
        ldata->setBbox(bbox.translated(ldata->pz));
        ldata->ns.clear();
        ldata->ns.push_back(SymId::timeSigCut2);
        ldata->ds.clear();
    } else if (sigType == TimeSigType::CUT_TRIPLE) {
        ldata->pz = PointF(0.0, yoff);
        RectF bbox = font->bbox(SymId::timeSigCut3, mag);
        ldata->setBbox(bbox.translated(ldata->pz));
        ldata->ns.clear();
        ldata->ns.push_back(SymId::timeSigCut3);
        ldata->ds.clear();
    } else {
        if (item->numeratorString().isEmpty()) {
            ldata->ns = timeSigSymIdsFromString(item->numeratorString().isEmpty()
                                                ? String::number(item->sig().numerator())
                                                : item->numeratorString());

            ldata->ds = timeSigSymIdsFromString(item->denominatorString().isEmpty()
                                                ? String::number(item->sig().denominator())
                                                : item->denominatorString());
        } else {
            ldata->ns = timeSigSymIdsFromString(item->numeratorString());
            ldata->ds = timeSigSymIdsFromString(item->denominatorString());
        }

        RectF numRect = font->bbox(ldata->ns, mag);
        RectF denRect = font->bbox(ldata->ds, mag);

        // position numerator and denominator; vertical displacement:
        // number of lines is odd: 0.0 (strings are directly above and below the middle line)
        // number of lines even:   0.05 (strings are moved up/down to leave 1/10sp between them)

        double displ = (numOfLines & 1) ? 0.0 : (0.05 * spatium);

        //align on the wider
        double pzY = yoff - (denRect.width() < 0.01 ? 0.0 : (displ + numRect.height() * 0.5));
        double pnY = yoff + displ + denRect.height() * 0.5;

        if (numRect.width() >= denRect.width()) {
            // numerator: one space above centre line, unless denomin. is empty (if so, directly centre in the middle)
            ldata->pz = PointF(0.0, pzY);
            // denominator: horiz: centred around centre of numerator | vert: one space below centre line
            ldata->pn = PointF((numRect.width() - denRect.width()) * 0.5, pnY);
        } else {
            // numerator: one space above centre line, unless denomin. is empty (if so, directly centre in the middle)
            ldata->pz = PointF((denRect.width() - numRect.width()) * 0.5, pzY);
            // denominator: horiz: centred around centre of numerator | vert: one space below centre line
            ldata->pn = PointF(0.0, pnY);
        }

        // centering of parenthesis so the middle of the parenthesis is at the divisor marking level
        int centerY = yoff / 2 + spatium;
        int widestPortion = numRect.width() > denRect.width() ? numRect.width() : denRect.width();
        ldata->pointLargeLeftParen = PointF(-spatium, centerY);
        ldata->pointLargeRightParen = PointF(widestPortion + spatium, centerY);

        ldata->setBbox(numRect.translated(ldata->pz));       // translate bounding boxes to actual string positions
        ldata->addBbox(denRect.translated(ldata->pn));
        if (item->largeParentheses()) {
            ldata->addBbox(RectF(ldata->pointLargeLeftParen.x(), ldata->pointLargeLeftParen.y() - denRect.height(), spatium / 2,
                                 numRect.height() + denRect.height()));
            ldata->addBbox(RectF(ldata->pointLargeRightParen.x(), ldata->pointLargeRightParen.y() - denRect.height(),  spatium / 2,
                                 numRect.height() + denRect.height()));
        }
    }
}

void SingleLayout::layout(TremoloSingleChord* item, const Context&)
{
    item->computeShape();      // set bbox
    item->setPath(item->basePath());
}

void SingleLayout::layout(TremoloTwoChord* item, const Context&)
{
    item->computeShape();      // set bbox
    item->setPath(item->basePath());
}

void SingleLayout::layout(TremoloBar* item, const Context&)
{
    TremoloBar::LayoutData* ldata = item->mutldata();
    double spatium = item->spatium();

    ldata->setPos(PointF());

    double timeFactor  = item->userMag() / 1.0;
    double pitchFactor = -spatium * 0.02;

    PolygonF polygon;
    for (const PitchValue& v : item->points()) {
        polygon << PointF(v.time * timeFactor, v.pitch * pitchFactor);
    }
    ldata->polygon = polygon;

    const double w = item->absoluteFromSpatium(item->lineWidth());
    ldata->setBbox(ldata->polygon.boundingRect().adjusted(-w, -w, w, w));
}

void SingleLayout::layout(Trill* item, const Context& ctx)
{
    layoutLine(static_cast<SLine*>(item), ctx);
}

void SingleLayout::layout(TrillSegment* item, const Context& ctx)
{
    TrillSegment::LayoutData* ldata = item->mutldata();
    if (item->spanner()->placeBelow()) {
        ldata->setPosY(0.0);
    }

    bool accidentalGoesBelow = item->trill()->trillType() == TrillType::DOWNPRALL_LINE;
    Trill* trill = item->trill();
    Ornament* ornament = trill->ornament();
    if (ornament) {
        if (item->isSingleBeginType()) {
            layout(ornament, ctx);
        }
        trill->setAccidental(accidentalGoesBelow ? ornament->accidentalBelow() : ornament->accidentalAbove());
        trill->setCueNoteChord(ornament->cueNoteChord());
        ArticulationAnchor anchor = ornament->anchor();
        if (anchor == ArticulationAnchor::AUTO) {
            trill->setPlacement(trill->track() % 2 ? PlacementV::BELOW : PlacementV::ABOVE);
        } else {
            trill->setPlacement(anchor == ArticulationAnchor::TOP ? PlacementV::ABOVE : PlacementV::BELOW);
        }
        trill->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::STYLED); // Ensures that the property isn't written (it is written by the ornamnent)
    }

    if (item->isSingleType() || item->isBeginType()) {
        switch (item->trill()->trillType()) {
        case TrillType::TRILL_LINE:
            item->symbolLine(SymId::ornamentTrill, SymId::wiggleTrill);
            break;
        case TrillType::PRALLPRALL_LINE:
            item->symbolLine(SymId::wiggleTrill, SymId::wiggleTrill);
            break;
        case TrillType::UPPRALL_LINE:
            item->symbolLine(SymId::ornamentBottomLeftConcaveStroke,
                             SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
            break;
        case TrillType::DOWNPRALL_LINE:
            item->symbolLine(SymId::ornamentLeftVerticalStroke,
                             SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
            break;
        }
        Accidental* a = item->trill()->accidental();
        if (a) {
            double vertMargin = 0.35 * item->spatium();
            RectF box = item->symBbox(item->symbols().front());
            double x = 0;
            double y = 0;
            x = 0.5 * (box.width() - a->width());
            double minVertDist = accidentalGoesBelow
                                 ? Shape(box).minVerticalDistance(a->shape())
                                 : a->shape().minVerticalDistance(Shape(box));
            y = accidentalGoesBelow ? minVertDist + vertMargin : -minVertDist - vertMargin;
            a->setPos(x, y);
            a->setParent(item);
        }
    } else {
        item->symbolLine(SymId::wiggleTrill, SymId::wiggleTrill);
    }

    item->setOffset(PointF());
}

void SingleLayout::layout(Vibrato* item, const Context& ctx)
{
    layoutLine(static_cast<SLine*>(item), ctx);
}

void SingleLayout::layout(VibratoSegment* item, const Context&)
{
    VibratoSegment::LayoutData* ldata = item->mutldata();
    if (item->spanner()->placeBelow()) {
        ldata->setPosY(0.0);
    }

    switch (item->vibrato()->vibratoType()) {
    case VibratoType::GUITAR_VIBRATO:
        item->symbolLine(SymId::guitarVibratoStroke, SymId::guitarVibratoStroke);
        break;
    case VibratoType::GUITAR_VIBRATO_WIDE:
        item->symbolLine(SymId::guitarWideVibratoStroke, SymId::guitarWideVibratoStroke);
        break;
    case VibratoType::VIBRATO_SAWTOOTH:
        item->symbolLine(SymId::wiggleSawtooth, SymId::wiggleSawtooth);
        break;
    case VibratoType::VIBRATO_SAWTOOTH_WIDE:
        item->symbolLine(SymId::wiggleSawtoothWide, SymId::wiggleSawtoothWide);
        break;
    }

    item->setOffset(PointF());
}

void SingleLayout::layout(Volta* item, const Context& ctx)
{
    layoutLine(item, ctx);
}

void SingleLayout::layout(VoltaSegment* item, const Context& ctx)
{
    layoutTextLineBaseSegment(item, ctx);
    item->setOffset(PointF());
    item->text()->setOffset(PointF(10.0, 54.0)); //! TODO
}

void SingleLayout::layout(Text* item, const Context& ctx)
{
    layoutTextBase(static_cast<TextBase*>(item), ctx, item->mutldata());
}

void SingleLayout::layoutTextBase(const TextBase* item, const Context& ctx, TextBase::LayoutData* ldata)
{
    ldata->setPos(PointF());
    const_cast<TextBase*>(item)->setOffset(PointF());

    if (item->placeBelow()) {
        ldata->setPosY(0.0);
    }

    layout1TextBase(item, ctx, ldata);
}

void SingleLayout::layout1TextBase(const TextBase* item, const Context&, TextBase::LayoutData* ldata)
{
    if (ldata->layoutInvalid) {
        item->createBlocks(ldata);
    }

    RectF bb;
    double y = 0;

    // adjust the bounding box for the text item
    for (size_t i = 0; i < ldata->rows(); ++i) {
        TextBlock& t = ldata->blocks[i];
        t.layout(item);
        const RectF* r = &t.boundingRect();

        if (r->height() == 0) {
            r = &ldata->blocks.at(0).boundingRect();
        }
        y += t.lineSpacing();
        t.setY(y);
        bb |= r->translated(0.0, y);
    }
    double yoff = 0;
    double h    = 0;

    ldata->setPos(PointF());

    if (item->align() == AlignV::BOTTOM) {
        yoff += h - bb.bottom();
    } else if (item->align() == AlignV::VCENTER) {
        yoff +=  (h - (bb.top() + bb.bottom())) * .5;
    } else if (item->align() == AlignV::BASELINE) {
        yoff += h * .5 - ldata->blocks.front().lineSpacing();
    } else {
        yoff += -bb.top();
    }

    for (TextBlock& t : ldata->blocks) {
        t.setY(t.y() + yoff);
    }

    bb.translate(0.0, yoff);

    ldata->setBbox(bb);
    if (item->hasFrame()) {
        item->layoutFrame(ldata);
    }
}

void SingleLayout::layoutLine(SLine* item, const Context& ctx)
{
    if (item->spannerSegments().empty()) {
        item->setLen(ctx.style().spatium() * 7);
    }

    LineSegment* lineSegm = item->frontSegment();
    layoutLineSegment(lineSegm, ctx);

    item->setbbox(lineSegm->ldata()->bbox(LD_ACCESS::BAD));
}

void SingleLayout::layoutTextLineBaseSegment(TextLineBaseSegment* item, const Context& ctx)
{
    TextLineBaseSegment::LayoutData* ldata = item->mutldata();
    item->npointsRef() = 0;
    TextLineBase* tl = item->textLineBase();
    double spatium = tl->spatium();

    if (item->spanner()->placeBelow()) {
        ldata->setPosY(0.0);
    }

    if (!tl->diagonal()) {
        item->setUserYoffset2(0);
    }

    auto alignBaseLine = [tl](Text* text, PointF& pp1, PointF& pp2) {
        PointF widthCorrection(0.0, tl->absoluteFromSpatium(tl->lineWidth()) / 2);
        switch (text->align().vertical) {
        case AlignV::TOP:
            pp1 += widthCorrection;
            pp2 += widthCorrection;
            break;
        case AlignV::VCENTER:
            break;
        case AlignV::BOTTOM:
            pp1 -= widthCorrection;
            pp2 -= widthCorrection;
            break;
        case AlignV::BASELINE:
            pp1 -= widthCorrection;
            pp2 -= widthCorrection;
            break;
        }
    };

    switch (item->spannerSegmentType()) {
    case SpannerSegmentType::SINGLE:
    case SpannerSegmentType::BEGIN:
        item->text()->setXmlText(tl->beginText());
        item->text()->setFamily(tl->beginFontFamily());
        item->text()->setSize(tl->beginFontSize());
        item->text()->setOffset(tl->beginTextOffset() * item->mag());
        item->text()->setAlign(tl->beginTextAlign());
        item->text()->setFontStyle(tl->beginFontStyle());
        break;
    case SpannerSegmentType::MIDDLE:
    case SpannerSegmentType::END:
        item->text()->setXmlText(tl->continueText());
        item->text()->setFamily(tl->continueFontFamily());
        item->text()->setSize(tl->continueFontSize());
        item->text()->setOffset(tl->continueTextOffset() * item->mag());
        item->text()->setAlign(tl->continueTextAlign());
        item->text()->setFontStyle(tl->continueFontStyle());
        break;
    }
    item->text()->setPlacement(PlacementV::ABOVE);

    layout(item->text(), ctx);

    if ((item->isSingleType() || item->isEndType())) {
        item->endText()->setXmlText(tl->endText());
        item->endText()->setFamily(tl->endFontFamily());
        item->endText()->setSize(tl->endFontSize());
        item->endText()->setOffset(tl->endTextOffset());
        item->endText()->setAlign(tl->endTextAlign());
        item->endText()->setFontStyle(tl->endFontStyle());
        item->endText()->setPlacement(PlacementV::ABOVE);
        item->endText()->setTrack(item->track());
        layout(item->endText(), ctx);
    } else {
        item->endText()->setXmlText(u"");
    }

    if (!item->textLineBase()->textSizeSpatiumDependent()) {
        item->text()->setSize(item->text()->size() * SPATIUM20 / item->spatium());
        item->endText()->setSize(item->endText()->size() * SPATIUM20 / item->spatium());
    }

    PointF pp1;
    PointF pp2(item->pos2());

    // line with no text or hooks - just use the basic rectangle for line
    if (item->text()->empty() && item->endText()->empty()
        && (!item->isSingleBeginType() || tl->beginHookType() == HookType::NONE)
        && (!item->isSingleEndType() || tl->endHookType() == HookType::NONE)) {
        item->npointsRef() = 2;
        item->pointsRef()[0] = pp1;
        item->pointsRef()[1] = pp2;
        item->setLineLength(sqrt(PointF::dotProduct(pp2 - pp1, pp2 - pp1)));

        item->setbbox(TextLineBaseSegment::boundingBoxOfLine(pp1, pp2, tl->absoluteFromSpatium(tl->lineWidth()) / 2,
                                                             tl->lineStyle() == LineType::DOTTED));
        return;
    }

    // line has text or hooks or is not diagonal - calculate reasonable bbox

    double x1 = std::min(0.0, pp2.x());
    double x2 = std::max(0.0, pp2.x());
    double y0 = -tl->absoluteFromSpatium(tl->lineWidth());
    double y1 = std::min(0.0, pp2.y()) + y0;
    double y2 = std::max(0.0, pp2.y()) - y0;

    double l = 0.0;
    if (!item->text()->empty()) {
        double gapBetweenTextAndLine = spatium * tl->gapBetweenTextAndLine().val();
        if ((item->isSingleBeginType() && (tl->beginTextPlace() == TextPlace::LEFT || tl->beginTextPlace() == TextPlace::AUTO))
            || (!item->isSingleBeginType() && (tl->continueTextPlace() == TextPlace::LEFT || tl->continueTextPlace() == TextPlace::AUTO))) {
            l = item->text()->pos().x() + item->text()->ldata()->bbox().width() + gapBetweenTextAndLine;
        }

        double h = item->text()->height();
        if (tl->beginTextPlace() == TextPlace::ABOVE) {
            y1 = std::min(y1, -h);
        } else if (tl->beginTextPlace() == TextPlace::BELOW) {
            y2 = std::max(y2, h);
        } else {
            y1 = std::min(y1, -h * .5);
            y2 = std::max(y2, h * .5);
        }
        x2 = std::max(x2, item->text()->width());
    }

    if (tl->endHookType() != HookType::NONE) {
        double h = pp2.y() + tl->endHookHeight().val() * spatium;
        if (h > y2) {
            y2 = h;
        } else if (h < y1) {
            y1 = h;
        }
    }

    if (tl->beginHookType() != HookType::NONE) {
        double h = tl->beginHookHeight().val() * spatium;
        if (h > y2) {
            y2 = h;
        } else if (h < y1) {
            y1 = h;
        }
    }
    ldata->setBbox(x1, y1, x2 - x1, y2 - y1);
    if (!item->text()->empty()) {
        ldata->addBbox(item->text()->ldata()->bbox().translated(item->text()->pos()));      // DEBUG
    }
    // set end text position and extend bbox
    if (!item->endText()->empty()) {
        item->endText()->mutldata()->moveX(ldata->bbox().right());
        ldata->addBbox(item->endText()->ldata()->bbox().translated(item->endText()->pos()));
    }

    if (tl->lineVisible()) {
        // Extends lines to fill the corner between them.
        // Assumes that l1p2 == l2p1 is the intersection between the lines.
        // If checkAngle is false, assumes that the lines are perpendicular,
        // and some calculations are saved.
        auto extendLines = [](const PointF& l1p1, PointF& l1p2, PointF& l2p1, const PointF& l2p2, double lineWidth, bool checkAngle)
        {
            PointF l1UnitVector = (l1p2 - l1p1).normalized();
            PointF l2UnitVector = (l2p1 - l2p2).normalized();

            double addedLength = lineWidth * 0.5;

            if (checkAngle) {
                double angle = M_PI - acos(PointF::dotProduct(l1UnitVector, l2UnitVector));

                if (angle <= M_PI_2) {
                    addedLength *= tan(0.5 * angle);
                }
            }

            l1p2 += l1UnitVector * addedLength;
            l2p1 += l2UnitVector * addedLength;
        };

        pp1 = PointF(l, 0.0);

        // Make sure baseline of text and line are properly aligned (accounting for line thickness)
        bool alignBeginText = tl->beginTextPlace() == TextPlace::LEFT || tl->beginTextPlace() == TextPlace::AUTO;
        bool alignContinueText = tl->continueTextPlace() == TextPlace::LEFT || tl->continueTextPlace() == TextPlace::AUTO;
        bool alignEndText = tl->endTextPlace() == TextPlace::LEFT || tl->endTextPlace() == TextPlace::AUTO;
        bool isSingleOrBegin = item->isSingleBeginType();
        bool hasBeginText = !item->text()->empty() && isSingleOrBegin;
        bool hasContinueText = !item->text()->empty() && !isSingleOrBegin;
        bool hasEndText = !item->endText()->empty() && item->isSingleEndType();
        if ((hasBeginText && alignBeginText) || (hasContinueText && alignContinueText)) {
            alignBaseLine(item->text(), pp1, pp2);
        } else if (hasEndText && alignEndText) {
            alignBaseLine(item->endText(), pp1, pp2);
        }

        double beginHookHeight = tl->beginHookHeight().val() * spatium;
        double endHookHeight = tl->endHookHeight().val() * spatium;
        double beginHookWidth = 0.0;
        double endHookWidth = 0.0;

        if (tl->beginHookType() == HookType::HOOK_45) {
            beginHookWidth = std::fabs(beginHookHeight * .4);
            pp1.rx() += beginHookWidth;
        }

        if (tl->endHookType() == HookType::HOOK_45) {
            endHookWidth = std::fabs(endHookHeight * .4);
            pp2.rx() -= endHookWidth;
        }

        // don't draw backwards lines (or hooks) if text is longer than nominal line length
        if (!item->text()->empty() && pp1.x() > pp2.x() && !tl->diagonal()) {
            return;
        }

        if (item->isSingleBeginType() && tl->beginHookType() != HookType::NONE) {
            // We use the term "endpoint" for the point that does not touch the main line.
            const PointF& beginHookEndpoint = item->pointsRef()[item->npointsRef()++]
                                                  = PointF(pp1.x() - beginHookWidth, pp1.y() + beginHookHeight);

            if (tl->beginHookType() == HookType::HOOK_90T) {
                // A T-hook needs to be drawn separately, so we add an extra point
                item->pointsRef()[item->npointsRef()++] = PointF(pp1.x() - beginHookWidth, pp1.y() - beginHookHeight);
            } else if (tl->lineStyle() != LineType::SOLID) {
                // For non-solid lines, we also draw the hook separately,
                // so that we can distribute the dashes/dots for each linepiece individually
                PointF& beginHookStartpoint = item->pointsRef()[item->npointsRef()++] = pp1;

                if (tl->lineStyle() == LineType::DASHED) {
                    // For dashes lines, we extend the lines somewhat,
                    // so that the corner between them gets filled
                    bool checkAngle = tl->beginHookType() == HookType::HOOK_45 || tl->diagonal();
                    extendLines(beginHookEndpoint, beginHookStartpoint, pp1, pp2, tl->absoluteFromSpatium(tl->lineWidth()), checkAngle);
                }
            }
        }

        item->pointsRef()[item->npointsRef()++] = pp1;
        PointF& pp22 = item->pointsRef()[item->npointsRef()++] = pp2; // Keep a reference so that we can modify later

        if (item->isSingleEndType() && tl->endHookType() != HookType::NONE) {
            const PointF endHookEndpoint = PointF(pp2.x() + endHookWidth, pp2.y() + endHookHeight);

            if (tl->endHookType() == HookType::HOOK_90T) {
                // A T-hook needs to be drawn separately, so we add an extra point
                item->pointsRef()[item->npointsRef()++] = PointF(pp2.x() + endHookWidth, pp2.y() - endHookHeight);
            } else if (tl->lineStyle() != LineType::SOLID) {
                // For non-solid lines, we also draw the hook separately,
                // so that we can distribute the dashes/dots for each linepiece individually
                PointF& endHookStartpoint = item->pointsRef()[item->npointsRef()++] = pp2;

                if (tl->lineStyle() == LineType::DASHED) {
                    bool checkAngle = tl->endHookType() == HookType::HOOK_45 || tl->diagonal();

                    // For dashes lines, we extend the lines somewhat,
                    // so that the corner between them gets filled
                    extendLines(pp1, pp22, endHookStartpoint, endHookEndpoint, tl->absoluteFromSpatium(tl->lineWidth()), checkAngle);
                }
            }

            item->pointsRef()[item->npointsRef()++] = endHookEndpoint;
        }

        item->setLineLength(sqrt(PointF::dotProduct(pp22 - pp1, pp22 - pp1)));
    }
}
