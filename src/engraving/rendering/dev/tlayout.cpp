/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#include "tlayout.h"

#include "global/realfn.h"
#include "draw/fontmetrics.h"

#include "iengravingconfiguration.h"
#include "infrastructure/rtti.h"
#include "infrastructure/ld_access.h"

#include "iengravingfont.h"
#include "types/typesconv.h"
#include "types/symnames.h"
#include "dom/score.h"
#include "dom/utils.h"

#include "dom/accidental.h"
#include "dom/ambitus.h"
#include "dom/arpeggio.h"
#include "dom/articulation.h"

#include "dom/barline.h"
#include "dom/beam.h"
#include "dom/bend.h"
#include "dom/box.h"
#include "dom/bracket.h"
#include "dom/breath.h"

#include "dom/chord.h"
#include "dom/chordline.h"
#include "dom/clef.h"
#include "dom/capo.h"

#include "dom/deadslapped.h"
#include "dom/dynamic.h"

#include "dom/expression.h"

#include "dom/fermata.h"
#include "dom/figuredbass.h"
#include "dom/fingering.h"
#include "dom/fret.h"
#include "dom/fretcircle.h"

#include "dom/glissando.h"
#include "dom/gradualtempochange.h"

#include "dom/hairpin.h"
#include "dom/harppedaldiagram.h"
#include "dom/harmonicmark.h"
#include "dom/harmony.h"
#include "dom/hook.h"

#include "dom/image.h"
#include "dom/instrchange.h"
#include "dom/instrumentname.h"

#include "dom/jump.h"

#include "dom/keysig.h"

#include "dom/layoutbreak.h"
#include "dom/ledgerline.h"
#include "dom/letring.h"
#include "dom/line.h"
#include "dom/lyrics.h"

#include "dom/marker.h"
#include "dom/measurebase.h"
#include "dom/measurenumber.h"
#include "dom/measurenumberbase.h"
#include "dom/measurerepeat.h"
#include "dom/mmrest.h"
#include "dom/mmrestrange.h"

#include "dom/note.h"
#include "dom/notedot.h"

#include "dom/ornament.h"
#include "dom/ottava.h"

#include "dom/page.h"
#include "dom/palmmute.h"
#include "dom/part.h"
#include "dom/pedal.h"
#include "dom/pickscrape.h"
#include "dom/playtechannotation.h"

#include "dom/rasgueado.h"
#include "dom/rehearsalmark.h"
#include "dom/rest.h"

#include "dom/shadownote.h"
#include "dom/slur.h"
#include "dom/spacer.h"
#include "dom/staff.h"
#include "dom/stafflines.h"
#include "dom/staffstate.h"
#include "dom/stafftext.h"
#include "dom/stafftype.h"
#include "dom/stafftypechange.h"
#include "dom/stem.h"
#include "dom/stemslash.h"
#include "dom/sticking.h"
#include "dom/stretchedbend.h"
#include "dom/bsymbol.h"
#include "dom/symbol.h"
#include "dom/system.h"
#include "dom/systemdivider.h"
#include "dom/systemtext.h"

#include "dom/tempotext.h"
#include "dom/text.h"
#include "dom/textframe.h"
#include "dom/textline.h"
#include "dom/tie.h"
#include "dom/timesig.h"
#include "dom/tremolo.h"
#include "dom/tremolobar.h"
#include "dom/trill.h"
#include "dom/tripletfeel.h"
#include "dom/tuplet.h"

#include "dom/vibrato.h"
#include "dom/volta.h"

#include "dom/whammybar.h"

#include "autoplace.h"
#include "beamlayout.h"
#include "chordlayout.h"
#include "lyricslayout.h"
#include "slurtielayout.h"
#include "tremololayout.h"
#include "tupletlayout.h"

using namespace mu::draw;
using namespace mu::engraving;
using namespace mu::engraving::rtti;
using namespace mu::engraving::rendering::dev;

void TLayout::layoutItem(EngravingItem* item, LayoutContext& ctx)
{
    //DO_ASSERT(!ctx.conf().isPaletteMode());

    EngravingItem::LayoutData* ldata = item->mutLayoutData();

    switch (item->type()) {
    case ElementType::ACCIDENTAL:
        if (!ldata->isValid()) {
            layout(item_cast<const Accidental*>(item), static_cast<Accidental::LayoutData*>(ldata), ctx.conf());
        }
        break;
    case ElementType::ACTION_ICON:
        if (!ldata->isValid()) {
            layout(item_cast<const ActionIcon*>(item), static_cast<ActionIcon::LayoutData*>(ldata));
        }
        break;
    case ElementType::AMBITUS:
        if (!ldata->isValid()) {
            layout(item_cast<const Ambitus*>(item), static_cast<Ambitus::LayoutData*>(ldata), ctx);
        }
        break;
    case ElementType::ARPEGGIO:
        //! NOTE Can be edited and relayout,
        //! in this case the reset layout data has not yet been done
        //if (!ldata->isValid()) {
        layout(item_cast<const Arpeggio*>(item), static_cast<Arpeggio::LayoutData*>(ldata), ctx.conf());
        //}
        break;
    case ElementType::ARTICULATION:
        if (!ldata->isValid()) {
            layout(item_cast<const Articulation*>(item), static_cast<Articulation::LayoutData*>(ldata));
        }
        break;
    case ElementType::BAR_LINE:
        layout(item_cast<const BarLine*>(item), static_cast<BarLine::LayoutData*>(ldata), ctx);
        break;
    case ElementType::BEAM:             layout(item_cast<Beam*>(item), ctx);
        break;
    case ElementType::BEND:
        layout(item_cast<const Bend*>(item), static_cast<Bend::LayoutData*>(ldata));
        break;
    case ElementType::STRETCHED_BEND:   layout(item_cast<StretchedBend*>(item), ctx);
        break;
    case ElementType::HBOX:
        layout(item_cast<const HBox*>(item), static_cast<HBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::VBOX:
        layout(item_cast<const VBox*>(item), static_cast<VBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::FBOX:
        layout(item_cast<const FBox*>(item), static_cast<FBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::TBOX:
        layout(item_cast<const TBox*>(item), static_cast<TBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::BRACKET:          layout(item_cast<Bracket*>(item), ctx);
        break;
    case ElementType::BREATH:           layout(item_cast<Breath*>(item), ctx);
        break;
    case ElementType::CHORD:            layout(item_cast<Chord*>(item), ctx);
        break;
    case ElementType::CHORDLINE:        layout(item_cast<ChordLine*>(item), ctx);
        break;
    case ElementType::CLEF:             layout(item_cast<Clef*>(item), ctx);
        break;
    case ElementType::CAPO:             layout(item_cast<Capo*>(item), ctx);
        break;
    case ElementType::DEAD_SLAPPED:     layout(item_cast<DeadSlapped*>(item), ctx);
        break;
    case ElementType::DYNAMIC:          layout(item_cast<Dynamic*>(item), ctx);
        break;
    case ElementType::EXPRESSION:       layout(item_cast<Expression*>(item), ctx);
        break;
    case ElementType::FERMATA:          layout(item_cast<Fermata*>(item), ctx);
        break;
    case ElementType::FIGURED_BASS:     layout(item_cast<FiguredBass*>(item), ctx);
        break;
    case ElementType::FINGERING:        layout(item_cast<Fingering*>(item), ctx);
        break;
    case ElementType::FRET_DIAGRAM:     layout(item_cast<FretDiagram*>(item), ctx);
        break;
    case ElementType::GLISSANDO:        layout(item_cast<Glissando*>(item), ctx);
        break;
    case ElementType::GLISSANDO_SEGMENT: layout(item_cast<GlissandoSegment*>(item), ctx);
        break;
    case ElementType::GRADUAL_TEMPO_CHANGE: layout(item_cast<GradualTempoChange*>(item), ctx);
        break;
    case ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT: layout(item_cast<GradualTempoChangeSegment*>(item), ctx);
        break;
    case ElementType::HAIRPIN:          layout(item_cast<Hairpin*>(item), ctx);
        break;
    case ElementType::HAIRPIN_SEGMENT:  layout(item_cast<HairpinSegment*>(item), ctx);
        break;
    case ElementType::HARP_DIAGRAM:     layout(item_cast<HarpPedalDiagram*>(item), ctx);
        break;
    case ElementType::HARMONY:          layout(item_cast<Harmony*>(item), ctx);
        break;
    case ElementType::HARMONIC_MARK_SEGMENT: layout(item_cast<HarmonicMarkSegment*>(item), ctx);
        break;
    case ElementType::HOOK:             layout(item_cast<Hook*>(item), ctx);
        break;
    case ElementType::IMAGE:            layout(item_cast<Image*>(item), ctx);
        break;
    case ElementType::INSTRUMENT_CHANGE: layout(item_cast<InstrumentChange*>(item), ctx);
        break;
    case ElementType::JUMP:             layout(item_cast<Jump*>(item), ctx);
        break;
    case ElementType::KEYSIG:           layout(item_cast<KeySig*>(item), ctx);
        break;
    case ElementType::LAYOUT_BREAK:     layout(item_cast<LayoutBreak*>(item), ctx);
        break;
    case ElementType::LET_RING:         layout(item_cast<LetRing*>(item), ctx);
        break;
    case ElementType::LET_RING_SEGMENT: layout(item_cast<LetRingSegment*>(item), ctx);
        break;
    case ElementType::LEDGER_LINE:      layout(item_cast<LedgerLine*>(item), ctx);
        break;
    case ElementType::LYRICS:           layout(item_cast<Lyrics*>(item), ctx);
        break;
    case ElementType::LYRICSLINE_SEGMENT: layout(item_cast<LyricsLineSegment*>(item), ctx);
        break;
    case ElementType::MARKER:           layout(item_cast<Marker*>(item), ctx);
        break;
    case ElementType::MEASURE_NUMBER:   layout(item_cast<MeasureNumber*>(item), ctx);
        break;
    case ElementType::MEASURE_REPEAT:   layout(item_cast<MeasureRepeat*>(item), ctx);
        break;
    case ElementType::MMREST:           layout(item_cast<MMRest*>(item), ctx);
        break;
    case ElementType::MMREST_RANGE:     layout(item_cast<MMRestRange*>(item), ctx);
        break;
    case ElementType::NOTE:             layout(item_cast<Note*>(item), ctx);
        break;
    case ElementType::NOTEDOT:          layout(item_cast<NoteDot*>(item), ctx);
        break;
    case ElementType::NOTEHEAD:         layout(item_cast<NoteHead*>(item), ctx);
        break;
    case ElementType::ORNAMENT:
        if (!ldata->isValid()) {
            layout(item_cast<const Ornament*>(item), static_cast<Ornament::LayoutData*>(ldata), ctx.conf());
        }
        break;
    case ElementType::OTTAVA:           layout(item_cast<Ottava*>(item), ctx);
        break;
    case ElementType::OTTAVA_SEGMENT:   layout(item_cast<OttavaSegment*>(item), ctx);
        break;
    case ElementType::PALM_MUTE:        layout(item_cast<PalmMute*>(item), ctx);
        break;
    case ElementType::PALM_MUTE_SEGMENT: layout(item_cast<PalmMuteSegment*>(item), ctx);
        break;
    case ElementType::PEDAL:            layout(item_cast<Pedal*>(item), ctx);
        break;
    case ElementType::PEDAL_SEGMENT:    layout(item_cast<PedalSegment*>(item), ctx);
        break;
    case ElementType::PLAYTECH_ANNOTATION: layout(item_cast<PlayTechAnnotation*>(item), ctx);
        break;
    case ElementType::RASGUEADO_SEGMENT: layout(item_cast<RasgueadoSegment*>(item), ctx);
        break;
    case ElementType::REHEARSAL_MARK:   layout(item_cast<RehearsalMark*>(item), ctx);
        break;
    case ElementType::REST:             layout(item_cast<Rest*>(item), ctx);
        break;
    case ElementType::SHADOW_NOTE:      layout(item_cast<ShadowNote*>(item), ctx);
        break;
    case ElementType::SLUR:             layout(item_cast<Slur*>(item), ctx);
        break;
    case ElementType::SPACER:           layout(item_cast<Spacer*>(item), ctx);
        break;
    case ElementType::STAFF_STATE:      layout(item_cast<StaffState*>(item), ctx);
        break;
    case ElementType::STAFF_TEXT:       layout(item_cast<StaffText*>(item), ctx);
        break;
    case ElementType::STAFFTYPE_CHANGE: layout(item_cast<StaffTypeChange*>(item), ctx);
        break;
    case ElementType::STEM:             layout(item_cast<Stem*>(item), ctx);
        break;
    case ElementType::STEM_SLASH:       layout(item_cast<StemSlash*>(item), ctx);
        break;
    case ElementType::STICKING:         layout(item_cast<Sticking*>(item), ctx);
        break;
    case ElementType::SYMBOL:           layout(item_cast<Symbol*>(item), ctx);
        break;
    case ElementType::FSYMBOL:          layout(item_cast<FSymbol*>(item), ctx);
        break;
    case ElementType::SYSTEM_DIVIDER:   layout(item_cast<SystemDivider*>(item), ctx);
        break;
    case ElementType::SYSTEM_TEXT:      layout(item_cast<SystemText*>(item), ctx);
        break;
    case ElementType::TEMPO_TEXT:       layout(item_cast<TempoText*>(item), ctx);
        break;
    case ElementType::TEXT:             layout(item_cast<Text*>(item), ctx);
        break;
    case ElementType::TEXTLINE:         layout(item_cast<TextLine*>(item), ctx);
        break;
    case ElementType::TEXTLINE_SEGMENT: layout(item_cast<TextLineSegment*>(item), ctx);
        break;
    case ElementType::TIE:              layout(item_cast<Tie*>(item), ctx);
        break;
    case ElementType::TIMESIG:          layout(item_cast<TimeSig*>(item), ctx);
        break;
    case ElementType::TREMOLO:          layout(item_cast<Tremolo*>(item), ctx);
        break;
    case ElementType::TREMOLOBAR:       layout(item_cast<TremoloBar*>(item), ctx);
        break;
    case ElementType::TRILL:            layout(item_cast<Trill*>(item), ctx);
        break;
    case ElementType::TRILL_SEGMENT:    layout(item_cast<TrillSegment*>(item), ctx);
        break;
    case ElementType::TRIPLET_FEEL:     layout(item_cast<TripletFeel*>(item), ctx);
        break;
    case ElementType::TUPLET:           layout(item_cast<Tuplet*>(item), ctx);
        break;
    case ElementType::VIBRATO:          layout(item_cast<Vibrato*>(item), ctx);
        break;
    case ElementType::VIBRATO_SEGMENT:  layout(item_cast<VibratoSegment*>(item), ctx);
        break;
    case ElementType::VOLTA:            layout(item_cast<Volta*>(item), ctx);
        break;
    case ElementType::VOLTA_SEGMENT:    layout(item_cast<VoltaSegment*>(item), ctx);
        break;
    case ElementType::WHAMMY_BAR_SEGMENT: layout(item_cast<WhammyBarSegment*>(item), ctx);
        break;
    default:
        LOGE() << "not found in layout types item: " << item->typeName();
        DO_ASSERT(false);
    }
}

void TLayout::layout(const Accidental* item, Accidental::LayoutData* ldata, const LayoutConfiguration& conf)
{
    LD_INDEPENDENT;

    ldata->syms.clear();

    // TODO: remove Accidental in layout
    // don't show accidentals for tab or slash notation
    if (item->onTabStaff() || (item->note() && item->note()->fixed())) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);
    ldata->setPos(PointF());

    if (item->accidentalType() == AccidentalType::NONE) {
        return;
    }

    auto accidentalSingleSym = [](const Accidental* item) -> SymId
    {
        // if the accidental is standard (doubleflat, flat, natural, sharp or double sharp)
        // and it has either no bracket or parentheses, then we have glyphs straight from smufl.

        if (item->bracket() == AccidentalBracket::PARENTHESIS && !item->parentNoteHasParentheses()) {
            switch (item->accidentalType()) {
            case AccidentalType::FLAT:      return SymId::accidentalFlatParens;
            case AccidentalType::FLAT2:     return SymId::accidentalDoubleFlatParens;
            case AccidentalType::NATURAL:   return SymId::accidentalNaturalParens;
            case AccidentalType::SHARP:     return SymId::accidentalSharpParens;
            case AccidentalType::SHARP2:    return SymId::accidentalDoubleSharpParens;
            default:
                break;
            }
        }
        return SymId::noSym;
    };

    auto accidentalBracketSyms = [](AccidentalBracket type) -> std::pair<SymId, SymId>
    {
        switch (type) {
        case AccidentalBracket::PARENTHESIS: return { SymId::accidentalParensLeft, SymId::accidentalParensRight };
        case AccidentalBracket::BRACKET: return { SymId::accidentalBracketLeft, SymId::accidentalBracketRight };
        case AccidentalBracket::BRACE: return { SymId::accidentalCombiningOpenCurlyBrace, SymId::accidentalCombiningCloseCurlyBrace };
        case AccidentalBracket::NONE: return { SymId::noSym, SymId::noSym };
        }
        return { SymId::noSym, SymId::noSym };
    };

    // Single?
    SymId singleSym = accidentalSingleSym(item);
    if (singleSym != SymId::noSym && conf.engravingFont()->isValid(singleSym)) {
        Accidental::LayoutData::Sym s(singleSym, 0.0, 0.0);
        ldata->syms.push_back(s);

        ldata->addBbox(item->symBbox(singleSym));
    }
    // Multi
    else {
        double margin = conf.styleMM(Sid::bracketedAccidentalPadding);
        double x = 0.0;

        std::pair<SymId, SymId> bracketSyms;
        bool isNeedBracket = item->bracket() != AccidentalBracket::NONE && !item->parentNoteHasParentheses();
        if (isNeedBracket) {
            bracketSyms = accidentalBracketSyms(item->bracket());
        }

        // Left
        if (bracketSyms.first != SymId::noSym) {
            Accidental::LayoutData::Sym ls(bracketSyms.first, 0.0,
                                           item->bracket() == AccidentalBracket::BRACE ? item->spatium() * 0.4 : 0.0);
            ldata->syms.push_back(ls);
            ldata->addBbox(item->symBbox(bracketSyms.first));

            x += item->symAdvance(bracketSyms.first) + margin;
        }

        // Main
        SymId mainSym = item->symId();
        Accidental::LayoutData::Sym ms(mainSym, x, 0.0);
        ldata->syms.push_back(ms);
        ldata->addBbox(item->symBbox(mainSym).translated(x, 0.0));

        // Right
        if (bracketSyms.second != SymId::noSym) {
            x += item->symAdvance(mainSym) + margin;

            Accidental::LayoutData::Sym rs(bracketSyms.second, x,
                                           item->bracket() == AccidentalBracket::BRACE ? item->spatium() * 0.4 : 0.0);
            ldata->syms.push_back(rs);
            ldata->addBbox(item->symBbox(bracketSyms.second).translated(x, 0.0));
        }
    }
}

void TLayout::layout(const ActionIcon* item, ActionIcon::LayoutData* ldata)
{
    LD_INDEPENDENT;

    FontMetrics fontMetrics(item->iconFont());
    ldata->setBbox(fontMetrics.boundingRect(Char(item->icon())));
    ldata->setPos(PointF());
}

void TLayout::layout(const Ambitus* item, Ambitus::LayoutData* ldata, const LayoutContext& ctx)
{
    LD_INDEPENDENT;

    const double spatium = item->spatium();
    const double headWdt = item->headWidth();

    Accidental::LayoutData* topAccData = item->topAccidental()->mutLayoutData();
    Accidental::LayoutData* bottomAccData = item->bottomAccidental()->mutLayoutData();

    ldata->setPos(PointF());

    //
    // NOTEHEADS Y POS
    //
    {
        const Fraction tick = item->segment()->tick();
        const Staff* stf = ctx.dom().staff(item->staffIdx());
        const ClefType clf = stf->clef(tick);
        const Key key = stf->key(tick);
        const double lineDist = stf->lineDistance(tick) * spatium;

        // top notehead
        if (item->topPitch() == INVALID_PITCH || item->topTpc() == Tpc::TPC_INVALID) {
            ldata->topPos.setY(0.0); // if uninitialized, set to top staff line
        } else {
            int topLine = Ambitus::staffLine(item->topTpc(), item->topPitch(), clf);
            ldata->topPos.setY(topLine * lineDist * 0.5);
            // compute accidental
            AccidentalType accidType = Ambitus::accidentalType(item->topTpc(), key);
            item->topAccidental()->setAccidentalType(accidType);
            TLayout::layout(item->topAccidental(), topAccData, ctx.conf());
            topAccData->setPosY(ldata->topPos.y());
        }

        // bottom notehead
        if (item->bottomPitch() == INVALID_PITCH || item->bottomTpc() == Tpc::TPC_INVALID) {
            const int numOfLines = stf->lines(tick);
            ldata->bottomPos.setY((numOfLines - 1) * lineDist);         // if uninitialized, set to last staff line
        } else {
            int bottomLine = Ambitus::staffLine(item->bottomTpc(), item->bottomPitch(), clf);
            ldata->bottomPos.setY(bottomLine * lineDist * 0.5);
            // compute accidental
            AccidentalType accidType = Ambitus::accidentalType(item->bottomTpc(), key);
            item->bottomAccidental()->setAccidentalType(accidType);
            TLayout::layout(item->bottomAccidental(), bottomAccData, ctx.conf());
            bottomAccData->setPosY(ldata->bottomPos.y());
        }
    }

    //
    // NOTEHEAD X POS
    //
    // Note: manages colliding accidentals
    //
    {
        double accNoteDist = item->point(ctx.conf().styleS(Sid::accidentalNoteDistance));
        double xAccidOffTop = topAccData->bbox().width() + accNoteDist;
        double xAccidOffBottom = bottomAccData->bbox().width() + accNoteDist;

        // if top accidental extends down more than bottom accidental extends up,
        // AND ambitus is not leaning right, bottom accidental needs to be displaced
        bool collision = (item->direction() != DirectionH::RIGHT)
                         && (topAccData->pos().y() + topAccData->bbox().y() + topAccData->bbox().height()
                             > bottomAccData->pos().y() + bottomAccData->bbox().y());
        if (collision) {
            // displace bottom accidental (also attempting to 'undercut' flats)
            AccidentalType bottomAccType = item->bottomAccidental()->accidentalType();
            xAccidOffBottom = xAccidOffTop + ((bottomAccType == AccidentalType::FLAT
                                               || bottomAccType == AccidentalType::FLAT2
                                               || bottomAccType == AccidentalType::NATURAL)
                                              ? bottomAccData->bbox().width() * 0.5 : bottomAccData->bbox().width());
        }

        switch (item->direction()) {
        case DirectionH::AUTO:                   // noteheads one above the other
            // left align noteheads and right align accidentals 'hanging' on the left
            ldata->topPos.setX(0.0);
            ldata->bottomPos.setX(0.0);
            topAccData->setPosX(-xAccidOffTop);
            bottomAccData->setPosX(-xAccidOffBottom);
            break;
        case DirectionH::LEFT:                   // top notehead at the left of bottom notehead
            // place top notehead at left margin; bottom notehead at right of top head;
            // top accid. 'hanging' on left of top head and bottom accid. 'hanging' at left of bottom head
            ldata->topPos.setX(0.0);
            ldata->bottomPos.setX(headWdt);
            topAccData->setPosX(-xAccidOffTop);
            bottomAccData->setPosX(collision ? -xAccidOffBottom : headWdt - xAccidOffBottom);
            break;
        case DirectionH::RIGHT:                  // top notehead at the right of bottom notehead
            // bottom notehead at left margin; top notehead at right of bottomnotehead
            // top accid. 'hanging' on left of top head and bottom accid. 'hanging' at left of bottom head
            ldata->topPos.setX(headWdt);
            ldata->bottomPos.setX(0.0);
            topAccData->setPosX(headWdt - xAccidOffTop);
            bottomAccData->setPosX(-xAccidOffBottom);
            break;
        }
    }

    // LINE
    // compute line from top note centre to bottom note centre
    {
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
    }

    // BBOX
    {
        RectF headRect(0, -0.5 * spatium, headWdt, 1 * spatium);
        ldata->setBbox(headRect.translated(ldata->topPos).united(headRect.translated(ldata->bottomPos))
                       .united(topAccData->bbox().translated(topAccData->pos()))
                       .united(bottomAccData->bbox().translated(bottomAccData->pos())));
    }
}

void TLayout::layout(const Arpeggio* item, Arpeggio::LayoutData* ldata, const LayoutConfiguration& conf, bool includeCrossStaffHeight)
{
    if (conf.styleB(Sid::ArpeggioHiddenInStdIfTab)) {
        if (item->staff() && item->staff()->isPitchedStaff(item->tick())) {
            for (Staff* s : item->staff()->staffList()) {
                if (s->onSameScore(item) && s->isTabStaff(item->tick()) && s->visible()) {
                    ldata->setIsSkipDraw(true);
                }
            }
        }
    }
    ldata->setIsSkipDraw(false);
    ldata->setPos(PointF());

    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    //! NOTE Must already be set previously
    Chord* parentChord = item->chord();
    LD_CONDITION(parentChord->upNote()->layoutData()->isSetPos());
    LD_CONDITION(parentChord->downNote()->layoutData()->isSetPos());

    auto computeHeight = [](const Arpeggio* item, bool includeCrossStaffHeight) -> double
    {
        Chord* chord = item->chord();
        double y = chord->upNote()->pagePos().y() - chord->upNote()->headHeight() * .5;

        Note* downNote = chord->downNote();
        if (includeCrossStaffHeight) {
            track_idx_t bottomTrack = item->track() + (item->span() - 1) * VOICES;
            EngravingItem* element = chord->segment()->element(bottomTrack);
            Chord* bottomChord = (element && element->isChord()) ? toChord(element) : chord;
            downNote = bottomChord->downNote();
        }

        double h = downNote->pagePos().y() + downNote->headHeight() * .5 - y;
        return h;
    };

    auto calcTop = [](const Arpeggio* item, const LayoutConfiguration& conf) -> double
    {
        double top = -item->userLen1();
        switch (item->arpeggioType()) {
        case ArpeggioType::BRACKET: {
            double lineWidth = conf.styleMM(Sid::ArpeggioLineWidth);
            return top - lineWidth / 2.0;
        }
        case ArpeggioType::NORMAL:
        case ArpeggioType::UP:
        case ArpeggioType::DOWN: {
            // if the top is in the staff on a space, move it up
            // if the bottom note is on a line, the distance is 0.25 spaces
            // if the bottom note is on a space, the distance is 0.5 spaces
            int topNoteLine = item->chord()->upNote()->line();
            int lines = item->staff()->lines(item->tick());
            int bottomLine = (lines - 1) * 2;
            if (topNoteLine <= 0 || topNoteLine % 2 == 0 || topNoteLine >= bottomLine) {
                return top;
            }
            int downNoteLine = item->chord()->downNote()->line();
            if (downNoteLine % 2 == 1 && downNoteLine < bottomLine) {
                return top - 0.4 * item->spatium();
            }
            return top - 0.25 * item->spatium();
        }
        default: {
            return top - item->spatium() / 4;
        }
        }
    };

    auto calcBottom = [](const Arpeggio* item, double arpeggioHeight, const LayoutConfiguration& conf) -> double
    {
        double top = -item->userLen1();
        double bottom = arpeggioHeight + item->userLen2();

        switch (item->arpeggioType()) {
        case ArpeggioType::BRACKET: {
            double lineWidth = conf.styleMM(Sid::ArpeggioLineWidth);
            return bottom - top + lineWidth;
        }
        case ArpeggioType::NORMAL:
        case ArpeggioType::UP:
        case ArpeggioType::DOWN: {
            return bottom;
        }
        default: {
            return bottom - top + item->spatium() / 2;
        }
        }
    };

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

    ldata->arpeggioHeight = computeHeight(item, includeCrossStaffHeight);
    ldata->top = calcTop(item, conf);
    ldata->bottom = calcBottom(item, ldata->arpeggioHeight, conf);

    ldata->setMag(item->staff() ? item->staff()->staffMag(item->tick()) : item->mag());
    ldata->magS = conf.magS(ldata->mag());

    std::shared_ptr<const IEngravingFont> font = conf.engravingFont();
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
        double w  = conf.styleS(Sid::ArpeggioHookLen).val() * item->spatium();
        ldata->setBbox(RectF(0.0, ldata->top, w, ldata->bottom));
    } break;
    }
}

void TLayout::layout(const Articulation* item, Articulation::LayoutData* ldata)
{
    if (item->isHiddenOnTabStaff()) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);
    ldata->setPos(PointF());

    //! NOTE Must already be set previously
    LD_CONDITION(ldata->isSetSymId());

    RectF bbox;

    if (item->textType() == ArticulationTextType::NO_TEXT) {
        bbox = item->symBbox(ldata->symId());
    } else {
        Font scaledFont(item->font());
        scaledFont.setPointSizeF(item->font().pointSizeF() * item->magS());
        FontMetrics fm(scaledFont);
        bbox = fm.boundingRect(scaledFont, TConv::text(item->textType()));
    }

    ldata->setBbox(bbox.translated(-0.5 * bbox.width(), 0.0));
}

static double barLineWidth(const BarLine* item, const MStyle& style, double dotWidth)
{
    double w = 0.0;
    switch (item->barLineType()) {
    case BarLineType::DOUBLE:
        w = style.styleMM(Sid::doubleBarWidth) * 2.0
            + style.styleMM(Sid::doubleBarDistance);
        break;
    case BarLineType::DOUBLE_HEAVY:
        w = style.styleMM(Sid::endBarWidth) * 2.0
            + style.styleMM(Sid::endBarDistance);
        break;
    case BarLineType::END_START_REPEAT:
        w = style.styleMM(Sid::endBarWidth)
            + style.styleMM(Sid::barWidth) * 2.0
            + style.styleMM(Sid::endBarDistance) * 2.0
            + style.styleMM(Sid::repeatBarlineDotSeparation) * 2.0
            + dotWidth * 2;
        break;
    case BarLineType::START_REPEAT:
    case BarLineType::END_REPEAT:
        w = style.styleMM(Sid::endBarWidth)
            + style.styleMM(Sid::barWidth)
            + style.styleMM(Sid::endBarDistance)
            + style.styleMM(Sid::repeatBarlineDotSeparation)
            + dotWidth;
        break;
    case BarLineType::END:
    case BarLineType::REVERSE_END:
        w = style.styleMM(Sid::endBarWidth)
            + style.styleMM(Sid::barWidth)
            + style.styleMM(Sid::endBarDistance);
        break;
    case BarLineType::BROKEN:
    case BarLineType::NORMAL:
    case BarLineType::DOTTED:
        w = style.styleMM(Sid::barWidth);
        break;
    case BarLineType::HEAVY:
        w = style.styleMM(Sid::endBarWidth);
        break;
    }
    return w;
}

void TLayout::layout(const BarLine* item, BarLine::LayoutData* ldata, const LayoutContext& ctx)
{
    // barlines hidden on this staff
    if (item->staff() && item->segment()) {
        if ((!item->staff()->staffTypeForElement(item)->showBarlines() && item->segment()->segmentType() == SegmentType::EndBarLine)
            || (item->staff()->hideSystemBarLine() && item->segment()->segmentType() == SegmentType::BeginBarLine)) {
            ldata->setBbox(RectF());
            ldata->setIsSkipDraw(true);
            return;
        }
    }

    ldata->setIsSkipDraw(false);
    ldata->setPos(PointF());

    // Check conditions

    //! NOTE Here the goal is not just to check whether the conditions are met,
    //! but the goal is to clearly show these conditions to the developer.
    for (const EngravingItem* e : *item->el()) {
        switch (e->type()) {
        case ElementType::ARTICULATION:
            // form Articulation layout
            LD_CONDITION(item_cast<const Articulation*>(e)->layoutData()->isSetSymId());
            break;
        case ElementType::SYMBOL:
            // not yet clear
            break;
        case ElementType::IMAGE:
            // not yet clear
            break;
        default:
            UNREACHABLE;
        }
    }

    ldata->setMag(ctx.conf().styleB(Sid::scaleBarlines) && item->staff() ? item->staff()->staffMag(item->tick()) : 1.0);
    // Note: the true values of y1 and y2 are computed in layout2() (can be done only
    // after staff distances are known). This is a temporary layout.
    const double spatium = item->spatium();

    ldata->y1 = spatium * .5 * item->spanFrom();
    ldata->y2 = spatium * .5 * (8.0 + item->spanTo());

    const IEngravingFontPtr font = ctx.engravingFont();
    const double magS = ctx.conf().magS(ldata->mag());

    double w = barLineWidth(item, ctx.conf().style(), font->width(SymId::repeatDot, magS)) * ldata->mag();
    RectF r(0.0, ldata->y1, w, ldata->y2 - ldata->y1);

    if (ctx.conf().styleB(Sid::repeatBarTips)) {
        switch (item->barLineType()) {
        case BarLineType::START_REPEAT: {
            r.unite(font->bbox(SymId::bracketTop, magS).translated(0, ldata->y1));
        } break;
        case BarLineType::END_REPEAT: {
            double w1 = 0.0;
            r.unite(font->bbox(SymId::reversedBracketTop, magS).translated(-w1, ldata->y1));
        } break;
        case BarLineType::END_START_REPEAT: {
            double w1 = 0.0;
            r.unite(font->bbox(SymId::reversedBracketTop, magS).translated(-w1, ldata->y1));
            r.unite(font->bbox(SymId::bracketTop, magS).translated(0, ldata->y1));
        } break;
        default:
            break;
        }
    }

    ldata->setBbox(r);

    //! NOTE The types are listed here explicitly to show what types there are (see add method)
    //! and accordingly show what the barline layout depends on.
    for (EngravingItem* e : *item->el()) {
        switch (e->type()) {
        case ElementType::ARTICULATION: {
            Articulation* a = item_cast<Articulation*>(e);
            Articulation::LayoutData* aldata = a->mutLayoutData();
            TLayout::layout(a, aldata);
            DirectionV dir = a->direction();
            double distance = 0.5 * spatium;
            double x = /*barline*/ ldata->bbox().width() * 0.5;
            if (dir == DirectionV::DOWN) {
                double botY = ldata->y2 + distance;
                aldata->setPos(PointF(x, botY));
            } else {
                double topY = ldata->y1 - distance;
                aldata->setPos(PointF(x, topY));
            }
        } break;
        case ElementType::SYMBOL:
            TLayout::layoutItem(e, const_cast<LayoutContext&>(ctx));
            break;
        case ElementType::IMAGE:
            TLayout::layoutItem(e, const_cast<LayoutContext&>(ctx));
            break;
        default:
            UNREACHABLE;
        }
    }
}

RectF TLayout::layoutRect(const BarLine* item, LayoutContext& ctx)
{
    const BarLine::LayoutData* ldata = item->layoutData();
    RectF bb = ldata->bbox();
    if (item->staff()) {
        // actual height may include span to next staff
        // but this should not be included in shapes or skylines
        double sp = item->spatium();
        int span = item->staff()->lines(item->tick()) - 1;
        int sFrom;
        int sTo;
        if (span == 0 && item->spanTo() == 0) {
            sFrom = BARLINE_SPAN_1LINESTAFF_FROM;
            sTo = item->spanStaff() ? 0 : BARLINE_SPAN_1LINESTAFF_TO;
        } else {
            sFrom = item->spanFrom();
            sTo = item->spanStaff() ? 0 : item->spanTo();
        }
        double y = sp * sFrom * 0.5;
        double h = sp * (span + (sTo - sFrom) * 0.5);
        if (ctx.conf().styleB(Sid::repeatBarTips)) {
            switch (item->barLineType()) {
            case BarLineType::START_REPEAT:
            case BarLineType::END_REPEAT:
            case BarLineType::END_START_REPEAT: {
                if (item->isTop()) {
                    double top = item->symBbox(SymId::bracketTop).height();
                    y -= top;
                    h += top;
                }
                if (item->isBottom()) {
                    double bottom = item->symBbox(SymId::bracketBottom).height();
                    h += bottom;
                }
            }
            default:
                break;
            }
        }
        bb.setTop(y);
        bb.setHeight(h);
    }
    return bb;
}

//---------------------------------------------------------
//    called after system layout; set vertical dimensions
//---------------------------------------------------------
void TLayout::layout2(BarLine* item, LayoutContext& ctx)
{
    BarLine::LayoutData* ldata = item->mutLayoutData();

    if (ldata->isSkipDraw()) {
        return;
    }

    item->calcY();
    RectF bbox = ldata->bbox();
    bbox.setTop(ldata->y1);
    bbox.setBottom(ldata->y2);

    if (ctx.conf().styleB(Sid::repeatBarTips)) {
        switch (item->barLineType()) {
        case BarLineType::START_REPEAT:
            bbox.unite(item->symBbox(SymId::bracketTop).translated(0, ldata->y1));
            bbox.unite(item->symBbox(SymId::bracketBottom).translated(0, ldata->y2));
            break;
        case BarLineType::END_REPEAT:
        {
            double w1 = 0.0;               //symBbox(SymId::reversedBracketTop).width();
            bbox.unite(item->symBbox(SymId::reversedBracketTop).translated(-w1, ldata->y1));
            bbox.unite(item->symBbox(SymId::reversedBracketBottom).translated(-w1, ldata->y2));
            break;
        }
        case BarLineType::END_START_REPEAT:
        {
            double w1 = 0.0;               //symBbox(SymId::reversedBracketTop).width();
            bbox.unite(item->symBbox(SymId::reversedBracketTop).translated(-w1, ldata->y1));
            bbox.unite(item->symBbox(SymId::reversedBracketBottom).translated(-w1, ldata->y2));
            bbox.unite(item->symBbox(SymId::bracketTop).translated(0, ldata->y1));
            bbox.unite(item->symBbox(SymId::bracketBottom).translated(0, ldata->y2));
            break;
        }
        default:
            break;
        }
    }

    ldata->setBbox(bbox);
}

void TLayout::layout(Beam* item, LayoutContext& ctx)
{
    BeamLayout::layout(item, ctx);
}

void TLayout::layout1(Beam* item, LayoutContext& ctx)
{
    BeamLayout::layout1(item, ctx);
}

void TLayout::layout(const Bend* item, Bend::LayoutData* ldata)
{
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    LD_CONDITION(item->note()->layoutData()->isSetPos());
    LD_CONDITION(item->note()->layoutData()->isSetBbox());

    double spatium = item->spatium();
    double lw = item->lineWidth();

    const Note::LayoutData* noteLD = item->note()->layoutData();
    PointF notePos = noteLD->pos();
    notePos.ry() = std::max(notePos.y(), 0.0);

    ldata->noteWidth = noteLD->bbox().width();
    ldata->notePos = notePos;

    RectF bb;

    mu::draw::FontMetrics fm(item->font(spatium));

    size_t n = item->points().size();
    double x = ldata->noteWidth;
    double y = -spatium * .8;
    double x2 = 0.0, y2 = 0.0;

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
                                     draw::AlignHCenter | draw::AlignBottom | draw::TextDontClip,
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
                                     draw::AlignHCenter | draw::AlignBottom | draw::TextDontClip,
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
    ldata->setPos(PointF());
}

void TLayout::layout(const Box* item, Box::LayoutData* ldata, const LayoutContext& ctx)
{
    switch (item->type()) {
    case ElementType::HBOX:
        TLayout::layout(static_cast<const HBox*>(item), static_cast<HBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::VBOX:
        TLayout::layout(static_cast<const VBox*>(item), static_cast<VBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::FBOX:
        TLayout::layout(static_cast<const FBox*>(item), static_cast<FBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::TBOX:
        TLayout::layout(static_cast<const TBox*>(item), static_cast<TBox::LayoutData*>(ldata), ctx);
        break;
    default:
        UNREACHABLE;
        break;
    }
}

void TLayout::layoutBox(const Box* item, Box::LayoutData* ldata, const LayoutContext& ctx)
{
    layoutMeasureBase(item, ldata, ctx);
}

void TLayout::layout(const HBox* item, HBox::LayoutData* ldata, const LayoutContext& ctx)
{
    if (item->explicitParent() && item->explicitParent()->isVBox()) {
        const VBox* parentVBox = toVBox(item->explicitParent());

        LD_CONDITION(parentVBox->layoutData()->isSetBbox());

        double x = parentVBox->leftMargin() * DPMM;
        double y = parentVBox->topMargin() * DPMM;
        double w = item->point(item->boxWidth());
        double h = parentVBox->layoutData()->bbox().height() - (parentVBox->topMargin() + parentVBox->bottomMargin()) * DPMM;
        ldata->setPos(x, y);
        ldata->setBbox(0.0, 0.0, w, h);
    } else if (item->system()) {
        const System* parentSystem = item->system();

        LD_CONDITION(parentSystem->layoutData()->isSetBbox());

        ldata->setPos(PointF());
        ldata->setBbox(0.0, 0.0, item->point(item->boxWidth()), parentSystem->layoutData()->bbox().height());
    } else {
        ldata->setPos(PointF());
        ldata->setBbox(0.0, 0.0, 50, 50);
    }
    layoutBox(item, ldata, ctx);
}

void TLayout::layout2(HBox* item, LayoutContext& ctx)
{
    layoutBox(item, item->mutLayoutData(), ctx);
}

void TLayout::layout(const VBox* item, VBox::LayoutData* ldata, const LayoutContext& ctx)
{
    ldata->setPos(PointF());

    if (item->system()) {
        const System* parentSystem = item->system();

        LD_CONDITION(parentSystem->layoutData()->isSetBbox());

        ldata->setBbox(0.0, 0.0, parentSystem->layoutData()->bbox().width(), item->point(item->boxHeight()));
    } else {
        ldata->setBbox(0.0, 0.0, 50, 50);
    }

    for (EngravingItem* e : item->el()) {
        layoutItem(e, const_cast<LayoutContext&>(ctx));
    }

    if (item->getProperty(Pid::BOX_AUTOSIZE).toBool()) {
        double contentHeight = item->contentRect().height();

        if (contentHeight < item->minHeight()) {
            contentHeight = item->minHeight();
        }

        ldata->setHeight(contentHeight);
    }

    if (MScore::noImages) {
        // adjustLayoutWithoutImages
        double calculatedVBoxHeight = 0;
        const int padding = ctx.conf().spatium();
        ElementList elist = item->el();
        for (EngravingItem* e : elist) {
            if (e->isText()) {
                Text* txt = toText(e);
                Text::LayoutData* txtLD = txt->mutLayoutData();

                LD_CONDITION(txtLD->isSetBbox());

                RectF bbox = txtLD->bbox();
                bbox.moveTop(0.0);
                txtLD->setBbox(bbox);
                calculatedVBoxHeight += txtLD->bbox().height() + padding;
            }
        }

        ldata->setHeight(calculatedVBoxHeight);
    }
}

void TLayout::layout(const FBox* item, EngravingItem::LayoutData* ldata, const LayoutContext& ctx)
{
    const System* parentSystem = item->system();

    LD_CONDITION(parentSystem->layoutData()->isSetBbox());

    ldata->setPos(PointF());
    ldata->setBbox(0.0, 0.0, parentSystem->layoutData()->bbox().width(), item->point(item->boxHeight()));
    layoutBox(item, ldata, ctx);
}

void TLayout::layout(const TBox* item, FBox::LayoutData* ldata, const LayoutContext& ctx)
{
    const System* parentSystem = item->system();

    LD_CONDITION(parentSystem->layoutData()->isSetBbox());

    ldata->setPos(PointF());
    ldata->setBbox(0.0, 0.0, parentSystem->layoutData()->bbox().width(), 0);

    TLayout::layout(item->text(), const_cast<LayoutContext&>(ctx));

    Text::LayoutData* textLD = item->text()->mutLayoutData();

    double h = 0.0;
    if (item->text()->empty()) {
        h = FontMetrics::ascent(item->text()->font());
    } else {
        h = textLD->bbox().height();
    }
    double y = item->topMargin() * DPMM;
    textLD->setPos(item->leftMargin() * DPMM, y);
    h += item->topMargin() * DPMM + item->bottomMargin() * DPMM;
    ldata->setBbox(0.0, 0.0, item->system()->width(), h);

    layoutMeasureBase(item, ldata, ctx);   // layout LayoutBreak's
}

static void layoutBracket(const Bracket* item, const LayoutContext& ctx, Bracket::LayoutData* ldata)
{
    if (RealIsNull(item->h2())) {
        return;
    }

    const_cast<Bracket*>(item)->setVisible(item->bi()->visible());
    ldata->braceSymbol = item->braceSymbol();

    switch (item->bracketType()) {
    case BracketType::BRACE: {
        String musicalSymbolFont = ctx.conf().styleSt(Sid::MusicalSymbolFont);
        if (musicalSymbolFont == "Emmentaler" || musicalSymbolFont == "Gonville") {
            ldata->braceSymbol = SymId::noSym;
            double w = ctx.conf().styleMM(Sid::akkoladeWidth);

#define XM(a) (a + 700) * w / 700
#define YM(a) (a + 7100) * item->h2() / 7100

            PainterPath path;
            path.moveTo(XM(-8), YM(-2048));
            path.cubicTo(XM(-8), YM(-3192), XM(-360), YM(-4304), XM(-360), YM(-5400));                 // c 0
            path.cubicTo(XM(-360), YM(-5952), XM(-264), YM(-6488), XM(32), YM(-6968));                 // c 1
            path.cubicTo(XM(36), YM(-6974), XM(38), YM(-6984), XM(38), YM(-6990));                     // c 0
            path.cubicTo(XM(38), YM(-7008), XM(16), YM(-7024), XM(0), YM(-7024));                      // c 0
            path.cubicTo(XM(-8), YM(-7024), XM(-22), YM(-7022), XM(-32), YM(-7008));                   // c 1
            path.cubicTo(XM(-416), YM(-6392), XM(-544), YM(-5680), XM(-544), YM(-4960));               // c 0
            path.cubicTo(XM(-544), YM(-3800), XM(-168), YM(-2680), XM(-168), YM(-1568));               // c 0
            path.cubicTo(XM(-168), YM(-1016), XM(-264), YM(-496), XM(-560), YM(-16));                  // c 1
            path.lineTo(XM(-560), YM(0));                    //  l 1
            path.lineTo(XM(-560), YM(16));                   //  l 1
            path.cubicTo(XM(-264), YM(496), XM(-168), YM(1016), XM(-168), YM(1568));                   // c 0
            path.cubicTo(XM(-168), YM(2680), XM(-544), YM(3800), XM(-544), YM(4960));                  // c 0
            path.cubicTo(XM(-544), YM(5680), XM(-416), YM(6392), XM(-32), YM(7008));                   // c 1
            path.cubicTo(XM(-22), YM(7022), XM(-8), YM(7024), XM(0), YM(7024));                        // c 0
            path.cubicTo(XM(16), YM(7024), XM(38), YM(7008), XM(38), YM(6990));                        // c 0
            path.cubicTo(XM(38), YM(6984), XM(36), YM(6974), XM(32), YM(6968));                        // c 1
            path.cubicTo(XM(-264), YM(6488), XM(-360), YM(5952), XM(-360), YM(5400));                  // c 0
            path.cubicTo(XM(-360), YM(4304), XM(-8), YM(3192), XM(-8), YM(2048));                      // c 0
            path.cubicTo(XM(-8), YM(1320), XM(-136), YM(624), XM(-512), YM(0));                        // c 1
            path.cubicTo(XM(-136), YM(-624), XM(-8), YM(-1320), XM(-8), YM(-2048));                    // c 0*/
            ldata->path = path;
            ldata->setBbox(path.boundingRect());
            ldata->shape.add(ldata->bbox());
        } else {
            if (item->braceSymbol() == SymId::noSym) {
                ldata->braceSymbol = SymId::brace;
            }
            double h = item->h2() * 2;
            double w = item->symWidth(item->braceSymbol()) * item->magx();
            ldata->setBbox(RectF(0, 0, w, h));
            ldata->shape.add(ldata->bbox());
        }
    }
    break;
    case BracketType::NORMAL: {
        double _spatium = item->spatium();
        double w = ctx.conf().styleMM(Sid::bracketWidth) * .5;
        double x = -w;

        double bd = (ctx.conf().styleSt(Sid::MusicalSymbolFont) == "Leland") ? _spatium * .5 : _spatium * .25;
        ldata->shape.add(RectF(x, -bd, w * 2, 2 * (item->h2() + bd)));
        ldata->shape.add(item->symBbox(SymId::bracketTop).translated(PointF(-w, -bd)));
        ldata->shape.add(item->symBbox(SymId::bracketBottom).translated(PointF(-w, bd + 2 * item->h2())));

        w += item->symWidth(SymId::bracketTop);
        double y = -item->symHeight(SymId::bracketTop) - bd;
        double h = (-y + item->h2()) * 2;
        ldata->setBbox(RectF(x, y, w, h));
    }
    break;
    case BracketType::SQUARE: {
        double w = ctx.conf().styleMM(Sid::staffLineWidth) * .5;
        double x = -w;
        double y = -w;
        double h = (item->h2() + w) * 2;
        w += (.5 * item->spatium() + 3 * w);
        ldata->setBbox(RectF(x, y, w, h));
        ldata->shape.add(ldata->bbox());
    }
    break;
    case BracketType::LINE: {
        double _spatium = item->spatium();
        double w = 0.67 * ctx.conf().styleMM(Sid::bracketWidth) * .5;
        double x = -w;
        double bd = _spatium * .25;
        double y = -bd;
        double h = (-y + item->h2()) * 2;
        ldata->setBbox(RectF(x, y, w, h));
        ldata->shape.add(ldata->bbox());
    }
    break;
    case BracketType::NO_BRACKET:
        break;
    }
}

void TLayout::layout(Bracket* item, LayoutContext& ctx)
{
    layoutBracket(item, ctx, item->mutLayoutData());
}

static void layoutBreath(const Breath* item, const LayoutContext& ctx, Breath::LayoutData* ldata)
{
    int voiceOffset = item->placeBelow() * (item->staff()->lines(item->tick()) - 1) * item->spatium();
    if (item->isCaesura()) {
        ldata->setPos(PointF(ldata->pos().x(), item->spatium() + voiceOffset));
    } else if ((ctx.conf().styleSt(Sid::MusicalSymbolFont) == "Emmentaler")
               && (item->symId() == SymId::breathMarkComma)) {
        ldata->setPos(PointF(ldata->pos().x(), 0.5 * item->spatium() + voiceOffset));
    } else {
        ldata->setPos(PointF(ldata->pos().x(), -0.5 * item->spatium() + voiceOffset));
    }

    ldata->setBbox(item->symBbox(item->symId()));
}

void TLayout::layout(Breath* item, LayoutContext& ctx)
{
    layoutBreath(item, ctx, item->mutLayoutData());
}

void TLayout::layout(Chord* item, LayoutContext& ctx)
{
    ChordLayout::layout(item, ctx);
}

static void layoutChordLine(const ChordLine* item, const LayoutContext& ctx, ChordLine::LayoutData* ldata)
{
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    ldata->setMag(item->chord()->mag());

    if (!item->modified()) {
        double x2 = 0;
        double y2 = 0;
        double baseLength = item->spatium() * item->chord()->intrinsicMag();
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

    Note* note = nullptr;

    if (item->note()) {
        note = item->chord()->findNote(item->note()->pitch());
    }

    if (!note) {
        note = item->chord()->upNote();
    }

    double x = 0.0;
    double y = note->pos().y();
    double horOffset = 0.33 * item->spatium();         // one third of a space away from the note
    double vertOffset = 0.25 * item->spatium();         // one quarter of a space from the center line
    // Get chord shape
    Shape chordShape = item->chord()->shape();
    // ...but remove from the shape items that the chordline shouldn't try to avoid
    // (especially the chordline itself)
    mu::remove_if(chordShape, [](ShapeElement& shapeEl){
        if (!shapeEl.toItem) {
            return true;
        }
        const EngravingItem* item = shapeEl.toItem;
        if (item->isChordLine() || item->isHarmony() || item->isLyrics()) {
            return true;
        }
        return false;
    });
    x += item->isToTheLeft() ? -chordShape.left() - horOffset : chordShape.right() + horOffset;
    y += item->isBelow() ? vertOffset : -vertOffset;

    /// TODO: calculate properly the position for wavy type
    if (item->isWavy()) {
        bool upDir = item->chordLineType() == ChordLineType::DOIT;
        y += note->height() * (upDir ? 0.8 : -0.3);
    }

    ldata->setPos(x, y);

    if (!item->isWavy()) {
        RectF r = ldata->path.boundingRect();
        int x1 = 0, y1 = 0, width = 0, height = 0;

        x1 = r.x();
        y1 = r.y();
        width = r.width();
        height = r.height();
        ldata->setBbox(x1, y1, width, height);
    } else {
        RectF r = ctx.engravingFont()->bbox(ChordLine::WAVE_SYMBOLS, item->magS());
        double angle = ChordLine::WAVE_ANGEL * M_PI / 180;

        r.setHeight(r.height() + r.width() * sin(angle));

        /// TODO: calculate properly the rect for wavy type
        if (item->chordLineType() == ChordLineType::DOIT) {
            r.setY(item->y() - r.height() * (item->onTabStaff() ? 1.25 : 1));
        }

        ldata->setBbox(r);
    }
}

void TLayout::layout(ChordLine* item, LayoutContext& ctx)
{
    layoutChordLine(item, ctx, item->mutLayoutData());
}

static void layoutClef(const Clef* item, const LayoutContext&, Clef::LayoutData* ldata)
{
    // determine current number of lines and line distance
    int lines;
    double lineDist;
    Segment* clefSeg  = item->segment();
    int stepOffset;

    // check clef visibility and type compatibility
    if (clefSeg && item->staff()) {
        Fraction tick = clefSeg->tick();
        const StaffType* st = item->staff()->staffType(tick);
        bool show     = st->genClef();            // check staff type allows clef display
        StaffGroup staffGroup = st->group();

        // if not tab, use instrument->useDrumset to set staffGroup (to allow pitched to unpitched in same staff)
        if (staffGroup != StaffGroup::TAB) {
            staffGroup = item->staff()->part()->instrument(item->tick())->useDrumset() ? StaffGroup::PERCUSSION : StaffGroup::STANDARD;
        }

        // check clef is compatible with staff type group:
        if (ClefInfo::staffGroup(item->clefType()) != staffGroup) {
            if (tick > Fraction(0, 1) && !item->generated()) {     // if clef is not generated, hide it
                show = false;
            } else {                            // if generated, replace with initial clef type
                // TODO : instead of initial staff clef (which is assumed to be compatible)
                // use the last compatible clef previously found in staff
                const_cast<Clef*>(item)->setClefType(item->staff()->clefType(Fraction(0, 1)));
            }
        }

        // if clef not to show or not compatible with staff group
        if (!show) {
            ldata->setBbox(RectF());
            ldata->symId = SymId::noSym;
            LOGD("invisible clef at tick %d(%d) staff %zu",
                 item->segment()->tick().ticks(), item->segment()->tick().ticks() / 1920, item->staffIdx());
            return;
        }
        lines      = st->lines();             // init values from staff type
        lineDist   = st->lineDistance().val();
        stepOffset = st->stepOffset();
    } else {
        lines      = 5;
        lineDist   = 1.0;
        stepOffset = 0;
    }

    double _spatium = item->spatium();
    double yoff     = 0.0;
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
    case ClefType::TAB:                                    // TAB clef
        // on tablature, position clef at half the number of spaces * line distance
        yoff = lineDist * (lines - 1) * .5;
        stepOffset = 0;           //  ignore stepOffset for TAB and percussion clefs
        break;
    case ClefType::TAB4:                                    // TAB clef 4 strings
        // on tablature, position clef at half the number of spaces * line distance
        yoff = lineDist * (lines - 1) * .5;
        stepOffset = 0;
        break;
    case ClefType::TAB_SERIF:                                   // TAB clef alternate style
        // on tablature, position clef at half the number of spaces * line distance
        yoff = lineDist * (lines - 1) * .5;
        stepOffset = 0;
        break;
    case ClefType::TAB4_SERIF:                                   // TAB clef alternate style
        // on tablature, position clef at half the number of spaces * line distance
        yoff = lineDist * (lines - 1) * .5;
        stepOffset = 0;
        break;
    case ClefType::PERC:                                   // percussion clefs
        yoff = lineDist * (lines - 1) * 0.5;
        stepOffset = 0;
        break;
    case ClefType::PERC2:
        yoff = lineDist * (lines - 1) * 0.5;
        stepOffset = 0;
        break;
    case ClefType::INVALID:
    case ClefType::MAX:
        LOGD("Clef::layout: invalid type");
        return;
    default:
        break;
    }
    // clefs on palette or at start of system/measure are left aligned
    // other clefs are right aligned
    RectF r(item->symBbox(ldata->symId));
    double x = item->segment() && item->segment()->rtick().isNotZero() ? -r.right() : 0.0;
    ldata->setPos(PointF(x, yoff * _spatium + (stepOffset * 0.5 * _spatium)));
    ldata->setBbox(r);
}

void TLayout::layout(Clef* item, LayoutContext& ctx)
{
    layoutClef(item, ctx, item->mutLayoutData());
}

static void layoutCapo(const Capo* item, const LayoutContext& ctx, Capo::LayoutData* ldata)
{
    //! NOTE Looks like it doesn't belong here
    if (item->shouldAutomaticallyGenerateText() || item->empty()) {
        if (const Part* part = item->part()) {
            if (const Instrument* instrument = part->instrument(item->tick())) {
                if (const StringData* stringData = instrument->stringData()) {
                    String text = item->generateText(stringData->strings());
                    const_cast<Capo*>(item)->setXmlText(text);
                }
            }
        }
    }

    TLayout::layoutTextBase(item, ctx, ldata);
    Autoplace::autoplaceSegmentElement(item, ldata);
}

void TLayout::layout(Capo* item, LayoutContext& ctx)
{
    layoutCapo(item, ctx, item->mutLayoutData());
}

static void layoutDeadSlapped(const DeadSlapped* item, const LayoutContext&, DeadSlapped::LayoutData* ldata)
{
    const double deadSlappedWidth = item->spatium() * 2;
    RectF rect = RectF(0, 0, deadSlappedWidth, item->staff()->height());
    ldata->setBbox(rect);

    // fillPath
    {
        constexpr double crossThinknessPercentage = 0.1;
        double height = rect.height();
        double width = rect.width();
        double crossThickness = width * crossThinknessPercentage;

        PointF topLeft = PointF(rect.x(), rect.y());
        PointF bottomRight = topLeft + PointF(width, height);
        PointF topRight = topLeft + PointF(width, 0);
        PointF bottomLeft = topLeft + PointF(0, height);
        PointF offsetX = PointF(crossThickness, 0);

        mu::draw::PainterPath path1;
        path1.moveTo(topLeft);
        path1.lineTo(topLeft + offsetX);
        path1.lineTo(bottomRight);
        path1.lineTo(bottomRight - offsetX);
        path1.lineTo(topLeft);

        mu::draw::PainterPath path2;
        path2.moveTo(topRight);
        path2.lineTo(topRight - offsetX);
        path2.lineTo(bottomLeft);
        path2.lineTo(bottomLeft + offsetX);
        path2.lineTo(topRight);

        ldata->path1 = path1;
        ldata->path2 = path2;
    }
}

void TLayout::layout(DeadSlapped* item, LayoutContext& ctx)
{
    layoutDeadSlapped(item, ctx, item->mutLayoutData());
}

static void layoutDynamic(const Dynamic* item, const LayoutContext& ctx, Dynamic::LayoutData* ldata)
{
    const_cast<Dynamic*>(item)->setSnappedExpression(nullptr); // Here we reset it. It will become known again when we layout expression

    const StaffType* stType = item->staffType();

    if (stType && stType->isHiddenElementOnTab(ctx.conf().style(), Sid::dynamicsShowTabCommon, Sid::dynamicsShowTabSimple)) {
        ldata->setIsSkipDraw(true);
        return;
    }

    ldata->setIsSkipDraw(false);

    TLayout::layoutTextBase(item, ctx, ldata);

    Segment* s = item->segment();
    if (!s || (!item->centerOnNotehead() && item->align().horizontal == AlignH::LEFT)) {
        return;
    }

    EngravingItem* itemToAlign = nullptr;
    track_idx_t startTrack = staff2track(item->staffIdx());
    track_idx_t endTrack = startTrack + VOICES;
    for (track_idx_t track = startTrack; track < endTrack; ++track) {
        EngravingItem* e = s->elementAt(track);
        if (!e || (e->isRest() && toRest(e)->ticks() >= item->measure()->ticks() && item->measure()->hasVoices(e->staffIdx()))) {
            continue;
        }
        itemToAlign = e;
        break;
    }

    if (!itemToAlign) {
        return;
    }

    if (!itemToAlign->isChord()) {
        ldata->moveX(itemToAlign->width() * 0.5);
        return;
    }

    Chord* chord = toChord(itemToAlign);
    bool centerOnNote = item->centerOnNotehead() || (!item->centerOnNotehead() && item->align().horizontal == AlignH::HCENTER);

    // Move to center of notehead width
    Note* note = chord->notes().at(0);
    double noteHeadWidth = note->headWidth();
    ldata->moveX(noteHeadWidth * (centerOnNote ? 0.5 : 1));

    if (!item->centerOnNotehead()) {
        return;
    }

    // Use Smufl optical center for dynamic if available
    SymId symId = TConv::symId(item->dynamicType());
    double opticalCenter = item->symSmuflAnchor(symId, SmuflAnchorId::opticalCenter).x();
    if (symId != SymId::noSym && opticalCenter) {
        double symWidth = item->symBbox(symId).width();
        double offset = symWidth / 2 - opticalCenter + item->symBbox(symId).left();
        double spatiumScaling = item->spatium() / ctx.conf().spatium();
        offset *= spatiumScaling;
        ldata->moveX(offset);
    }

    // If the dynamic contains custom text, keep it aligned
    ldata->moveX(-item->customTextOffset());
}

void TLayout::layout(Dynamic* item, LayoutContext& ctx)
{
    layoutDynamic(item, ctx, item->mutLayoutData());
}

static void layoutExpression(const Expression* item, const LayoutContext& ctx, Expression::LayoutData* ldata)
{
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    TLayout::layoutTextBase(item, ctx, ldata);

    Segment* segment = toSegment(item->explicitParent());

    if (item->align().horizontal != AlignH::LEFT) {
        Chord* chordToAlign = nullptr;
        // Look for chord in this staff
        track_idx_t startTrack = track2staff(item->staffIdx());
        track_idx_t endTrack = startTrack + VOICES;
        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            EngravingItem* e = segment->elementAt(track);
            if (e && e->isChord()) {
                chordToAlign = toChord(e);
                break;
            }
        }

        if (chordToAlign && !chordToAlign->notes().empty()) {
            const Note* note = chordToAlign->notes().at(0);
            double headWidth = note->headWidth();
            bool center = item->align().horizontal == AlignH::HCENTER;
            ldata->moveX(headWidth * (center ? 0.5 : 1));
        }
    }

    const_cast<Expression*>(item)->setSnappedDynamic(nullptr);

    if (!item->autoplace()) {
        return;
    }

    if (!item->snapToDynamics()) {
        Autoplace::autoplaceSegmentElement(item, ldata);
        return;
    }

    Dynamic* dynamic = toDynamic(segment->findAnnotation(ElementType::DYNAMIC, item->track(), item->track()));
    if (!dynamic || dynamic->placeAbove() != item->placeAbove() || !dynamic->visible()) {
        Autoplace::autoplaceSegmentElement(item, ldata);
        return;
    }

    const_cast<Expression*>(item)->setSnappedDynamic(dynamic);
    dynamic->setSnappedExpression(const_cast<Expression*>(item));

    // If there is a dynamic on same segment and track, lock this expression to it
    double padding = item->computeDynamicExpressionDistance();
    double dynamicRight = dynamic->shape().translate(dynamic->pos()).right();
    double expressionLeft = ldata->bbox().translated(item->pos()).left();
    double difference = expressionLeft - dynamicRight - padding;
    ldata->moveX(-difference);

    // Keep expression and dynamic vertically aligned
    Autoplace::autoplaceSegmentElement(item, ldata);
    bool above = item->placeAbove();
    double yExpression = item->pos().y();
    double yDynamic = dynamic->pos().y();
    bool expressionIsOuter = above ? yExpression < yDynamic : yExpression > yDynamic;
    if (expressionIsOuter) {
        dynamic->mutLayoutData()->moveY((yExpression - yDynamic));
    } else {
        ldata->moveY((yDynamic - yExpression));
    }
}

void TLayout::layout(Expression* item, LayoutContext& ctx)
{
    layoutExpression(item, ctx, item->mutLayoutData());
}

static void layoutFermata(const Fermata* item, const LayoutContext& ctx, Fermata::LayoutData* ldata)
{
    const StaffType* stType = item->staffType();

    if (stType && stType->isHiddenElementOnTab(ctx.conf().style(), Sid::fermataShowTabCommon, Sid::fermataShowTabSimple)) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);
    ldata->setPos(PointF());

    Segment* s = item->segment();

    if (item->isStyled(Pid::OFFSET)) {
        const_cast<Fermata*>(item)->setOffset(item->propertyDefault(Pid::OFFSET).value<PointF>());
    }

    double x = 0.0;
    double y = 0.0;
    EngravingItem* e = s->element(item->track());
    if (e) {
        if (e->isChord()) {
            Chord* chord = toChord(e);
            x = chord->x() + chord->centerX();
            y = chord->y();
        } else {
            x = e->x() - e->shape().left() + e->width() * item->staff()->staffMag(Fraction(0, 1)) * .5;
            y = e->y();
        }
    }

    String name = String::fromAscii(SymNames::nameForSymId(item->symId()).ascii());
    if (item->placeAbove()) {
        if (name.endsWith(u"Below")) {
            //! NOTE It is not clear whether SymId is layout data or given data.
            const_cast<Fermata*>(item)->setSymId(SymNames::symIdByName(name.left(name.size() - 5) + u"Above"));
        }
    } else {
        if (name.endsWith(u"Above")) {
            //! NOTE It is not clear whether SymId is layout data or given data.
            const_cast<Fermata*>(item)->setSymId(SymNames::symIdByName(name.left(name.size() - 5) + u"Below"));
        }
    }
    ldata->setPos(x, y);
    RectF b(item->symBbox(item->symId()));
    ldata->setBbox(b.translated(-0.5 * b.width(), 0.0));

    Autoplace::autoplaceSegmentElement(item, ldata);
}

void TLayout::layout(Fermata* item, LayoutContext& ctx)
{
    layoutFermata(item, ctx, item->mutLayoutData());
}

//---------------------------------------------------------
//   FiguredBassItem layout
//    creates the display text (set as element text) and computes
//    the horiz. offset needed to align the right part as well as the vert. offset
//---------------------------------------------------------
static void layoutFiguredBassItem(const FiguredBassItem* item, const LayoutContext& ctx, FiguredBassItem::LayoutData* ldata)
{
    double h, w, x, x1, x2, y;

    // construct font metrics
    int fontIdx = 0;
    mu::draw::Font f(FiguredBass::FBFonts().at(fontIdx).family, draw::Font::Type::Tablature);

    // font size in pixels, scaled according to spatium()
    // (use the same font selection as used in draw() below)
    double m = ctx.conf().styleD(Sid::figuredBassFontSize) * item->spatium() / SPATIUM20;
    f.setPointSizeF(m);
    mu::draw::FontMetrics fm(f);

    String str;
    x  = item->symWidth(SymId::noteheadBlack) * .5;
    x1 = x2 = 0.0;

    // create display text
    int font = 0;
    int style = ctx.conf().styleI(Sid::figuredBassStyle);

    if (item->parenth1() != FiguredBassItem::Parenthesis::NONE) {
        str.append(FiguredBass::FBFonts().at(font).displayParenthesis[int(item->parenth1())]);
    }

    // prefix
    if (item->prefix() != FiguredBassItem::Modifier::NONE) {
        // if no digit, the string created so far 'hangs' to the left of the note
        if (item->digit() == FBIDigitNone) {
            x1 = fm.width(str);
        }
        str.append(FiguredBass::FBFonts().at(font).displayAccidental[int(item->prefix())]);
        // if no digit, the string from here onward 'hangs' to the right of the note
        if (item->digit() == FBIDigitNone) {
            x2 = fm.width(str);
        }
    }

    if (item->parenth2() != FiguredBassItem::Parenthesis::NONE) {
        str.append(FiguredBass::FBFonts().at(font).displayParenthesis[int(item->parenth2())]);
    }

    // digit
    if (item->digit() != FBIDigitNone) {
        // if some digit, the string created so far 'hangs' to the left of the note
        x1 = fm.width(str);
        // if suffix is a combining shape, combine it with digit (multi-digit numbers cannot be combined)
        // unless there is a parenthesis in between
        if ((item->digit() < 10)
            && (item->suffix() == FiguredBassItem::Modifier::CROSS
                || item->suffix() == FiguredBassItem::Modifier::BACKSLASH
                || item->suffix() == FiguredBassItem::Modifier::SLASH)
            && item->parenth3() == FiguredBassItem::Parenthesis::NONE) {
            str.append(FiguredBass::FBFonts().at(font).displayDigit[style][item->digit()][int(item->suffix())
                                                                                          - (int(FiguredBassItem::Modifier::CROSS)
                                                                                             - 1)]);
        }
        // if several digits or no shape combination, convert _digit to font styled chars
        else {
            String digits;
            int digit = item->digit();
            while (true) {
                digits.prepend(FiguredBass::FBFonts().at(font).displayDigit[style][(digit % 10)][0]);
                digit /= 10;
                if (digit == 0) {
                    break;
                }
            }
            str.append(digits);
        }
        // if some digit, the string from here onward 'hangs' to the right of the note
        x2 = fm.width(str);
    }

    if (item->parenth3() != FiguredBassItem::Parenthesis::NONE) {
        str.append(FiguredBass::FBFonts().at(font).displayParenthesis[int(item->parenth3())]);
    }

    // suffix
    // append only if non-combining shape or cannot combine (no digit or parenthesis in between)
    if (item->suffix() != FiguredBassItem::Modifier::NONE
        && ((item->suffix() != FiguredBassItem::Modifier::CROSS
             && item->suffix() != FiguredBassItem::Modifier::BACKSLASH
             && item->suffix() != FiguredBassItem::Modifier::SLASH)
            || item->digit() == FBIDigitNone
            || item->parenth3() != FiguredBassItem::Parenthesis::NONE)) {
        str.append(FiguredBass::FBFonts().at(font).displayAccidental[int(item->suffix())]);
    }

    if (item->parenth4() != FiguredBassItem::Parenthesis::NONE) {
        str.append(FiguredBass::FBFonts().at(font).displayParenthesis[int(item->parenth4())]);
    }

    //! NOTE Look like, this text not layout data (not depends by position)
    ldata->displayText = str;             // this text will be displayed

    if (str.size()) {                     // if some text
        x = x - (x1 + x2) * 0.5;          // position the text so that [x1<-->x2] is centered below the note
    } else {                              // if no text (but possibly a line)
        x = 0;                            // start at note left margin
    }
    // vertical position
    h = fm.lineSpacing();
    h *= ctx.conf().styleD(Sid::figuredBassLineHeight);
    if (ctx.conf().styleI(Sid::figuredBassAlignment) == 0) {          // top alignment: stack down from first item
        y = h * item->ord();
    } else {                                                      // bottom alignment: stack up from last item
        y = -h * (item->figuredBass()->itemsCount() - item->ord());
    }
    ldata->setPos(x, y);
    // determine bbox from text width
//      w = fm.width(str);
    w = fm.width(str);
    ldata->textWidth = w;
    // if there is a cont.line, extend width to cover the whole FB element duration line
    int lineLen;
    if (item->contLine() != FiguredBassItem::ContLine::NONE
        && (lineLen = item->figuredBass()->layoutData()->lineLength(0)) > w) {
        w = lineLen;
    }
    ldata->setBbox(0, 0, w, h);
}

void TLayout::layout(FiguredBassItem* item, LayoutContext& ctx)
{
    layoutFiguredBassItem(item, ctx, item->mutLayoutData());
}

//    lays out the duration indicator line(s), filling the _lineLengths array
//    and the length of printed lines (used by continuation lines)

static void layoutLines(const FiguredBass* item, const LayoutContext& ctx, FiguredBass::LayoutData* ldata)
{
    std::vector<double> lineLengths = ldata->lineLengths;
    if (item->ticks() <= Fraction(0, 1) || !item->segment()) {
        lineLengths.resize(1);                             // be sure to always have
        lineLengths[0] = 0;                                // at least 1 item in array
        ldata->lineLengths = lineLengths;
        return;
    }

    ChordRest* lastCR  = nullptr;                         // the last ChordRest of this
    Segment* nextSegm = nullptr;                          // the Segment beyond this' segment
    Fraction nextTick = item->segment()->tick() + item->ticks();       // the tick beyond this' duration

    // locate the measure containing the last tick of this; it is either:
    // the same measure containing nextTick, if nextTick is not the first tick of a measure
    //    (and line should stop right before it)
    // or the previous measure, if nextTick is the first tick of a measure
    //    (and line should stop before any measure terminal segment (bar, clef, ...) )

    const Measure* m = ctx.dom().tick2measure(nextTick - Fraction::fromTicks(1));
    if (m) {
        // locate the first segment (of ANY type) right after this' last tick
        for (nextSegm = m->first(SegmentType::All); nextSegm; nextSegm = nextSegm->next()) {
            if (nextSegm->tick() >= nextTick) {
                break;
            }
        }
        // locate the last ChordRest of this
        if (nextSegm) {
            track_idx_t startTrack = trackZeroVoice(item->track());
            track_idx_t endTrack = startTrack + VOICES;
            for (const Segment* seg = nextSegm->prev1(); seg; seg = seg->prev1()) {
                for (track_idx_t t = startTrack; t < endTrack; ++t) {
                    EngravingItem* el = seg->element(t);
                    if (el && el->isChordRest()) {
                        lastCR = toChordRest(el);
                        break;
                    }
                }
                if (lastCR) {
                    break;
                }
            }
        }
    }
    if (!m || !nextSegm) {
        LOGD("FiguredBass layout: no segment found for tick %d", nextTick.ticks());
        lineLengths.resize(1);                             // be sure to always have
        lineLengths[0] = 0;                                // at least 1 item in array
        ldata->lineLengths = lineLengths;
        return;
    }

    // get length of printed lines from horiz. page position of lastCR
    // (enter a bit 'into' the ChordRest for clarity)
    double printedLineLength = lastCR ? lastCR->pageX() - item->pageX() + 1.5 * item->spatium() : 3 * item->spatium();
    ldata->printedLineLength = printedLineLength;

    // get duration indicator line(s) from page position of nextSegm
    const std::vector<System*>& systems = ctx.dom().systems();
    System* s1  = item->segment()->measure()->system();
    System* s2  = nextSegm->measure()->system();
    system_idx_t sysIdx1 = mu::indexOf(systems, s1);
    system_idx_t sysIdx2 = mu::indexOf(systems, s2);

    if (sysIdx2 == mu::nidx || sysIdx2 < sysIdx1) {
        sysIdx2 = sysIdx1;
        nextSegm = item->segment()->next1();
        // TODO
        // During layout of figured bass next systems' numbers may be still
        // undefined (then sysIdx2 == mu::nidx) or change in the future.
        // A layoutSystem() approach similar to that for spanners should
        // probably be implemented.
    }

    system_idx_t i;
    int len;
    size_t segIdx = 0;
    for (i = sysIdx1, segIdx = 0; i <= sysIdx2; ++i, ++segIdx) {
        len = 0;
        if (sysIdx1 == sysIdx2 || i == sysIdx1) {
            // single line
            len = nextSegm->pageX() - item->pageX() - 4;               // stop 4 raster units before next segm
        } else if (i == sysIdx1) {
            // initial line
            double w   = s1->staff(item->staffIdx())->bbox().right();
            double x   = s1->pageX() + w;
            len = x - item->pageX();
        } else if (i > 0 && i != sysIdx2) {
            // middle line
            LOGD("FiguredBass: duration indicator middle line not implemented");
        } else if (i == sysIdx2) {
            // end line
            LOGD("FiguredBass: duration indicator end line not implemented");
        }
        // store length item, reusing array items if already present
        if (lineLengths.size() <= segIdx) {
            lineLengths.push_back(len);
        } else {
            lineLengths[segIdx] = len;
        }
    }
    // if more array items than needed, truncate array
    if (lineLengths.size() > segIdx) {
        lineLengths.resize(segIdx);
    }

    ldata->lineLengths = lineLengths;
}

static void layoutFiguredBass(const FiguredBass* item, const LayoutContext& ctx, FiguredBass::LayoutData* ldata)
{
    // VERTICAL POSITION:
    const double y = ctx.conf().styleD(Sid::figuredBassYOffset) * item->spatium();
    ldata->setPos(PointF(0.0, y));

    // BOUNDING BOX and individual item layout (if required)
    TLayout::layout1TextBase(item, ctx, ldata);  // prepare structs and data expected by Text methods
    // if element could be parsed into items, layout each element
    // Items list will be empty in edit mode (see FiguredBass::startEdit).
    // TODO: consider disabling specific layout in case text style is changed (tid() != TextStyleName::FIGURED_BASS).
    if (item->items().size() > 0) {
        layoutLines(item, ctx, ldata);
        ldata->setBbox(0, 0, ldata->lineLength(0), 0);
        // layout each item and enlarge bbox to include items bboxes
        for (FiguredBassItem* fit : item->items()) {
            FiguredBassItem::LayoutData* fildata = fit->mutLayoutData();
            layoutFiguredBassItem(fit, ctx, fildata);
            ldata->addBbox(fildata->bbox().translated(fit->pos()));
        }
    }
}

void TLayout::layout(FiguredBass* item, LayoutContext& ctx)
{
    layoutFiguredBass(item, ctx, item->mutLayoutData());
}

static void layoutFingering(const Fingering* item, const LayoutContext& ctx, Fingering::LayoutData* ldata)
{
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    Fraction tick = item->parentItem()->tick();
    const Staff* st = item->staff();

    if (st && st->isTabStaff(tick)
        && (!st->staffType(tick)->showTabFingering() || item->textStyleType() == TextStyleType::STRING_NUMBER)) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    TLayout::layoutTextBase(item, ctx, ldata);
    ldata->setPosY(0.0); // handle placement below

    if (item->autoplace() && item->note()) {
        Note* n      = item->note();
        Chord* chord = n->chord();
        bool voices  = chord->measure()->hasVoices(chord->staffIdx(), chord->tick(), chord->actualTicks());
        bool tight   = voices && chord->notes().size() == 1 && !chord->beam() && item->textStyleType() != TextStyleType::STRING_NUMBER;

        double headWidth = n->bboxRightPos();

        // update offset after drag
        double rebase = 0.0;
        if (ldata->offsetChanged() != OffsetChange::NONE && !tight) {
            rebase = Autoplace::rebaseOffset(item, ldata);
        }

        // temporarily exclude self from chord shape
        const_cast<Fingering*>(item)->setAutoplace(false);

        if (item->layoutType() == ElementType::CHORD) {
            bool above = item->placeAbove();
            Stem* stem = chord->stem();
            Segment* s = chord->segment();
            Measure* m = s->measure();
            double sp = item->spatium();
            double md = item->minDistance().val() * sp;
            SysStaff* ss = m->system()->staff(chord->vStaffIdx());
            Staff* vStaff = chord->staff();           // TODO: use current height at tick

            if (n->mirror()) {
                ldata->moveX(-n->layoutData()->pos().x());
            }
            ldata->moveX(headWidth * .5);
            if (above) {
                if (tight) {
                    if (chord->stem()) {
                        ldata->moveX(-0.8 * sp);
                    }
                    ldata->moveY(-1.5 * sp);
                } else {
                    RectF r = ldata->bbox().translated(m->pos() + s->pos() + chord->pos() + n->pos() + item->pos());
                    SkylineLine sk(false);
                    sk.add(r.x(), r.bottom(), r.width());
                    double d = sk.minDistance(ss->skyline().north());
                    double yd = 0.0;
                    if (d > 0.0 && item->isStyled(Pid::MIN_DISTANCE)) {
                        yd -= d + item->height() * .25;
                    }
                    // force extra space above staff & chord (but not other fingerings)
                    double top;
                    if (chord->up() && chord->beam() && stem) {
                        top = stem->y() + stem->layoutData()->bbox().top();
                    } else {
                        Note* un = chord->upNote();
                        top = std::min(0.0, un->y() + un->layoutData()->bbox().top());
                    }
                    top -= md;
                    double diff = (ldata->bbox().bottom() + ldata->pos().y() + yd + n->y()) - top;
                    if (diff > 0.0) {
                        yd -= diff;
                    }
                    if (ldata->offsetChanged() != OffsetChange::NONE) {
                        // user moved element within the skyline
                        // we may need to adjust minDistance, yd, and/or offset
                        bool inStaff = above ? r.bottom() + rebase > 0.0 : r.top() + rebase < item->staff()->height();
                        Autoplace::rebaseMinDistance(item, ldata, md, yd, sp, rebase, above, inStaff);
                    }
                    ldata->moveY(yd);
                }
            } else {
                if (tight) {
                    if (chord->stem()) {
                        ldata->moveX(0.8 * sp);
                    }
                    ldata->moveY(1.5 * sp);
                } else {
                    RectF r = ldata->bbox().translated(m->pos() + s->pos() + chord->pos() + n->pos() + item->pos());
                    SkylineLine sk(true);
                    sk.add(r.x(), r.top(), r.width());
                    double d = ss->skyline().south().minDistance(sk);
                    double yd = 0.0;
                    if (d > 0.0 && item->isStyled(Pid::MIN_DISTANCE)) {
                        yd += d + item->height() * .25;
                    }
                    // force extra space below staff & chord (but not other fingerings)
                    double bottom;
                    if (!chord->up() && chord->beam() && stem) {
                        bottom = stem->y() + stem->layoutData()->bbox().bottom();
                    } else {
                        Note* dn = chord->downNote();
                        bottom = std::max(vStaff->height(), dn->y() + dn->layoutData()->bbox().bottom());
                    }
                    bottom += md;
                    double diff = bottom - (ldata->bbox().top() + ldata->pos().y() + yd + n->y());
                    if (diff > 0.0) {
                        yd += diff;
                    }
                    if (ldata->offsetChanged() != OffsetChange::NONE) {
                        // user moved element within the skyline
                        // we may need to adjust minDistance, yd, and/or offset
                        bool inStaff = above ? r.bottom() + rebase > 0.0 : r.top() + rebase < item->staff()->height();
                        Autoplace::rebaseMinDistance(item, ldata, md, yd, sp, rebase, above, inStaff);
                    }
                    ldata->moveY(yd);
                }
            }
        } else if (item->textStyleType() == TextStyleType::LH_GUITAR_FINGERING) {
            // place to left of note
            double left = n->shape().left();
            if (left - n->x() > 0.0) {
                ldata->moveX(-left);
            } else {
                ldata->moveX(-n->x());
            }
        }
        // for other fingering styles, do not autoplace

        // restore autoplace
        const_cast<Fingering*>(item)->setAutoplace(true);
    } else if (ldata->offsetChanged() != OffsetChange::NONE) {
        // rebase horizontally too, as autoplace may have adjusted it
        Autoplace::rebaseOffset(item, ldata, false);
    }
    Autoplace::setOffsetChanged(item, ldata, false);
}

void TLayout::layout(Fingering* item, LayoutContext& ctx)
{
    layoutFingering(item, ctx, item->mutLayoutData());
}

static void layoutFretDiagram(const FretDiagram* item, const LayoutContext& ctx, FretDiagram::LayoutData* ldata)
{
    double _spatium  = item->spatium() * item->userMag();
    ldata->stringLw = _spatium * 0.08;
    ldata->nutLw = ((item->fretOffset() || !item->showNut()) ? ldata->stringLw : _spatium * 0.2);
    ldata->stringDist = ctx.conf().styleMM(Sid::fretStringSpacing) * item->userMag();
    ldata->fretDist = ctx.conf().styleMM(Sid::fretFretSpacing) * item->userMag();
    ldata->markerSize = ldata->stringDist * .8;

    double w = ldata->stringDist * (item->strings() - 1) + ldata->markerSize;
    double h = (item->frets() + 1) * ldata->fretDist + ldata->markerSize;
    double y = -(ldata->markerSize * .5 + ldata->fretDist);
    double x = -(ldata->markerSize * .5);

    // Allocate space for fret offset number
    if (item->fretOffset() > 0) {
        mu::draw::Font scaledFont(item->font());
        scaledFont.setPointSizeF(item->font().pointSizeF() * item->userMag());

        double fretNumMag = ctx.conf().styleD(Sid::fretNumMag);
        scaledFont.setPointSizeF(scaledFont.pointSizeF() * fretNumMag);
        mu::draw::FontMetrics fm2(scaledFont);
        double numw = fm2.width(String::number(item->fretOffset() + 1));
        double xdiff = numw + ldata->stringDist * .4;
        w += xdiff;
        x += (item->numPos() == 0) == (item->orientation() == Orientation::VERTICAL) ? -xdiff : 0;
    }

    if (item->orientation() == Orientation::HORIZONTAL) {
        double tempW = w,
               tempX = x;
        w = h;
        h = tempW;
        x = y;
        y = tempX;
    }

    // When changing how bbox is calculated, don't forget to update the centerX and rightX methods too.
    ldata->setBbox(x, y, w, h);

    if (!item->explicitParent()->isSegment()) {
        ldata->setPos(PointF());
        return;
    }

    // We need to get the width of the notehead/rest in order to position the fret diagram correctly
    Segment* pSeg = toSegment(item->explicitParent());
    double noteheadWidth = 0;
    if (pSeg->isChordRestType()) {
        staff_idx_t idx = item->staff()->idx();
        for (EngravingItem* e = pSeg->firstElementOfSegment(pSeg, idx); e; e = pSeg->nextElementOfSegment(pSeg, e, idx)) {
            if (e->isRest()) {
                Rest* r = toRest(e);
                noteheadWidth = item->symWidth(r->layoutData()->sym);
                break;
            } else if (e->isNote()) {
                Note* n = toNote(e);
                noteheadWidth = n->headWidth();
                break;
            }
        }
    }

    double mainWidth = 0.0;
    if (item->orientation() == Orientation::VERTICAL) {
        mainWidth = ldata->stringDist * (item->strings() - 1);
    } else if (item->orientation() == Orientation::HORIZONTAL) {
        mainWidth = ldata->fretDist * (item->frets() + 0.5);
    }
    ldata->setPos((noteheadWidth - mainWidth) / 2, -(h + item->styleP(Sid::fretY)));

    Autoplace::autoplaceSegmentElement(item, ldata);

    // don't display harmony in palette
    if (!item->explicitParent()) {
        return;
    }

    Harmony* harmony = item->harmony();
    if (harmony) {
        TLayout::layout(harmony, const_cast<LayoutContext&>(ctx));
    }

    if (harmony && harmony->autoplace() && harmony->explicitParent()) {
        Segment* s = toSegment(item->explicitParent());
        Measure* m = s->measure();
        staff_idx_t si = item->staffIdx();

        SysStaff* ss = m->system()->staff(si);
        RectF r = harmony->layoutData()->bbox().translated(m->pos() + s->pos() + item->pos() + harmony->pos());

        double minDistance = harmony->minDistance().val() * item->spatium();
        SkylineLine sk(false);
        sk.add(r.x(), r.bottom(), r.width());
        double d = sk.minDistance(ss->skyline().north());
        if (d > -minDistance) {
            double yd = d + minDistance;
            yd *= -1.0;
            harmony->mutLayoutData()->moveY(yd);
            r.translate(PointF(0.0, yd));
        }
        if (harmony->addToSkyline()) {
            ss->skyline().add(r);
        }
    }
}

void TLayout::layout(FretDiagram* item, LayoutContext& ctx)
{
    layoutFretDiagram(item, ctx, item->mutLayoutData());
}

static void layoutFretCircle(const FretCircle* item, const LayoutContext&, FretCircle::LayoutData* ldata)
{
    if (!item->tabEllipseEnabled()) {
        ldata->setIsSkipDraw(true);
        ldata->setBbox(RectF());
        return;
    }
    ldata->setIsSkipDraw(false);

    double lw = item->spatium() * FretCircle::CIRCLE_WIDTH / 2;
    ldata->rect = item->ellipseRect();

    RectF chordRect;
    double minWidth = item->chord()->upNote()->width();
    for (const Note* note : item->chord()->notes()) {
        chordRect |= note->layoutData()->bbox();
        minWidth = std::min(minWidth, note->width());
    }

    double offsetFromUpNote = (ldata->rect.height() - chordRect.height()
                               - (item->chord()->downNote()->pos().y() - item->chord()->upNote()->pos().y())
                               ) / 2;
    ldata->offsetFromUpNote = offsetFromUpNote;
    ldata->sideOffset = ((ldata->rect.width() - minWidth) / 2);

    ldata->setBbox(ldata->rect.adjusted(-lw, -lw, lw, lw));
}

void TLayout::layout(FretCircle* item, LayoutContext& ctx)
{
    layoutFretCircle(item, ctx, item->mutLayoutData());
}

static void layoutGlissando(const Glissando* item, LayoutContext& ctx, Glissando::LayoutData* ldata)
{
    double _spatium = item->spatium();

    TLayout::layoutLine(const_cast<Glissando*>(item), ctx);

    if (item->spannerSegments().empty()) {
        LOGD("no segments");
        return;
    }
    ldata->setPos(0.0, 0.0);

    Note* anchor1 = toNote(item->startElement());
    Note* anchor2 = toNote(item->endElement());
    Chord* cr1 = anchor1->chord();
    Chord* cr2 = anchor2->chord();
    GlissandoSegment* segm1 = toGlissandoSegment(const_cast<Glissando*>(item)->frontSegment());
    GlissandoSegment* segm2 = toGlissandoSegment(const_cast<Glissando*>(item)->backSegment());

    // Note: line segments are defined by
    // initial point: ipos() (relative to system origin)
    // ending point:  pos2() (relative to initial point)

    // LINE ENDING POINTS TO NOTEHEAD CENTRES

    // assume gliss. line goes from centre of initial note centre to centre of ending note:
    // move first segment origin and last segment ending point from notehead origin to notehead centre
    // For TAB: begin at the right-edge of initial note rather than centre
    PointF offs1 = (cr1->staff()->isTabStaff(cr1->tick()))
                   ? PointF(anchor1->layoutData()->bbox().right(), 0.0)
                   : PointF(anchor1->headWidth() * 0.5, 0.0);

    PointF offs2 = PointF(anchor2->headWidth() * 0.5, 0.0);

    // AVOID HORIZONTAL LINES

    // for microtonality read tuning, or check note accidental
    double tune1 = anchor1->tuning();
    double tune2 = anchor2->tuning();
    AccidentalType acc1 = anchor1->accidentalType();
    AccidentalType acc2 = anchor2->accidentalType();
    if (RealIsNull(tune1) && Accidental::isMicrotonal(acc1)) {
        tune1 = Accidental::subtype2centOffset(acc1);
    }
    if (RealIsNull(tune2) && Accidental::isMicrotonal(acc2)) {
        tune2 = Accidental::subtype2centOffset(acc2);
    }

    int upDown = (0 < (anchor2->ppitch() - anchor1->ppitch())) - ((anchor2->ppitch() - anchor1->ppitch()) < 0);
    // same note, so compare tunings
    if (upDown == 0) {
        upDown = (0 < (tune2 - tune1)) - ((tune2 - tune1) < 0);
    }

    // on TAB's, glissando are by necessity on the same string, this gives an horizontal glissando line;
    // make bottom end point lower and top ending point higher
    if (cr1->staff()->isTabStaff(cr1->tick())) {
        double yOff = cr1->staff()->lineDistance(cr1->tick()) * 0.4 * _spatium;
        offs1.ry() += yOff * upDown;
        offs2.ry() -= yOff * upDown;
    }
    // if not TAB, angle glissando between notes on the same line
    else {
        if (anchor1->line() == anchor2->line()) {
            offs1.ry() += _spatium * 0.25 * upDown;
            offs2.ry() -= _spatium * 0.25 * upDown;
        }
    }

    // move initial point of first segment and adjust its length accordingly
    segm1->setPos(segm1->layoutData()->pos() + offs1);
    segm1->setPos2(segm1->ipos2() - offs1);
    // adjust ending point of last segment
    segm2->setPos2(segm2->ipos2() + offs2);

    // INTERPOLATION OF INTERMEDIATE POINTS
    // This probably belongs to SLine class itself; currently it does not seem
    // to be needed for anything else than Glissando, though

    // get total x-width and total y-height of all segments
    double xTot = 0.0;
    for (SpannerSegment* segm : item->spannerSegments()) {
        xTot += segm->ipos2().x();
    }
    double y0   = segm1->layoutData()->pos().y();
    double yTot = segm2->layoutData()->pos().y() + segm2->ipos2().y() - y0;
    yTot -= yStaffDifference(segm2->system(), segm2->staffIdx(), segm1->system(), segm1->staffIdx());
    double ratio = yTot / xTot;
    // interpolate y-coord of intermediate points across total width and height
    double xCurr = 0.0;
    double yCurr;
    for (unsigned i = 0; i + 1 < item->spannerSegments().size(); i++) {
        SpannerSegment* segm = const_cast<Glissando*>(item)->segmentAt(i);
        xCurr += segm->ipos2().x();
        yCurr = y0 + ratio * xCurr;
        segm->rypos2() = yCurr - segm->layoutData()->pos().y();           // position segm. end point at yCurr
        // next segment shall start where this segment stopped, corrected for the staff y-difference
        SpannerSegment* nextSeg = const_cast<Glissando*>(item)->segmentAt(i + 1);
        yCurr += yStaffDifference(nextSeg->system(), nextSeg->staffIdx(), segm->system(), segm->staffIdx());
        segm = nextSeg;
        segm->rypos2() += segm->layoutData()->pos().y() - yCurr;          // adjust next segm. vertical length
        segm->mutLayoutData()->setPosY(yCurr);                                // position next segm. start point at yCurr
    }

    // KEEP CLEAR OF ALL ELEMENTS OF THE CHORD
    // Remove offset already applied
    offs1 *= -1.0;
    offs2 *= -1.0;
    // Look at chord shapes (but don't consider lyrics)
    Shape cr1shape = cr1->shape();
    mu::remove_if(cr1shape, [](ShapeElement& s) {
        if (!s.toItem || s.toItem->isLyrics()) {
            return true;
        } else {
            return false;
        }
    });
    offs1.rx() += cr1shape.right() - anchor1->pos().x();
    if (!cr2->staff()->isTabStaff(cr2->tick())) {
        offs2.rx() -= cr2->shape().left() + anchor2->pos().x();
    }
    // Add note distance
    const double glissNoteDist = 0.25 * item->spatium(); // TODO: style
    offs1.rx() += glissNoteDist;
    offs2.rx() -= glissNoteDist;

    // apply offsets: shorten first segment by x1 (and proportionally y) and adjust its length accordingly
    offs1.ry() = segm1->ipos2().y() * offs1.x() / segm1->ipos2().x();
    segm1->setPos(segm1->layoutData()->pos() + offs1);
    segm1->setPos2(segm1->ipos2() - offs1);
    // adjust last segment length by x2 (and proportionally y)
    offs2.ry() = segm2->ipos2().y() * offs2.x() / segm2->ipos2().x();
    segm2->setPos2(segm2->ipos2() + offs2);

    for (SpannerSegment* segm : item->spannerSegments()) {
        TLayout::layoutItem(segm, ctx);
    }

    // compute glissando bbox as the bbox of the last segment, relative to the end anchor note
    PointF anchor2PagePos = anchor2->pagePos();
    PointF system2PagePos;
    IF_ASSERT_FAILED(cr2->segment()->system()) {
        system2PagePos = segm2->pos();
    } else {
        system2PagePos = cr2->segment()->system()->pagePos();
    }

    PointF anchor2SystPos = anchor2PagePos - system2PagePos;
    RectF r = RectF(anchor2SystPos - segm2->pos(), anchor2SystPos - segm2->pos() - segm2->pos2()).normalized();
    double lw = item->lineWidth() * .5;
    ldata->setBbox(r.adjusted(-lw, -lw, lw, lw));

    const_cast<Glissando*>(item)->addLineAttachPoints();
}

void TLayout::layout(Glissando* item, LayoutContext& ctx)
{
    layoutGlissando(item, ctx, item->mutLayoutData());
}

void TLayout::layout(GlissandoSegment* item, LayoutContext&)
{
    GlissandoSegment::LayoutData* ldata = item->mutLayoutData();
    if (item->pos2().x() <= 0) {
        ldata->setBbox(RectF());
        return;
    }

    if (item->staff()) {
        ldata->setMag(item->staff()->staffMag(item->tick()));
    }
    RectF r = RectF(0.0, 0.0, item->pos2().x(), item->pos2().y()).normalized();
    double lw = item->glissando()->lineWidth() * .5;
    item->setbbox(r.adjusted(-lw, -lw, lw, lw));
}

void TLayout::layout(GraceNotesGroup* item, LayoutContext& ctx)
{
    Shape _shape;
    for (size_t i = item->size() - 1; i != mu::nidx; --i) {
        Chord* grace = item->at(i);
        Shape graceShape = grace->shape();
        Shape groupShape = _shape;
        mu::remove_if(groupShape, [grace](ShapeElement& s) {
            if (!s.toItem || (s.toItem->isStem() && s.toItem->vStaffIdx() != grace->vStaffIdx())) {
                return true;
            }
            return false;
        });
        double offset;
        offset = -std::max(graceShape.minHorizontalDistance(groupShape), 0.0);
        // Adjust spacing for cross-beam situations
        if (i < item->size() - 1) {
            Chord* prevGrace = item->at(i + 1);
            if (prevGrace->up() != grace->up()) {
                double crossCorrection = grace->notes().front()->headWidth() - grace->stem()->width();
                if (prevGrace->up() && !grace->up()) {
                    offset += crossCorrection;
                } else {
                    offset -= crossCorrection;
                }
            }
        }
        _shape.add(graceShape.translated(mu::PointF(offset, 0.0)));
        double xpos = offset - item->parent()->rxoffset() - item->parent()->layoutData()->pos().x();
        grace->setPos(xpos, 0.0);
    }
    double xPos = -_shape.minHorizontalDistance(item->appendedSegment()->staffShape(item->parent()->staffIdx()));
    // If the parent chord is cross-staff, also check against shape in the other staff and take the minimum
    if (item->parent()->staffMove() != 0) {
        double xPosCross = -_shape.minHorizontalDistance(item->appendedSegment()->staffShape(item->parent()->vStaffIdx()));
        xPos = std::min(xPos, xPosCross);
    }
    // Same if the grace note itself is cross-staff
    Chord* firstGN = item->back();
    if (firstGN->staffMove() != 0) {
        double xPosCross = -_shape.minHorizontalDistance(item->appendedSegment()->staffShape(firstGN->vStaffIdx()));
        xPos = std::min(xPos, xPosCross);
    }
    // Safety net in case the shape checks don't succeed
    xPos = std::min(xPos, -double(ctx.conf().styleMM(Sid::graceToMainNoteDist) + firstGN->notes().front()->headWidth() / 2));
    item->setPos(xPos, 0.0);
}

void TLayout::layout(GradualTempoChangeSegment* item, LayoutContext& ctx)
{
    GradualTempoChangeSegment::LayoutData* ldata = item->mutLayoutData();
    layoutTextLineBaseSegment(item, ctx);

    if (item->isStyled(Pid::OFFSET)) {
        item->roffset() = item->tempoChange()->propertyDefault(Pid::OFFSET).value<PointF>();
    }
    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layout(GradualTempoChange* item, LayoutContext& ctx)
{
    layoutLine(item, ctx);
}

void TLayout::layout(HairpinSegment* item, LayoutContext& ctx)
{
    HairpinSegment::LayoutData* ldata = item->mutLayoutData();

    const StaffType* stType = item->staffType();

    if (stType && stType->isHiddenElementOnTab(ctx.conf().style(), Sid::hairpinShowTabCommon, Sid::hairpinShowTabSimple)) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    const double _spatium = item->spatium();
    const track_idx_t _trck = item->track();
    Dynamic* sd = nullptr;
    Dynamic* ed = nullptr;
    double dymax = item->hairpin()->placeBelow() ? -10000.0 : 10000.0;
    if (item->autoplace() && !ctx.conf().isPaletteMode()
        && item->explicitParent() // TODO: remove this line (this might happen when Ctrl+Shift+Dragging an item)
        ) {
        Segment* start = item->hairpin()->startSegment();
        Segment* end = item->hairpin()->endSegment();
        // Try to fit between adjacent dynamics
        double minDynamicsDistance = ctx.conf().styleMM(Sid::autoplaceHairpinDynamicsDistance) * item->staff()->staffMag(item->tick());
        const System* sys = item->system();
        if (item->isSingleType() || item->isBeginType()) {
            if (start && start->system() == sys) {
                sd = toDynamic(start->findAnnotation(ElementType::DYNAMIC, _trck, _trck));
                if (!sd) {
                    // Dynamics might have been added to the previous
                    // segment rather than exactly to hairpin start,
                    // search in that segment too.
                    start = start->prev(SegmentType::ChordRest);
                    if (start && start->system() == sys) {
                        sd = toDynamic(start->findAnnotation(ElementType::DYNAMIC, _trck, _trck));
                    }
                }
            }
            if (sd && sd->addToSkyline() && sd->placement() == item->hairpin()->placement()) {
                double segmentXPos = sd->segment()->pos().x() + sd->measure()->pos().x();
                double sdRight = sd->pos().x() + segmentXPos + sd->layoutData()->bbox().right();
                if (sd->snappedExpression()) {
                    Expression* expression = sd->snappedExpression();
                    double exprRight = expression->pos().x() + segmentXPos + expression->layoutData()->bbox().right();
                    sdRight = std::max(sdRight, exprRight);
                }
                const double dist    = std::max(sdRight - item->pos().x() + minDynamicsDistance, 0.0);
                ldata->moveX(dist);
                item->rxpos2() -= dist;
                // prepare to align vertically
                dymax = sd->pos().y();
            }
        }
        if (item->isSingleType() || item->isEndType()) {
            if (end && end->tick() < sys->endTick() && start != end) {
                // checking ticks rather than systems
                // systems may be unknown at layout stage.
                ed = toDynamic(end->findAnnotation(ElementType::DYNAMIC, _trck, _trck));
            }
            if (ed && ed->addToSkyline() && ed->placement() == item->hairpin()->placement()) {
                const double edLeft = ed->layoutData()->bbox().left() + ed->pos().x()
                                      + ed->segment()->pos().x() + ed->measure()->pos().x();
                const double dist = edLeft - item->pos2().x() - item->pos().x() - minDynamicsDistance;
                const double extendThreshold = 3.0 * _spatium;           // TODO: style setting
                if (dist < 0.0) {
                    item->rxpos2() += dist;                 // always shorten
                } else if (dist >= extendThreshold && item->hairpin()->endText().isEmpty() && minDynamicsDistance > 0.0) {
                    item->rxpos2() += dist;                 // lengthen only if appropriate
                }
                // prepare to align vertically
                if (item->hairpin()->placeBelow()) {
                    dymax = std::max(dymax, ed->pos().y());
                } else {
                    dymax = std::min(dymax, ed->pos().y());
                }
            }
        }
    }

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
            x1 = item->text()->width() + _spatium * .5;
        }

        Transform t;
        double h1 = item->hairpin()->hairpinHeight().val() * _spatium * .5;
        double h2 = item->hairpin()->hairpinContHeight().val() * _spatium * .5;

        double x = item->pos2().x();
        if (!item->endText()->empty()) {
            x -= (item->endText()->width() + _spatium * .5);             // 0.5 spatium distance
        }
        if (x < _spatium) {               // minimum size of hairpin
            x = _spatium;
        }
        double y = item->pos2().y();
        double len = sqrt(x * x + y * y);
        t.rotateRadians(asin(y / len));

        item->setDrawCircledTip(item->hairpin()->hairpinCircledTip());
        item->setCircledTipRadius(item->drawCircledTip() ? 0.6 * _spatium * .5 : 0.0);

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

        item->joinedHairpinRef().clear();
        if (item->spannerSegmentType() != SpannerSegmentType::MIDDLE) {
            if (type == HairpinType::DECRESC_HAIRPIN && item->spannerSegmentType() != SpannerSegmentType::BEGIN) {
                item->joinedHairpinRef() << item->pointsRef()[0] << item->pointsRef()[1] << item->pointsRef()[2]; // [top-left, joint, bottom-left]
            } else if (type == HairpinType::CRESC_HAIRPIN && item->spannerSegmentType() != SpannerSegmentType::END) {
                item->joinedHairpinRef() << item->pointsRef()[1] << item->pointsRef()[0] << item->pointsRef()[3]; // [top-right, joint, bottom-right]
            }
        }

        RectF r = RectF(l1.p1(), l1.p2()).normalized().united(RectF(l2.p1(), l2.p2()).normalized());
        if (!item->text()->empty()) {
            r.unite(item->text()->layoutData()->bbox());
        }
        if (!item->endText()->empty()) {
            r.unite(item->endText()->layoutData()->bbox().translated(x + item->endText()->layoutData()->bbox().width(), 0.0));
        }
        double w  = item->point(ctx.conf().styleS(Sid::hairpinLineWidth));
        item->setbbox(r.adjusted(-w * .5, -w * .5, w, w));
    }

    if (!item->explicitParent()) {
        item->setPos(PointF());
        item->roffset() = PointF();
        return;
    }

    if (item->isStyled(Pid::OFFSET)) {
        item->roffset() = item->hairpin()->propertyDefault(Pid::OFFSET).value<PointF>();
    }

    // rebase vertical offset on drag
    double rebase = 0.0;
    if (ldata->offsetChanged() != OffsetChange::NONE) {
        rebase = Autoplace::rebaseOffset(item, ldata);
    }

    if (item->autoplace()) {
        double ymax = item->pos().y();
        double d;
        double ddiff = item->hairpin()->isLineType() ? 0.0 : _spatium * 0.5;

        double sp = item->spatium();

        // TODO: in the future, there should be a minDistance style setting for hairpinLines as well as hairpins.
        double minDist = item->twoLines() ? item->minDistance().val() : ctx.conf().styleS(Sid::dynamicsMinDistance).val();
        double md = minDist * sp;

        bool above = item->spanner()->placeAbove();
        SkylineLine sl(!above);
        Shape sh = item->shape();
        sl.add(sh.translated(item->pos()));
        if (above) {
            d  = item->system()->topDistance(item->staffIdx(), sl);
            if (d > -md) {
                ymax -= d + md;
            }
            // align hairpin with dynamics
            if (!item->hairpin()->diagonal()) {
                ymax = std::min(ymax, dymax - ddiff);
            }
        } else {
            d  = item->system()->bottomDistance(item->staffIdx(), sl);
            if (d > -md) {
                ymax += d + md;
            }
            // align hairpin with dynamics
            if (!item->hairpin()->diagonal()) {
                ymax = std::max(ymax, dymax - ddiff);
            }
        }
        double yd = ymax - item->pos().y();
        if (yd != 0.0) {
            if (ldata->offsetChanged() != OffsetChange::NONE) {
                // user moved element within the skyline
                // we may need to adjust minDistance, yd, and/or offset
                double adj = item->pos().y() + rebase;
                bool inStaff = above ? sh.bottom() + adj > 0.0 : sh.top() + adj < item->staff()->height();
                Autoplace::rebaseMinDistance(item, ldata, md, yd, sp, rebase, above, inStaff);
            }
            ldata->moveY(yd);
        }

        if (item->hairpin()->addToSkyline() && !item->hairpin()->diagonal()) {
            // align dynamics with hairpin
            if (sd && sd->autoplace() && sd->placement() == item->hairpin()->placement()) {
                double ny = item->y() + ddiff - sd->offset().y();
                if (sd->placeAbove()) {
                    ny = std::min(ny, sd->layoutData()->pos().y());
                } else {
                    ny = std::max(ny, sd->layoutData()->pos().y());
                }
                if (sd->layoutData()->pos().y() != ny) {
                    sd->mutLayoutData()->setPosY(ny);
                    if (sd->snappedExpression()) {
                        sd->snappedExpression()->mutLayoutData()->setPosY(ny);
                    }
                    if (sd->addToSkyline()) {
                        Segment* s = sd->segment();
                        Measure* m = s->measure();
                        RectF r = sd->layoutData()->bbox().translated(sd->pos());
                        s->staffShape(sd->staffIdx()).add(r);
                        r = sd->layoutData()->bbox().translated(sd->pos() + s->pos() + m->pos());
                        m->system()->staff(sd->staffIdx())->skyline().add(r);
                    }
                }
            }
            if (ed && ed->autoplace() && ed->placement() == item->hairpin()->placement()) {
                double ny = item->y() + ddiff - ed->offset().y();
                if (ed->placeAbove()) {
                    ny = std::min(ny, ed->layoutData()->pos().y());
                } else {
                    ny = std::max(ny, ed->layoutData()->pos().y());
                }
                if (ed->layoutData()->pos().y() != ny) {
                    ed->mutLayoutData()->setPosY(ny);
                    Expression* snappedExpression = ed->snappedExpression();
                    if (snappedExpression) {
                        double yOffsetDiff = snappedExpression->offset().y() - ed->offset().y();
                        snappedExpression->mutLayoutData()->setPosY(ny - yOffsetDiff);
                    }
                    if (ed->addToSkyline()) {
                        Segment* s = ed->segment();
                        Measure* m = s->measure();
                        RectF r = ed->layoutData()->bbox().translated(ed->pos());
                        s->staffShape(ed->staffIdx()).add(r);
                        r = ed->layoutData()->bbox().translated(ed->pos() + s->pos() + m->pos());
                        m->system()->staff(ed->staffIdx())->skyline().add(r);
                    }
                }
            }
        }
    }
    Autoplace::setOffsetChanged(item, ldata, false);
}

void TLayout::layout(Hairpin* item, LayoutContext& ctx)
{
    item->setPos(0.0, 0.0);
    layoutTextLineBase(item, ctx);
}

void TLayout::layout(HarpPedalDiagram* item, LayoutContext& ctx)
{
    item->updateDiagramText();
    layoutTextBase(item, ctx, item->mutLayoutData());
    Autoplace::autoplaceSegmentElement(item, item->mutLayoutData());
}

void TLayout::layout(HarmonicMarkSegment* item, LayoutContext& ctx)
{
    HarmonicMarkSegment::LayoutData* ldata = item->mutLayoutData();
    const StaffType* stType = item->staffType();

    if (stType
        && (!stType->isTabStaff()
            || stType->isHiddenElementOnTab(ctx.conf().style(), Sid::harmonicMarkShowTabCommon, Sid::harmonicMarkShowTabSimple))) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    layoutTextLineBaseSegment(item, ctx);

    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layout(Harmony* item, LayoutContext& ctx)
{
    if (!item->explicitParent()) {
        item->setPos(0.0, 0.0);
        item->setOffset(0.0, 0.0);
        layout1(item, ctx);
        return;
    }
    //if (isStyled(Pid::OFFSET))
    //      setOffset(propertyDefault(Pid::OFFSET).value<PointF>());

    layout1(item, ctx);
}

void TLayout::layout1(Harmony* item, const LayoutContext& ctx)
{
    Harmony::LayoutData* ldata = item->mutLayoutData();
    if (ldata->layoutInvalid) {
        item->createBlocks(ldata);
    }

    if (ldata->blocks.empty()) {
        ldata->blocks.push_back(TextBlock());
    }

    auto positionPoint = calculateBoundingRect(item, ctx);

    if (item->hasFrame()) {
        item->layoutFrame();
    }

    item->setPos(positionPoint);
}

PointF TLayout::calculateBoundingRect(Harmony* item, const LayoutContext& ctx)
{
    const Harmony::LayoutData* ldata = item->layoutData();
    const double ypos = (item->placeBelow() && item->staff()) ? item->staff()->height() : 0.0;
    const FretDiagram* fd = (item->explicitParent() && item->explicitParent()->isFretDiagram())
                            ? toFretDiagram(item->explicitParent())
                            : nullptr;

    const double cw = item->symWidth(SymId::noteheadBlack);

    double newPosX = 0.0;
    double newPosY = 0.0;

    if (item->textList().empty()) {
        layout1TextBase(item, ctx);

        if (fd) {
            newPosY = ldata->pos().y();
        } else {
            newPosY = ypos - ((item->align() == AlignV::BOTTOM) ? item->harmonyHeight() - ldata->bbox().height() : 0.0);
        }
    } else {
        RectF bb;
        for (TextSegment* ts : item->textList()) {
            bb.unite(ts->tightBoundingRect().translated(ts->x, ts->y));
        }

        double xx = 0.0;
        switch (item->align().horizontal) {
        case AlignH::LEFT:
            xx = -bb.left();
            break;
        case AlignH::HCENTER:
            xx = -(bb.center().x());
            break;
        case AlignH::RIGHT:
            xx = -bb.right();
            break;
        }

        double yy = -bb.y();      // Align::TOP
        if (item->align() == AlignV::VCENTER) {
            yy = -bb.y() / 2.0;
        } else if (item->align() == AlignV::BASELINE) {
            yy = 0.0;
        } else if (item->align() == AlignV::BOTTOM) {
            yy = -bb.height() - bb.y();
        }

        if (fd) {
            newPosY = ypos - yy - ctx.conf().styleMM(Sid::harmonyFretDist);
        } else {
            newPosY = ypos;
        }

        for (TextSegment* ts : item->textList()) {
            ts->offset = PointF(xx, yy);
        }

        item->setbbox(bb.translated(xx, yy));
        item->setHarmonyHeight(ldata->bbox().height());
    }

    if (fd) {
        switch (item->align().horizontal) {
        case AlignH::LEFT:
            newPosX = 0.0;
            break;
        case AlignH::HCENTER:
            newPosX = fd->centerX();
            break;
        case AlignH::RIGHT:
            newPosX = fd->rightX();
            break;
        }
    } else {
        switch (item->align().horizontal) {
        case AlignH::LEFT:
            newPosX = 0.0;
            break;
        case AlignH::HCENTER:
            newPosX = cw * 0.5;
            break;
        case AlignH::RIGHT:
            newPosX = cw;
            break;
        }
    }

    return PointF(newPosX, newPosY);
}

void TLayout::layout(Hook* item, LayoutContext&)
{
    Hook::LayoutData* ldata = item->mutLayoutData();
    ldata->setBbox(item->symBbox(item->sym()));
}

static void layoutImage(const Image* item, const LayoutContext&, Image::LayoutData* ldata)
{
    ldata->setPos(0.0, 0.0);
    const_cast<Image*>(item)->init();

    SizeF imageSize = item->size();

    // if autoscale && inside a box, scale to box relevant size
    if (item->autoScale()
        && item->explicitParent()
        && ((item->explicitParent()->isHBox() || item->explicitParent()->isVBox()))) {
        if (item->lockAspectRatio()) {
            double f = item->sizeIsSpatium() ? item->spatium() : DPMM;
            SizeF size(item->imageSize());
            double ratio = size.width() / size.height();
            double w = item->parentItem()->width();
            double h = item->parentItem()->height();
            if ((w / h) < ratio) {
                imageSize.setWidth(w / f);
                imageSize.setHeight((w / ratio) / f);
            } else {
                imageSize.setHeight(h / f);
                imageSize.setWidth(h * ratio / f);
            }
        } else {
            imageSize = item->pixel2size(item->parentItem()->layoutData()->bbox().size());
        }
    }

    const_cast<Image*>(item)->setSize(imageSize);

    // in any case, adjust position relative to parent
    ldata->setBbox(RectF(PointF(), item->size2pixel(imageSize)));
}

void TLayout::layout(Image* item, LayoutContext& ctx)
{
    layoutImage(item, ctx, item->mutLayoutData());
}

void TLayout::layout(InstrumentChange* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx, item->mutLayoutData());
    Autoplace::autoplaceSegmentElement(item, item->mutLayoutData());
}

void TLayout::layout(InstrumentName* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx, item->mutLayoutData());
}

void TLayout::layout(Jump* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx, item->mutLayoutData());
    Autoplace::autoplaceMeasureElement(item, item->mutLayoutData());
}

static void keySigAddLayout(const KeySig* item, const LayoutContext& ctx, SymId sym, int line, KeySig::LayoutData* ldata)
{
    double _spatium = item->spatium();
    double step = _spatium * (item->staff() ? item->staff()->staffTypeForElement(item)->lineDistance().val() * 0.5 : 0.5);
    KeySym ks;
    ks.sym = sym;
    double x = 0.0;
    if (ldata->keySymbols.size() > 0) {
        const KeySym& previous = ldata->keySymbols.back();
        double accidentalGap = ctx.conf().styleS(Sid::keysigAccidentalDistance).val();
        if (previous.sym != sym) {
            accidentalGap *= 2;
        } else if (previous.sym == SymId::accidentalNatural && sym == SymId::accidentalNatural) {
            accidentalGap = ctx.conf().styleS(Sid::keysigNaturalDistance).val();
        }
        double previousWidth = item->symWidth(previous.sym) / _spatium;
        x = previous.xPos + previousWidth + accidentalGap;
        bool isAscending = line < previous.line;
        SmuflAnchorId currentCutout = isAscending ? SmuflAnchorId::cutOutSW : SmuflAnchorId::cutOutNW;
        SmuflAnchorId previousCutout = isAscending ? SmuflAnchorId::cutOutNE : SmuflAnchorId::cutOutSE;
        PointF cutout = item->symSmuflAnchor(sym, currentCutout);
        double currentCutoutY = line * step + cutout.y();
        double previousCutoutY = previous.line * step + item->symSmuflAnchor(previous.sym, previousCutout).y();
        if ((isAscending && currentCutoutY < previousCutoutY) || (!isAscending && currentCutoutY > previousCutoutY)) {
            x -= cutout.x() / _spatium;
        }
    }
    ks.xPos = x;
    ks.line = line;
    ldata->keySymbols.push_back(ks);
}

static void layoutKeySig(const KeySig* item, const LayoutContext& ctx, KeySig::LayoutData* ldata)
{
    double _spatium = item->spatium();
    double step = _spatium * (item->staff() ? item->staff()->staffTypeForElement(item)->lineDistance().val() * 0.5 : 0.5);

    ldata->setBbox(RectF());

    ldata->keySymbols.clear();
    if (item->staff() && !item->staff()->staffType(item->tick())->genKeysig()) {
        return;
    }

    // determine current clef for this staff
    ClefType clef = ClefType::G;
    if (item->staff()) {
        // Look for a clef before the key signature at the same tick
        Clef* c = nullptr;
        if (item->segment()) {
            for (Segment* seg = item->segment()->prev1(); !c && seg && seg->tick() == item->tick(); seg = seg->prev1()) {
                if (seg->isClefType() || seg->isHeaderClefType()) {
                    c = toClef(seg->element(item->track()));
                }
            }
        }
        if (c) {
            clef = c->clefType();
        } else {
            // no clef found, so get the clef type from the clefs list, using the previous tick
            clef = item->staff()->clef(item->tick() - Fraction::fromTicks(1));
        }
    }

    int t1 = int(item->key());

    if (item->isCustom() && !item->isAtonal()) {
        double accidentalGap = ctx.conf().styleS(Sid::keysigAccidentalDistance).val();
        // add standard key accidentals first, if necessary
        for (int i = 1; i <= abs(t1) && abs(t1) <= 7; ++i) {
            bool drop = false;
            for (const CustDef& cd: item->customKeyDefs()) {
                int degree = item->degInKey(cd.degree);
                // if custom keysig accidental takes place, don't create tonal accidental
                if ((degree * 2 + 2) % 7 == (t1 < 0 ? 8 - i : i) % 7) {
                    drop = true;
                    break;
                }
            }
            if (!drop) {
                KeySym ks;
                int lineIndexOffset = t1 > 0 ? -1 : 6;
                ks.sym = t1 > 0 ? SymId::accidentalSharp : SymId::accidentalFlat;
                ks.line = ClefInfo::lines(clef)[lineIndexOffset + i];
                if (ldata->keySymbols.size() > 0) {
                    const KeySym& previous = ldata->keySymbols.back();
                    double previousWidth = item->symWidth(previous.sym) / _spatium;
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
                const KeySym& previous = ldata->keySymbols.back();
                double previousWidth = item->symWidth(previous.sym) / _spatium;
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
                    ks.sym = t1 > 0 ? SymId::accidentalSharp : SymId::accidentalFlat;
                    sym = cd.sym;
                }
                ldata->keySymbols.push_back(ks);
                xpos += t1 < 0 ? 0.7 : 1; // flats closer
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
        int accidentals = 0, naturals = 0;
        switch (std::abs(t1)) {
        case 7: accidentals = 0x7f;
            break;
        case 6: accidentals = 0x3f;
            break;
        case 5: accidentals = 0x1f;
            break;
        case 4: accidentals = 0xf;
            break;
        case 3: accidentals = 0x7;
            break;
        case 2: accidentals = 0x3;
            break;
        case 1: accidentals = 0x1;
            break;
        case 0: accidentals = 0;
            break;
        default:
            LOGD("illegal t1 key %d", t1);
            break;
        }

        // manage display of naturals:
        // naturals are shown if there is some natural AND prev. measure has no section break
        // AND style says they are not off
        // OR key sig is CMaj/Amin (in which case they are always shown)

        bool naturalsOn = false;
        Measure* prevMeasure = item->measure() ? item->measure()->prevMeasure() : 0;

        // If we're not force hiding naturals (Continuous panel), use score style settings
        if (!item->hideNaturals()) {
            const bool newSection = (!item->segment()
                                     || (item->segment()->rtick().isZero() && (!prevMeasure || prevMeasure->sectionBreak()))
                                     );
            naturalsOn = !newSection && (ctx.conf().styleI(Sid::keySigNaturals) != int(KeySigNatural::NONE) || (t1 == 0));
        }

        // Don't repeat naturals if shown in courtesy
        if (item->measure() && item->measure()->system() && item->measure()->isFirstInSystem()
            && prevMeasure && prevMeasure->findSegment(SegmentType::KeySigAnnounce, item->tick())
            && !item->segment()->isKeySigAnnounceType()) {
            naturalsOn = false;
        }
        if (item->track() == mu::nidx) {
            naturalsOn = false;
        }

        int coffset = 0;
        Key t2      = Key::C;
        if (naturalsOn) {
            if (item->staff()) {
                t2 = item->staff()->key(item->tick() - Fraction(1, 480 * 4));
            }
            if (t2 == Key::C) {
                naturalsOn = false;
            } else {
                switch (std::abs(int(t2))) {
                case 7: naturals = 0x7f;
                    break;
                case 6: naturals = 0x3f;
                    break;
                case 5: naturals = 0x1f;
                    break;
                case 4: naturals = 0xf;
                    break;
                case 3: naturals = 0x7;
                    break;
                case 2: naturals = 0x3;
                    break;
                case 1: naturals = 0x1;
                    break;
                case 0: naturals = 0;
                    break;
                default:
                    LOGD("illegal t2 key %d", int(t2));
                    break;
                }
                // remove redundant naturals
                if (!((t1 > 0) ^ (t2 > 0))) {
                    naturals &= ~accidentals;
                }
                if (t2 < 0) {
                    coffset = 7;
                }
            }
        }

        // naturals should go BEFORE accidentals if style says so
        // OR going from sharps to flats or vice versa (i.e. t1 & t2 have opposite signs)

        bool prefixNaturals = naturalsOn
                              && (ctx.conf().styleI(Sid::keySigNaturals) == int(KeySigNatural::BEFORE)
                                  || t1 * int(t2) < 0);

        // naturals should go AFTER accidentals if they should not go before!
        bool suffixNaturals = naturalsOn && !prefixNaturals;

        const signed char* lines = ClefInfo::lines(clef);

        if (prefixNaturals) {
            for (int i = 0; i < 7; ++i) {
                if (naturals & (1 << i)) {
                    keySigAddLayout(item, ctx, SymId::accidentalNatural, lines[i + coffset], ldata);
                }
            }
        }
        if (abs(t1) <= 7) {
            SymId symbol = t1 > 0 ? SymId::accidentalSharp : SymId::accidentalFlat;
            int lineIndexOffset = t1 > 0 ? 0 : 7;
            for (int i = 0; i < abs(t1); ++i) {
                keySigAddLayout(item, ctx, symbol, lines[lineIndexOffset + i], ldata);
            }
        } else {
            LOGD("illegal t1 key %d", t1);
        }

        // add suffixed naturals, if any
        if (suffixNaturals) {
            for (int i = 0; i < 7; ++i) {
                if (naturals & (1 << i)) {
                    keySigAddLayout(item, ctx, SymId::accidentalNatural, lines[i + coffset], ldata);
                }
            }
        }

        // Follow stepOffset
        if (item->staffType()) {
            ldata->setPosY(item->staffType()->stepOffset() * 0.5 * _spatium);
        }
    }

    // compute bbox
    for (const KeySym& ks : ldata->keySymbols) {
        double x = ks.xPos * _spatium;
        double y = ks.line * step;
        ldata->addBbox(item->symBbox(ks.sym).translated(x, y));
    }
}

void TLayout::layout(KeySig* item, LayoutContext& ctx)
{
    layoutKeySig(item, ctx, item->mutLayoutData());
}

void TLayout::layout(LayoutBreak* item, LayoutContext&)
{
    LayoutBreak::LayoutData* ldata = item->mutLayoutData();
    double lw = item->lineWidth();
    ldata->setBbox(item->iconBorderRect().adjusted(-lw, -lw, lw, lw));
}

static void layoutLedgerLine(const LedgerLine* item, const LayoutContext& ctx, LedgerLine::LayoutData* ldata)
{
    ldata->lineWidth = ctx.conf().styleMM(Sid::ledgerLineWidth) * item->chord()->mag();
    if (item->staff()) {
        const_cast<LedgerLine*>(item)->setColor(item->staff()->staffType(item->tick())->color());
    }
    double w2 = ldata->lineWidth * .5;

    //Adjust Y position to staffType offset
    if (item->staffType()) {
        ldata->moveY(item->staffType()->yoffset().val() * item->spatium());
    }

    if (item->vertical()) {
        ldata->setBbox(-w2, 0, w2, item->len());
    } else {
        ldata->setBbox(0, -w2, item->len(), w2);
    }
}

void TLayout::layout(LedgerLine* item, LayoutContext& ctx)
{
    layoutLedgerLine(item, ctx, item->mutLayoutData());
}

void TLayout::layout(LetRing* item, LayoutContext& ctx)
{
    layoutLine(item, ctx);
}

void TLayout::layout(LetRingSegment* item, LayoutContext& ctx)
{
    HarmonicMarkSegment::LayoutData* ldata = item->mutLayoutData();

    const StaffType* stType = item->staffType();

    if (stType && stType->isHiddenElementOnTab(ctx.conf().style(), Sid::letRingShowTabCommon, Sid::letRingShowTabSimple)) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    layoutTextLineBaseSegment(item, ctx);
    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layout(LineSegment* item, LayoutContext& ctx)
{
    layoutItem(item, ctx);
}

void TLayout::layout(Lyrics* item, LayoutContext& ctx)
{
    LyricsLayout::layout(item, ctx);
}

void TLayout::layout(LyricsLine* item, LayoutContext& ctx)
{
    LyricsLayout::layout(item, ctx);
}

void TLayout::layout(LyricsLineSegment* item, LayoutContext& ctx)
{
    LyricsLayout::layout(item, ctx);
}

static void layoutMarker(const Marker* item, const LayoutContext& ctx, Marker::LayoutData* ldata)
{
    TLayout::layoutTextBase(item, ctx, ldata);

    // although normally laid out to parent (measure) width,
    // force to center over barline if left-aligned

    if (!ctx.conf().isPaletteMode() && item->layoutToParentWidth() && item->align() == AlignH::LEFT) {
        ldata->moveX(-item->width() * 0.5);
    }

    Autoplace::autoplaceMeasureElement(item, ldata);
}

void TLayout::layout(Marker* item, LayoutContext& ctx)
{
    layoutMarker(item, ctx, item->mutLayoutData());
}

void TLayout::layout(MeasureBase* item, LayoutContext& ctx)
{
    layoutItem(item, ctx);
}

void TLayout::layoutMeasureBase(const MeasureBase* item, MeasureBase::LayoutData* ldata, const LayoutContext& ctx)
{
    LD_CONDITION(ldata->isSetBbox());

    int breakCount = 0;

    for (EngravingItem* e : item->el()) {
        if (e->isLayoutBreak()) {
            TLayout::layoutItem(e, const_cast<LayoutContext&>(ctx));
            EngravingItem::LayoutData* eldata = e->mutLayoutData();
            double spatium = item->spatium();
            double x = 0.0;
            double y = 0.0;
            if (toLayoutBreak(e)->isNoBreak()) {
                x = /*mb*/ ldata->bbox().width() + ctx.conf().styleMM(Sid::barWidth) - eldata->bbox().width() * .5;
            } else {
                x = /*mb*/ ldata->bbox().width()
                    + ctx.conf().styleMM(Sid::barWidth)
                    - eldata->bbox().width()
                    - breakCount * (eldata->bbox().width() + spatium * .5);
                breakCount++;
            }
            y = -2.5 * spatium - eldata->bbox().height();
            eldata->setPos(x, y);
        } else if (e->isMarker() || e->isJump()) {
        } else {
            layoutItem(e, const_cast<LayoutContext&>(ctx));
        }
    }
}

void TLayout::layout(MeasureNumber* item, LayoutContext& ctx)
{
    layoutMeasureNumberBase(item, ctx, item->mutLayoutData());
}

void TLayout::layoutMeasureNumberBase(const MeasureNumberBase* item, const LayoutContext& ctx, MeasureNumberBase::LayoutData* ldata)
{
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    ldata->setPos(PointF());

    layout1TextBase(item, ctx, ldata);

    if (item->placeBelow()) {
        double yoff = ldata->bbox().height();

        // If there is only one line, the barline spans outside the staff lines, so the default position is not correct.
        if (item->staff()->constStaffType(item->measure()->tick())->lines() == 1) {
            yoff += 2.0 * item->spatium();
        } else {
            yoff += item->staff()->height();
        }

        ldata->setPosY(yoff);
    } else {
        double yoff = 0.0;

        // If there is only one line, the barline spans outside the staff lines, so the default position is not correct.
        if (item->staff()->constStaffType(item->measure()->tick())->lines() == 1) {
            yoff -= 2.0 * item->spatium();
        }

        ldata->setPosY(yoff);
    }

    if (item->hPlacement() == PlacementH::CENTER) {
        // measure numbers should be centered over where there can be notes.
        // This means that header and trailing segments should be ignored,
        // which includes all timesigs, clefs, keysigs, etc.
        // This is how it should be centered:
        // |bb 4/4 notes-chords #| other measure |
        // |      ------18------ | other measure |

        //    x1 - left measure position of free space
        //    x2 - right measure position of free space

        const Measure* mea = item->measure();

        // find first chordrest
        Segment* chordRest = mea->first(SegmentType::ChordRest);

        Segment* s1 = chordRest->prevActive();
        // unfortunately, using !s1->header() does not work
        while (s1 && (s1->isChordRestType()
                      || s1->isBreathType()
                      || s1->isClefType()
                      || s1->isBarLineType()
                      || !s1->element(item->staffIdx() * VOICES))) {
            s1 = s1->prevActive();
        }

        Segment* s2 = chordRest->next();
        // unfortunately, using !s1->trailer() does not work
        while (s2 && (s2->isChordRestType()
                      || s2->isBreathType()
                      || s2->isClefType()
                      || s2->isBarLineType()
                      || !s2->element(item->staffIdx() * VOICES))) {
            s2 = s2->nextActive();
        }

        // if s1/s2 does not exist, it means there is no header/trailer segment. Align with start/end of measure.
        double x1 = s1 ? s1->x() + s1->minRight() : 0;
        double x2 = s2 ? s2->x() - s2->minLeft() : mea->width();

        ldata->setPosX((x1 + x2) * 0.5);
    } else if (item->hPlacement() == PlacementH::RIGHT) {
        ldata->setPosX(item->measure()->width());
    }
}

static void layoutMeasureRepeat(const MeasureRepeat* item, const LayoutContext& ctx, MeasureRepeat::LayoutData* ldata)
{
    switch (item->numMeasures()) {
    case 1:
    {
        ldata->setSymId(SymId::repeat1Bar);
        if (ctx.conf().styleB(Sid::mrNumberSeries) && item->track() != mu::nidx) {
            int placeInSeries = 2; // "1" would be the measure actually being repeated
            staff_idx_t staffIdx = item->staffIdx();
            Measure* m = item->measure();
            while (m && m->isOneMeasureRepeat(staffIdx) && m->prevIsOneMeasureRepeat(staffIdx)) {
                placeInSeries++;
                m = m->prevMeasure();
            }
            if (placeInSeries % ctx.conf().styleI(Sid::mrNumberEveryXMeasures) == 0) {
                if (ctx.conf().styleB(Sid::mrNumberSeriesWithParentheses)) {
                    ldata->setNumberSym(String(u"(%1)").arg(placeInSeries));
                } else {
                    ldata->setNumberSym(placeInSeries);
                }
            } else {
                ldata->clearNumberSym();
            }
        } else if (ctx.conf().styleB(Sid::oneMeasureRepeatShow1)) {
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

    if (item->track() != mu::nidx) { // if this is in score rather than a palette cell
        // For unknown reasons, the symbol has some offset in almost all SMuFL fonts
        // We compensate for it, to make sure the symbol is visually centered around the staff line
        double offset = (-bbox.top() - bbox.bottom()) / 2.0;

        const StaffType* staffType = item->staffType();

        // Only need to set y position here; x position is handled in MeasureLayout::layoutMeasureElements()
        ldata->setPos(0, std::floor(staffType->middleLine() / 2.0) * staffType->lineDistance().val() * item->spatium() + offset);
    }

    ldata->setBbox(bbox);

    if (item->track() != mu::nidx && !ldata->numberSym.empty()) {
        ldata->addBbox(item->numberRect());
    }
}

void TLayout::layout(MeasureRepeat* item, LayoutContext& ctx)
{
    for (EngravingItem* e : item->el()) {
        layoutItem(e, ctx);
    }

    layoutMeasureRepeat(item, ctx, item->mutLayoutData());
}

static void layoutMMRest(const MMRest* item, const LayoutContext& ctx, MMRest::LayoutData* ldata)
{
    //! NOTE This is not look like layout data, perhaps this is should be set not here
    ldata->number = item->measure()->mmRestCount();
    ldata->setNumberSym(ldata->number);

    if (ctx.conf().styleB(Sid::oldStyleMultiMeasureRests)) {
        SymIdList restSyms;
        double symsWidth = 0.0;

        int remaining = ldata->number;
        double spacing = ctx.conf().styleMM(Sid::mmRestOldStyleSpacing);
        SymId sym;

        while (remaining > 0) {
            if (remaining >= 4) {
                sym = SymId::restLonga;
                remaining -= 4;
            } else if (remaining >= 2) {
                sym = SymId::restDoubleWhole;
                remaining -= 2;
            } else {
                sym = SymId::restWhole;
                remaining -= 1;
            }

            restSyms.push_back(sym);
            symsWidth += item->symBbox(sym).width();

            if (remaining > 0) { // do not add spacing after last symbol
                symsWidth += spacing;
            }
        }

        ldata->restSyms = restSyms;
        ldata->symsWidth = symsWidth;

        double symHeight = item->symBbox(ldata->restSyms.at(0)).height();
        ldata->setBbox(RectF((item->width() - ldata->symsWidth) * .5, -item->spatium(), ldata->symsWidth, symHeight));
    } else { // H-bar
        double vStrokeHeight = ctx.conf().styleMM(Sid::mmRestHBarVStrokeHeight);
        ldata->setBbox(RectF(0.0, -(vStrokeHeight * .5), item->width(), vStrokeHeight));
    }

    // Only need to set y position here; x position is handled in MeasureLayout::layoutMeasureElements()
    const StaffType* staffType = item->staffType();
    ldata->setPos(0, (staffType->middleLine() / 2.0) * staffType->lineDistance().val() * item->spatium());

    if (item->numberVisible()) {
        ldata->addBbox(item->numberRect());
    }
}

void TLayout::layout(MMRest* item, LayoutContext& ctx)
{
    for (EngravingItem* e : item->el()) {
        layoutItem(e, ctx);
    }

    layoutMMRest(item, ctx, item->mutLayoutData());
}

void TLayout::layout(MMRestRange* item, LayoutContext& ctx)
{
    layoutMeasureNumberBase(item, ctx, item->mutLayoutData());
}

static void layoutNote(const Note* item, const LayoutContext&, Note::LayoutData* ldata)
{
    bool useTablature = item->staff() && item->staff()->isTabStaff(item->chord()->tick());
    if (useTablature) {
        if (item->displayFret() == Note::DisplayFretOption::Hide) {
            return;
        }

        const Staff* st = item->staff();
        const StaffType* tab = st->staffTypeForElement(item);
        double mags = item->magS();
        // not complete but we need systems to be laid out to add parenthesis
        if (item->fixed()) {
            const_cast<Note*>(item)->setFretString(u"/");
        } else {
            const_cast<Note*>(item)->setFretString(tab->fretString(fabs(item->fret()), item->string(), item->deadNote()));

            if (item->negativeFretUsed()) {
                const_cast<Note*>(item)->setFretString(u"-" + item->fretString());
            }

            if (item->displayFret() == Note::DisplayFretOption::ArtificialHarmonic) {
                const_cast<Note*>(item)->setFretString(String(u"%1 <%2>").arg(item->fretString(), String::number(item->harmonicFret())));
            } else if (item->displayFret() == Note::DisplayFretOption::NaturalHarmonic) {
                const_cast<Note*>(item)->setFretString(String(u"<%1>").arg(String::number(item->harmonicFret())));
            }
        }

        if ((item->ghost() && !Note::engravingConfiguration()->tablatureParenthesesZIndexWorkaround())) {
            const_cast<Note*>(item)->setFretString(String(u"(%1)").arg(item->fretString()));
        }

        double w = item->tabHeadWidth(tab);     // !! use _fretString
        ldata->setBbox(0, tab->fretBoxY() * mags, w, tab->fretBoxH() * mags);

        if (item->ghost() && Note::engravingConfiguration()->tablatureParenthesesZIndexWorkaround()) {
            ldata->setWidth(w + item->symWidth(SymId::noteheadParenthesisLeft) + item->symWidth(SymId::noteheadParenthesisRight));
        } else {
            ldata->setWidth(w);
        }
    } else {
        if (item->deadNote()) {
            const_cast<Note*>(item)->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
        } else if (item->harmonic()) {
            const_cast<Note*>(item)->setHeadGroup(NoteHeadGroup::HEAD_DIAMOND);
        }
        SymId nh = item->noteHead();
        if (Note::engravingConfiguration()->crossNoteHeadAlwaysBlack() && ((nh == SymId::noteheadXHalf) || (nh == SymId::noteheadXWhole))) {
            nh = SymId::noteheadXBlack;
        }

        ldata->cachedNoteheadSym = nh;

        if (item->isNoteName()) {
            ldata->cachedSymNull = SymId::noteEmptyBlack;
            NoteHeadType ht = item->headType() == NoteHeadType::HEAD_AUTO ? item->chord()->durationType().headType() : item->headType();
            if (ht == NoteHeadType::HEAD_WHOLE) {
                ldata->cachedSymNull = SymId::noteEmptyWhole;
            } else if (ht == NoteHeadType::HEAD_HALF) {
                ldata->cachedSymNull = SymId::noteEmptyHalf;
            }
        } else {
            ldata->cachedSymNull = SymId::noSym;
        }
        ldata->setBbox(item->symBbox(nh));
    }
}

void TLayout::layout(Note* item, LayoutContext& ctx)
{
    layoutNote(item, ctx, item->mutLayoutData());
}

static void layoutNoteDot(const NoteDot* item, const LayoutContext&, NoteDot::LayoutData* ldata)
{
    ldata->setBbox(item->symBbox(SymId::augmentationDot));
}

void TLayout::layout(NoteDot* item, LayoutContext& ctx)
{
    layoutNoteDot(item, ctx, item->mutLayoutData());
}

void TLayout::layout(const Ornament* item, Ornament::LayoutData* ldata, const LayoutConfiguration& conf)
{
    layout(static_cast<const Articulation*>(item), ldata);

    double _spatium = item->spatium();
    double vertMargin = 0.35 * _spatium;
    static constexpr double ornamentAccidentalMag = 0.6; // TODO: style?

    for (size_t i = 0; i < item->accidentalsAboveAndBelow().size(); ++i) {
        bool above = (i == 0);
        Accidental* accidental = item->accidentalsAboveAndBelow()[i];
        if (!accidental) {
            continue;
        }
        accidental->computeMag();
        Accidental::LayoutData* accLData = accidental->mutLayoutData();
        accLData->setMag(accLData->mag() * ornamentAccidentalMag);
        layout(accidental, accLData, conf);
        Shape accidentalShape = accidental->shape();
        double minVertDist = above
                             ? accidentalShape.minVerticalDistance(ldata->bbox())
                             : Shape(ldata->bbox()).minVerticalDistance(accidentalShape);

        accLData->setPos(-0.5 * accLData->bbox().width(), above ? (-minVertDist - vertMargin) : (minVertDist + vertMargin));
    }
}

void TLayout::layoutOrnamentCueNote(Ornament* item, LayoutContext& ctx)
{
    if (!item->explicitParent()) {
        return;
    }

    Chord* parentChord = toChord(item->parentItem());
    Chord* cueNoteChord = item->cueNoteChord();

    if (!cueNoteChord) {
        return;
    }

    const std::vector<Note*>& notes = cueNoteChord->notes();
    Note* cueNote = notes.empty() ? nullptr : notes.front();

    if (!cueNote) {
        return;
    }

    ChordLayout::layoutChords3(ctx.conf().style(), { cueNoteChord }, { cueNote }, item->staff(), ctx);
    layout(cueNoteChord, ctx);

    Shape noteShape = cueNoteChord->shape();
    Shape parentChordShape = parentChord->shape();
    double minDist = parentChordShape.minHorizontalDistance(noteShape);
    // Check for possible other chords in same segment
    staff_idx_t startStaff = staff2track(parentChord->staffIdx());
    for (staff_idx_t staff = startStaff; staff < startStaff + VOICES; ++staff) {
        Segment* segment = parentChord->segment();
        ChordRest* cr = segment->elementAt(staff) ? toChordRest(segment->elementAt(staff)) : nullptr;
        if (cr) {
            minDist = std::max(minDist, cr->shape().minHorizontalDistance(noteShape));
        }
    }
    cueNoteChord->mutLayoutData()->setPosX(minDist);
}

void TLayout::layout(Ottava* item, LayoutContext& ctx)
{
    layoutLine(item, ctx);
}

void TLayout::layout(OttavaSegment* item, LayoutContext& ctx)
{
    OttavaSegment::LayoutData* ldata = item->mutLayoutData();
    layoutTextLineBaseSegment(item, ctx);
    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layout(PalmMute* item, LayoutContext& ctx)
{
    layoutLine(item, ctx);
}

void TLayout::layout(PalmMuteSegment* item, LayoutContext& ctx)
{
    PalmMuteSegment::LayoutData* ldata = item->mutLayoutData();

    const StaffType* stType = item->staffType();

    if (stType && stType->isHiddenElementOnTab(ctx.conf().style(), Sid::palmMuteShowTabCommon, Sid::palmMuteShowTabSimple)) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    layoutTextLineBaseSegment(item, ctx);
    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layout(Pedal* item, LayoutContext& ctx)
{
    layoutLine(item, ctx);
}

void TLayout::layout(PedalSegment* item, LayoutContext& ctx)
{
    PedalSegment::LayoutData* ldata = item->mutLayoutData();

    layoutTextLineBaseSegment(item, ctx);
    if (item->isStyled(Pid::OFFSET)) {
        item->roffset() = item->pedal()->propertyDefault(Pid::OFFSET).value<PointF>();
    }
    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layout(PickScrapeSegment* item, LayoutContext& ctx)
{
    PickScrapeSegment::LayoutData* ldata = item->mutLayoutData();
    layoutTextLineBaseSegment(item, ctx);
    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layout(PlayTechAnnotation* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx, item->mutLayoutData());
    Autoplace::autoplaceSegmentElement(item, item->mutLayoutData());
}

void TLayout::layout(RasgueadoSegment* item, LayoutContext& ctx)
{
    RasgueadoSegment::LayoutData* ldata = item->mutLayoutData();
    const StaffType* stType = item->staffType();

    if (stType && stType->isHiddenElementOnTab(ctx.conf().style(), Sid::rasgueadoShowTabCommon, Sid::rasgueadoShowTabSimple)) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    layoutTextLineBaseSegment(item, ctx);
    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layout(RehearsalMark* item, LayoutContext& ctx)
{
    RehearsalMark::LayoutData* ldata = item->mutLayoutData();
    layoutTextBase(item, ctx);

    Segment* s = item->segment();
    if (s) {
        if (s->rtick().isZero()) {
            // first CR of measure, alignment is hcenter or right (the usual cases)
            // align with barline, point just after header, or start of measure depending on context

            Measure* m = s->measure();
            Segment* header = s->prev();        // possibly just a start repeat
            double measureX = -s->x();
            Segment* repeat = m->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
            double barlineX = repeat ? repeat->x() - s->x() : measureX;
            System* sys = m->system();
            bool systemFirst = (sys && m->isFirstInSystem());

            if (!header || repeat || !systemFirst) {
                // no header, or header with repeat, or header mid-system - align with barline
                ldata->setPosX(barlineX);
            } else {
                // header at start of system
                // align to a point just after the header
                EngravingItem* e = header->element(item->track());
                double w = e ? e->width() : header->width();
                ldata->setPosX(header->x() + w - s->x());

                // special case for right aligned rehearsal marks at start of system
                // left align with start of measure if that is further left
                if (item->align() == AlignH::RIGHT) {
                    ldata->setPosX(std::min(ldata->pos().x(), measureX + item->width()));
                }
            }
        }
        Autoplace::autoplaceSegmentElement(item, item->mutLayoutData());
    }
}

static void layoutRestDots(const Rest* item, const LayoutContext& ctx, Rest::LayoutData* ldata)
{
    const_cast<Rest*>(item)->checkDots();
    double x = item->symWidthNoLedgerLines(ldata) + ctx.conf().styleMM(Sid::dotNoteDistance) * item->mag();
    double dx = ctx.conf().styleMM(Sid::dotDotDistance) * item->mag();
    double y = item->dotLine() * item->spatium() * .5;
    for (NoteDot* dot : item->dotList()) {
        NoteDot::LayoutData* dotldata = dot->mutLayoutData();
        layoutNoteDot(dot, ctx, dotldata);
        dotldata->setPos(x, y);
        x += dx;
    }
}

static void layoutRest(const Rest* item, const LayoutContext& ctx, Rest::LayoutData* ldata)
{
    if (item->deadSlapped()) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    double _spatium = item->spatium();

    ldata->setPosX(0.0);
    const StaffType* stt = item->staffType();
    if (stt && stt->isTabStaff()) {
        // if rests are shown and note values are shown as duration symbols
        if (stt->showRests() && stt->genDurations()) {
            DurationType type = item->durationType().type();
            int dots = item->durationType().dots();
            // if rest is whole measure, convert into actual type and dot values
            if (type == DurationType::V_MEASURE && item->measure()) {
                Fraction ticks = item->measure()->ticks();
                TDuration dur  = TDuration(ticks).type();
                type           = dur.type();
                dots           = dur.dots();
            }
            // symbol needed; if not exist, create, if exists, update duration
            if (!item->tabDur()) {
                const_cast<Rest*>(item)->setTabDur(new TabDurationSymbol(const_cast<Rest*>(item), stt, type, dots));
            } else {
                item->tabDur()->setDuration(type, dots, stt);
            }
            item->tabDur()->setParent(const_cast<Rest*>(item));
// needed?        _tabDur->setTrack(track());
            TLayout::layout(item->tabDur(), const_cast<LayoutContext&>(ctx));
            ldata->setBbox(item->tabDur()->layoutData()->bbox());
            ldata->setPos(0.0, 0.0);                   // no rest is drawn: reset any position might be set for it
            return;
        }
        // if no rests or no duration symbols, delete any dur. symbol and chain into standard staff mngmt
        // this is to ensure horiz space is reserved for rest, even if they are not displayed
        // Rest::draw() will skip their drawing, if not needed
        if (item->tabDur()) {
            delete item->tabDur();
            const_cast<Rest*>(item)->setTabDur(nullptr);
        }
    }

    const_cast<Rest*>(item)->setDotLine(Rest::getDotline(item->durationType().type()));

    double yOff = item->offset().y();
    const Staff* stf = item->staff();
    const StaffType* st = stf ? stf->staffTypeForElement(item) : 0;
    double lineDist = st ? st->lineDistance().val() : 1.0;
    int userLine   = yOff == 0.0 ? 0 : lrint(yOff / (lineDist * _spatium));
    int lines      = st ? st->lines() : 5;

    int naturalLine = item->computeNaturalLine(lines); // Measured in 1sp steps
    int voiceOffset = item->computeVoiceOffset(lines, ldata); // Measured in 1sp steps
    int wholeRestOffset = item->computeWholeRestOffset(voiceOffset, lines);
    int finalLine = naturalLine + voiceOffset + wholeRestOffset;

    ldata->sym = item->getSymbol(item->durationType().type(), finalLine + userLine, lines);

    ldata->setPosY(finalLine * lineDist * _spatium);
    if (!item->shouldNotBeDrawn()) {
        ldata->setBbox(item->symBbox(ldata->sym));
    }
    layoutRestDots(item, ctx, ldata);
}

void TLayout::layout(Rest* item, LayoutContext& ctx)
{
    if (item->isGap()) {
        return;
    }
    for (EngravingItem* e : item->el()) {
        layoutItem(e, ctx);
    }

    layoutRest(item, ctx, item->mutLayoutData());
}

void TLayout::layout(ShadowNote* item, LayoutContext& ctx)
{
    if (!item->isValid()) {
        item->setbbox(RectF());
        return;
    }
    double _spatium = item->spatium();
    RectF newBbox;
    RectF noteheadBbox = item->symBbox(item->noteheadSymbol());
    bool up = item->computeUp();

    // TODO: Take into account accidentals and articulations?

    // Layout dots
    double dotWidth = 0;
    if (item->duration().dots() > 0) {
        double noteheadWidth = noteheadBbox.width();
        double d  = ctx.conf().styleMM(Sid::dotNoteDistance) * item->mag();
        double dd = ctx.conf().styleMM(Sid::dotDotDistance) * item->mag();
        dotWidth = (noteheadWidth + d);
        if (item->hasFlag() && up) {
            dotWidth = std::max(dotWidth, noteheadWidth + item->symBbox(item->flagSym()).right());
        }
        for (int i = 0; i < item->duration().dots(); i++) {
            dotWidth += dd * i;
        }
    }
    newBbox.setRect(noteheadBbox.x(), noteheadBbox.y(), noteheadBbox.width() + dotWidth, noteheadBbox.height());

    // Layout stem and flag
    if (item->hasStem()) {
        double x = noteheadBbox.x();
        double w = noteheadBbox.width();

        double stemWidth = ctx.conf().styleMM(Sid::stemWidth);
        double stemLength = (up ? -3.5 : 3.5) * _spatium;
        double stemAnchor = item->symSmuflAnchor(item->noteheadSymbol(), up ? SmuflAnchorId::stemUpSE : SmuflAnchorId::stemDownNW).y();
        newBbox |= RectF(up ? x + w - stemWidth : x,
                         stemAnchor,
                         stemWidth,
                         stemLength - stemAnchor);

        if (item->hasFlag()) {
            RectF flagBbox = item->symBbox(item->flagSym());
            newBbox |= RectF(up ? x + w - stemWidth : x,
                             stemAnchor + stemLength + flagBbox.y(),
                             flagBbox.width(),
                             flagBbox.height());
        }
    }

    // Layout ledger lines if needed
    if (!item->isRest() && item->lineIndex() < 100 && item->lineIndex() > -100) {
        double extraLen = ctx.conf().styleMM(Sid::ledgerLineLength) * item->mag();
        double step = 0.5 * _spatium * item->staffType()->lineDistance().val();
        double x = noteheadBbox.x() - extraLen;
        double w = noteheadBbox.width() + 2 * extraLen;

        double lw = ctx.conf().styleMM(Sid::ledgerLineWidth);

        RectF r(x, -lw * .5, w, lw);
        for (int i = -2; i >= item->lineIndex(); i -= 2) {
            newBbox |= r.translated(PointF(0, step * (i - item->lineIndex())));
        }
        int l = item->staffType()->lines() * 2; // first ledger line below staff
        for (int i = l; i <= item->lineIndex(); i += 2) {
            newBbox |= r.translated(PointF(0, step * (i - item->lineIndex())));
        }
    }
    item->setbbox(newBbox);
}

void TLayout::layoutLine(SLine* item, LayoutContext& ctx)
{
    item->computeStartElement();
    item->computeEndElement();

    System* s1;
    System* s2;
    PointF p1(item->linePos(Grip::START, &s1));
    PointF p2(item->linePos(Grip::END,   &s2));

    const std::vector<System*>& systems = ctx.dom().systems();
    system_idx_t sysIdx1 = mu::indexOf(systems, s1);
    system_idx_t sysIdx2 = mu::indexOf(systems, s2);
    int segmentsNeeded = 0;

    if (sysIdx1 == mu::nidx || sysIdx2 == mu::nidx) {
        return;
    }

    for (system_idx_t i = sysIdx1; i <= sysIdx2; ++i) {
        if (systems.at(i)->vbox()) {
            continue;
        }
        ++segmentsNeeded;
    }

    int segCount = int(item->spannerSegments().size());

    if (segmentsNeeded != segCount) {
        item->fixupSegments(segmentsNeeded, [item](System* parent) { return item->createLineSegment(parent); });
        if (segmentsNeeded > segCount) {
            for (int i = segCount; i < segmentsNeeded; ++i) {
                LineSegment* lineSegm = item->segmentAt(i);
                // set user offset to previous segment's offset
                if (segCount > 0) {
                    lineSegm->setOffset(PointF(0, item->segmentAt(i - 1)->offset().y()));
                } else {
                    lineSegm->setOffset(PointF(0, item->offset().y()));
                }
            }
        }
    }

    int segIdx = 0;
    for (system_idx_t i = sysIdx1; i <= sysIdx2; ++i) {
        System* system = systems.at(i);
        if (system->vbox()) {
            continue;
        }
        LineSegment* lineSegm = item->segmentAt(segIdx++);
        lineSegm->setTrack(item->track());           // DEBUG
        lineSegm->setSystem(system);

        if (sysIdx1 == sysIdx2) {
            // single segment
            lineSegm->setSpannerSegmentType(SpannerSegmentType::SINGLE);
            double len = p2.x() - p1.x();
            // enforcing a minimum length would be possible but inadvisable
            // the line length calculations are tuned well enough that this should not be needed
            //if (anchor() == Anchor::SEGMENT && type() != ElementType::PEDAL)
            //      len = std::max(1.0 * spatium(), len);
            lineSegm->setPos(p1);
            lineSegm->setPos2(PointF(len, p2.y() - p1.y()));
        } else if (i == sysIdx1) {
            // start segment
            lineSegm->setSpannerSegmentType(SpannerSegmentType::BEGIN);
            lineSegm->setPos(p1);
            double x2 = system->endingXForOpenEndedLines();
            lineSegm->setPos2(PointF(x2 - p1.x(), 0.0));
        } else if (i > 0 && i != sysIdx2) {
            // middle segment
            lineSegm->setSpannerSegmentType(SpannerSegmentType::MIDDLE);
            double x1 = system->firstNoteRestSegmentX(true);
            double x2 = system->endingXForOpenEndedLines();
            lineSegm->setPos(PointF(x1, p1.y()));
            lineSegm->setPos2(PointF(x2 - x1, 0.0));
        } else if (i == sysIdx2) {
            // end segment
            double minLen = 0.0;
            double x1 = system->firstNoteRestSegmentX(true);
            double len = std::max(minLen, p2.x() - x1);
            lineSegm->setSpannerSegmentType(SpannerSegmentType::END);
            lineSegm->setPos(PointF(p2.x() - len, p2.y()));
            lineSegm->setPos2(PointF(len, 0.0));
        }
        layout(lineSegm, ctx);
    }
}

void TLayout::layout(Slur* item, LayoutContext& ctx)
{
    SlurTieLayout::layout(item, ctx);
}

void TLayout::layout(Spacer* item, LayoutContext&)
{
    UNUSED(item);
}

void TLayout::layout(Spanner* item, LayoutContext& ctx)
{
    layoutItem(item, ctx);
}

void TLayout::layout(StaffLines* item, LayoutContext& ctx)
{
    layoutForWidth(item, item->measure()->width(), ctx);
}

void TLayout::layoutForWidth(StaffLines* item, double w, LayoutContext& ctx)
{
    StaffLines::LayoutData* ldata = item->mutLayoutData();
    const Staff* s = item->staff();
    double _spatium = item->spatium();
    double dist     = _spatium;
    item->setPos(PointF(0.0, 0.0));
    int _lines;
    if (s) {
        ldata->setMag(s->staffMag(item->measure()->tick()));
        item->setVisible(!s->isLinesInvisible(item->measure()->tick()));
        item->setColor(s->color(item->measure()->tick()));
        const StaffType* st = s->staffType(item->measure()->tick());
        dist *= st->lineDistance().val();
        _lines = st->lines();
        ldata->setPosY(st->yoffset().val() * _spatium);
//            if (_lines == 1)
//                  rypos() = 2 * _spatium;
    } else {
        _lines = 5;
        item->setColor(StaffLines::engravingConfiguration()->defaultColor());
    }
    item->setLw(ctx.conf().styleS(Sid::staffLineWidth).val() * _spatium);
    double x1 = item->pos().x();
    double x2 = x1 + w;
    double y  = item->pos().y();
    ldata->setBbox(x1, -item->lw() * .5 + y, w, (_lines - 1) * dist + item->lw());

    std::vector<mu::LineF> ll;
    for (int i = 0; i < _lines; ++i) {
        ll.push_back(LineF(x1, y, x2, y));
        y += dist;
    }
    item->setLines(ll);
}

static void layoutStaffState(const StaffState* item, const LayoutContext&, StaffState::LayoutData* ldata)
{
    double _spatium = item->spatium();
    ldata->lw = (_spatium * 0.3);
    double h  = _spatium * 4;
    double w  = _spatium * 2.5;
//      double w1 = w * .6;

    PainterPath path;
    switch (item->staffStateType()) {
    case StaffStateType::INSTRUMENT:
        path.lineTo(w, 0.0);
        path.lineTo(w, h);
        path.lineTo(0.0, h);
        path.lineTo(0.0, 0.0);
        path.moveTo(w * .5, h - _spatium * .5);
        path.lineTo(w * .5, _spatium * 2);
        path.moveTo(w * .5, _spatium * .8);
        path.lineTo(w * .5, _spatium * 1.0);
        break;

    case StaffStateType::TYPE:
        path.lineTo(w, 0.0);
        path.lineTo(w, h);
        path.lineTo(0.0, h);
        path.lineTo(0.0, 0.0);
        break;

    case StaffStateType::VISIBLE:
        path.lineTo(w, 0.0);
        path.lineTo(w, h);
        path.lineTo(0.0, h);
        path.lineTo(0.0, 0.0);
        break;

    case StaffStateType::INVISIBLE:
        path.lineTo(w, 0.0);
        path.lineTo(w, h);
        path.lineTo(0.0, h);
        path.lineTo(0.0, 0.0);
        break;

    default:
        LOGD("unknown layout break symbol");
        break;
    }

    ldata->path = path;

    RectF bb(0, 0, w, h);
    bb.adjust(-ldata->lw, -ldata->lw, ldata->lw, ldata->lw);
    ldata->setBbox(bb);
    ldata->setPos(0.0, _spatium * -6.0);
}

void TLayout::layout(StaffState* item, LayoutContext& ctx)
{
    layoutStaffState(item, ctx, item->mutLayoutData());
}

void TLayout::layout(StaffText* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx, item->mutLayoutData());
    Autoplace::autoplaceSegmentElement(item, item->mutLayoutData());
}

static void layoutStaffTypeChange(const StaffTypeChange* item, const LayoutContext& ctx, StaffTypeChange::LayoutData* ldata)
{
    double _spatium = ctx.conf().spatium();
    ldata->setBbox(RectF(-item->lw() * .5, -item->lw() * .5, _spatium * 2.5 + item->lw(), _spatium * 2.5 + item->lw()));
    if (item->measure()) {
        double y = -1.5 * _spatium - item->height() + item->measure()->system()->staff(item->staffIdx())->y();
        ldata->setPos(_spatium * .8, y);
    } else {
        ldata->setPos(0.0, 0.0);
    }
}

void TLayout::layout(StaffTypeChange* item, LayoutContext& ctx)
{
    layoutStaffTypeChange(item, ctx, item->mutLayoutData());
}

static void layoutStem(const Stem* item, const LayoutContext& ctx, Stem::LayoutData* ldata)
{
    const bool up = item->up();
    const double _up = up ? -1.0 : 1.0;

    double y1 = 0.0; // vertical displacement to match note attach point
    double y2 = _up * (item->length());

    bool isTabStaff = false;
    if (item->chord()) {
        ldata->setMag(item->chord()->mag());

        const Staff* staff = item->staff();
        const StaffType* staffType = staff ? staff->staffTypeForElement(item->chord()) : nullptr;
        isTabStaff = staffType && staffType->isTabStaff();

        if (isTabStaff) {
            if (staffType->stemThrough()) {
                // if stems through staves, gets Y pos. of stem-side note relative to chord other side
                const double staffLinesDistance = staffType->lineDistance().val() * item->spatium();
                y1 = (item->chord()->downString() - item->chord()->upString()) * _up * staffLinesDistance;

                // if fret marks above lines, raise stem beginning by 1/2 line distance
                if (!staffType->onLines()) {
                    y1 -= staffLinesDistance * 0.5;
                }

                // shorten stem by 1/2 lineDist to clear the note and a little more to keep 'air' between stem and note
                y1 += _up * staffLinesDistance * 0.7;
            }
            // in other TAB types, no correction
        } else { // non-TAB
            // move stem start to note attach point
            Note* note = up ? item->chord()->downNote() : item->chord()->upNote();
            if ((up && !note->mirror()) || (!up && note->mirror())) {
                y1 = note->stemUpSE().y();
            } else {
                y1 = note->stemDownNW().y();
            }

            ldata->setPosY(note->layoutData()->pos().y());
        }

        if (item->chord()->hook() && !item->chord()->beam()) {
            y2 += item->chord()->hook()->smuflAnchor().y();
        }

        if (item->chord()->beam()) {
            y2 -= _up * item->point(ctx.conf().styleS(Sid::beamWidth)) * .5 * item->chord()->beam()->mag();
        }
    }

    double lineWidthCorrection = item->lineWidthMag() * 0.5;
    double lineX = isTabStaff ? 0.0 : _up * lineWidthCorrection;

    LineF line = LineF(lineX, y1, lineX, y2);
    ldata->line = line;

    // HACK: if there is a beam, extend the bounding box of the stem (NOT the stem itself) by half beam width.
    // This way the bbox of the stem covers also the beam position. Hugely helps with all the collision checks.
    double beamCorrection = (item->chord() && item->chord()->beam()) ? _up * ctx.conf().styleMM(Sid::beamWidth) * item->mag() / 2 : 0.0;
    // compute line and bounding rectangle
    RectF rect(line.p1(), line.p2() + PointF(0.0, beamCorrection));
    ldata->setBbox(rect.normalized().adjusted(-lineWidthCorrection, 0, lineWidthCorrection, 0));
}

void TLayout::layout(Stem* item, LayoutContext& ctx)
{
    layoutStem(item, ctx, item->mutLayoutData());
}

static void layoutStemSlash(const StemSlash* item, const LayoutContext& ctx, StemSlash::LayoutData* ldata)
{
    if (!item->chord() || !item->chord()->stem()) {
        return;
    }
    Chord* c = item->chord();
    Stem* stem = c->stem();
    Hook* hook = c->hook();
    Beam* beam = c->beam();

    static constexpr double heightReduction = 0.66;
    static constexpr double angleIncrease = 1.2;
    static constexpr double lengthIncrease = 1.1;
    const double mag = c->mag();

    double up = c->up() ? -1 : 1;
    double stemTipY = c->up()
                      ? stem->layoutData()->bbox().translated(stem->pos()).top()
                      : stem->layoutData()->bbox().translated(stem->pos()).bottom();
    double leftHang = ctx.conf().noteHeadWidth() * mag / 2;
    double angle = ctx.conf().styleD(Sid::stemSlashAngle) * M_PI / 180; // converting to radians
    bool straight = ctx.conf().styleB(Sid::useStraightNoteFlags);
    double graceNoteMag = mag;

    double startX = stem->layoutData()->bbox().translated(stem->pos()).right() - leftHang;

    double startY;
    if (straight || beam) {
        startY = stemTipY - up * graceNoteMag * ctx.conf().styleMM(Sid::stemSlashPosition) * heightReduction;
    } else {
        startY = stemTipY - up * graceNoteMag * ctx.conf().styleMM(Sid::stemSlashPosition);
    }

    double endX = 0;
    double endY = 0;

    if (hook) {
        auto musicFont = ctx.conf().styleSt(Sid::MusicalSymbolFont);
        // HACK: adjust slash angle for fonts with "fat" hooks. In future, we must use smufl cutOut
        if (c->beams() >= 2 && !straight
            && (musicFont == "Bravura" || musicFont == "Finale Maestro" || musicFont == "Gonville")) {
            angle *= angleIncrease;
        }
        endX = hook->layoutData()->bbox().translated(hook->pos()).right(); // always ends at the right bbox margin of the hook
        endY = startY + up * (endX - startX) * tan(angle);
    }
    if (beam) {
        PointF p1 = beam->startAnchor();
        PointF p2 = beam->endAnchor();
        double beamAngle = p2.x() > p1.x() ? atan((p2.y() - p1.y()) / (p2.x() - p1.x())) : 0;
        angle += up * beamAngle / 2;
        double length = 2 * item->spatium();
        bool obtuseAngle = (c->up() && beamAngle < 0) || (!c->up() && beamAngle > 0);
        if (obtuseAngle) {
            length *= lengthIncrease; // needs to be slightly longer
        }
        endX = startX + length * cos(angle);
        endY = startY + up * length * sin(angle);
    }

    ldata->line = LineF(PointF(startX, startY), PointF(endX, endY));
    ldata->stemWidth = ctx.conf().styleMM(Sid::stemSlashThickness) * graceNoteMag;

    RectF bbox = RectF(ldata->line.p1(), ldata->line.p2()).normalized();
    bbox = bbox.adjusted(-ldata->stemWidth / 2, -ldata->stemWidth / 2, ldata->stemWidth, ldata->stemWidth);
    ldata->setBbox(bbox);
}

void TLayout::layout(StemSlash* item, LayoutContext& ctx)
{
    layoutStemSlash(item, ctx, item->mutLayoutData());
}

void TLayout::layout(Sticking* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx, item->mutLayoutData());
    Autoplace::autoplaceSegmentElement(item, item->mutLayoutData());
}

void TLayout::layout(StretchedBend* item, LayoutContext& ctx)
{
    item->fillArrows(ctx.conf().styleMM(Sid::bendArrowWidth));
    item->fillSegments();
    item->fillStretchedSegments(false);

    item->setbbox(item->calculateBoundingRect());
    item->setPos(0.0, 0.0);
}

void TLayout::layoutStretched(StretchedBend* item, LayoutContext& ctx)
{
    UNUSED(ctx);
    item->fillStretchedSegments(true);
    item->setbbox(item->calculateBoundingRect());
    item->setPos(0.0, 0.0);
}

static void layoutBaseSymbol(const BSymbol* item, const LayoutContext& ctx, BSymbol::LayoutData* ldata)
{
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    if (item->parentItem()->isNote()) {
        ldata->setMag(item->parentItem()->mag());
    } else if (item->staff()) {
        ldata->setMag(item->staff()->staffMag(item->tick()));
    }

    for (EngravingItem* e : item->leafs()) {
        TLayout::layoutItem(e, const_cast<LayoutContext&>(ctx));
    }
}

static void layoutSymbol(const Symbol* item, const LayoutContext& ctx, Symbol::LayoutData* ldata)
{
    ldata->setBbox(item->scoreFont() ? item->scoreFont()->bbox(item->sym(), item->magS()) : item->symBbox(item->sym()));
    double w = ldata->bbox().width();
    PointF p;
    if (item->align() == AlignV::BOTTOM) {
        p.setY(-ldata->bbox().height());
    } else if (item->align() == AlignV::VCENTER) {
        p.setY((-ldata->bbox().height()) * .5);
    } else if (item->align() == AlignV::BASELINE) {
        p.setY(-item->baseLine());
    }
    if (item->align() == AlignH::RIGHT) {
        p.setX(-w);
    } else if (item->align() == AlignH::HCENTER) {
        p.setX(-(w * .5));
    }
    ldata->setPos(p);
    layoutBaseSymbol(item, ctx, ldata);
}

void TLayout::layout(Symbol* item, LayoutContext& ctx)
{
    layoutSymbol(item, ctx, item->mutLayoutData());
}

void TLayout::layout(FSymbol* item, LayoutContext&)
{
    FSymbol::LayoutData* ldata = item->mutLayoutData();
    ldata->setBbox(mu::draw::FontMetrics::boundingRect(item->font(), item->toString()));
}

static void layoutSystemDivider(const SystemDivider* item, const LayoutContext& ctx, SystemDivider::LayoutData* ldata)
{
    SymId sid;

    //! NOTE Look like, this is should not be here, maybe it should be in type setter.
    if (item->dividerType() == SystemDivider::Type::LEFT) {
        sid = SymNames::symIdByName(ctx.conf().styleSt(Sid::dividerLeftSym));
    } else {
        sid = SymNames::symIdByName(ctx.conf().styleSt(Sid::dividerRightSym));
    }
    const_cast<SystemDivider*>(item)->setSym(sid, ctx.engravingFont());

    layoutSymbol(item, ctx, ldata);
}

void TLayout::layout(SystemDivider* item, LayoutContext& ctx)
{
    layoutSystemDivider(item, ctx, item->mutLayoutData());
}

void TLayout::layout(SystemText* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx, item->mutLayoutData());
    Autoplace::autoplaceSegmentElement(item, item->mutLayoutData());
}

static void layoutTabDurationSymbol(const TabDurationSymbol* item, const LayoutContext&, TabDurationSymbol::LayoutData* ldata)
{
    static constexpr double TAB_RESTSYMBDISPL = 2.0;

    if (!item->tab()) {
        ldata->setBbox(RectF());
        return;
    }
    double _spatium    = item->spatium();
    double hbb, wbb, xbb, ybb;   // bbox sizes
    double xpos, ypos;           // position coords

    ldata->beamGrid = TabBeamGrid::NONE;
    Chord* chord = item->explicitParent() && item->explicitParent()->isChord() ? toChord(item->explicitParent()) : nullptr;
// if no chord (shouldn't happens...) or not a special beam mode, layout regular symbol
    if (!chord || !chord->isChord()
        || (chord->beamMode() != BeamMode::BEGIN && chord->beamMode() != BeamMode::MID
            && chord->beamMode() != BeamMode::END)) {
        mu::draw::FontMetrics fm(item->tab()->durationFont());
        hbb   = item->tab()->durationBoxH();
        wbb   = fm.width(item->text());
        xbb   = 0.0;
        xpos  = 0.0;
        ypos  = item->tab()->durationFontYOffset();
        ybb   = item->tab()->durationBoxY() - ypos;
        // with rests, move symbol down by half its displacement from staff
        if (item->explicitParent() && item->explicitParent()->isRest()) {
            ybb  += TAB_RESTSYMBDISPL * _spatium;
            ypos += TAB_RESTSYMBDISPL * _spatium;
        }
    }
// if on a chord with special beam mode, layout an 'English'-style duration grid
    else {
        TablatureDurationFont font = item->tab()->_durationFonts[item->tab()->_durationFontIdx];
        hbb   = font.gridStemHeight * _spatium;         // bbox height is stem height
        wbb   = font.gridStemWidth * _spatium;          // bbox width is stem width
        xbb   = -wbb * 0.5;                             // bbox is half at left and half at right of stem centre
        ybb   = -hbb;                                   // bbox top is at top of stem height
        xpos  = 0.75 * _spatium;                        // conventional centring of stem on fret marks
        ypos  = item->tab()->durationGridYOffset();      // stem start is at bottom
        if (chord->beamMode() == BeamMode::BEGIN) {
            ldata->beamGrid = TabBeamGrid::INITIAL;
            ldata->beamLength = 0.0;
        } else if (chord->beamMode() == BeamMode::MID || chord->beamMode() == BeamMode::END) {
            ldata->beamLevel = (static_cast<int>(chord->durationType().type()) - static_cast<int>(font.zeroBeamLevel));
            ldata->beamGrid = (ldata->beamLevel < 1 ? TabBeamGrid::INITIAL : TabBeamGrid::MEDIALFINAL);
            // _beamLength and bbox x and width will be set in layout2(),
            // once horiz. positions of chords are known
        }
    }
// set this' mag from parent chord mag (include staff mag)
    double mag = chord != nullptr ? chord->mag() : 1.0;
    ldata->setMag(mag);
    mag = item->magS();         // local mag * score mag
// set magnified bbox and position
    ldata->setBbox(xbb * mag, ybb * mag, wbb * mag, hbb * mag);
    ldata->setPos(xpos * mag, ypos * mag);
}

void TLayout::layout(TabDurationSymbol* item, LayoutContext& ctx)
{
    layoutTabDurationSymbol(item, ctx, item->mutLayoutData());
}

void TLayout::layout(TempoText* item, LayoutContext& ctx)
{
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    TempoText::LayoutData* ldata = item->mutLayoutData();

    layoutTextBase(item, ctx);

    Segment* s = item->segment();

    // tempo text on first chordrest of measure should align over time sig if present
    //
    if (item->autoplace() && s->rtick().isZero()) {
        Segment* p = item->segment()->prev(SegmentType::TimeSig);
        if (p) {
            ldata->moveX(-(s->x() - p->x()));
            EngravingItem* e = p->element(item->staffIdx() * VOICES);
            if (e) {
                ldata->moveX(e->x());
            }
        }
    }
    Autoplace::autoplaceSegmentElement(item, item->mutLayoutData());
}

void TLayout::layout(TextBase* item, LayoutContext& ctx)
{
    layoutItem(item, ctx);
}

void TLayout::layoutTextBase(const TextBase* item, const LayoutContext& ctx, TextBase::LayoutData* ldata)
{
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    ldata->setPos(PointF());

    if (item->placeBelow()) {
        ldata->setPosY(item->staff() ? item->staff()->height() : 0.0);
    }

    if (Harmony::classof(item)) {
        layout1(static_cast<Harmony*>(const_cast<TextBase*>(item)), ctx);
    } else {
        layout1TextBase(const_cast<TextBase*>(item), ctx);
    }
}

void TLayout::layoutTextBase(TextBase* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx, item->mutLayoutData());
}

void TLayout::layout1TextBase(const TextBase* item, const LayoutContext&, TextBase::LayoutData* ldata)
{
    if (ldata->layoutInvalid) {
        item->createBlocks(ldata);
    }

    RectF bb;
    double y = 0.0;

    // adjust the bounding box for the text item
    for (size_t i = 0; i < ldata->blocks.size(); ++i) {
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
    if (item->explicitParent()) {
        if (item->layoutToParentWidth()) {
            if (item->explicitParent()->isTBox()) {
                // hack: vertical alignment is always TOP
                const_cast<TextBase*>(item)->setAlign({ item->align().horizontal, AlignV::TOP });
            } else if (item->explicitParent()->isBox()) {
                // consider inner margins of frame
                Box* b = toBox(item->explicitParent());
                yoff = b->topMargin() * DPMM;

                if (b->height() < bb.bottom()) {
                    h = b->height() / 2 + bb.height();
                } else {
                    h  = b->height() - yoff - b->bottomMargin() * DPMM;
                }
            } else if (item->explicitParent()->isPage()) {
                Page* p = toPage(item->explicitParent());
                h = p->height() - p->tm() - p->bm();
                yoff = p->tm();
            } else if (item->explicitParent()->isMeasure()) {
            } else {
                h  = item->parentItem()->height();
            }
        }
    } else {
        ldata->setPos(PointF());
    }

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

void TLayout::layout1TextBase(TextBase* item, const LayoutContext& ctx)
{
    layout1TextBase(item, ctx, item->mutLayoutData());
}

void TLayout::layout(Text* item, LayoutContext& ctx)
{
    layoutTextBase(static_cast<TextBase*>(item), ctx);
}

void TLayout::layout(TextLine* item, LayoutContext& ctx)
{
    layoutLine(item, ctx);
}

void TLayout::layout(TextLineSegment* item, LayoutContext& ctx)
{
    TextLineSegment::LayoutData* ldata = item->mutLayoutData();
    layoutTextLineBaseSegment(item, ctx);
    if (item->isStyled(Pid::OFFSET)) {
        item->roffset() = item->textLine()->propertyDefault(Pid::OFFSET).value<PointF>();
    }
    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

// Extends lines to fill the corner between them.
// Assumes that l1p2 == l2p1 is the intersection between the lines.
// If checkAngle is false, assumes that the lines are perpendicular,
// and some calculations are saved.
static inline void extendLines(const PointF& l1p1, PointF& l1p2, PointF& l2p1, const PointF& l2p2, double lineWidth, bool checkAngle)
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
}

void TLayout::layoutTextLineBase(TextLineBase* item, LayoutContext& ctx)
{
    layoutLine(item, ctx);
}

void TLayout::layoutTextLineBaseSegment(TextLineBaseSegment* item, LayoutContext& ctx)
{
    TextLineBaseSegment::LayoutData* ldata = item->mutLayoutData();
    item->npointsRef() = 0;
    TextLineBase* tl = item->textLineBase();
    double _spatium = tl->spatium();

    if (item->spanner()->placeBelow()) {
        ldata->setPosY(item->staff() ? item->staff()->height() : 0.0);
    }

    // adjust Y pos to staffType offset
    if (const StaffType* st = item->staffType()) {
        ldata->moveY(st->yoffset().val() * item->spatium());
    }

    if (!tl->diagonal()) {
        item->setUserYoffset2(0);
    }

    auto alignBaseLine = [tl](Text* text, PointF& pp1, PointF& pp2) {
        PointF widthCorrection(0.0, tl->lineWidth() / 2);
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
    item->text()->setTrack(item->track());
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

        item->setbbox(TextLineBaseSegment::boundingBoxOfLine(pp1, pp2, tl->lineWidth() / 2, tl->lineStyle() == LineType::DOTTED));
        return;
    }

    // line has text or hooks or is not diagonal - calculate reasonable bbox

    double x1 = std::min(0.0, pp2.x());
    double x2 = std::max(0.0, pp2.x());
    double y0 = -tl->lineWidth();
    double y1 = std::min(0.0, pp2.y()) + y0;
    double y2 = std::max(0.0, pp2.y()) - y0;

    double l = 0.0;
    if (!item->text()->empty()) {
        double gapBetweenTextAndLine = _spatium * tl->gapBetweenTextAndLine().val();
        if ((item->isSingleBeginType() && (tl->beginTextPlace() == TextPlace::LEFT || tl->beginTextPlace() == TextPlace::AUTO))
            || (!item->isSingleBeginType() && (tl->continueTextPlace() == TextPlace::LEFT || tl->continueTextPlace() == TextPlace::AUTO))) {
            l = item->text()->pos().x() + item->text()->layoutData()->bbox().width() + gapBetweenTextAndLine;
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
        double h = pp2.y() + tl->endHookHeight().val() * _spatium;
        if (h > y2) {
            y2 = h;
        } else if (h < y1) {
            y1 = h;
        }
    }

    if (tl->beginHookType() != HookType::NONE) {
        double h = tl->beginHookHeight().val() * _spatium;
        if (h > y2) {
            y2 = h;
        } else if (h < y1) {
            y1 = h;
        }
    }
    ldata->setBbox(x1, y1, x2 - x1, y2 - y1);
    if (!item->text()->empty()) {
        ldata->addBbox(item->text()->layoutData()->bbox().translated(item->text()->pos()));      // DEBUG
    }
    // set end text position and extend bbox
    if (!item->endText()->empty()) {
        item->endText()->mutLayoutData()->moveX(ldata->bbox().right());
        ldata->addBbox(item->endText()->layoutData()->bbox().translated(item->endText()->pos()));
    }

    if (!(tl->lineVisible() || ctx.conf().isShowInvisible())) {
        return;
    }

    if (tl->lineVisible() || !ctx.conf().isPrintingMode()) {
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

        double beginHookHeight = tl->beginHookHeight().val() * _spatium;
        double endHookHeight = tl->endHookHeight().val() * _spatium;
        double beginHookWidth = 0.0;
        double endHookWidth = 0.0;

        if (tl->beginHookType() == HookType::HOOK_45) {
            beginHookWidth = fabs(beginHookHeight * .4);
            pp1.rx() += beginHookWidth;
        }

        if (tl->endHookType() == HookType::HOOK_45) {
            endHookWidth = fabs(endHookHeight * .4);
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
                    extendLines(beginHookEndpoint, beginHookStartpoint, pp1, pp2, tl->lineWidth() * item->mag(), checkAngle);
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
                    extendLines(pp1, pp22, endHookStartpoint, endHookEndpoint, tl->lineWidth() * item->mag(), checkAngle);
                }
            }

            item->pointsRef()[item->npointsRef()++] = endHookEndpoint;
        }

        item->setLineLength(sqrt(PointF::dotProduct(pp22 - pp1, pp22 - pp1)));
    }
}

void TLayout::layout(Tie* item, LayoutContext&)
{
    UNUSED(item);
}

static void layoutTimeSig(const TimeSig* item, const LayoutContext& ctx, TimeSig::LayoutData* ldata)
{
    ldata->setPos(0.0, 0.0);
    double _spatium = item->spatium();

    ldata->setBbox(RectF());                    // prepare for an empty time signature

    ldata->pointLargeLeftParen = PointF();
    ldata->pz = PointF();
    ldata->pn = PointF();
    ldata->pointLargeRightParen = PointF();

    double lineDist;
    int numOfLines;
    TimeSigType sigType = item->timeSigType();
    const Staff* _staff = item->staff();

    if (_staff) {
        // if staff is without time sig, format as if no text at all
        if (!_staff->staffTypeForElement(item)->genTimesig()) {
            // reset position and box sizes to 0
            // LOGD("staff: no time sig");
            ldata->pointLargeLeftParen.rx() = 0.0;
            ldata->pn.rx() = 0.0;
            ldata->pz.rx() = 0.0;
            ldata->pointLargeRightParen.rx() = 0.0;
            ldata->setBbox(RectF());
            // leave everything else as it is:
            // draw() will anyway skip any drawing if staff type has no time sigs
            return;
        }
        numOfLines  = _staff->lines(item->tick());
        lineDist    = _staff->lineDistance(item->tick());
    } else {
        // assume dimensions of a standard staff
        lineDist = 1.0;
        numOfLines = 5;
    }

    // if some symbol
    // compute vert. displacement to center in the staff height
    // determine middle staff position:

    double yoff = _spatium * (numOfLines - 1) * .5 * lineDist;

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

        double displ = (numOfLines & 1) ? 0.0 : (0.05 * _spatium);

        //align on the wider
        double pzY = yoff - (denRect.width() < 0.01 ? 0.0 : (displ + numRect.height() * .5));
        double pnY = yoff + displ + denRect.height() * .5;

        if (numRect.width() >= denRect.width()) {
            // numerator: one space above centre line, unless denomin. is empty (if so, directly centre in the middle)
            ldata->pz = PointF(0.0, pzY);
            // denominator: horiz: centred around centre of numerator | vert: one space below centre line
            ldata->pn = PointF((numRect.width() - denRect.width()) * .5, pnY);
        } else {
            // numerator: one space above centre line, unless denomin. is empty (if so, directly centre in the middle)
            ldata->pz = PointF((denRect.width() - numRect.width()) * .5, pzY);
            // denominator: horiz: centred around centre of numerator | vert: one space below centre line
            ldata->pn = PointF(0.0, pnY);
        }

        // centering of parenthesis so the middle of the parenthesis is at the divisor marking level
        int centerY = yoff / 2 + _spatium;
        int widestPortion = numRect.width() > denRect.width() ? numRect.width() : denRect.width();
        ldata->pointLargeLeftParen = PointF(-_spatium, centerY);
        ldata->pointLargeRightParen = PointF(widestPortion + _spatium, centerY);

        ldata->setBbox(numRect.translated(ldata->pz));       // translate bounding boxes to actual string positions
        ldata->addBbox(denRect.translated(ldata->pn));
        if (item->largeParentheses()) {
            ldata->addBbox(RectF(ldata->pointLargeLeftParen.x(), ldata->pointLargeLeftParen.y() - denRect.height(), _spatium / 2,
                                 numRect.height() + denRect.height()));
            ldata->addBbox(RectF(ldata->pointLargeRightParen.x(), ldata->pointLargeRightParen.y() - denRect.height(),  _spatium / 2,
                                 numRect.height() + denRect.height()));
        }
    }
}

void TLayout::layout(TimeSig* item, LayoutContext& ctx)
{
    layoutTimeSig(item, ctx, item->mutLayoutData());
}

void TLayout::layout(Tremolo* item, LayoutContext& ctx)
{
    TremoloLayout::layout(item, ctx);
}

static void layoutTremoloBar(const TremoloBar* item, const LayoutContext&, TremoloBar::LayoutData* ldata)
{
    double _spatium = item->spatium();

    ldata->setPos(0.0, -_spatium * 3.0);

    /* we place the tremolo bars starting slightly before the
     *  notehead, and end it slightly after, drawing above the
     *  note. The values specified in Guitar Pro are very large, too
     *  large for the scale used in Musescore. We used the
     *  timeFactor and pitchFactor below to reduce these values down
     *  consistently to values that make sense to draw with the
     *  Musescore scale. */

    double timeFactor  = item->userMag() / 1.0;
    double pitchFactor = -_spatium * .02;

    PolygonF polygon;
    for (const PitchValue& v : item->points()) {
        polygon << PointF(v.time * timeFactor, v.pitch * pitchFactor);
    }
    ldata->polygon = polygon;

    double w = item->lineWidth().val();
    ldata->setBbox(ldata->polygon.boundingRect().adjusted(-w, -w, w, w));
}

void TLayout::layout(TremoloBar* item, LayoutContext& ctx)
{
    layoutTremoloBar(item, ctx, item->mutLayoutData());
}

void TLayout::layout(TrillSegment* item, LayoutContext& ctx)
{
    TrillSegment::LayoutData* ldata = item->mutLayoutData();
    EngravingItem* startItem = item->trill()->startElement();
    Chord* startChord = startItem && startItem->isChord() ? toChord(startItem) : nullptr;
    if (startChord) {
        // Semi-hack: spanners don't have staffMove property, so we change
        // the staffIdx itself to follow cross-staff chords if needed
        item->setStaffIdx(startChord->vStaffIdx());
    }

    if (item->staff()) {
        ldata->setMag(item->staff()->staffMag(item->tick()));
    }
    if (item->spanner()->placeBelow()) {
        ldata->setPosY(item->staff() ? item->staff()->height() : 0.0);
    }

    bool accidentalGoesBelow = item->trill()->trillType() == TrillType::DOWNPRALL_LINE;
    Trill* trill = item->trill();
    Ornament* ornament = trill->ornament();
    if (ornament) {
        if (item->isSingleBeginType()) {
            TLayout::layout(ornament, ornament->mutLayoutData(), ctx.conf());
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
            double minVertDist = accidentalGoesBelow ? Shape(box).minVerticalDistance(a->shape())
                                 : a->shape().minVerticalDistance(Shape(box));
            y = accidentalGoesBelow ? minVertDist + vertMargin : -minVertDist - vertMargin;
            a->setPos(x, y);
            a->setParent(item);
        }
    } else {
        item->symbolLine(SymId::wiggleTrill, SymId::wiggleTrill);
    }

    if (item->isStyled(Pid::OFFSET)) {
        item->roffset() = item->trill()->propertyDefault(Pid::OFFSET).value<PointF>();
    }

    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layout(TripletFeel* item, LayoutContext& ctx)
{
    layout(static_cast<SystemText*>(item), ctx);
}

void TLayout::layout(Trill* item, LayoutContext& ctx)
{
    layoutLine(static_cast<SLine*>(item), ctx);
}

void TLayout::layout(Tuplet* item, LayoutContext& ctx)
{
    TupletLayout::layout(item, ctx);
}

void TLayout::layout(VibratoSegment* item, LayoutContext& ctx)
{
    VibratoSegment::LayoutData* ldata = item->mutLayoutData();
    if (item->staff()) {
        ldata->setMag(item->staff()->staffMag(item->tick()));
    }
    if (item->spanner()->placeBelow()) {
        ldata->setPosY(item->staff() ? item->staff()->height() : 0.0);
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

    if (item->isStyled(Pid::OFFSET)) {
        item->roffset() = item->vibrato()->propertyDefault(Pid::OFFSET).value<PointF>();
    }

    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layout(Vibrato* item, LayoutContext& ctx)
{
    layoutLine(static_cast<SLine*>(item), ctx);

    if (ctx.conf().isPaletteMode()) {
        return;
    }
    if (item->spannerSegments().empty()) {
        LOGD("Vibrato: no segments");
        return;
    }
}

void TLayout::layout(Volta* item, LayoutContext& ctx)
{
    layoutLine(item, ctx);
}

void TLayout::layout(VoltaSegment* item, LayoutContext& ctx)
{
    VoltaSegment::LayoutData* ldata = item->mutLayoutData();
    layoutTextLineBaseSegment(item, ctx);
    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layout(WhammyBarSegment* item, LayoutContext& ctx)
{
    WhammyBarSegment::LayoutData* ldata = item->mutLayoutData();
    layoutTextLineBaseSegment(item, ctx);
    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

using LayoutSystemTypes = rtti::TypeList<LyricsLine, Slur, Volta>;

class LayoutSystemVisitor : public rtti::Visitor<LayoutSystemVisitor>
{
public:
    template<typename T>
    static bool doVisit(Spanner* item, System* system, LayoutContext& ctx, SpannerSegment** segOut)
    {
        if (T::classof(item)) {
            *segOut = TLayout::layoutSystem(static_cast<T*>(item), system, ctx);
            return true;
        }
        return false;
    }
};

SpannerSegment* TLayout::layoutSystem(Spanner* item, System* system, LayoutContext& ctx)
{
    SpannerSegment* seg = nullptr;
    bool found = LayoutSystemVisitor::visit(LayoutSystemTypes {}, item, system, ctx, &seg);
    if (!found) {
        SLine* line = dynamic_cast<SLine*>(item);
        if (line) {
            seg = layoutSystemSLine(line, system, ctx);
        } else {
            DO_ASSERT(found);
        }
    }
    return seg;
}

SpannerSegment* TLayout::getNextLayoutSystemSegment(Spanner* spanner, System* system,
                                                    std::function<SpannerSegment* (System* parent)> createSegment)
{
    SpannerSegment* seg = nullptr;
    for (SpannerSegment* ss : spanner->spannerSegments()) {
        if (!ss->system()) {
            seg = ss;
            break;
        }
    }
    if (!seg) {
        if ((seg = spanner->popUnusedSegment())) {
            spanner->reuse(seg);
        } else {
            seg = createSegment(system);
            assert(seg);
            spanner->add(seg);
        }
    }
    seg->setSystem(system);
    seg->setSpanner(spanner);
    seg->setTrack(spanner->track());
    seg->setVisible(spanner->visible());
    return seg;
}

// layout spannersegment for system
SpannerSegment* TLayout::layoutSystemSLine(SLine* line, System* system, LayoutContext& ctx)
{
    Fraction stick = system->firstMeasure()->tick();
    Fraction etick = system->lastMeasure()->endTick();

    LineSegment* lineSegm = toLineSegment(TLayout::getNextLayoutSystemSegment(line, system, [line](System* parent) {
        return line->createLineSegment(parent);
    }));

    SpannerSegmentType sst;
    if (line->tick() >= stick) {
        //
        // this is the first call to layoutSystem,
        // processing the first line segment
        //
        line->computeStartElement();
        line->computeEndElement();
        sst = line->tick2() <= etick ? SpannerSegmentType::SINGLE : SpannerSegmentType::BEGIN;
    } else if (line->tick() < stick && line->tick2() > etick) {
        sst = SpannerSegmentType::MIDDLE;
    } else {
        //
        // this is the last call to layoutSystem
        // processing the last line segment
        //
        sst = SpannerSegmentType::END;
    }
    lineSegm->setSpannerSegmentType(sst);

    switch (sst) {
    case SpannerSegmentType::SINGLE: {
        System* s;
        PointF p1 = line->linePos(Grip::START, &s);
        PointF p2 = line->linePos(Grip::END,   &s);
        double len = p2.x() - p1.x();
        lineSegm->setPos(p1);
        lineSegm->setPos2(PointF(len, p2.y() - p1.y()));
    }
    break;
    case SpannerSegmentType::BEGIN: {
        System* s;
        PointF p1 = line->linePos(Grip::START, &s);
        lineSegm->setPos(p1);
        double x2 = system->endingXForOpenEndedLines();
        lineSegm->setPos2(PointF(x2 - p1.x(), 0.0));
    }
    break;
    case SpannerSegmentType::MIDDLE: {
        double x1 = system->firstNoteRestSegmentX(true);
        double x2 = system->endingXForOpenEndedLines();
        System* s;
        PointF p1 = line->linePos(Grip::START, &s);
        lineSegm->setPos(PointF(x1, p1.y()));
        lineSegm->setPos2(PointF(x2 - x1, 0.0));
    }
    break;
    case SpannerSegmentType::END: {
        System* s;
        PointF p2 = line->linePos(Grip::END,   &s);
        double x1 = system->firstNoteRestSegmentX(true);
        double len = p2.x() - x1;
        lineSegm->setPos(PointF(p2.x() - len, p2.y()));
        lineSegm->setPos2(PointF(len, 0.0));
    }
    break;
    }

    layout(lineSegm, ctx);

    return lineSegm;
}

SpannerSegment* TLayout::layoutSystem(LyricsLine* line, System* system, LayoutContext& ctx)
{
    if (!line->lyrics()) {
        return nullptr; // a lyrics line with no lyrics shouldn't exist
    }
    Fraction stick = system->firstMeasure()->tick();
    Fraction etick = system->lastMeasure()->endTick();

    LyricsLineSegment* lineSegm = toLyricsLineSegment(TLayout::getNextLayoutSystemSegment(line, system, [line](System* parent) {
        return line->createLineSegment(parent);
    }));

    SpannerSegmentType sst;
    if (line->tick() >= stick) {
        TLayout::layout(line, ctx);
        if (line->ticks().isZero() && line->isEndMelisma()) { // only do layout if some time span
            // dash lines still need to be laid out, though
            return nullptr;
        }

        TLayout::layoutLine(line, ctx);
        //
        // this is the first call to layoutSystem,
        // processing the first line segment
        //
        line->computeStartElement();
        line->computeEndElement();
        sst = line->tick2() <= etick ? SpannerSegmentType::SINGLE : SpannerSegmentType::BEGIN;
    } else if (line->tick() < stick && line->tick2() > etick) {
        sst = SpannerSegmentType::MIDDLE;
    } else {
        //
        // this is the last call to layoutSystem
        // processing the last line segment
        //
        sst = SpannerSegmentType::END;
    }
    lineSegm->setSpannerSegmentType(sst);

    switch (sst) {
    case SpannerSegmentType::SINGLE: {
        System* s;
        PointF p1 = line->linePos(Grip::START, &s);
        PointF p2 = line->linePos(Grip::END,   &s);
        double len = p2.x() - p1.x();
        lineSegm->setPos(p1);
        lineSegm->setPos2(PointF(len, p2.y() - p1.y()));
    }
    break;
    case SpannerSegmentType::BEGIN: {
        System* s;
        PointF p1 = line->linePos(Grip::START, &s);
        lineSegm->setPos(p1);
        double x2 = system->endingXForOpenEndedLines();
        lineSegm->setPos2(PointF(x2 - p1.x(), 0.0));
    }
    break;
    case SpannerSegmentType::MIDDLE: {
        bool leading = (line->anchor() == Spanner::Anchor::SEGMENT || line->anchor() == Spanner::Anchor::MEASURE);
        double x1 = system->firstNoteRestSegmentX(leading);
        double x2 = system->endingXForOpenEndedLines();
        System* s;
        PointF p1 = line->linePos(Grip::START, &s);
        lineSegm->setPos(PointF(x1, p1.y()));
        lineSegm->setPos2(PointF(x2 - x1, 0.0));
    }
    break;
    case SpannerSegmentType::END: {
        System* s;
        PointF p2 = line->linePos(Grip::END, &s);
        bool leading = (line->anchor() == Spanner::Anchor::SEGMENT || line->anchor() == Spanner::Anchor::MEASURE);
        double x1 = system->firstNoteRestSegmentX(leading);
        double len = p2.x() - x1;
        lineSegm->setPos(PointF(p2.x() - len, p2.y()));
        lineSegm->setPos2(PointF(len, 0.0));
    }
    break;
    }

    TLayout::layout(lineSegm, ctx);
    if (!line->lyrics()) {
        // this line could have been removed in the process of laying out surrounding lyrics
        return nullptr;
    }
    // if temp melisma extend the first line segment to be
    // after the lyrics syllable (otherwise the melisma segment
    // will be too short).
    const bool tempMelismaTicks = (line->lyrics()->ticks() == Fraction::fromTicks(Lyrics::TEMP_MELISMA_TICKS));
    if (tempMelismaTicks && line->spannerSegments().size() > 0 && line->spannerSegments().front() == lineSegm) {
        lineSegm->rxpos2() += line->lyrics()->width();
    }
    // avoid backwards melisma
    if (lineSegm->pos2().x() < 0) {
        lineSegm->rxpos2() = 0;
    }
    return lineSegm;
}

SpannerSegment* TLayout::layoutSystem(Volta* line, System* system, LayoutContext& ctx)
{
    SpannerSegment* voltaSegment = layoutSystemSLine(line, system, ctx);

    // we need set tempo in layout because all tempos of score is set in layout
    // so fermata in seconda volta works correct because fermata apply itself tempo during layouting
    line->setTempo();

    return voltaSegment;
}

SpannerSegment* TLayout::layoutSystem(Slur* line, System* system, LayoutContext& ctx)
{
    return SlurTieLayout::layoutSystem(line, system, ctx);
}

// Called after layout of all systems is done so precise
// number of systems for this spanner becomes available.
void TLayout::layoutSystemsDone(Spanner* item)
{
    std::vector<SpannerSegment*> validSegments;
    for (SpannerSegment* seg : item->spannerSegments()) {
        if (seg->system()) {
            validSegments.push_back(seg);
        } else { // TODO: score()->selection().remove(ss); needed?
            item->pushUnusedSegment(seg);
        }
    }
    item->setSpannerSegments(validSegments);
}
