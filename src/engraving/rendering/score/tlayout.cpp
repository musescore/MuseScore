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
#include <cfloat>

#include "tlayout.h"

#include "global/realfn.h"
#include "global/types/number.h"
#include "draw/fontmetrics.h"

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

#include "dom/glissando.h"
#include "dom/gradualtempochange.h"
#include "dom/guitarbend.h"

#include "dom/hairpin.h"
#include "dom/hammeronpulloff.h"
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
#include "dom/noteline.h"

#include "dom/ornament.h"
#include "dom/ottava.h"

#include "dom/page.h"
#include "dom/palmmute.h"
#include "dom/parenthesis.h"
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
#include "dom/stringtunings.h"
#include "dom/symbol.h"
#include "dom/system.h"
#include "dom/systemdivider.h"
#include "dom/systemtext.h"
#include "dom/soundflag.h"

#include "dom/tapping.h"
#include "dom/tempotext.h"
#include "dom/text.h"
#include "dom/textline.h"
#include "dom/tie.h"
#include "dom/timesig.h"
#include "dom/tremolosinglechord.h"
#include "dom/tremolotwochord.h"
#include "dom/tremolobar.h"
#include "dom/trill.h"
#include "dom/tripletfeel.h"
#include "dom/tuplet.h"

#include "dom/vibrato.h"
#include "dom/volta.h"

#include "dom/whammybar.h"

#include "dom/factory.h"

#include "accidentalslayout.h"
#include "autoplace.h"
#include "beamlayout.h"
#include "chordlayout.h"
#include "dynamicslayout.h"
#include "guitarbendlayout.h"
#include "lyricslayout.h"
#include "masklayout.h"
#include "slurtielayout.h"
#include "tremololayout.h"
#include "tupletlayout.h"
#include "horizontalspacing.h"
#include "measurelayout.h"
#include "tappinglayout.h"

using namespace muse;
using namespace muse::draw;
using namespace mu::engraving;
using namespace mu::engraving::rtti;
using namespace mu::engraving::rendering::score;

#define LAYOUT_CALL_ITEM(item) LAYOUT_CALL() << LAYOUT_ITEM_INFO(item);

void TLayout::layoutItem(EngravingItem* item, LayoutContext& ctx)
{
    //DO_ASSERT(!ctx.conf().isPaletteMode());

    EngravingItem::LayoutData* ldata = item->mutldata();

    switch (item->type()) {
    case ElementType::ACCIDENTAL:
        layoutAccidental(item_cast<const Accidental*>(item), static_cast<Accidental::LayoutData*>(ldata), ctx.conf());
        break;
    case ElementType::ACTION_ICON:
        layoutActionIcon(item_cast<const ActionIcon*>(item), static_cast<ActionIcon::LayoutData*>(ldata));
        break;
    case ElementType::AMBITUS:
        layoutAmbitus(item_cast<const Ambitus*>(item), static_cast<Ambitus::LayoutData*>(ldata), ctx);
        break;
    case ElementType::ARPEGGIO:
        layoutArpeggio(item_cast<const Arpeggio*>(item), static_cast<Arpeggio::LayoutData*>(ldata), ctx.conf());
        break;
    case ElementType::ARTICULATION:
        layoutArticulation(item_cast<const Articulation*>(item), static_cast<Articulation::LayoutData*>(ldata));
        break;
    case ElementType::BAR_LINE:
        layoutBarLine(item_cast<const BarLine*>(item), static_cast<BarLine::LayoutData*>(ldata), ctx);
        break;
    case ElementType::BEAM:             layoutBeam(item_cast<Beam*>(item), ctx);
        break;
    case ElementType::BEND:
        layoutBend(item_cast<const Bend*>(item), static_cast<Bend::LayoutData*>(ldata));
        break;
    case ElementType::HBOX:
        layoutHBox(item_cast<const HBox*>(item), static_cast<HBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::VBOX:
        layoutVBox(item_cast<const VBox*>(item), static_cast<VBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::FBOX:
        layoutFBox(item_cast<const FBox*>(item), static_cast<FBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::TBOX:
        layoutTBox(item_cast<const TBox*>(item), static_cast<TBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::BRACKET:
        layoutBracket(item_cast<const Bracket*>(item), static_cast<Bracket::LayoutData*>(ldata), ctx.conf());
        break;
    case ElementType::BREATH:
        layoutBreath(item_cast<const Breath*>(item), static_cast<Breath::LayoutData*>(ldata), ctx.conf());
        break;
    case ElementType::CHORD:            layoutChord(item_cast<Chord*>(item), ctx);
        break;
    case ElementType::CHORDLINE:
        layoutChordLine(item_cast<const ChordLine*>(item), static_cast<ChordLine::LayoutData*>(ldata), ctx.conf());
        break;
    case ElementType::CLEF:
        layoutClef(item_cast<const Clef*>(item), static_cast<Clef::LayoutData*>(ldata), ctx.conf());
        break;
    case ElementType::CAPO:
        layoutCapo(item_cast<const Capo*>(item), static_cast<Capo::LayoutData*>(ldata), ctx);
        break;
    case ElementType::DEAD_SLAPPED:
        layoutDeadSlapped(item_cast<const DeadSlapped*>(item), static_cast<DeadSlapped::LayoutData*>(ldata));
        break;
    case ElementType::DYNAMIC:
        layoutDynamic(item_cast<Dynamic*>(item), static_cast<Dynamic::LayoutData*>(ldata), ctx.conf());
        break;
    case ElementType::EXPRESSION:
        layoutExpression(item_cast<const Expression*>(item), static_cast<Expression::LayoutData*>(ldata));
        break;
    case ElementType::FERMATA:
        layoutFermata(item_cast<const Fermata*>(item), static_cast<Fermata::LayoutData*>(ldata), ctx.conf());
        break;
    case ElementType::FIGURED_BASS:
        layoutFiguredBass(item_cast<const FiguredBass*>(item), static_cast<FiguredBass::LayoutData*>(ldata), ctx);
        break;
    case ElementType::FINGERING:
        layoutFingering(item_cast<const Fingering*>(item), static_cast<Fingering::LayoutData*>(ldata));
        break;
    case ElementType::FRET_DIAGRAM:
        layoutFretDiagram(item_cast<const FretDiagram*>(item), static_cast<FretDiagram::LayoutData*>(ldata), ctx);
        break;
    case ElementType::GLISSANDO:        layoutGlissando(item_cast<Glissando*>(item), ctx);
        break;
    case ElementType::GLISSANDO_SEGMENT: layoutGlissandoSegment(item_cast<GlissandoSegment*>(item), ctx);
        break;
    case ElementType::GRADUAL_TEMPO_CHANGE: layoutGradualTempoChange(item_cast<GradualTempoChange*>(item), ctx);
        break;
    case ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT: layoutGradualTempoChangeSegment(item_cast<GradualTempoChangeSegment*>(item), ctx);
        break;
    case ElementType::GUITAR_BEND: layoutGuitarBend(item_cast<GuitarBend*>(item), ctx);
        break;
    case ElementType::GUITAR_BEND_SEGMENT: layoutGuitarBendSegment(item_cast<GuitarBendSegment*>(item), ctx);
        break;
    case ElementType::GUITAR_BEND_HOLD_SEGMENT: GuitarBendLayout::layoutHoldLine(item_cast<GuitarBendHoldSegment*>(item));
        break;
    case ElementType::HAIRPIN:          layoutHairpin(item_cast<Hairpin*>(item), ctx);
        break;
    case ElementType::HAIRPIN_SEGMENT:  layoutHairpinSegment(item_cast<HairpinSegment*>(item), ctx);
        break;
    case ElementType::HAMMER_ON_PULL_OFF: layoutHammerOnPullOff(item_cast<HammerOnPullOff*>(item), ctx);
        break;
    case ElementType::HAMMER_ON_PULL_OFF_SEGMENT: layoutHammerOnPullOffSegment(item_cast<HammerOnPullOffSegment*>(item), ctx);
        break;
    case ElementType::HAMMER_ON_PULL_OFF_TEXT: layoutHammerOnPullOffText(item_cast<HammerOnPullOffText*>(item), ctx);
        break;
    case ElementType::HARP_DIAGRAM:
        layoutHarpPedalDiagram(item_cast<const HarpPedalDiagram*>(item), static_cast<HarpPedalDiagram::LayoutData*>(ldata));
        break;
    case ElementType::HARMONY:
        layoutHarmony(item_cast<const Harmony*>(item), static_cast<Harmony::LayoutData*>(ldata), ctx);
        break;
    case ElementType::HARMONIC_MARK_SEGMENT: layoutHarmonicMarkSegment(item_cast<HarmonicMarkSegment*>(item), ctx);
        break;
    case ElementType::HOOK:
        layoutHook(item_cast<const Hook*>(item), static_cast<Hook::LayoutData*>(ldata));
        break;
    case ElementType::IMAGE:
        layoutImage(item_cast<const Image*>(item), static_cast<Image::LayoutData*>(ldata));
        break;
    case ElementType::INSTRUMENT_CHANGE:
        layoutInstrumentChange(item_cast<const InstrumentChange*>(item), static_cast<InstrumentChange::LayoutData*>(ldata));
        break;
    case ElementType::INSTRUMENT_NAME:
        layoutInstrumentName(item_cast<const InstrumentName*>(item), static_cast<InstrumentName::LayoutData*>(ldata));
        break;
    case ElementType::JUMP:
        layoutJump(item_cast<const Jump*>(item), static_cast<Jump::LayoutData*>(ldata));
        break;
    case ElementType::KEYSIG:
        layoutKeySig(item_cast<const KeySig*>(item), static_cast<KeySig::LayoutData*>(ldata), ctx.conf());
        break;
    case ElementType::LAISSEZ_VIB:
        layoutLaissezVib(item_cast<LaissezVib*>(item));
        break;
    case ElementType::LAYOUT_BREAK:
        layoutLayoutBreak(item_cast<const LayoutBreak*>(item), static_cast<LayoutBreak::LayoutData*>(ldata));
        break;
    case ElementType::SYSTEM_LOCK_INDICATOR:
        layoutSystemLockIndicator(item_cast<const SystemLockIndicator*>(item), static_cast<SystemLockIndicator::LayoutData*>(ldata));
        break;
    case ElementType::LET_RING:         layoutLetRing(item_cast<LetRing*>(item), ctx);
        break;
    case ElementType::LET_RING_SEGMENT: layoutLetRingSegment(item_cast<LetRingSegment*>(item), ctx);
        break;
    case ElementType::LEDGER_LINE:      layoutLedgerLine(item_cast<LedgerLine*>(item), ctx);
        break;
    case ElementType::LYRICS:           layoutLyrics(item_cast<Lyrics*>(item), ctx);
        break;
    case ElementType::LYRICSLINE_SEGMENT: layoutLyricsLineSegment(item_cast<LyricsLineSegment*>(item), ctx);
        break;
    case ElementType::PARTIAL_LYRICSLINE_SEGMENT: layoutLyricsLineSegment(item_cast<LyricsLineSegment*>(item), ctx);
        break;
    case ElementType::MARKER:
        layoutMarker(item_cast<const Marker*>(item), static_cast<Marker::LayoutData*>(ldata));
        break;
    case ElementType::MEASURE_NUMBER:
        layoutMeasureNumber(item_cast<const MeasureNumber*>(item), static_cast<MeasureNumber::LayoutData*>(ldata), ctx);
        break;
    case ElementType::MEASURE_REPEAT:
        layoutMeasureRepeat(item_cast<const MeasureRepeat*>(item), static_cast<MeasureRepeat::LayoutData*>(ldata), ctx);
        break;
    case ElementType::MMREST:
        layoutMMRest(item_cast<const MMRest*>(item), static_cast<MMRest::LayoutData*>(ldata), ctx);
        break;
    case ElementType::MMREST_RANGE:
        layoutMMRestRange(item_cast<const MMRestRange*>(item), static_cast<MMRestRange::LayoutData*>(ldata), ctx);
        break;
    case ElementType::NOTE:
        layoutNote(item_cast<const Note*>(item), static_cast<Note::LayoutData*>(ldata));
        break;
    case ElementType::NOTEDOT:
        layoutNoteDot(item_cast<const NoteDot*>(item), static_cast<NoteDot::LayoutData*>(ldata));
        break;
    case ElementType::NOTEHEAD:
        layoutSymbol(item_cast<const NoteHead*>(item), static_cast<NoteHead::LayoutData*>(ldata), ctx);
        break;
    case ElementType::NOTELINE:
        layoutNoteLine(item_cast<NoteLine*>(item), ctx);
        break;
    case ElementType::NOTELINE_SEGMENT:
        layoutNoteLineSegment(item_cast<NoteLineSegment*>(item), ctx);
        break;
    case ElementType::ORNAMENT:
        layoutOrnament(item_cast<const Ornament*>(item), static_cast<Ornament::LayoutData*>(ldata), ctx.conf());
        break;
    case ElementType::OTTAVA:           layoutOttava(item_cast<Ottava*>(item), ctx);
        break;
    case ElementType::OTTAVA_SEGMENT:   layoutOttavaSegment(item_cast<OttavaSegment*>(item), ctx);
        break;
    case ElementType::PALM_MUTE:        layoutPalmMute(item_cast<PalmMute*>(item), ctx);
        break;
    case ElementType::PALM_MUTE_SEGMENT: layoutPalmMuteSegment(item_cast<PalmMuteSegment*>(item), ctx);
        break;
    case ElementType::PARENTHESIS:      layoutParenthesis(item_cast<Parenthesis*>(item), ctx);
        break;
    case ElementType::PEDAL:            layoutPedal(item_cast<Pedal*>(item), ctx);
        break;
    case ElementType::PEDAL_SEGMENT:    layoutPedalSegment(item_cast<PedalSegment*>(item), ctx);
        break;
    case ElementType::PICK_SCRAPE_SEGMENT: layoutPickScrapeSegment(item_cast<PickScrapeSegment*>(item), ctx);
        break;
    case ElementType::PLAYTECH_ANNOTATION:
        layoutPlayTechAnnotation(item_cast<const PlayTechAnnotation*>(item), static_cast<PlayTechAnnotation::LayoutData*>(ldata));
        break;
    case ElementType::RASGUEADO_SEGMENT: layoutRasgueadoSegment(item_cast<RasgueadoSegment*>(item), ctx);
        break;
    case ElementType::REHEARSAL_MARK:
        layoutRehearsalMark(item_cast<const RehearsalMark*>(item), static_cast<RehearsalMark::LayoutData*>(ldata));
        break;
    case ElementType::REST:
        layoutRest(item_cast<const Rest*>(item), static_cast<Rest::LayoutData*>(ldata), ctx);
        break;
    case ElementType::SHADOW_NOTE:      layoutShadowNote(item_cast<ShadowNote*>(item), ctx);
        break;
    case ElementType::SLUR:             layoutSlur(item_cast<Slur*>(item), ctx);
        break;
    case ElementType::SPACER:           layoutSpacer(item_cast<Spacer*>(item), ctx);
        break;
    case ElementType::STAFF_STATE:
        layoutStaffState(item_cast<const StaffState*>(item), static_cast<StaffState::LayoutData*>(ldata));
        break;
    case ElementType::STAFF_TEXT:
        layoutStaffText(item_cast<const StaffText*>(item), static_cast<StaffText::LayoutData*>(ldata));
        break;
    case ElementType::STAFFTYPE_CHANGE:
        layoutStaffTypeChange(item_cast<const StaffTypeChange*>(item), static_cast<StaffTypeChange::LayoutData*>(ldata), ctx.conf());
        break;
    case ElementType::SOUND_FLAG:
        layoutSoundFlag(item_cast<const SoundFlag*>(item), static_cast<SoundFlag::LayoutData*>(ldata));
        break;
    case ElementType::STEM:
        layoutStem(item_cast<const Stem*>(item), static_cast<Stem::LayoutData*>(ldata), ctx.conf());
        break;
    case ElementType::STEM_SLASH:
        layoutStemSlash(item_cast<const StemSlash*>(item), static_cast<StemSlash::LayoutData*>(ldata), ctx.conf());
        break;
    case ElementType::STICKING:
        layoutSticking(item_cast<const Sticking*>(item), static_cast<Sticking::LayoutData*>(ldata));
        break;
    case ElementType::STRING_TUNINGS:
        layoutStringTunings(item_cast<StringTunings*>(item), ctx);
        break;
    case ElementType::SYMBOL:
        layoutSymbol(item_cast<const Symbol*>(item), static_cast<Symbol::LayoutData*>(ldata), ctx);
        break;
    case ElementType::FSYMBOL:
        layoutFSymbol(item_cast<const FSymbol*>(item), static_cast<FSymbol::LayoutData*>(ldata));
        break;
    case ElementType::SYSTEM_DIVIDER:
        layoutSystemDivider(item_cast<const SystemDivider*>(item), static_cast<SystemDivider::LayoutData*>(ldata), ctx);
        break;
    case ElementType::SYSTEM_TEXT:
        layoutSystemText(item_cast<const SystemText*>(item), static_cast<SystemText::LayoutData*>(ldata));
        break;
    case ElementType::TAPPING:
        layoutTapping(toTapping(item), static_cast<Tapping::LayoutData*>(ldata), ctx);
        break;
    case ElementType::TAPPING_HALF_SLUR:
        layoutTappingHalfSlur(toTappingHalfSlur(item));
        break;
    case ElementType::TEMPO_TEXT:
        layoutTempoText(item_cast<const TempoText*>(item), static_cast<TempoText::LayoutData*>(ldata));
        break;
    case ElementType::TEXT:
        layoutText(item_cast<const Text*>(item), static_cast<Text::LayoutData*>(ldata));
        break;
    case ElementType::TEXTLINE:         layoutTextLine(item_cast<TextLine*>(item), ctx);
        break;
    case ElementType::TEXTLINE_SEGMENT: layoutTextLineSegment(item_cast<TextLineSegment*>(item), ctx);
        break;
    case ElementType::TIE:              layoutTie(item_cast<Tie*>(item), ctx);
        break;
    case ElementType::TIMESIG:
        layoutTimeSig(item_cast<const TimeSig*>(item), static_cast<TimeSig::LayoutData*>(ldata), ctx);
        break;
    case ElementType::TIME_TICK_ANCHOR: layoutTimeTickAnchor(static_cast<TimeTickAnchor*>(item), ctx);
        break;
    case ElementType::TREMOLO_SINGLECHORD: layoutTremoloSingle(item_cast<TremoloSingleChord*>(item), ctx);
        break;
    case ElementType::TREMOLO_TWOCHORD:    layoutTremoloTwo(item_cast<TremoloTwoChord*>(item), ctx);
        break;
    case ElementType::TREMOLOBAR:
        layoutTremoloBar(item_cast<const TremoloBar*>(item), static_cast<TremoloBar::LayoutData*>(ldata));
        break;
    case ElementType::TRILL:            layoutTrill(item_cast<Trill*>(item), ctx);
        break;
    case ElementType::TRILL_SEGMENT:    layoutTrillSegment(item_cast<TrillSegment*>(item), ctx);
        break;
    case ElementType::TRIPLET_FEEL:
        layoutTripletFeel(item_cast<const TripletFeel*>(item), static_cast<TripletFeel::LayoutData*>(ldata));
        break;
    case ElementType::TUPLET:           layoutTuplet(item_cast<Tuplet*>(item), ctx);
        break;
    case ElementType::VIBRATO:          layoutVibrato(item_cast<Vibrato*>(item), ctx);
        break;
    case ElementType::VIBRATO_SEGMENT:  layoutVibratoSegment(item_cast<VibratoSegment*>(item), ctx);
        break;
    case ElementType::VOLTA:            layoutVolta(item_cast<Volta*>(item), ctx);
        break;
    case ElementType::VOLTA_SEGMENT:    layoutVoltaSegment(item_cast<VoltaSegment*>(item), ctx);
        break;
    case ElementType::WHAMMY_BAR_SEGMENT: layoutWhammyBarSegment(item_cast<WhammyBarSegment*>(item), ctx);
        break;
    default:
        LOGE() << "not found in layout types item: " << item->typeName();
        DO_ASSERT(false);
    }
}

void TLayout::layoutAccidental(const Accidental* item, Accidental::LayoutData* ldata, const LayoutConfiguration& conf)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    if (ldata->isValid()) {
        return;
    }

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

    ldata->column = 0;

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

    Shape shape;

    // Single?
    SymId singleSym = accidentalSingleSym(item);
    if (singleSym != SymId::noSym && conf.engravingFont()->isValid(singleSym)) {
        Accidental::LayoutData::Sym s(singleSym, 0.0, 0.0);
        ldata->syms.push_back(s);

        shape.add(item->symShapeWithCutouts(singleSym));
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
            shape.add(item->symBbox(bracketSyms.first), item);

            x += item->symAdvance(bracketSyms.first) + margin;
        }

        // Main
        SymId mainSym = item->symId();
        Accidental::LayoutData::Sym ms(mainSym, x, 0.0);
        ldata->syms.push_back(ms);
        shape.add(item->symShapeWithCutouts(mainSym).translated(PointF(x, 0.0)));

        // Right
        if (bracketSyms.second != SymId::noSym) {
            x += item->symAdvance(mainSym) + margin;

            Accidental::LayoutData::Sym rs(bracketSyms.second, x,
                                           item->bracket() == AccidentalBracket::BRACE ? item->spatium() * 0.4 : 0.0);
            ldata->syms.push_back(rs);
            shape.add(item->symBbox(bracketSyms.second).translated(x, 0.0), item);
        }
    }

    ldata->setShape(shape);
}

void TLayout::layoutActionIcon(const ActionIcon* item, ActionIcon::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    if (ldata->isValid()) {
        return;
    }

    FontMetrics fontMetrics(item->iconFont());
    ldata->setBbox(fontMetrics.boundingRect(Char(item->icon())));
    ldata->setPos(PointF());
}

void TLayout::layoutAmbitus(const Ambitus* item, Ambitus::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    if (ldata->isValid()) {
        return;
    }

    const double spatium = item->spatium();
    const double headWdt = item->headWidth();

    Accidental::LayoutData* topAccData = item->topAccidental()->mutldata();
    Accidental::LayoutData* bottomAccData = item->bottomAccidental()->mutldata();

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
            TLayout::layoutAccidental(item->topAccidental(), topAccData, ctx.conf());
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
            TLayout::layoutAccidental(item->bottomAccidental(), bottomAccData, ctx.conf());
            bottomAccData->setPosY(ldata->bottomPos.y());
        }
    }

    //
    // NOTEHEAD X POS
    //
    // Note: manages colliding accidentals
    //
    {
        double accNoteDist = item->absoluteFromSpatium(ctx.conf().styleS(Sid::accidentalNoteDistance));
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
        if (!muse::RealIsNull(yDelta)) {
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

void TLayout::layoutArpeggio(const Arpeggio* item, Arpeggio::LayoutData* ldata, const LayoutConfiguration& conf,
                             bool includeCrossStaffHeight)
{
    LAYOUT_CALL_ITEM(item);
    UNUSED(includeCrossStaffHeight);

    //! NOTE Can be edited and relayout,
    //! in this case the reset layout data has not yet been done
//    if (ldata->isValid()) {
//        return;
//    }

    if (conf.styleB(Sid::arpeggioHiddenInStdIfTab)) {
        if (item->staff() && item->staff()->isPitchedStaff(item->tick())) {
            for (Staff* s : item->staff()->staffList()) {
                if (s->onSameScore(item) && s->isTabStaff(item->tick()) && s->visible()) {
                    ldata->setIsSkipDraw(true);
                }
            }
        }
    }
    ldata->setIsSkipDraw(false);
    if (!ldata->isSetPos()) {
        ldata->setPos(PointF());
    }

    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    //! NOTE Must already be set previously
    Chord* parentChord = item->chord();
    LD_CONDITION(parentChord->upNote()->ldata()->isSetPos());
    LD_CONDITION(parentChord->downNote()->ldata()->isSetPos());

    auto computeHeight = [](const Arpeggio* item) -> double
    {
        Chord* chord = item->chord();
        double y = chord->upNote()->pagePos().y() - chord->upNote()->headHeight() * .5;

        Note* downNote = chord->downNote();
        EngravingItem* e = chord->segment()->element(item->track() + item->span() - 1);
        if (e && e->isChord()) {
            downNote = toChord(e)->downNote();
        }
        double h = downNote->pagePos().y() + downNote->headHeight() * .5 - y;
        return h;
    };

    auto calcTop = [](const Arpeggio* item, const LayoutConfiguration& conf) -> double
    {
        double top = -item->userLen1();
        switch (item->arpeggioType()) {
        case ArpeggioType::BRACKET: {
            double lineWidth = conf.styleMM(Sid::arpeggioLineWidth);
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
            double lineWidth = conf.styleMM(Sid::arpeggioLineWidth);
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

    ldata->arpeggioHeight = computeHeight(item);
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
        double w  = conf.styleS(Sid::arpeggioHookLen).val() * item->spatium();
        ldata->setBbox(RectF(0.0, ldata->top, w, ldata->bottom));
    } break;
    }

    // Loop through staves spanned & regenerate chord shape
    // This makes sure the arpeggio's shape is added to the shape of each chord it spans
    Chord* chord = item->chord();
    Segment* seg = chord->segment();
    staff_idx_t staveSpan = (item->track() + item->span() - 1) / VOICES;
    for (staff_idx_t staffIdx = item->staffIdx(); staffIdx <= staveSpan; staffIdx++) {
        seg->createShape(staffIdx);
    }
}

void TLayout::layoutArticulation(const Articulation* item, Articulation::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    if (item->isHiddenOnTabStaff()) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);
    ldata->setPos(PointF());

    //! NOTE Must already be set previously
    LD_CONDITION(ldata->symId.has_value());

    RectF bbox;

    if (item->textType() == ArticulationTextType::NO_TEXT) {
        bbox = item->symBbox(ldata->symId);
    } else {
        Font scaledFont(item->font());
        scaledFont.setPointSizeF(item->font().pointSizeF() * item->magS());
        FontMetrics fm(scaledFont);
        bbox = fm.boundingRect(scaledFont, TConv::text(item->textType()));
    }

    fillArticulationShape(item, ldata);
}

void TLayout::fillArticulationShape(const Articulation* item, Articulation::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);

    SymId sym = item->symId();
    if (sym == SymId::articAccentAbove || sym == SymId::articAccentBelow) {
        RectF symBbox = item->symBbox(sym);
        double width = symBbox.width();
        double height = symBbox.height();
        double thirdWidth = width / 3;
        double thirdHeight = height / 3;
        RectF base = RectF(0.0, 0.0, thirdWidth, height);
        RectF center = RectF(thirdWidth, thirdHeight, thirdWidth, thirdHeight);
        RectF tip = RectF(2 * thirdWidth, 1.25 * thirdHeight, thirdWidth, 0.5 * thirdHeight);
        Shape shape;
        shape.add(base, item);
        shape.add(center, item);
        shape.add(tip, item);
        PointF translate(0.0, sym == SymId::articAccentAbove ? -height : 0.0);
        ldata->setShape(shape.translate(translate));
    } else {
        ldata->setShape(Shape(item->symBbox(sym), item));
    }
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

void TLayout::layoutBarLine(const BarLine* item, BarLine::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);

    // barlines hidden on this staff
    if (item->staff() && item->segment()) {
        if ((!item->staff()->staffTypeForElement(item)->showBarlines() && item->segment()->segmentType() == SegmentType::EndBarLine)
            || (item->staff()->hideSystemBarLine() && item->segment()->segmentType() == SegmentType::BeginBarLine)) {
            ldata->setPos(PointF());
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
            LD_CONDITION(item_cast<const Articulation*>(e)->ldata()->symId.has_value());
            break;
        case ElementType::SYMBOL:
            LD_INDEPENDENT;
            break;
        case ElementType::IMAGE:
            // will be done
            // LD_CONDITION(parentLD->isSetBbox());
            LD_INDEPENDENT;
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

    updateBarlineShape(item, ldata, ctx);

    //! NOTE The types are listed here explicitly to show what types there are (see add method)
    //! and accordingly show what the barline layout depends on.
    for (EngravingItem* e : *item->el()) {
        switch (e->type()) {
        case ElementType::ARTICULATION: {
            Articulation* a = item_cast<Articulation*>(e);
            Articulation::LayoutData* aldata = a->mutldata();
            TLayout::layoutArticulation(a, aldata);
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
        case ElementType::SYMBOL: {
            Symbol* sb = item_cast<Symbol*>(e);
            TLayout::layoutSymbol(sb, sb->mutldata(), ctx);
        } break;
        case ElementType::IMAGE: {
            Image* im = item_cast<Image*>(e);
            TLayout::layoutImage(im, im->mutldata());
        } break;
        default:
            UNREACHABLE;
        }
    }

    if (!item->segment()) {
        return;
    }

    if (Fermata* fermata = toFermata(item->segment()->findAnnotation(ElementType::FERMATA, item->track(), item->track() + VOICES))) {
        layoutFermata(fermata, fermata->mutldata(), ctx.conf());
    }
}

void TLayout::updateBarlineShape(const BarLine* item, BarLine::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);

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

    ldata->setShape(Shape(bb, item));
}

//---------------------------------------------------------
//    called after system layout; set vertical dimensions
//---------------------------------------------------------
void TLayout::layoutBarLine2(BarLine* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);

    BarLine::LayoutData* ldata = item->mutldata();

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

void TLayout::layoutBeam(Beam* item, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    BeamLayout::layout(item, ctx);
}

void TLayout::layoutBeam1(Beam* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    BeamLayout::layout1(item, ctx);
}

void TLayout::layoutBend(const Bend* item, Bend::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    LD_CONDITION(item->note()->ldata()->isSetPos());
    LD_CONDITION(item->note()->ldata()->isSetBbox());

    double spatium = item->spatium();
    double lw = item->absoluteFromSpatium(item->lineWidth());

    const Note::LayoutData* noteLD = item->note()->ldata();
    PointF notePos = noteLD->pos();
    notePos.ry() = std::max(notePos.y(), 0.0);

    ldata->noteWidth = noteLD->bbox().width();
    ldata->notePos = notePos;

    RectF bb;

    FontMetrics fm(item->font(spatium));

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
    ldata->setPos(PointF());
}

void TLayout::layoutBox(const Box* item, Box::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    switch (item->type()) {
    case ElementType::HBOX:
        TLayout::layoutHBox(static_cast<const HBox*>(item), static_cast<HBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::VBOX:
        TLayout::layoutVBox(static_cast<const VBox*>(item), static_cast<VBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::FBOX:
        TLayout::layoutFBox(static_cast<const FBox*>(item), static_cast<FBox::LayoutData*>(ldata), ctx);
        break;
    case ElementType::TBOX:
        TLayout::layoutTBox(static_cast<const TBox*>(item), static_cast<TBox::LayoutData*>(ldata), ctx);
        break;
    default:
        UNREACHABLE;
        break;
    }
}

void TLayout::layoutBaseBox(const Box* item, Box::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    layoutBaseMeasureBase(item, ldata, ctx);
}

void TLayout::layoutHBox(const HBox* item, HBox::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    if (item->explicitParent() && item->explicitParent()->isVBox()) {
        const VBox* parentVBox = toVBox(item->explicitParent());

        LD_CONDITION(parentVBox->ldata()->isSetBbox());

        double x = parentVBox->leftMargin() * DPMM;
        double y = parentVBox->topMargin() * DPMM;
        double w = item->absoluteFromSpatium(item->boxWidth());
        double h = parentVBox->ldata()->bbox().height() - (parentVBox->topMargin() + parentVBox->bottomMargin()) * DPMM;
        ldata->setPos(x, y);
        ldata->setBbox(0.0, 0.0, w, h);
    } else if (item->system()) {
        const System* parentSystem = item->system();

        LD_CONDITION(parentSystem->ldata()->isSetBbox());

        if (!ldata->isSetPos()) {
            ldata->setPos(PointF());
        }
        ldata->setBbox(0.0, 0.0, item->absoluteFromSpatium(item->boxWidth()), parentSystem->ldata()->bbox().height());
    } else {
        ldata->setPos(PointF());
        ldata->setBbox(0.0, 0.0, 50, 50);
    }
    layoutBaseBox(item, ldata, ctx);
}

void TLayout::layoutHBox2(HBox* item, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    layoutBaseBox(item, item->mutldata(), ctx);
}

void TLayout::layoutVBox(const VBox* item, VBox::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    ldata->setPos(PointF());

    if (item->system()) {
        const System* parentSystem = item->system();

        LD_CONDITION(parentSystem->ldata()->isSetBbox());

        ldata->setBbox(0.0, 0.0, parentSystem->ldata()->bbox().width(), item->absoluteFromSpatium(item->boxHeight()));
    } else {
        ldata->setBbox(0.0, 0.0, 50, 50);
    }

    for (EngravingItem* e : item->el()) {
        layoutItem(e, const_cast<LayoutContext&>(ctx));
    }
    bool boxAutoSize = item->getProperty(Pid::BOX_AUTOSIZE).toBool();
    bool heightChanged = false;
    if (boxAutoSize) {
        double contentHeight = item->contentRect().height();

        if (contentHeight < item->minHeight()) {
            contentHeight = item->minHeight();
        }

        ldata->setHeight(contentHeight);
        heightChanged = true;
    }

    if (boxAutoSize && MScore::noImages) {
        // adjustLayoutWithoutImages
        double calculatedVBoxHeight = 0;
        const int padding = item->sizeIsSpatiumDependent() ? ctx.conf().spatium() : ctx.conf().style().defaultSpatium();
        ElementList elist = item->el();
        for (EngravingItem* e : elist) {
            if (e->isText()) {
                Text* txt = toText(e);
                Text::LayoutData* txtLD = txt->mutldata();

                LD_CONDITION(txtLD->isSetBbox());

                RectF bbox = txtLD->bbox();
                bbox.moveTop(0.0);
                txtLD->setBbox(bbox);
                calculatedVBoxHeight += txtLD->bbox().height() + padding;
            }
        }

        ldata->setHeight(calculatedVBoxHeight);
        heightChanged = true;
    }
    if (heightChanged) {
        for (EngravingItem* e : item->el()) {
            layoutItem(e, const_cast<LayoutContext&>(ctx));
        }
    }
}

void TLayout::layoutFBox(const FBox* item, FBox::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    const System* parentSystem = item->system();

    LD_CONDITION(parentSystem->ldata()->isSetBbox());

    ldata->setPos(PointF());

    const ElementList& elements = item->el();

    std::vector<FretDiagram*> fretDiagrams;
    for (EngravingItem* element : elements) {
        if (!element || !element->isFretDiagram() || !element->visible()) {
            continue;
        }

        fretDiagrams.emplace_back(toFretDiagram(element));
    }

    //! NOTE: layout fret diagrams and calculate sizes

    const size_t totalDiagrams = fretDiagrams.size();
    double maxFretDiagramWidth = 0.0;

    for (size_t i = 0; i < totalDiagrams; ++i) {
        FretDiagram* fretDiagram = fretDiagrams[i];
        fretDiagram->setUserMag(item->diagramScale());

        Harmony* harmony = fretDiagram->harmony();
        harmony->setUserMag(item->textScale());
        harmony->render();

        layoutItem(fretDiagram, const_cast<LayoutContext&>(ctx));

        double width = fretDiagram->ldata()->bbox().width();
        maxFretDiagramWidth = std::max(maxFretDiagramWidth, width);
    }

    //! NOTE: table view layout

    const double spatium = item->spatium();
    const size_t chordsPerRow = item->chordsPerRow();
    const double rowGap = item->rowGap().val() * spatium;
    const double columnGap = item->columnGap().val() * spatium;

    //! The height of each row is determined by the height of the tallest cell in that row
    std::vector<double> rowHeights;
    for (size_t i = 0; i < totalDiagrams; i += chordsPerRow) {
        size_t itemsInRow = std::min(chordsPerRow, totalDiagrams - i);
        double maxRowHeight = 0.0;

        for (size_t j = 0; j < itemsInRow; ++j) {
            FretDiagram* fretDiagram = fretDiagrams[i + j];

            RectF fretRect = fretDiagram->ldata()->bbox();

            const Harmony::LayoutData* harmonyLdata = fretDiagram->harmony()->ldata();
            RectF harmonyRect = harmonyLdata->bbox();
            harmonyRect.moveTo(harmonyLdata->pos());

            double height = fretRect.united(harmonyRect).height();
            maxRowHeight = std::max(maxRowHeight, height);
        }

        rowHeights.push_back(maxRowHeight);
    }

    const double cellWidth = maxFretDiagramWidth;
    const size_t rows = rowHeights.size();
    const size_t columns = std::min(totalDiagrams, chordsPerRow);

    double totalTableHeight = 0.0;
    for (size_t i = 0; i < rows; ++i) {
        totalTableHeight += rowHeights[i];
        if (i > 0) {
            totalTableHeight += rowGap;
        }
    }

    if (muse::RealIsNull(totalTableHeight)) {
        totalTableHeight = item->minHeight();
    }

    const double totalTableWidth = cellWidth * columns + (columns - 1) * columnGap;

    ldata->totalTableHeight = totalTableHeight;
    ldata->totalTableWidth = totalTableWidth;

    ldata->setBbox(0.0, 0.0, parentSystem->ldata()->bbox().width(), totalTableHeight);

    AlignH alignH = item->contentHorizontallAlignment();
    const double leftMargin = item->getProperty(Pid::LEFT_MARGIN).toDouble() * spatium;
    const double rightMargin = item->getProperty(Pid::RIGHT_MARGIN).toDouble() * spatium;
    const double topMargin = item->getProperty(Pid::TOP_MARGIN).toDouble() * spatium;
    const double bottomMargin = item->getProperty(Pid::BOTTOM_MARGIN).toDouble() * spatium;

    const double startX = alignH == AlignH::HCENTER
                          ? (item->width() - totalTableWidth) / 2
                          : alignH == AlignH::RIGHT ? item->width() - totalTableWidth : 0.0;
    const double startY = !muse::RealIsNull(topMargin) ? topMargin : -bottomMargin;

    const double shapeMarginAboveDiagram = ctx.conf().styleMM(Sid::harmonyFretDist).val() * 1.5;

    for (size_t i = 0; i < totalDiagrams; ++i) {
        FretDiagram* fretDiagram = fretDiagrams[i];

        size_t row = i / chordsPerRow;
        size_t col = i % chordsPerRow;

        size_t itemsInRow = std::min(chordsPerRow, totalDiagrams - row * chordsPerRow);
        double rowOffsetX = alignH == AlignH::HCENTER
                            ? (totalTableWidth - (itemsInRow * cellWidth + (itemsInRow - 1) * columnGap)) / 2
                            : alignH == AlignH::RIGHT
                            ? totalTableWidth - (itemsInRow * cellWidth + (itemsInRow - 1) * columnGap) - rightMargin + spatium
                            : leftMargin + spatium;

        double x = startX + rowOffsetX + col * (cellWidth + columnGap);

        double y = startY;
        for (size_t r = 0; r < row; ++r) {
            y += rowHeights[r] + rowGap;
        }

        double fretDiagramX = x;
        double fretDiagramY = y + fretDiagram->harmony()->ldata()->harmonyHeight + shapeMarginAboveDiagram;

        fretDiagram->mutldata()->setPos(PointF(fretDiagramX, fretDiagramY));
    }
}

void TLayout::layoutTBox(const TBox* item, TBox::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    const System* parentSystem = item->system();

    LD_CONDITION(parentSystem->ldata()->isSetBbox());

    ldata->setPos(PointF());
    ldata->setBbox(0.0, 0.0, parentSystem->ldata()->bbox().width(), 0);

    TLayout::layoutText(item->text(), item->text()->mutldata());

    Text::LayoutData* textLD = item->text()->mutldata();

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

    layoutBaseMeasureBase(item, ldata, ctx);   // layout LayoutBreak's
}

void TLayout::layoutBracket(const Bracket* item, Bracket::LayoutData* ldata, const LayoutConfiguration& conf)
{
    LAYOUT_CALL_ITEM(item);
    LD_CONDITION(ldata->bracketHeight.has_value());
    if (!ldata->bracketHeight.has_value()) {
        return;
    }

    const_cast<Bracket*>(item)->setVisible(item->bi()->visible());
    ldata->braceSymbol = item->braceSymbol();

    switch (item->bracketType()) {
    case BracketType::BRACE: {
        String musicalSymbolFont = conf.styleSt(Sid::musicalSymbolFont);
        if (musicalSymbolFont == "Emmentaler" || musicalSymbolFont == "Gonville") {
            ldata->braceSymbol = SymId::noSym;
            double w = conf.styleMM(Sid::akkoladeWidth);
            double h2 = ldata->bracketHeight * 0.5;

#define XM(a) (a + 700) * w / 700
#define YM(a) (a + 7100) * h2 / 7100

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
            ldata->bracketWidth = w + conf.styleMM(Sid::akkoladeBarDistance);
        } else {
            if (item->braceSymbol() == SymId::noSym) {
                ldata->braceSymbol = SymId::brace;
            }
            double h = ldata->bracketHeight;
            double w = item->symWidth(ldata->braceSymbol) * item->magx();
            ldata->setBbox(RectF(0, 0, w, h));
            ldata->shape.add(ldata->bbox());
            ldata->bracketWidth = w + conf.styleMM(Sid::akkoladeBarDistance);
        }
    }
    break;
    case BracketType::NORMAL: {
        double w = conf.styleMM(Sid::bracketWidth) * .5;
        double bd = item->spatium() * (conf.styleSt(Sid::musicalSymbolFont) == "Leland" ? .5 : .25);

        Shape shape = RectF(-w, -bd, w * 2, 2 * (ldata->bracketHeight * 0.5 + bd));
        shape.add(item->symBbox(SymId::bracketTop).translated(PointF(-w, -bd)));
        shape.add(item->symBbox(SymId::bracketBottom).translated(PointF(-w, bd + ldata->bracketHeight)));
        ldata->setShape(shape);

        ldata->bracketWidth = conf.styleMM(Sid::bracketWidth) + conf.styleMM(Sid::bracketDistance);
    }
    break;
    case BracketType::SQUARE: {
        double w = conf.styleMM(Sid::staffLineWidth) * .5;
        double x = -w;
        double y = -w;
        double h = (ldata->bracketHeight * 0.5 + w) * 2;
        w += (.5 * item->spatium() + 3 * w);
        ldata->setBbox(RectF(x, y, w, h));
        ldata->shape.add(ldata->bbox());

        ldata->bracketWidth = conf.styleMM(Sid::staffLineWidth) / 2 + 0.5 * item->spatium();
    }
    break;
    case BracketType::LINE: {
        double _spatium = item->spatium();
        double w = 0.67 * conf.styleMM(Sid::bracketWidth) * .5;
        double x = -w;
        double bd = _spatium * .25;
        double y = -bd;
        double h = (-y + ldata->bracketHeight * 0.5) * 2;
        ldata->setBbox(RectF(x, y, w, h));
        ldata->shape.add(ldata->bbox());

        ldata->bracketWidth = 0.67 * conf.styleMM(Sid::bracketWidth) + conf.styleMM(Sid::bracketDistance);
    }
    break;
    case BracketType::NO_BRACKET:
        break;
    }
}

void TLayout::layoutBreath(const Breath* item, Breath::LayoutData* ldata, const LayoutConfiguration& conf)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    if (ldata->isValid()) {
        return;
    }

    int voiceOffset = item->placeBelow() * (item->staff()->lines(item->tick()) - 1) * item->spatium();
    if (item->isCaesura()) {
        ldata->setPosY(item->spatium() + voiceOffset);
    } else if ((conf.styleSt(Sid::musicalSymbolFont) == "Emmentaler") && (item->symId() == SymId::breathMarkComma)) {
        ldata->setPosY(0.5 * item->spatium() + voiceOffset);
    } else {
        ldata->setPosY(-0.5 * item->spatium() + voiceOffset);
    }

    ldata->setBbox(item->symBbox(item->symId()));
}

void TLayout::layoutChord(Chord* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    ChordLayout::layout(item, ctx);
}

void TLayout::layoutChordLine(const ChordLine* item, ChordLine::LayoutData* ldata, const LayoutConfiguration& conf)
{
    LAYOUT_CALL_ITEM(item);
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    Note* note = nullptr;
    const double spatium = item->spatium();

    if (item->note()) {
        note = item->chord()->findNote(item->note()->pitch());
    }

    if (!note) {
        note = item->chord()->upNote();
    }

    const Note::LayoutData* noteLD = note->ldata();

    //! NOTE Temporary, instead of item->chord()->shape()
    LD_CONDITION(item->chord()->ldata()->isSetBbox());

    LD_CONDITION(noteLD->isSetPos());
    LD_CONDITION(noteLD->isSetBbox());

    ldata->setMag(item->chord()->mag());

    if (!item->modified()) {
        double x2 = 0;
        double y2 = 0;
        double baseLength = spatium * item->chord()->intrinsicMag();
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

    double x = 0.0;
    double y = noteLD->pos().y();
    double horOffset = 0.33 * spatium;         // one third of a space away from the note
    double vertOffset = 0.25 * spatium;         // one quarter of a space from the center line
    // Get chord shape
    Shape chordShape = item->chord()->shape();
    // ...but remove from the shape items that the chordline shouldn't try to avoid
    // (especially the chordline itself)
    chordShape.remove_if([](ShapeElement& shapeEl){
        if (!shapeEl.item()) {
            return true;
        }
        const EngravingItem* item = shapeEl.item();
        if (item->isChordLine() || item->isHarmony() || item->isLyrics()) {
            return true;
        }
        return false;
    });

    const RectF r = item->isWavy() ? conf.engravingFont()->bbox(item->waveSym(), item->magS()) : ldata->path.boundingRect();

    y += item->isBelow() ? vertOffset : -vertOffset;

    const double yBelow = y + r.height();
    double chordEdge
        = item->isToTheLeft() ? chordShape.leftMostEdgeAtHeight(y, yBelow) : chordShape.rightMostEdgeAtHeight(y, yBelow);

    if (!item->isToTheLeft()) {
        // Always place lines to the right of dots
        double xMin = chordShape.left();
        for (ShapeElement& shapeEl : chordShape.elements()) {
            if (!shapeEl.item() || !shapeEl.item()->isNoteDot()) {
                continue;
            }

            xMin = std::max(xMin, shapeEl.right());
        }
        chordEdge = std::max(chordEdge, xMin);
    }

    x += item->isToTheLeft() ? chordEdge - horOffset : chordEdge + horOffset;
    ldata->setPos(x, y);

    if (!item->isWavy()) {
        int x1 = 0, y1 = 0, width = 0, height = 0;

        x1 = r.x();
        y1 = r.y();
        width = r.width();
        height = r.height();
        ldata->setBbox(x1, y1, width, height);
    } else {
        // Align an eighth space above/below the centre of the note
        if (item->isBelow()) {
            ldata->moveY(r.height() - spatium * 0.125);
        } else {
            ldata->moveY(spatium * 0.075);
        }

        if (item->isToTheLeft()) {
            ldata->moveX(-r.width());
        }

        ldata->setBbox(r);
    }
}

void TLayout::layoutClef(const Clef* item, Clef::LayoutData* ldata, const LayoutConfiguration& conf)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    if (ldata->isValid()) {
        return;
    }

    // determine current number of lines and line distance
    int lines = 5;
    double lineDist = 1.0;
    int stepOffset = 0;
    double staffOffsetY = 0;

    Segment* clefSeg  = item->segment();

    // check clef visibility and type compatibility
    if (clefSeg && item->staff()) {
        const Fraction tick = clefSeg->tick();
        const Fraction tickPrev = tick - Fraction::eps();
        const StaffType* st = item->staff()->staffType(tick);
        const StaffType* stPrev = !tickPrev.negative() ? item->staff()->staffType(tickPrev) : nullptr;
        bool show = st->genClef();            // check staff type allows clef display
        StaffGroup staffGroup = st->group();
        const bool hideClef = st->isTabStaff() ? conf.styleB(Sid::hideTabClefAfterFirst) : !conf.styleB(Sid::genClef);

        // if not tab, use instrument->useDrumset to set staffGroup (to allow pitched to unpitched in same staff)
        if (staffGroup != StaffGroup::TAB) {
            staffGroup = item->staff()->part()->instrument(item->tick())->useDrumset() ? StaffGroup::PERCUSSION : StaffGroup::STANDARD;
        }

        // check clef is compatible with staff type group:
        if (ClefInfo::staffGroup(item->clefType()) != staffGroup) {
            if (tick > Fraction(0, 1) && (!item->generated() || hideClef)) {     // if clef is not generated, hide it
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

        const double stOffset = st->yoffset().val();
        const double stPrevOffset = stPrev && clefSeg->rtick() != Fraction(0, 1) ? stPrev->yoffset().val() : 0.0;
        staffOffsetY = item->isHeader() ? 0.0 : stOffset - stPrevOffset;
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
    case ClefType::TAB:                                     // TAB clef
    case ClefType::TAB4:                                    // TAB clef 4 strings
    case ClefType::TAB_SERIF:                               // TAB clef alternate style
    case ClefType::TAB4_SERIF:                              // TAB clef alternate style
        // on tablature, position clef at half the number of spaces * line distance
        yoff = lineDist * (lines - 1) * .5;
        stepOffset = 0;           //  ignore stepOffset for TAB and percussion clefs
        break;
    case ClefType::PERC:                                   // percussion clefs
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
    Shape shape(item->symShapeWithCutouts(ldata->symId));
    bool isMidMeasureClef = item->isMidMeasureClef();
    double x = isMidMeasureClef ? -shape.right() : 0.0;
    ldata->setPos(PointF(x, (yoff + (stepOffset * 0.5) + staffOffsetY) * _spatium));
    if (item->isMidMeasureClef()) {
        ldata->setShape(shape);
    } else {
        ldata->setBbox(item->symBbox(ldata->symId));
    }
}

void TLayout::layoutCapo(const Capo* item, Capo::LayoutData* ldata, const LayoutContext&)
{
    LAYOUT_CALL_ITEM(item);
    //! NOTE Looks like it doesn't belong here
    if (item->shouldAutomaticallyGenerateText() || item->empty()) {
        if (const Part* part = item->part()) {
            if (const StringData* stringData = part->stringData(item->tick(), item->staff()->idx())) {
                String text = item->generateText(stringData->strings());
                const_cast<Capo*>(item)->setXmlText(text);
            }
        }
    }

    TLayout::layoutBaseTextBase(item, ldata);

    if (item->autoplace()) {
        const Segment* s = item->segment();
        const Measure* m = s->measure();
        LD_CONDITION(ldata->isSetPos());
        LD_CONDITION(m->ldata()->isSetPos());
        LD_CONDITION(s->ldata()->isSetPos());
    }

    Autoplace::autoplaceSegmentElement(item, ldata);
}

void TLayout::layoutDeadSlapped(const DeadSlapped* item, DeadSlapped::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    if (ldata->isValid()) {
        return;
    }

    const double deadSlappedWidth = item->spatium() * 2;
    RectF rect = RectF(0, 0, deadSlappedWidth, item->staff()->staffHeight());
    ldata->setBbox(rect);
    ldata->setPos(PointF());

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

        PainterPath path1;
        path1.moveTo(topLeft);
        path1.lineTo(topLeft + offsetX);
        path1.lineTo(bottomRight);
        path1.lineTo(bottomRight - offsetX);
        path1.lineTo(topLeft);

        PainterPath path2;
        path2.moveTo(topRight);
        path2.lineTo(topRight - offsetX);
        path2.lineTo(bottomLeft);
        path2.lineTo(bottomLeft + offsetX);
        path2.lineTo(topRight);

        ldata->path1 = path1;
        ldata->path2 = path2;
    }
}

void TLayout::layoutDynamic(Dynamic* item, Dynamic::LayoutData* ldata, const LayoutConfiguration& conf)
{
    DynamicsLayout::layoutDynamic(item, ldata, conf);
}

void TLayout::layoutExpression(const Expression* item, Expression::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    const_cast<Expression*>(item)->setPlacementBasedOnVoiceAssignment(item->style().styleV(
                                                                          Sid::dynamicsHairpinVoiceBasedPlacement).value<DirectionV>());

    TLayout::layoutBaseTextBase(item, ldata);

    const Segment* segment = item->segment();

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

    ldata->disconnectSnappedItems();

    if (!item->autoplace()) {
        return;
    }

    const Segment* s = item->segment();
    const Measure* m = s->measure();
    LD_CONDITION(ldata->isSetPos());
    LD_CONDITION(m->ldata()->isSetPos());
    LD_CONDITION(s->ldata()->isSetPos());

    Dynamic* dynamic = toDynamic(segment->findAnnotation(ElementType::DYNAMIC, item->track(), item->track()));
    if (!dynamic) {
        Segment* timeTickSeg = m->findSegmentR(SegmentType::TimeTick, s->rtick());
        dynamic = timeTickSeg ? toDynamic(timeTickSeg->findAnnotation(ElementType::DYNAMIC, item->track(), item->track())) : nullptr;
    }
    if (!dynamic || dynamic->placeAbove() != item->placeAbove() || dynamic->voiceAssignment() != item->voiceAssignment()
        || !dynamic->visible()) {
        Autoplace::autoplaceSegmentElement(item, ldata);
        return;
    }

    LD_CONDITION(dynamic->ldata()->isSetBbox()); // dynamic->shape()
    LD_CONDITION(dynamic->ldata()->isSetPos());

    // If there is a dynamic on same segment and track make space for it horizontally
    double padding = item->computeDynamicExpressionDistance(dynamic);
    double dynamicRight = dynamic->shape().translate(dynamic->pos()).right();
    double expressionLeft = ldata->bbox().translated(item->pos()).left();
    double difference = expressionLeft - dynamicRight - padding;
    ldata->moveX(-difference);

    if (item->snapToDynamics()) {
        ldata->connectItemSnappedBefore(dynamic);
    }

    Autoplace::autoplaceSegmentElement(item, ldata);
}

void TLayout::layoutFermata(const Fermata* item, Fermata::LayoutData* ldata, const LayoutConfiguration& conf)
{
    LAYOUT_CALL_ITEM(item);
    const StaffType* stType = item->staffType();
    if (stType && stType->isHiddenElementOnTab(conf.style(), Sid::fermataShowTabCommon, Sid::fermataShowTabSimple)) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);
    ldata->setPos(PointF());

    if (item->isStyled(Pid::OFFSET)) {
        const_cast<Fermata*>(item)->setOffset(item->propertyDefault(Pid::OFFSET).value<PointF>());
    }

    double x = 0.0;
    double y = item->placeAbove() ? 0.0 : item->staff()->staffHeight(item->tick());
    const Segment* s = item->segment();
    const EngravingItem* e = s->element(item->track());

    if (e) {
        LD_CONDITION(e->ldata()->isSetBbox()); // e->shape()
        LD_CONDITION(e->ldata()->isSetPos());

        if (e->isChord()) {
            const Chord* chord = toChord(e);
            x = chord->x() + ChordLayout::centerX(chord);
        } else if (e->isRest()) {
            const Rest* rest = toRest(e);
            x = rest->x() + rest->centerX();
        } else {
            x = e->x() - e->shape().left() + e->width() * item->staff()->staffMag(Fraction(0, 1)) * .5;
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

    ldata->setShape(Shape(item->symBbox(item->symId()), item));
    x -= 0.5 * ldata->bbox().width();

    if (item->isStyled(Pid::OFFSET)) {
        y += item->offset().y();
    }
    Shape staffShape = item->segment()->staffShape(item->staffIdx());
    staffShape.removeTypes({ ElementType::FERMATA });
    if (item->placeAbove()) {
        double minDist = ldata->shape().minVerticalDistance(staffShape) + item->minDistance().toMM(item->spatium());
        y = std::min(y, -minDist);
    } else {
        double minDist = staffShape.minVerticalDistance(ldata->shape()) + item->minDistance().toMM(item->spatium());
        y = std::max(y, minDist);
    }
    if (item->isStyled(Pid::OFFSET)) {
        y -= item->offset().y();
    }

    ldata->setPos(x, y);

    if (item->autoplace()) {
        const Segment* s2 = item->segment();
        const Measure* m = s2->measure();
        LD_CONDITION(ldata->isSetPos());
        LD_CONDITION(m->ldata()->isSetPos());
        LD_CONDITION(s2->ldata()->isSetPos());
    }

    Autoplace::autoplaceSegmentElement(item, ldata);
}

//---------------------------------------------------------
//   FiguredBassItem layout
//    creates the display text (set as element text) and computes
//    the horiz. offset needed to align the right part as well as the vert. offset
//---------------------------------------------------------
void TLayout::layoutFiguredBassItem(const FiguredBassItem* item, FiguredBassItem::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    // construct font metrics
    int fontIdx = 0;
    Font f(FiguredBass::FBFonts().at(fontIdx).family, Font::Type::Tablature);

    // font size in pixels, scaled according to spatium()
    // (use the same font selection as used in draw() below)
    double m = ctx.conf().styleD(Sid::figuredBassFontSize) * item->spatium() / SPATIUM20;
    f.setPointSizeF(m);
    FontMetrics fm(f);

    String str;
    double x = item->symWidth(SymId::noteheadBlack) * .5;
    double x1 = 0.0;
    double x2 = 0.0;

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
    double h = fm.lineSpacing();
    h *= ctx.conf().styleD(Sid::figuredBassLineHeight);
    double y = 0.0;
    if (ctx.conf().styleI(Sid::figuredBassAlignment) == 0) {          // top alignment: stack down from first item
        y = h * item->ord();
    } else {                                                      // bottom alignment: stack up from last item
        y = -h * (item->figuredBass()->itemsCount() - item->ord());
    }
    ldata->setPos(x, y);
    // determine bbox from text width
//      w = fm.width(str);
    double w = fm.width(str);
    ldata->textWidth = w;
    // if there is a cont.line, extend width to cover the whole FB element duration line
    int lineLen = 0;
    if (item->contLine() != FiguredBassItem::ContLine::NONE
        && (lineLen = item->figuredBass()->ldata()->lineLength(0)) > w) {
        w = lineLen;
    }
    ldata->setBbox(0, 0, w, h);
}

//    lays out the duration indicator line(s), filling the _lineLengths array
//    and the length of printed lines (used by continuation lines)

static void layoutLines(const FiguredBass* item, FiguredBass::LayoutData* ldata, const LayoutContext& ctx)
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
    system_idx_t sysIdx1 = muse::indexOf(systems, s1);
    system_idx_t sysIdx2 = muse::indexOf(systems, s2);

    if (sysIdx2 == muse::nidx || sysIdx2 < sysIdx1) {
        sysIdx2 = sysIdx1;
        nextSegm = item->segment()->next1();
        // TODO
        // During layout of figured bass next systems' numbers may be still
        // undefined (then sysIdx2 == muse::nidx) or change in the future.
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

void TLayout::layoutFiguredBass(const FiguredBass* item, FiguredBass::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    // VERTICAL POSITION:
    const double y = ctx.conf().styleD(Sid::figuredBassYOffset) * item->spatium();
    ldata->setPos(PointF(0.0, y));

    // BOUNDING BOX and individual item layout (if required)
    TLayout::layoutBaseTextBase1(item, ldata);  // prepare structs and data expected by Text methods
    // if element could be parsed into items, layout each element
    // Items list will be empty in edit mode (see FiguredBass::startEdit).
    // TODO: consider disabling specific layout in case text style is changed (tid() != TextStyleName::FIGURED_BASS).
    if (item->items().size() > 0) {
        Shape shape;
        layoutLines(item, ldata, ctx);
        // layout each item and enlarge bbox to include items bboxes
        for (FiguredBassItem* fit : item->items()) {
            FiguredBassItem::LayoutData* fildata = fit->mutldata();
            layoutFiguredBassItem(fit, fildata, ctx);
            shape.add(fildata->bbox().translated(fit->pos()), fit);
        }
        ldata->setShape(shape);
    }
}

void TLayout::layoutFingering(const Fingering* item, Fingering::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
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

    TLayout::layoutBaseTextBase(item, ldata);
    ldata->setPosY(0.0); // handle placement below

    if (item->autoplace()) {
        const Note* note  = item->note();
        const Chord* chord = note->chord();

        bool voices  = chord->measure()->hasVoices(chord->staffIdx(), chord->tick(), chord->actualTicks());
        bool tight   = voices && chord->notes().size() == 1 && !chord->beam() && item->textStyleType() != TextStyleType::STRING_NUMBER;

        double headWidth = note->bboxRightPos();

        // update offset after drag
        double rebase = 0.0;
        if (ldata->offsetChanged() != OffsetChange::NONE && !tight) {
            rebase = Autoplace::rebaseOffset(item, ldata);
        }

        // temporarily exclude self from chord shape
        const_cast<Fingering*>(item)->setAutoplace(false);

        if (item->layoutType() == ElementType::CHORD) {
            bool above = item->placeAbove();
            const Stem* stem = chord->stem();
            const Segment* segment = chord->segment();
            const Measure* measure = segment->measure();
            double sp = item->spatium();
            double md = item->minDistance().val() * sp;
            const SysStaff* ss = measure->system()->staff(chord->vStaffIdx());
            const Staff* vStaff = chord->staff();           // TODO: use current height at tick

            LD_CONDITION(measure->ldata()->isSetPos());
            LD_CONDITION(segment->ldata()->isSetPos());
            LD_CONDITION(chord->ldata()->isSetPos());
            LD_CONDITION(note->ldata()->isSetPos());
            LD_CONDITION(note->ldata()->isSetBbox());
            //! NOTE A lot of spam
            // LD_CONDITION(note->ldata()->mirror.has_value());
            if (stem) {
                LD_CONDITION(stem->ldata()->isSetPos());
                LD_CONDITION(stem->ldata()->isSetBbox());
            }
            LD_CONDITION(chord->upNote()->ldata()->isSetBbox());
            LD_CONDITION(chord->upNote()->ldata()->isSetPos());
            LD_CONDITION(chord->downNote()->ldata()->isSetBbox());
            LD_CONDITION(chord->downNote()->ldata()->isSetPos());

            if (note->ldata()->mirror.value(LD_ACCESS::BAD)) {
                ldata->moveX(-note->ldata()->pos().x());
            }
            ldata->moveX(headWidth * .5);
            if (above) {
                if (tight) {
                    if (chord->stem()) {
                        ldata->moveX(-0.8 * sp);
                    }
                    ldata->moveY(-1.5 * sp);
                } else {
                    RectF r = ldata->bbox().translated(measure->pos() + segment->pos() + chord->pos() + note->pos() + item->pos());
                    SkylineLine sk(false);
                    sk.add(r, const_cast<Fingering*>(item));
                    double d = sk.minDistance(ss->skyline().north());
                    double yd = 0.0;
                    if (d > 0.0 && item->isStyled(Pid::MIN_DISTANCE)) {
                        yd -= d + item->ldata()->bbox().height() * .25;
                    }
                    // force extra space above staff & chord (but not other fingerings)
                    double top = 0.0;
                    if (chord->up() && chord->beam() && stem) {
                        top = stem->y() + stem->ldata()->bbox().top();
                    } else {
                        const Note* un = chord->upNote();
                        top = std::min(0.0, un->y() + un->ldata()->bbox().top());
                    }
                    top -= md;
                    double diff = (ldata->bbox().bottom() + ldata->pos().y() + yd + note->y()) - top;
                    if (diff > 0.0) {
                        yd -= diff;
                    }
                    if (ldata->offsetChanged() != OffsetChange::NONE) {
                        // user moved element within the skyline
                        // we may need to adjust minDistance, yd, and/or offset
                        bool inStaff = above ? r.bottom() + rebase > 0.0 : r.top() + rebase < item->staff()->staffHeight(item->tick());
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
                    RectF r = ldata->bbox().translated(measure->pos() + segment->pos() + chord->pos() + note->pos() + item->pos());
                    SkylineLine sk(true);
                    sk.add(r, const_cast<Fingering*>(item));
                    double d = ss->skyline().south().minDistance(sk);
                    double yd = 0.0;
                    if (d > 0.0 && item->isStyled(Pid::MIN_DISTANCE)) {
                        yd += d + item->ldata()->bbox().height() * .25;
                    }
                    // force extra space below staff & chord (but not other fingerings)
                    double bottom;
                    if (!chord->up() && chord->beam() && stem) {
                        bottom = stem->y() + stem->ldata()->bbox().bottom();
                    } else {
                        const Note* dn = chord->downNote();
                        bottom = std::max(vStaff->staffHeight(item->tick()), dn->y() + dn->ldata()->bbox().bottom());
                    }
                    bottom += md;
                    double diff = bottom - (ldata->bbox().top() + ldata->pos().y() + yd + note->y());
                    if (diff > 0.0) {
                        yd += diff;
                    }
                    if (ldata->offsetChanged() != OffsetChange::NONE) {
                        // user moved element within the skyline
                        // we may need to adjust minDistance, yd, and/or offset
                        bool inStaff = above ? r.bottom() + rebase > 0.0 : r.top() + rebase < item->staff()->staffHeight(item->tick());
                        Autoplace::rebaseMinDistance(item, ldata, md, yd, sp, rebase, above, inStaff);
                    }
                    ldata->moveY(yd);
                }
            }
        } else if (item->textStyleType() == TextStyleType::LH_GUITAR_FINGERING) {
            // place to left of note
            double left = note->shape().left();
            if (left - note->x() > 0.0) {
                ldata->moveX(-left);
            } else {
                ldata->moveX(-note->x());
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

void TLayout::layoutFretDiagram(const FretDiagram* item, FretDiagram::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    double spatium  = item->spatium() * item->userMag();
    ldata->stringLineWidth = spatium * 0.08;
    ldata->nutLineWidth = ((item->fretOffset() || !item->showNut()) ? ldata->stringLineWidth
                           : ctx.conf().styleMM(Sid::fretNutThickness) * item->userMag());
    ldata->nutY = -0.5 * (ldata->nutLineWidth - ldata->stringLineWidth);
    ldata->stringDist = ctx.conf().styleMM(Sid::fretStringSpacing) * item->userMag();
    ldata->fretDist = ctx.conf().styleMM(Sid::fretFretSpacing) * item->userMag();
    ldata->markerSize = ldata->stringDist * .8;
    ldata->markerY = ldata->nutY - 0.5 * ldata->nutLineWidth - ldata->markerSize - 0.20 * spatium;
    bool extendedStyle = item->style().styleB(Sid::fretStyleExtended);
    ldata->stringExtendTop = item->fretOffset() && extendedStyle ? -spatium * .2 : 0.0;
    ldata->stringExtendBottom = extendedStyle ? 0.5 * ldata->fretDist : 0.0;
    ldata->dotDiameter = ctx.conf().styleMM(Sid::fretDotSpatiumSize) * item->userMag();

    double shapeMarginAboveDiagram = ldata->fretDist * 1.5;
    double w = ldata->stringDist * (item->strings() - 1) + ldata->markerSize;
    double h = item->frets() * ldata->fretDist + ldata->stringExtendBottom + shapeMarginAboveDiagram;
    double y = -shapeMarginAboveDiagram;
    double x = -(ldata->markerSize * .5);

    bool isVertical = item->orientation() == Orientation::VERTICAL;

    // Allocate space for fret offset number
    if (item->fretOffset() > 0) {
        double padding = 0.4 * ldata->stringDist;
        for (auto i : item->barres()) {
            FretItem::Barre barre = i.second;
            if (!barre.exists()) {
                continue;
            }
            if ((barre.startString == 0 && item->numPos() == 0)
                || ((barre.endString == -1 || barre.endString == item->strings() - 1) && item->numPos() == 1)) {
                padding += 0.20 * ldata->dotDiameter * ctx.conf().styleD(Sid::barreLineWidth);
                break;
            }
        }
        Font scaledFont(item->fretNumFont());
        FontMetrics fm2(scaledFont);
        String fretText = String::number(item->fretOffset() + 1);
        if (ctx.conf().styleB(Sid::fretUseCustomSuffix)) {
            fretText += ctx.conf().styleSt(Sid::fretCustomSuffix);
        }
        ldata->fretText = fretText;
        double numw = isVertical ? fm2.width(fretText) : fm2.tightBoundingRect(fretText).height();
        double xdiff = numw + padding;
        w += xdiff;
        x += (item->numPos() == 0) == isVertical ? -xdiff : 0;
        ldata->fretNumPadding = padding;
    }

    if (!isVertical) {
        double tempW = w,
               tempX = x;
        w = h;
        h = tempW;
        x = y;
        y = tempX;
    }

    // When changing how bbox is calculated, don't forget to update the centerX and rightX methods too.
    Shape shape;
    shape.add(RectF(x, y, w, h), item);

    ldata->fingeringItems.clear();
    if (item->showFingering()) {
        const double padding = 0.2 * spatium;
        Font fingeringFont = item->fingeringFont();
        FontMetrics fontMetrics(fingeringFont);
        double fontHeight = fontMetrics.capHeight();
        for (size_t i = 0; i < item->fingering().size(); ++i) {
            int finger = item->fingering()[i];
            if (finger == 0) {
                continue;
            }
            String fingerS = String::number(finger);
            double width = fontMetrics.width(fingerS);
            double digitHeight = fontMetrics.tightBoundingRect(fingerS).height();
            double xOff = -0.5 * width;
            double fingerX = isVertical ? (item->strings() - i - 1) * ldata->stringDist + xOff : x + w + padding;
            double fingerY = isVertical ? y + h + fontHeight + padding : i * ldata->stringDist + 0.5 * digitHeight;
            ldata->fingeringItems.push_back(FretDiagram::FingeringItem(fingerS, PointF(fingerX, fingerY)));
            shape.add(ShapeElement(fontMetrics.tightBoundingRect(fingerS).translated(PointF(fingerX, fingerY)), item));
        }
    }

    ldata->setShape(shape);

    if (item->explicitParent()->isSegment()) {
        // We need to get the width of the notehead/rest in order to position the fret diagram correctly
        Segment* pSeg = item->segment();
        double noteheadWidth = 0;
        if (pSeg->isChordRestType()) {
            staff_idx_t idx = item->staff()->idx();
            for (EngravingItem* e = pSeg->firstElementOfSegment(idx); e; e = pSeg->nextElementOfSegment(e, idx)) {
                if (e->isRest()) {
                    const Rest* r = toRest(e);
                    LD_CONDITION(r->ldata()->sym.has_value());
                    noteheadWidth = item->symWidth(r->ldata()->sym());
                    break;
                } else if (e->isNote()) {
                    Note* n = toNote(e);
                    noteheadWidth = n->headWidth();
                    break;
                }
            }
        }

        ldata->setPos((noteheadWidth - item->mainWidth()) / 2, -(ldata->shape().bottom() + item->styleP(Sid::fretY)));
    }

    Harmony* harmony = item->harmony();
    if (harmony) {
        TLayout::layoutHarmony(harmony, harmony->mutldata(), ctx);
        double vertDist = ldata->bbox().top() - harmony->ldata()->bbox().translated(harmony->pos()).bottom();
        double diff = vertDist - harmony->minDistance().val() * item->spatium();
        if (diff < 0) {
            harmony->mutldata()->moveY(diff);
        }
    }

    for (auto i : item->barres()) {
        FretItem::Barre barre = i.second;
        if (!barre.exists()) {
            continue;
        }
        int startString = barre.startString;
        int endString = barre.endString != -1 ? barre.endString : item->strings();
        int fret = i.first;
        if (!ctx.conf().styleB(Sid::barreAppearanceSlur)) {
            for (int string = startString; string < endString; ++string) {
                const_cast<FretDiagram*>(item)->removeDotForDotStyleBarre(string, fret);
            }
            break;
        }
        for (int string = startString; string < endString; ++string) {
            const_cast<FretDiagram*>(item)->addDotForDotStyleBarre(string, fret);
        }
        double insetX = 2 * ldata->stringLineWidth;
        double insetY = fret == 1 ? ldata->nutLineWidth + ldata->stringLineWidth : insetX;
        double startX = startString * ldata->stringDist + insetX;
        double endX = (endString - 1) * ldata->stringDist - insetX;
        double shoulderXoffset = 0.2 * (endX - startX);
        double startEndY = (fret - 1) * ldata->fretDist - insetY;
        double shoulderY = startEndY - 0.5 * ldata->fretDist;
        double slurThickness = 0.1 * item->spatium() * item->userMag();
        double shoulderYfor = shoulderY - slurThickness;
        double shoulderYback = shoulderY + slurThickness;
        PointF bezier1for = PointF(startX + shoulderXoffset, shoulderYfor);
        PointF bezier2for = PointF(endX - shoulderXoffset, shoulderYfor);
        PointF bezier1back = PointF(startX + shoulderXoffset, shoulderYback);
        PointF bezier2back = PointF(endX - shoulderXoffset, shoulderYback);
        PainterPath slurPath = PainterPath();
        slurPath.moveTo(startX, startEndY);
        slurPath.cubicTo(bezier1for, bezier2for, PointF(endX, startEndY));
        slurPath.cubicTo(bezier2back, bezier1back, PointF(startX, startEndY));
        ldata->slurPath = slurPath;
        break;
    }
}

void TLayout::layoutGlissando(Glissando* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    TLayout::layoutLine(const_cast<Glissando*>(item), ctx);

    if (item->spannerSegments().empty()) {
        LOGD("no segments");
        return;
    }
    Glissando::LayoutData* ldata = item->mutldata();
    ldata->setPos(0.0, 0.0);

    String instrId = item->staff()->part()->instrumentId(item->tick());
    bool harpStaff = instrId == "harp";
    if (!item->isHarpGliss().has_value()) {
        item->setIsHarpGliss(harpStaff);
    } else {
        if (harpStaff != item->isHarpGliss().value()) {
            // Preserve whether this gliss has its default playback style
            bool defaultStyle = false;
            if (item->isStyled(Pid::GLISS_STYLE)) {
                defaultStyle = true;
            }
            item->setIsHarpGliss(harpStaff);
            if (defaultStyle) {
                item->resetProperty(Pid::GLISS_STYLE);
            }

            // Make sure harp glisses can only be diatonic and chromatic
            GlissandoStyle glissStyle = item->glissandoStyle();
            if (item->isHarpGliss().value()
                && (glissStyle != GlissandoStyle::DIATONIC
                    && glissStyle != GlissandoStyle::CHROMATIC)) {
                item->setGlissandoStyle(GlissandoStyle::DIATONIC);
            }
        }
    }

    layoutNoteAnchoredLine(item, ldata, ctx);

    ChordLayout::addLineAttachPoints(item);
}

void TLayout::layoutGlissandoSegment(GlissandoSegment* item, LayoutContext&)
{
    LAYOUT_CALL_ITEM(item);

    GlissandoSegment::LayoutData* ldata = item->mutldata();
    if (item->pos2().x() <= 0) {
        ldata->setBbox(RectF());
        return;
    }

    if (item->staff()) {
        ldata->setMag(item->staff()->staffMag(item->tick()));
    }
    RectF r = RectF(0.0, 0.0, item->pos2().x(), item->pos2().y()).normalized();
    double lw = item->absoluteFromSpatium(item->lineWidth()) * .5;
    item->setbbox(r.adjusted(-lw, -lw, lw, lw));
}

void TLayout::layoutGraceNotesGroup(GraceNotesGroup* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    Shape _shape;
    for (size_t i = item->size() - 1; i != muse::nidx; --i) {
        Chord* grace = item->at(i);
        Shape graceShape = grace->shape();
        Shape groupShape = _shape;
        groupShape.remove_if([grace](ShapeElement& s) {
            if (!s.item() || (s.item()->isStem() && s.item()->vStaffIdx() != grace->vStaffIdx())) {
                return true;
            }
            return false;
        });
        double offset;
        offset = -std::max(HorizontalSpacing::minHorizontalDistance(graceShape, groupShape, grace->spatium()), 0.0);
        // Adjust spacing for cross-beam situations
        if (i < item->size() - 1 && grace->stem()) {
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
        _shape.add(graceShape.translate(PointF(offset, 0.0)));
        double xpos = offset - item->parent()->rxoffset() - item->parent()->ldata()->pos().x();
        grace->setPos(xpos, 0.0);
    }

    const Segment* appendedSeg = item->appendedSegment();
    Chord* parentChord = toChord(item->parent());
    Shape staffShape = appendedSeg->staffShape(parentChord->staffIdx());
    bool isTabStaff = parentChord->staffType() && parentChord->staffType()->isTabStaff();
    if (isTabStaff) {
        staffShape.remove_if([](ShapeElement& el) {
            // Ignore stems on tab staves
            return el.item() && el.item()->isStem();
        });
    }
    double _shapeSpatium = HorizontalSpacing::shapeSpatium(_shape);
    double xPos = -HorizontalSpacing::minHorizontalDistance(_shape, staffShape, _shapeSpatium);

    // If the parent chord is cross-staff, also check against shape in the other staff and take the minimum
    if (item->parent()->staffMove() != 0) {
        double xPosCross = -HorizontalSpacing::minHorizontalDistance(_shape,
                                                                     appendedSeg->staffShape(item->parent()->vStaffIdx()),
                                                                     _shapeSpatium);
        xPos = std::min(xPos, xPosCross);
    }
    // Same if the grace note itself is cross-staff
    Chord* firstGN = item->back();
    if (firstGN->staffMove() != 0) {
        double xPosCross = -HorizontalSpacing::minHorizontalDistance(_shape,
                                                                     appendedSeg->staffShape(firstGN->vStaffIdx()),
                                                                     _shapeSpatium);
        xPos = std::min(xPos, xPosCross);
    }
    // Safety net in case the shape checks don't succeed
    xPos = std::min(xPos, -double(ctx.conf().styleMM(Sid::graceToMainNoteDist) + firstGN->notes().front()->headWidth() / 2));

    // Exceptions for bends in TAB staves
    if (firstGN->preOrGraceBendSpacingExceptionInTab()) {
        xPos = 0.0;
    }

    item->setPos(xPos, 0.0);

    if (isTabStaff) {
        ChordLayout::layoutStem(parentChord, ctx);
    }
}

void TLayout::layoutGraceNotesGroup2(const GraceNotesGroup* item, GraceNotesGroup::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    //! NOTE A lot of spam
    // LD_CONDITION(ldata->isSetPos());

    Shape shape(Shape::Type::Composite);
    for (const Chord* grace : *item) {
        LD_CONDITION(grace->ldata()->isSetShape());
        LD_CONDITION(grace->ldata()->isSetPos());

        shape.add(grace->shape(LD_ACCESS::PASS).translate(grace->pos() - item->pos()));
    }
    ldata->setShape(shape);
}

void TLayout::layoutGradualTempoChangeSegment(GradualTempoChangeSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);

    GradualTempoChangeSegment::LayoutData* ldata = item->mutldata();

    ldata->disconnectSnappedItems();

    layoutTextLineBaseSegment(item, ctx);

    GradualTempoChangeSegment* tempoChangeSegmentSnappedBefore = item->findElementToSnapBefore();
    if (tempoChangeSegmentSnappedBefore) {
        ldata->connectItemSnappedBefore(tempoChangeSegmentSnappedBefore);
    }

    TempoText* tempoTextSnappedAfter = item->findElementToSnapAfter();
    if (tempoTextSnappedAfter) {
        ldata->connectItemSnappedAfter(tempoTextSnappedAfter);
    }

    if (item->isStyled(Pid::OFFSET)) {
        item->roffset() = item->tempoChange()->propertyDefault(Pid::OFFSET).value<PointF>();
    }

    Shape sh = textLineBaseSegmentShape(item);
    ldata->setShape(sh);

    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layoutGradualTempoChange(GradualTempoChange* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    layoutLine(item, ctx);
}

void TLayout::layoutGuitarBend(GuitarBend* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    item->computeBendAmount();

    GuitarBendLayout::updateSegmentsAndLayout(item, ctx);

    if (item->staffType()->isTabStaff()) {
        item->updateHoldLine();
        if (item->holdLine()) {
            GuitarBendLayout::updateSegmentsAndLayout(item->holdLine(), ctx);
        }
    }
}

void TLayout::layoutGuitarBendSegment(GuitarBendSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    GuitarBendSegment::LayoutData* ldata = item->mutldata();
    bool tabStaff = item->staffType()->isTabStaff();
    if (tabStaff) {
        GuitarBendLayout::layoutTabStaff(item, ctx);
    } else {
        GuitarBendLayout::layoutStandardStaff(item, ctx);
    }

    SysStaff* staff = item->system()->staff(item->staffIdx());
    Skyline& skyline = staff->skyline();
    SkylineLine& skylineLine = tabStaff ? skyline.north() : (item->guitarBend()->ldata()->up() ? skyline.north() : skyline.south());
    skylineLine.add(item->shape().translate(item->pos()));

    fillGuitarBendSegmentShape(item, ldata);
}

void TLayout::fillGuitarBendSegmentShape(const GuitarBendSegment* item, GuitarBendSegment::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    Shape shape;
    shape.add(ldata->bbox(), item);
    if (!item->bendText()->empty()) {
        shape.add(item->bendText()->shape().translate(item->bendText()->pos()));
    }
    ldata->setShape(shape);
}

void TLayout::layoutHairpinSegment(HairpinSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    HairpinSegment::LayoutData* ldata = item->mutldata();

    const StaffType* stType = item->staffType();

    if (stType && stType->isHiddenElementOnTab(ctx.conf().style(), Sid::hairpinShowTabCommon, Sid::hairpinShowTabSimple)) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    const double _spatium = item->spatium();

    ldata->disconnectSnappedItems();

    manageHairpinSnapping(item, ctx);

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
            r.unite(item->text()->ldata()->bbox());
        }
        if (!item->endText()->empty()) {
            r.unite(item->endText()->ldata()->bbox().translated(x + item->endText()->ldata()->bbox().width(), 0.0));
        }
        double w  = item->absoluteFromSpatium(ctx.conf().styleS(Sid::hairpinLineWidth));
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
    if (ldata->offsetChanged() != OffsetChange::NONE) {
        Autoplace::rebaseOffset(item, ldata);
    }

    if (item->autoplace()) {
        Autoplace::autoplaceSpannerSegment(item, ldata, item->spatium());
    }

    Autoplace::setOffsetChanged(item, ldata, false);
}

void TLayout::manageHairpinSnapping(HairpinSegment* item, LayoutContext& ctx)
{
    HairpinSegment::LayoutData* ldata = item->mutldata();

    EngravingItem* possibleSnapBeforeElement = nullptr;
    EngravingItem* possibleSnapAfterElement = nullptr;
    if (item->isSingleBeginType()) {
        possibleSnapBeforeElement = item->findElementToSnapBefore();
    }
    if (item->isSingleEndType() && item->hairpin()->ticks().isNotZero()) {
        possibleSnapAfterElement = item->findElementToSnapAfter();
    }

    bool doSnapBefore = possibleSnapBeforeElement && item->hairpin()->snapToItemBefore();
    if (doSnapBefore && possibleSnapBeforeElement->isHairpinSegment()) {
        doSnapBefore = doSnapBefore && toHairpinSegment(possibleSnapBeforeElement)->hairpin()->snapToItemAfter()
                       && possibleSnapBeforeElement->ldata()->itemSnappedBefore() != item;
    }

    if (doSnapBefore) {
        ldata->connectItemSnappedBefore(possibleSnapBeforeElement);
    }

    bool doSnapAfter = possibleSnapAfterElement && item->hairpin()->snapToItemAfter();
    if (doSnapAfter && possibleSnapAfterElement->isHairpinSegment()) {
        doSnapAfter = doSnapAfter && toHairpinSegment(possibleSnapAfterElement)->hairpin()->snapToItemBefore()
                      && possibleSnapAfterElement->ldata()->itemSnappedAfter() != item;
    }

    if (doSnapAfter) {
        ldata->connectItemSnappedAfter(possibleSnapAfterElement);
    }

    // In case of dynamics/expressions before or after, make space for them horizontally
    double hairpinDistToDynOrExpr = ctx.conf().style().styleMM(Sid::autoplaceHairpinDynamicsDistance);

    bool makeSpaceBefore = (doSnapBefore && possibleSnapBeforeElement->isTextBase())
                           || (possibleSnapBeforeElement && possibleSnapBeforeElement->isDynamic());

    if (makeSpaceBefore && possibleSnapBeforeElement->findAncestor(ElementType::SYSTEM) == item->system()) {
        double xItemPos = possibleSnapBeforeElement->pageX() - item->system()->pageX();
        double itemRightEdge = xItemPos + possibleSnapBeforeElement->ldata()->bbox().right();
        double xMinHairpinStart = itemRightEdge + hairpinDistToDynOrExpr;
        double xStartDiff = ldata->pos().x() - xMinHairpinStart;
        if (xStartDiff < 0) {
            ldata->setPosX(xMinHairpinStart);
            item->rxpos2() += xStartDiff;
        }
    }

    bool makeSpaceAfter = (doSnapAfter && possibleSnapAfterElement->isTextBase())
                          || (possibleSnapAfterElement && possibleSnapAfterElement->isDynamic());

    if (makeSpaceAfter && possibleSnapAfterElement->findAncestor(ElementType::SYSTEM) == item->system()) {
        double xItemPos = possibleSnapAfterElement->pageX() - item->system()->pageX();
        double itemLeftEdge = xItemPos + possibleSnapAfterElement->ldata()->bbox().left();
        double xMaxHairpinEnd = itemLeftEdge - hairpinDistToDynOrExpr;
        double xEndDiff = xMaxHairpinEnd - (item->pos().x() + item->pos2().x());
        const double EXTEND_THRESHOLD = 3.0 * item->spatium();
        if (xEndDiff < 0) {
            item->rxpos2() += xEndDiff;
        } else if (item->hairpin()->snapToItemAfter() && xEndDiff > EXTEND_THRESHOLD) {
            item->rxpos2() += xEndDiff;
        }
    }
}

void TLayout::layoutHairpin(Hairpin* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    item->setPos(0.0, 0.0);
    layoutTextLineBase(item, ctx);
}

void TLayout::layoutHammerOnPullOff(HammerOnPullOff* item, LayoutContext& ctx)
{
    layoutSlur(static_cast<Slur*>(item), ctx);
}

void TLayout::layoutHammerOnPullOffSegment(HammerOnPullOffSegment* item, LayoutContext& ctx)
{
    // The layout of the slur has already been done. Here we layout the H/P letters.
    item->updateHopoText();

    System* system = item->system();
    Fraction systemEndTick = system->endTick();
    Skyline& sk = system->staff(item->staffIdx())->skyline();

    for (HammerOnPullOffText* hopoText : item->hopoText()) {
        bool above = hopoText->placeAbove();

        Align align;
        align.vertical = above ? AlignV::BASELINE : AlignV::TOP;
        align.horizontal = AlignH::HCENTER;
        hopoText->setAlign(align);
        layoutItem(hopoText, ctx);

        const Chord* startChord = hopoText->startChord();
        const Chord* endChord = hopoText->endChord();
        double startX = startChord->systemPos().x() + startChord->upNote()->headWidth();
        double endX = startX;
        if (endChord->tick() < systemEndTick) {
            endX = endChord->systemPos().x();
        } else {
            // The last endChord of this segment is in next system. Use end barline instead.
            Segment* endSeg = system->lastMeasure()->last(SegmentType::BarLineType);
            endX = endSeg ? endSeg->systemPos().x() : endX;
        }
        if (startChord->stem() && endChord->stem() && startChord->up() == above && endChord->up() == above) {
            // Mid-way between centered on the notes and centered on the stems
            endX += (above ? 0.5 : -0.5) * endChord->upNote()->headWidth();
        }
        double centerX = 0.5 * (startX + endX);

        double vertPadding = 0.5 * item->spatium();
        Shape hopoTextShape = hopoText->ldata()->shape().translated(PointF(centerX, 0.0));
        SkylineLine& skl = above ? sk.north() : sk.south();
        SkylineLine skyline = skl.getFilteredCopy([](const ShapeElement& el) {
            return el.item() && el.item()->isArticulationFamily();
        });

        double y = above ? -skyline.minDistanceToShapeAbove(hopoTextShape) : skyline.minDistanceToShapeBelow(hopoTextShape);
        y += above ? -vertPadding : vertPadding;
        y = above ? std::min(y, -vertPadding) : std::max(y, item->staff()->staffHeight(item->tick()) + vertPadding);

        Note* startNote = above ? startChord->upNote() : startChord->downNote();
        Note* endNote = above ? endChord->upNote() : endChord->downNote();
        double yNoteLimit = above ? std::min(startNote->y(), endNote->y()) - 2 * vertPadding
                            : std::max(startNote->y(), endNote->y()) + 2 * vertPadding;
        y = above ? std::min(y, yNoteLimit) : std::max(y, yNoteLimit);

        hopoText->mutldata()->setPos(centerX, y);

        hopoTextShape.translateY(y);
        skyline.add(hopoTextShape);
    }

    Shape hopoSegmentShape = item->mutldata()->shape();
    for (HammerOnPullOffText* hopoText : item->hopoText()) {
        hopoSegmentShape.add(hopoText->ldata()->shape().translated(hopoText->pos()));
    }

    item->mutldata()->setShape(hopoSegmentShape);
}

void TLayout::layoutHammerOnPullOffText(HammerOnPullOffText* item, LayoutContext& ctx)
{
    layoutBaseTextBase(item, ctx);
}

void TLayout::fillHairpinSegmentShape(const HairpinSegment* item, HairpinSegment::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    Shape sh;
    switch (item->hairpin()->hairpinType()) {
    case HairpinType::CRESC_HAIRPIN:
    case HairpinType::DECRESC_HAIRPIN:
        sh = Shape(item->ldata()->bbox(), item);
        break;
    case HairpinType::DECRESC_LINE:
    case HairpinType::CRESC_LINE:
    default:
        sh = textLineBaseSegmentShape(item);
    }

    ldata->setShape(sh);
}

void TLayout::layoutHarpPedalDiagram(const HarpPedalDiagram* item, HarpPedalDiagram::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    const_cast<HarpPedalDiagram*>(item)->updateDiagramText();

    layoutBaseTextBase(item, ldata);

    if (item->autoplace()) {
        const Segment* s = toSegment(item->explicitParent());
        const Measure* m = s->measure();

        LD_CONDITION(ldata->isSetPos());
        LD_CONDITION(m->ldata()->isSetPos());
        LD_CONDITION(s->ldata()->isSetPos());
    }

    Autoplace::autoplaceSegmentElement(item, ldata);
}

void TLayout::layoutHarmonicMarkSegment(HarmonicMarkSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    HarmonicMarkSegment::LayoutData* ldata = item->mutldata();
    const StaffType* stType = item->staffType();
    if (stType
        && (!stType->isTabStaff()
            || stType->isHiddenElementOnTab(ctx.conf().style(), Sid::harmonicMarkShowTabCommon, Sid::harmonicMarkShowTabSimple))) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    layoutTextLineBaseSegment(item, ctx);

    Shape sh = textLineBaseSegmentShape(item);
    ldata->setShape(sh);

    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layoutHarmony(const Harmony* item, Harmony::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    if (!item->explicitParent()) {
        ldata->setPos(0.0, 0.0);
        const_cast<Harmony*>(item)->setOffset(0.0, 0.0);
    }

    if (ldata->layoutInvalid) {
        item->createBlocks(ldata);
    }

    if (ldata->blocks.empty()) {
        ldata->blocks.push_back(TextBlock());
    }

    auto calculateBoundingRect = [](const Harmony* item, Harmony::LayoutData* ldata, const LayoutContext& ctx) -> PointF
    {
        const double ypos = (item->placeBelow() && item->staff()) ? item->staff()->staffHeight(item->tick()) : 0.0;
        const FretDiagram* fd = (item->explicitParent() && item->explicitParent()->isFretDiagram())
                                ? toFretDiagram(item->explicitParent())
                                : nullptr;

        const double cw = item->symWidth(SymId::noteheadBlack);

        double newPosX = 0.0;
        double newPosY = 0.0;

        if (item->textList().empty()) {
            layoutBaseTextBase1(item, ldata);

            if (fd) {
                newPosY = ldata->pos().y();
            } else {
                newPosY = ypos - ((item->align() == AlignV::BOTTOM) ? -ldata->bbox().height() : 0.0);
            }
        } else {
            RectF bb;
            RectF hAlignBox;
            for (TextSegment* ts : item->textList()) {
                RectF tsBbox = ts->tightBoundingRect().translated(ts->x(), ts->y());
                bb.unite(tsBbox);

                if (ts->align()) {
                    hAlignBox.unite(tsBbox);
                }
            }

            double xx = 0.0;
            if (fd) {
                switch (ctx.conf().styleV(Sid::chordAlignmentToFretboard).value<AlignH>()) {
                case AlignH::LEFT:
                    xx = -hAlignBox.left();
                    break;
                case AlignH::HCENTER:
                    xx = -(hAlignBox.center().x());
                    break;
                case AlignH::RIGHT:
                    xx = -hAlignBox.right();
                    break;
                }
            } else {
                switch (item->noteheadAlign()) {
                case AlignH::LEFT:
                    xx = -hAlignBox.left();
                    break;
                case AlignH::HCENTER:
                    xx = -(hAlignBox.center().x());
                    break;
                case AlignH::RIGHT:
                    xx = -hAlignBox.right();
                    break;
                }
            }

            double yy = -bb.y();      // Align::TOP
            if (item->align() == AlignV::VCENTER) {
                yy = -bb.y() / 2.0;
            } else if (item->align() == AlignV::BASELINE) {
                yy = item->baseLine();
            } else if (item->align() == AlignV::BOTTOM) {
                yy = -bb.height() - bb.y();
            }

            if (fd) {
                newPosY = ypos - yy - ctx.conf().styleMM(Sid::harmonyFretDist);
            } else {
                newPosY = ypos;
            }

            for (TextSegment* ts : item->textList()) {
                ts->setOffset(PointF(xx, yy));
            }

            ldata->setBbox(bb.translated(xx, yy));
            ldata->harmonyHeight = ldata->bbox().height();
        }

        if (fd) {
            switch (ctx.conf().styleV(Sid::chordAlignmentToFretboard).value<AlignH>()) {
            case AlignH::LEFT:
                newPosX = 0.0;
                break;
            case AlignH::HCENTER:
                newPosX = 0.5 * fd->mainWidth();
                break;
            case AlignH::RIGHT:
                newPosX = fd->mainWidth();
                break;
            }
        } else {
            switch (item->noteheadAlign()) {
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
    };

    auto positionPoint = calculateBoundingRect(item, ldata, ctx);

    if (item->isPolychord()) {
        for (LineF& line : ldata->polychordDividerLines.mut_value()) {
            line.setP1(PointF(ldata->bbox().left(), line.y1()));
            line.setP2(PointF(ldata->bbox().right(), line.y2()));
        }
    }

    if (item->hasFrame()) {
        item->layoutFrame(ldata);
    }

    ldata->setPos(positionPoint);
}

void TLayout::layoutHook(const Hook* item, Hook::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    if (ldata->isValid()) {
        return;
    }

    ldata->setBbox(item->symBbox(item->sym()));
}

void TLayout::layoutImage(const Image* item, Image::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    ldata->setPos(0.0, 0.0);
    const_cast<Image*>(item)->init();

    SizeF imageSize = item->size();

    // if autoscale && inside a box, scale to box relevant size
    if (item->autoScale()
        && ((item->explicitParent()->isHBox() || item->explicitParent()->isVBox()))) {
        const EngravingItem::LayoutData* parentLD = item->parentItem()->ldata();

        LD_CONDITION(parentLD->isSetBbox());

        if (item->lockAspectRatio()) {
            double f = item->sizeIsSpatium() ? item->spatium() : DPMM;
            SizeF size(item->imageSize());
            double ratio = size.width() / size.height();
            double w = parentLD->bbox().width();
            double h = parentLD->bbox().height();
            if ((w / h) < ratio) {
                imageSize.setWidth(w / f);
                imageSize.setHeight((w / ratio) / f);
            } else {
                imageSize.setHeight(h / f);
                imageSize.setWidth(h * ratio / f);
            }
        } else {
            imageSize = item->pixel2size(parentLD->bbox().size());
        }
    }

    const_cast<Image*>(item)->setSize(imageSize);

    // in any case, adjust position relative to parent
    ldata->setBbox(RectF(PointF(), item->size2pixel(imageSize)));
}

void TLayout::layoutInstrumentChange(const InstrumentChange* item, InstrumentChange::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    layoutBaseTextBase(item, ldata);

    if (item->autoplace()) {
        const Segment* s = toSegment(item->explicitParent());
        const Measure* m = s->measure();
        LD_CONDITION(ldata->isSetPos());
        LD_CONDITION(m->ldata()->isSetPos());
        LD_CONDITION(s->ldata()->isSetPos());
    }

    Autoplace::autoplaceSegmentElement(item, ldata);
}

void TLayout::layoutInstrumentName(const InstrumentName* item, InstrumentName::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;
    //! NOTE There are problems, an investigation is required
//    if (ldata->isValid()) {
//        return;
//    }

    layoutBaseTextBase(item, ldata);
}

void TLayout::layoutJump(const Jump* item, Jump::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    LD_CONDITION(item->parentItem()->ldata()->isSetBbox());

    layoutBaseTextBase(item, ldata);

    if (item->autoplace()) {
        const Measure* m = toMeasure(item->explicitParent());
        LD_CONDITION(ldata->isSetPos());
        LD_CONDITION(ldata->isSetBbox());
        LD_CONDITION(m->ldata()->isSetPos());
    }

    Autoplace::autoplaceMeasureElement(item, ldata);
}

static void keySigAddLayout(const KeySig* item, const LayoutConfiguration& conf, SymId sym, int line, KeySig::LayoutData* ldata)
{
    double _spatium = item->spatium();
    double step = _spatium * (item->staff() ? item->staff()->staffTypeForElement(item)->lineDistance().val() * 0.5 : 0.5);
    KeySym ks;
    ks.sym = sym;
    double x = 0.0;
    if (ldata->keySymbols.size() > 0) {
        const KeySym& previous = ldata->keySymbols.back();
        double accidentalGap = conf.styleS(Sid::keysigAccidentalDistance).val();
        if (previous.sym != sym) {
            accidentalGap *= 2;
        } else if (previous.sym == SymId::accidentalNatural && sym == SymId::accidentalNatural) {
            accidentalGap = conf.styleS(Sid::keysigNaturalDistance).val();
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

void TLayout::layoutKeySig(const KeySig* item, KeySig::LayoutData* ldata, const LayoutConfiguration& conf)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    //! NOTE There are problems, an investigation is required
//    if (ldata->isValid()) {
//        return;
//    }

    double spatium = item->spatium();
    double step = spatium * (item->staff() ? item->staff()->staffTypeForElement(item)->lineDistance().val() * 0.5 : 0.5);

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
                const bool isClefSeg
                    = (seg->isClefType() || seg->isHeaderClefType()
                       || (seg->isClefRepeatAnnounceType() && item->segment()->isKeySigRepeatAnnounceType()));
                if (seg->enabled() && isClefSeg) {
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
        double accidentalGap = conf.styleS(Sid::keysigAccidentalDistance).val();
        // add standard key accidentals first, if necessary
        for (int i = 1; i <= std::abs(t1) && std::abs(t1) <= 7; ++i) {
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
                const KeySym& previous = ldata->keySymbols.back();
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
            naturalsOn = !newSection && (conf.styleI(Sid::keySigNaturals) != int(KeySigNatural::NONE) || (t1 == 0));
        }

        // Don't repeat naturals if shown in courtesy
        if (item->measure() && item->measure()->system() && item->measure()->isFirstInSystem()
            && prevMeasure && prevMeasure->findSegment(SegmentType::KeySigAnnounce, item->tick())
            && !item->segment()->isKeySigAnnounceType()) {
            naturalsOn = false;
        }
        if (item->track() == muse::nidx) {
            naturalsOn = false;
        }

        int coffset = 0;
        Key t2      = Key::C;
        if (naturalsOn) {
            if (item->staff()) {
                t2 = item->staff()->key(item->tick() - Fraction(1, 480 * 4));
            }
            if (item->segment() && item->segment()->isType(SegmentType::KeySigStartRepeatAnnounce)) {
                // Handle naturals in continuation courtesy
                Segment* prevCourtesySeg
                    = prevMeasure ? prevMeasure->findSegmentR(SegmentType::KeySigRepeatAnnounce, prevMeasure->ticks()) : nullptr;
                EngravingItem* prevCourtesy = prevCourtesySeg ? prevCourtesySeg->element(item->track()) : nullptr;
                t2 = prevCourtesy && prevCourtesy->isKeySig() ? toKeySig(prevCourtesy)->key() : t2;
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
                              && (conf.styleI(Sid::keySigNaturals) == int(KeySigNatural::BEFORE)
                                  || t1 * int(t2) < 0);

        // naturals should go AFTER accidentals if they should not go before!
        bool suffixNaturals = naturalsOn && !prefixNaturals;

        const signed char* lines = ClefInfo::lines(clef);

        if (prefixNaturals) {
            for (int i = 0; i < 7; ++i) {
                if (naturals & (1 << i)) {
                    keySigAddLayout(item, conf, SymId::accidentalNatural, lines[i + coffset], ldata);
                }
            }
        }
        if (std::abs(t1) <= 7) {
            SymId symbol = t1 > 0 ? SymId::accidentalSharp : SymId::accidentalFlat;
            int lineIndexOffset = t1 > 0 ? 0 : 7;
            for (int i = 0; i < std::abs(t1); ++i) {
                keySigAddLayout(item, conf, symbol, lines[lineIndexOffset + i], ldata);
            }
        } else {
            LOGD("illegal t1 key %d", t1);
        }

        // add suffixed naturals, if any
        if (suffixNaturals) {
            for (int i = 0; i < 7; ++i) {
                if (naturals & (1 << i)) {
                    keySigAddLayout(item, conf, SymId::accidentalNatural, lines[i + coffset], ldata);
                }
            }
        }

        // Follow stepOffset
        if (item->staffType()) {
            ldata->setPosY(item->staffType()->stepOffset() * 0.5 * spatium);
        }
    }

    ldata->moveY(item->staffOffsetY());

    Shape keySigShape;
    for (const KeySym& ks : ldata->keySymbols) {
        double x = ks.xPos * spatium;
        double y = ks.line * step;
        keySigShape.add(item->symBbox(ks.sym).translated(x, y), item);
    }
    ldata->setShape(keySigShape);
}

void TLayout::layoutLaissezVib(LaissezVib* item)
{
    UNUSED(item);
}

void TLayout::layoutLayoutBreak(const LayoutBreak* item, LayoutBreak::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    if (ldata->isValid() || MScore::testMode) {
        // Don't layout in test mode because these are essentially UI elements,
        // and they need to know about the Icon font, which isn't available in test mode.
        return;
    }

    FontMetrics metrics(item->font());
    RectF bbox = metrics.boundingRect(item->iconCode());
    ldata->setShape(Shape(bbox, item));
}

void TLayout::layoutSystemLockIndicator(const SystemLockIndicator* item, SystemLockIndicator::LayoutData* ldata)
{
    if (MScore::testMode) {
        // Don't layout in test mode because these are essentially UI elements,
        // and they need to know about the Icon font, which isn't available in test mode.
        return;
    }

    Shape shape;

    FontMetrics metrics(item->font());
    RectF lockBox = metrics.boundingRect(item->iconCode());
    shape.add(lockBox, item);

    if (item->selected()) {
        const SystemLock* lock = item->systemLock();
        double xStart = lock->startMB()->x();
        double xEnd = lock->endMB()->x() + lock->endMB()->width();
        double width = xEnd - xStart;
        double y = lockBox.top() - 0.5 * item->spatium();
        double height = lockBox.height() + item->spatium();
        ldata->rangeRect = RectF(xStart, y, width, height).translated(-item->x(), 0.0);
        shape.add(ldata->rangeRect);
    }

    ldata->setShape(shape);

    double spatium = item->spatium();

    const MeasureBase* endMB = item->systemLock()->endMB();
    double x = endMB->x() + endMB->width();
    x -= lockBox.right() + 0.5 * spatium;

    double xLayoutBreaks = endMB->x() + endMB->width();
    for (EngravingItem* el : endMB->el()) {
        if (el->isLayoutBreak()) {
            xLayoutBreaks = std::min(xLayoutBreaks, endMB->x() + el->x() + el->ldata()->bbox().left() - spatium);
        }
    }

    x = std::min(x, xLayoutBreaks - lockBox.right());

    ldata->setPos(PointF(x, -2.5 * spatium));

    // Ensure it goes behind notation and LayoutBreak
    const_cast<SystemLockIndicator*>(item)->setZ(-100);
}

static void _layoutLedgerLine(const LedgerLine* item, const LayoutContext& ctx, LedgerLine::LayoutData* ldata)
{
    double chordMag = item->chord()->mag();
    ldata->setMag(chordMag);
    ldata->lineWidth = ctx.conf().styleMM(Sid::ledgerLineWidth) * chordMag;
    if (item->staff()) {
        const_cast<LedgerLine*>(item)->setColor(item->staff()->staffType(item->tick())->color());
    }
    double w2 = ldata->lineWidth * .5;

    //Adjust Y position to staffType offset
    ldata->moveY(item->staffOffsetY());

    if (item->vertical()) {
        ldata->setBbox(-w2, 0, w2, item->len());
    } else {
        ldata->setBbox(0, -w2, item->len(), 2 * w2);
    }
}

void TLayout::layoutLedgerLine(LedgerLine* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    _layoutLedgerLine(item, ctx, item->mutldata());
}

void TLayout::layoutLetRing(LetRing* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    layoutLine(item, ctx);
}

void TLayout::layoutLetRingSegment(LetRingSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    HarmonicMarkSegment::LayoutData* ldata = item->mutldata();

    const StaffType* stType = item->staffType();

    if (stType && stType->isHiddenElementOnTab(ctx.conf().style(), Sid::letRingShowTabCommon, Sid::letRingShowTabSimple)) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    layoutTextLineBaseSegment(item, ctx);

    Shape sh = textLineBaseSegmentShape(item);
    ldata->setShape(sh);

    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layoutLineSegment(LineSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    layoutItem(item, ctx);
}

void TLayout::layoutLyrics(Lyrics* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    LyricsLayout::layout(item, ctx);
}

void TLayout::layoutLyricsLine(LyricsLine* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    LyricsLayout::layout(item, ctx);
}

void TLayout::layoutLyricsLineSegment(LyricsLineSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    LyricsLayout::layout(item, ctx);
}

void TLayout::layoutMarker(const Marker* item, Marker::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    LD_CONDITION(item->parentItem()->ldata()->isSetBbox());

    TLayout::layoutBaseTextBase(item, ldata);

    // although normally laid out to parent (measure) width,
    // force to center over barline if left-aligned
    if (item->layoutToParentWidth() && item->align() == AlignH::LEFT) {
        ldata->moveX(-ldata->bbox().width() * 0.5);
    }

    if (item->autoplace()) {
        const Measure* m = toMeasure(item->explicitParent());
        LD_CONDITION(ldata->isSetPos());
        LD_CONDITION(ldata->isSetBbox());
        LD_CONDITION(m->ldata()->isSetPos());
    }

    Autoplace::autoplaceMeasureElement(item, ldata);
}

void TLayout::layoutMeasureBase(MeasureBase* item, LayoutContext& ctx)
{
    layoutItem(item, ctx);
}

void TLayout::layoutBaseMeasureBase(const MeasureBase* item, MeasureBase::LayoutData* ldata, const LayoutContext& ctx)
{
    LD_CONDITION(ldata->isSetBbox());

    double x = ldata->bbox().width();
    for (EngravingItem* e : item->el()) {
        if (e->isLayoutBreak()) {
            TLayout::layoutItem(e, const_cast<LayoutContext&>(ctx));
            EngravingItem::LayoutData* eldata = e->mutldata();
            double spatium = item->spatium();
            double y = 0.0;
            if (toLayoutBreak(e)->isNoBreak()) {
                x -= eldata->bbox().right() * .5;
            } else {
                x -= eldata->bbox().right() + 0.5 * spatium;
            }
            y = -2.5 * spatium;
            eldata->setPos(x, y);
            x += eldata->bbox().left() - 0.5 * spatium;
        } else if (e->isMarker() || e->isJump()) {
        } else {
            layoutItem(e, const_cast<LayoutContext&>(ctx));
        }
    }
}

void TLayout::layoutMeasureNumber(const MeasureNumber* item, MeasureNumber::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    LD_CONDITION(item->measure()->ldata()->isSetBbox()); // layoutMeasureNumberBase

    layoutMeasureNumberBase(item, ldata, ctx);
}

void TLayout::layoutMeasureNumberBase(const MeasureNumberBase* item, MeasureNumberBase::LayoutData* ldata, const LayoutContext& ctx)
{
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    LD_CONDITION(item->measure()->ldata()->isSetBbox());

    ldata->setPos(PointF());

    layoutBaseTextBase1(item, ldata);

    if (item->placeBelow()) {
        double yoff = ldata->bbox().height();

        // If there is only one line, the barline spans outside the staff lines, so the default position is not correct.
        if (item->staff()->constStaffType(item->measure()->tick())->lines() == 1) {
            yoff += 2.0 * item->spatium();
        } else {
            yoff += item->staff()->staffHeight();
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

        const Measure* measure = item->measure();

        // find first chordrest
        const Segment* crSeg = measure->first(SegmentType::ChordRest);

        const MeasureLayout::MeasureStartEndPos measureStartEnd = MeasureLayout::getMeasureStartEndPos(measure, crSeg,
                                                                                                       item->staffIdx(), true, false, ctx);
        const double x1 = measureStartEnd.x1;
        const double x2 = measureStartEnd.x2;

        ldata->setPosX((x1 + x2) * 0.5);
    } else if (item->hPlacement() == PlacementH::RIGHT) {
        ldata->setPosX(item->measure()->ldata()->bbox().width());
    }
}

void TLayout::layoutMeasureRepeat(const MeasureRepeat* item, MeasureRepeat::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);

    //! NOTE The types are listed here explicitly to show what types there are (see Rest::add method)
    //! and accordingly show what depends on.
    for (EngravingItem* e : item->el()) {
        switch (e->type()) {
        case ElementType::DEAD_SLAPPED: {
            DeadSlapped* ds = item_cast<DeadSlapped*>(e);
            LD_INDEPENDENT;
            layoutDeadSlapped(ds, ds->mutldata());
        } break;
        case ElementType::SYMBOL: {
            Symbol* s = item_cast<Symbol*>(e);
            // LD_X not clear yet
            layoutSymbol(s, s->mutldata(), ctx);
        } break;
        case ElementType::IMAGE: {
            Image* im = item_cast<Image*>(e);
            LD_INDEPENDENT;
            layoutImage(im, im->mutldata());
        } break;
        default:
            UNREACHABLE;
        }
    }

    switch (item->numMeasures()) {
    case 1:
    {
        ldata->setSymId(SymId::repeat1Bar);
        if (ctx.conf().styleB(Sid::mrNumberSeries) && item->track() != muse::nidx) {
            int placeInSeries = 2; // "1" would be the measure actually being repeated
            staff_idx_t staffIdx = item->staffIdx();
            const Measure* m = item->measure();
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

    if (item->track() != muse::nidx) { // if this is in score rather than a palette cell
        // For unknown reasons, the symbol has some offset in almost all SMuFL fonts
        // We compensate for it, to make sure the symbol is visually centered around the staff line
        double offset = (-bbox.top() - bbox.bottom()) / 2.0;

        const StaffType* staffType = item->staffType();

        // Only need to set y position here; x position is handled in MeasureLayout::layoutMeasureElements()
        ldata->setPos(0, std::floor(staffType->middleLine() / 2.0) * staffType->lineDistance().val() * item->spatium() + offset);
    }

    ChordLayout::fillShape(item, ldata, ctx.conf());
}

void TLayout::layoutMeasureRepeatExtender(const MeasureRepeat* item, MeasureRepeat::LayoutData* ldata, const LayoutContext& ctx)
{
    ldata->extenderLineLeft = LineF();
    ldata->extenderLineRight = LineF();

    Measure* parentMeasure = item->measure();
    if (!parentMeasure) {
        return;
    }

    Measure* startMeasure = parentMeasure->prevMeasure();
    if (!startMeasure || startMeasure->system() != parentMeasure->system()) {
        return;
    }

    Measure* endMeasure = parentMeasure->nextMeasure();
    if (endMeasure) {
        endMeasure = endMeasure->nextMeasure();
    }
    if (!endMeasure || endMeasure->system() != parentMeasure->system()) {
        return;
    }

    // Reuse the function that calculates start and end pos for mmRests,
    // except start and end point are on different measures
    staff_idx_t staffIdx = item->staffIdx();
    double xStart = MeasureLayout::getMeasureStartEndPos(startMeasure, startMeasure->first(SegmentType::ChordRest), staffIdx,
                                                         /*needHeaderException*/ startMeasure->header(), /*modernMMrest*/ true, ctx).x1;
    double xEnd = MeasureLayout::getMeasureStartEndPos(endMeasure, endMeasure->first(SegmentType::ChordRest), staffIdx,
                                                       /*needHeaderException*/ false, /*modernMRest*/ true, ctx).x2;

    double margin = ctx.conf().styleMM(Sid::multiMeasureRestMargin);
    xStart += margin;
    xEnd -= margin;

    double xItemInSysCoords = item->x() + item->segment()->x() + parentMeasure->x();

    PointF startPoint(xStart + startMeasure->x() - xItemInSysCoords, 0.0);
    PointF midPointL(-0.5 * item->spatium(), 0.0);
    ldata->extenderLineLeft = LineF(startPoint, midPointL);

    PointF midPointR(item->symWidth(ldata->symId) + 0.5 * item->spatium(), 0.0);
    PointF endPoint(xEnd + endMeasure->x() - xItemInSysCoords, 0.0);
    ldata->extenderLineRight = LineF(midPointR, endPoint);
}

void TLayout::layoutMMRest(const MMRest* item, MMRest::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);

    //! NOTE The types are listed here explicitly to show what types there are (see Rest::add method)
    //! and accordingly show what depends on.
    for (EngravingItem* e : item->el()) {
        switch (e->type()) {
        case ElementType::DEAD_SLAPPED: {
            DeadSlapped* ds = item_cast<DeadSlapped*>(e);
            LD_INDEPENDENT;
            layoutDeadSlapped(ds, ds->mutldata());
        } break;
        case ElementType::SYMBOL: {
            Symbol* s = item_cast<Symbol*>(e);
            // LD_X not clear yet
            layoutSymbol(s, s->mutldata(), ctx);
        } break;
        case ElementType::IMAGE: {
            Image* im = item_cast<Image*>(e);
            LD_INDEPENDENT;
            layoutImage(im, im->mutldata());
        } break;
        default:
            UNREACHABLE;
        }
    }

    LD_CONDITION(ldata->restWidth.has_value());

    //! NOTE This is not look like layout data, perhaps this is should be set not here
    ldata->number = item->measure()->mmRestCount();
    ldata->setNumberSym(ldata->number);
    ldata->yNumberPos = ctx.conf().styleMM(Sid::mmRestNumberPos);

    const Staff* staff = item->staff();
    if (staff->lines(item->tick()) == 1) {
        ldata->yNumberPos = std::min(ldata->yNumberPos, -item->spatium());
    }

    if (item->isOldStyle()) {
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
    }

    // Only need to set y position here; x position is handled in MeasureLayout::layoutMeasureElements()
    const StaffType* staffType = item->staffType();
    double midline = staffType->middleLine() / 2.0;
    if (item->isOldStyle()) {
        midline = std::max(1.0, midline);
    }
    ldata->setPos(0, midline * staffType->lineDistance().val() * item->spatium());

    ChordLayout::fillShape(item, ldata, ctx.conf());
}

void TLayout::layoutMMRestRange(const MMRestRange* item, MMRestRange::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    LD_CONDITION(item->measure()->ldata()->isSetBbox()); // layoutMeasureNumberBase

    layoutMeasureNumberBase(item, ldata, ctx);
}

void TLayout::layoutNote(const Note* item, Note::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    if (!ldata->isSetPos()) {
        ldata->setPos(PointF());
    }

    bool useTablature = item->staff() && item->staff()->isTabStaff(item->chord()->tick());
    ldata->useTablature.set_value(useTablature);

    RectF noteBBox;
    if (useTablature) {
        if (item->displayFret() == Note::DisplayFretOption::Hide) {
            return;
        }

        const Staff* st = item->staff();
        const StaffType* tab = st->staffTypeForElement(item);
        // not complete but we need systems to be laid out to add parenthesis
        if (item->fixed()) {
            const_cast<Note*>(item)->setFretString(u"/");
        } else {
            const_cast<Note*>(item)->setFretString(tab->fretString(std::fabs(item->fret()), item->string(), item->deadNote()));

            if (item->negativeFretUsed()) {
                const_cast<Note*>(item)->setFretString(u"-" + item->fretString());
            }

            if (item->displayFret() == Note::DisplayFretOption::ArtificialHarmonic) {
                const_cast<Note*>(item)->setFretString(String(u"%1 <%2>").arg(item->fretString(), String::number(item->harmonicFret())));
            } else if (item->displayFret() == Note::DisplayFretOption::NaturalHarmonic) {
                const_cast<Note*>(item)->setFretString(String(u"<%1>").arg(String::number(item->harmonicFret())));
            }
        }

        if (item->ghost()) {
            const_cast<Note*>(item)->setHeadHasParentheses(true, /* addToLinked= */ false, /* generated= */ true);
        } else {
            const_cast<Note*>(item)->setHeadHasParentheses(false, /*addToLinked=*/ false, /* generated= */ true);
        }

        double w = item->tabHeadWidth(tab);
        double mags = item->magS();

        const MStyle& style = item->style();

        double y = item->deadNote() ? tab->deadFretBoxY(style) : tab->fretBoxY(style);
        double height = item->deadNote() ? tab->deadFretBoxH(style) : tab->fretBoxH(style);

        noteBBox = RectF(0, y * mags, w, height * mags);
    } else {
        if (item->deadNote()) {
            const_cast<Note*>(item)->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
        } else if (item->harmonic()) {
            const_cast<Note*>(item)->setHeadGroup(NoteHeadGroup::HEAD_DIAMOND);
        }

        if (item->configuration()->shouldAddParenthesisOnStandardStaff()) {
            if (item->ghost()) {
                const_cast<Note*>(item)->setHeadHasParentheses(true, /* addToLinked= */ false, /* generated= */ true);
            } else {
                const_cast<Note*>(item)->setHeadHasParentheses(false, /* addToLinked= */ false, /* generated= */ true);
            }
        }

        SymId nh = item->noteHead();
        if (item->configuration()->crossNoteHeadAlwaysBlack() && ((nh == SymId::noteheadXHalf) || (nh == SymId::noteheadXWhole))) {
            nh = SymId::noteheadXBlack;
        }

        ldata->cachedNoteheadSym.set_value(nh);

        if (item->isNoteName()) {
            ldata->cachedSymNull.set_value(SymId::noteEmptyBlack);
            NoteHeadType ht = item->headType() == NoteHeadType::HEAD_AUTO ? item->chord()->durationType().headType() : item->headType();
            if (ht == NoteHeadType::HEAD_WHOLE) {
                ldata->cachedSymNull.set_value(SymId::noteEmptyWhole);
            } else if (ht == NoteHeadType::HEAD_HALF) {
                ldata->cachedSymNull.set_value(SymId::noteEmptyHalf);
            }
        } else {
            ldata->cachedSymNull.set_value(SymId::noSym);
        }
        noteBBox = item->symBbox(nh);
    }

    ldata->setBbox(noteBBox);

    fillNoteShape(item, ldata);
}

void TLayout::fillNoteShape(const Note* item, Note::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    RectF noteBBox = ldata->bbox();

    Shape shape(Shape::Type::Composite);
    shape.add(noteBBox, item);

    for (const NoteDot* dot : item->dots()) {
        if (!dot->addToSkyline()) {
            continue;
        }
        shape.add(item->symBbox(SymId::augmentationDot).translated(dot->pos()), dot);
    }

    Accidental* acc = item->accidental();
    if (acc && acc->addToSkyline()) {
        shape.add(acc->ldata()->shape().translated(acc->pos()));
    }
    const LaissezVib* lv = item->laissezVib();
    const LaissezVibSegment* lvSeg = lv && !lv->segmentsEmpty() ? lv->frontSegment() : nullptr;
    if (lvSeg && lvSeg->addToSkyline()) {
        const PointF cornerClosestToNote = lv->up() ? lvSeg->shape().bbox().bottomLeft() : lvSeg->shape().bbox().topLeft();
        const PointF pos = lvSeg->ldata()->posRelativeToNote.value() - cornerClosestToNote;
        shape.add(lvSeg->shape().translate(pos));
    }
    for (auto e : item->el()) {
        if (e->addToSkyline()) {
            if (e->isFingering() && toFingering(e)->layoutType() != ElementType::NOTE) {
                continue;
            }
            shape.add(e->ldata()->bbox().translated(e->pos()), e);
        }
    }

    // This method is also called from SingleLayout, where `part` may be nullptr
    Part* part = item->part();
    if (part && part->instrument()->hasStrings() && !item->staffType()->isTabStaff()) {
        GuitarBend* bend = item->bendFor();
        if (bend && bend->addToSkyline() && bend->type() == GuitarBendType::SLIGHT_BEND && !bend->segmentsEmpty()) {
            GuitarBendSegment* bendSeg = toGuitarBendSegment(bend->frontSegment());
            // Semi-hack: the relative position of note and bend
            // isn't fully known yet, so we use an approximation
            double sp = item->spatium();
            PointF approxRelPos(noteBBox.width() + 0.25 * sp, -0.25 * sp);
            shape.add(bendSeg->shape().translate(approxRelPos));
        }
    }

    ldata->setShape(shape);
}

void TLayout::layoutNoteDot(const NoteDot* item, NoteDot::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    if (ldata->isValid()) {
        return;
    }

    ldata->setBbox(item->symBbox(SymId::augmentationDot));
}

void TLayout::layoutOrnament(const Ornament* item, Ornament::LayoutData* ldata, const LayoutConfiguration& conf)
{
    LAYOUT_CALL_ITEM(item);
    LD_CONDITION(ldata->symId.has_value());

    ldata->setShape(Shape());

    layoutArticulation(static_cast<const Articulation*>(item), ldata);

    double _spatium = item->spatium();
    double vertMargin = 0.35 * _spatium;
    static constexpr double ornamentAccidentalMag = 0.6; // TODO: style?

    for (size_t i = 0; i < item->accidentalsAboveAndBelow().size(); ++i) {
        bool above = (i == 0);
        Accidental* accidental = item->accidentalsAboveAndBelow()[i];
        if (!accidental) {
            continue;
        }
        Accidental::LayoutData* accLData = accidental->mutldata();

        LD_CONDITION(accLData->isSetBbox());

        accidental->computeMag();

        accLData->setMag(accLData->mag() * ornamentAccidentalMag);
        accLData->syms.clear(); // Invalidate
        layoutAccidental(accidental, accLData, conf);
        Shape accidentalShape = accidental->shape();
        double minVertDist = above
                             ? accidentalShape.minVerticalDistance(Shape(ldata->bbox()))
                             : Shape(ldata->bbox()).minVerticalDistance(accidentalShape);

        accLData->setPos(-0.5 * accLData->bbox().width() + 0.5 * ldata->bbox().width(),
                         above ? (-minVertDist - vertMargin) : (minVertDist + vertMargin));
    }

    Shape sh(Shape::Type::Composite);
    sh.add(ldata->bbox(), item); // from Articulation
    for (const Accidental* accidental : item->accidentalsAboveAndBelow()) {
        if (accidental && accidental->visible()) {
            LD_CONDITION(accidental->ldata()->isSetShape());
            LD_CONDITION(accidental->ldata()->isSetPos());
            sh.add(accidental->shape().translate(accidental->pos()));
        }
    }

    ldata->setShape(sh);
}

void TLayout::layoutOrnamentCueNote(Ornament* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
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

    ChordLayout::layoutChords3({ cueNoteChord }, { cueNote }, item->staff(), ctx);
    ChordLayout::layoutLedgerLines({ cueNoteChord }, ctx);
    AccidentalsLayout::layoutAccidentals({ cueNoteChord }, ctx);
    layoutChord(cueNoteChord, ctx);

    Shape noteShape = cueNoteChord->shape();
    Shape parentChordShape = parentChord->shape();
    double minDist = HorizontalSpacing::minHorizontalDistance(parentChordShape, noteShape, parentChord->spatium());
    // Check for possible other chords in same segment
    staff_idx_t startStaff = staff2track(parentChord->staffIdx());
    for (staff_idx_t staff = startStaff; staff < startStaff + VOICES; ++staff) {
        Segment* segment = parentChord->segment();
        ChordRest* cr = segment->elementAt(staff) ? toChordRest(segment->elementAt(staff)) : nullptr;
        if (cr) {
            minDist = std::max(minDist, HorizontalSpacing::minHorizontalDistance(cr->shape(), noteShape, cr->spatium()));
        }
    }
    cueNoteChord->mutldata()->setPosX(minDist);
}

void TLayout::layoutOttava(Ottava* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    layoutLine(item, ctx);
}

void TLayout::layoutOttavaSegment(OttavaSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    OttavaSegment::LayoutData* ldata = item->mutldata();
    layoutTextLineBaseSegment(item, ctx);
    Shape sh = textLineBaseSegmentShape(item);
    ldata->setShape(sh);
    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layoutPalmMute(PalmMute* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    layoutLine(item, ctx);
}

void TLayout::layoutPalmMuteSegment(PalmMuteSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    PalmMuteSegment::LayoutData* ldata = item->mutldata();

    const StaffType* stType = item->staffType();

    if (stType && stType->isHiddenElementOnTab(ctx.conf().style(), Sid::palmMuteShowTabCommon, Sid::palmMuteShowTabSimple)) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    layoutTextLineBaseSegment(item, ctx);

    Shape sh = textLineBaseSegmentShape(item);
    ldata->setShape(sh);

    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layoutParenthesis(Parenthesis* item, LayoutContext& ctx)
{
    UNUSED(ctx);

    Parenthesis::LayoutData* ldata = item->mutldata();
    ldata->setPos(PointF());
    ldata->reset();
    ldata->path.reset();

    const Staff* staff = item->staff();
    const Segment* seg = item->segment();
    const bool isClefSeg = seg->isType(SegmentType::ClefType);
    const Fraction tick = item->tick();
    const Fraction tickPrev = tick - Fraction::eps();
    const double spatium = item->spatium();
    const double mag = item->mag();
    const bool leftBracket = item->direction() == DirectionH::LEFT;

    const StaffType* st = staff->staffType(tick);
    const StaffType* stPrev = !tickPrev.negative() ? item->staff()->staffType(tickPrev) : nullptr;

    double startY = ldata->startY;
    double height = ldata->height;

    if (isClefSeg && seg->rtick() == seg->measure()->ticks()) {
        double offset = st->yoffset().val() - (stPrev ? stPrev->yoffset().val() : 0);
        startY += offset * spatium;
    }

    const double heightInSpatium = height / spatium;
    const double shoulderYOffset = 0.2 * height;
    const double thickness = height / 60 * mag; // 0.1sp for a height of 6sp
    ldata->thickness.set_value(thickness);
    const double shoulderX = 0.2 * height * mag;

    PointF start = PointF(0.0, startY);
    const PointF end = PointF(0.0, start.y() + height);
    const PointF endNormalised = end - start;

    const int direction = leftBracket ? -1 : 1;
    const double shoulderForX = direction * shoulderX + thickness * direction;
    const double shoulderBackX = direction * shoulderX + thickness * direction * -1;

    const PointF bezier1for = PointF(shoulderForX, shoulderYOffset);
    const PointF bezier2for = PointF(shoulderForX, endNormalised.y() - shoulderYOffset);
    const PointF bezier1back = PointF(shoulderBackX, endNormalised.y() - shoulderYOffset);
    const PointF bezier2back = PointF(shoulderBackX, shoulderYOffset);

    PainterPath path = PainterPath();
    path.moveTo(PointF());
    path.cubicTo(bezier1for, bezier2for, endNormalised);
    path.cubicTo(bezier1back, bezier2back, PointF());

    ldata->path = path;

    // Fill shape
    Shape shape(Shape::Type::Composite);

    PointF startPoint = PointF();
    double midThickness = 2 * thickness;
    int nbShapes = round(5.0 * heightInSpatium);
    nbShapes = std::clamp(nbShapes, 20, 50);
    PointF bezier1mid = bezier1for - PointF(thickness * direction, 0.0);
    PointF bezier2mid = bezier2for - PointF(thickness * direction, 0.0);
    const CubicBezier b(startPoint, bezier1mid, bezier2mid, endNormalised);
    for (int i = 1; i <= nbShapes; i++) {
        double percent = pow(sin(0.5 * M_PI * (double(i) / double(nbShapes))), 2);
        const PointF point = b.pointAtPercent(percent);
        RectF re = RectF(startPoint, point).normalized();
        double approxThicknessAtPercent = (1 - 2 * std::abs(0.5 - percent)) * midThickness;
        if (re.width() < approxThicknessAtPercent) {
            double adjust = (approxThicknessAtPercent - re.width()) * .5;
            re.adjust(-adjust, 0.0, adjust, 0.0);
        }
        shape.add(re, item);
        startPoint = point;
    }

    item->mutldata()->setShape(shape);

    item->setPos(start);
}

void TLayout::layoutPedal(Pedal* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    layoutLine(item, ctx);
}

void TLayout::layoutPedalSegment(PedalSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    PedalSegment::LayoutData* ldata = item->mutldata();

    layoutTextLineBaseSegment(item, ctx);
    if (item->isStyled(Pid::OFFSET)) {
        item->roffset() = item->pedal()->propertyDefault(Pid::OFFSET).value<PointF>();
    }

    Text* endText = item->endText();
    if (endText && !endText->empty()) { // Rosette
        PointF endPoint = item->pos2();
        double endTextWidth = endText->ldata()->bbox().width();
        double xEndText = endPoint.x() - endTextWidth;
        if (const Text* startText = item->text()) {
            xEndText = std::max(xEndText, startText->ldata()->bbox().width());
        }
        endText->mutldata()->setPosX(xEndText);

        double lineTextGap = item->getProperty(Pid::GAP_BETWEEN_TEXT_AND_LINE).toDouble() * item->spatium();
        PointF& endOfLine = item->pointsRef()[1];
        endOfLine.setX(xEndText - lineTextGap);
    }

    Shape sh = textLineBaseSegmentShape(item);
    ldata->setShape(sh);

    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layoutPickScrapeSegment(PickScrapeSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    PickScrapeSegment::LayoutData* ldata = item->mutldata();
    layoutTextLineBaseSegment(item, ctx);
    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layoutPlayTechAnnotation(const PlayTechAnnotation* item, PlayTechAnnotation::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    layoutBaseTextBase(item, ldata);

    if (item->autoplace()) {
        const Segment* s = toSegment(item->explicitParent());
        const Measure* m = s->measure();
        LD_CONDITION(ldata->isSetPos());
        LD_CONDITION(m->ldata()->isSetPos());
        LD_CONDITION(s->ldata()->isSetPos());
    }

    Autoplace::autoplaceSegmentElement(item, ldata);
}

void TLayout::layoutRasgueadoSegment(RasgueadoSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    RasgueadoSegment::LayoutData* ldata = item->mutldata();
    const StaffType* stType = item->staffType();

    if (stType && stType->isHiddenElementOnTab(ctx.conf().style(), Sid::rasgueadoShowTabCommon, Sid::rasgueadoShowTabSimple)) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    layoutTextLineBaseSegment(item, ctx);

    Shape sh = textLineBaseSegmentShape(item);
    ldata->setShape(sh);

    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layoutRehearsalMark(const RehearsalMark* item, RehearsalMark::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    layoutBaseTextBase(item, ldata);

    const Segment* s = item->segment();

    LD_CONDITION(s->ldata()->isSetPos());

    if (!s->rtick().isZero()) {
        return;
    }

    // first CR of measure, alignment is hcenter or right (the usual cases)
    // align with barline, point just after header, or start of measure depending on context
    const Measure* m = s->measure();
    const Segment* header = s->prev();                // possibly just a start repeat
    while (header && header->isType(Segment::CHORD_REST_OR_TIME_TICK_TYPE)) {
        header = header->prev();
    }
    double measureX = -s->x();
    const Segment* repeat = m->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));

    if (repeat) {
        LD_CONDITION(repeat->ldata()->isSetPos());
    }

    double barlineX = repeat ? repeat->x() - s->x() : measureX;
    const System* sys = m->system();
    bool systemFirst = (sys && m->isFirstInSystem());

    if (!header || repeat || !systemFirst) {
        // no header, or header with repeat, or header mid-system - align with barline
        ldata->setPosX(barlineX);
        checkRehearsalMarkVSBigTimeSig(item, ldata);
        return;
    }

    // header at start of system
    // align to a point just after the header
    EngravingItem* e = header->element(item->track());

    if (e) {
        LD_CONDITION(e->ldata()->isSetBbox());
    }

    LD_CONDITION(header->ldata()->isSetBbox());

    double w = e ? e->ldata()->bbox().width() : header->ldata()->bbox().width();
    ldata->setPosX(header->x() + w - s->x());

    // special case for right aligned rehearsal marks at start of system
    // left align with start of measure if that is further left
    if (item->align() == AlignH::RIGHT) {
        ldata->setPosX(std::min(ldata->pos().x(), measureX + ldata->bbox().width()));
    }

    checkRehearsalMarkVSBigTimeSig(item, ldata);
}

void TLayout::checkRehearsalMarkVSBigTimeSig(const RehearsalMark* item, TextBase::LayoutData* ldata)
{
    if (item->score()->style().styleV(Sid::timeSigPlacement).value<TimeSigPlacement>() != TimeSigPlacement::ABOVE_STAVES) {
        return;
    }

    const Segment* s = item->segment();
    TimeSig* bigTimeSig = nullptr;
    for (const Segment* segment = s; segment && segment->tick() == s->tick(); segment = segment->prevActive()) {
        if (!segment->isType(SegmentType::TimeSigType)) {
            continue;
        }
        TimeSig* timeSig = toTimeSig(segment->element(item->track()));
        if (timeSig && timeSig->visible()) {
            bigTimeSig = timeSig;
            break;
        }
    }

    if (!bigTimeSig) {
        return;
    }

    double timeSigRightEdge = bigTimeSig->ldata()->bbox().translated(bigTimeSig->ldata()->pos()).right()
                              + bigTimeSig->segment()->x() - s->x();
    double rehMarkLeftEdge = ldata->bbox().translated(item->ldata()->pos()).left();
    double distance = rehMarkLeftEdge - timeSigRightEdge;
    const double margin = 0.5 * item->fontMetrics().xHeight();
    if (distance < margin) {
        ldata->moveX(margin - distance);
    }
}

void TLayout::layoutRest(const Rest* item, Rest::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    if (item->isGap()) {
        return;
    }

    //! NOTE The types are listed here explicitly to show what types there are (see Rest::add method)
    //! and accordingly show what depends on.
    for (EngravingItem* e : item->el()) {
        switch (e->type()) {
        case ElementType::DEAD_SLAPPED: {
            DeadSlapped* ds = item_cast<DeadSlapped*>(e);
            LD_INDEPENDENT;
            layoutDeadSlapped(ds, ds->mutldata());
        } break;
        case ElementType::SYMBOL: {
            Symbol* s = item_cast<Symbol*>(e);
            // LD_X not clear yet
            layoutSymbol(s, s->mutldata(), ctx);
        } break;
        case ElementType::IMAGE: {
            Image* im = item_cast<Image*>(e);
            LD_INDEPENDENT;
            layoutImage(im, im->mutldata());
        } break;
        default:
            UNREACHABLE;
        }
    }

    if (item->deadSlapped()) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    double spatium = item->spatium();

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
            TLayout::layoutTabDurationSymbol(item->tabDur(), item->tabDur()->mutldata());
            ldata->setBbox(item->tabDur()->ldata()->bbox());
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
    int userLine   = RealIsNull(yOff) ? 0 : lrint(yOff / (lineDist * spatium));
    int lines      = st ? st->lines() : 5;

    int naturalLine = item->computeNaturalLine(lines); // Measured in 1sp steps
    int voiceOffset = item->computeVoiceOffset(lines, ldata); // Measured in 1sp steps
    int wholeRestOffset = item->computeWholeOrBreveRestOffset(voiceOffset, lines);
    int finalLine = naturalLine + voiceOffset + wholeRestOffset;

    ldata->sym = item->getSymbol(item->durationType().type(), finalLine + userLine, lines);

    ldata->setPosY(finalLine * lineDist * spatium);
    if (!item->shouldNotBeDrawn()) {
        ChordLayout::fillShape(item, ldata, ctx.conf());
    }

    auto layoutRestDots = [](const Rest* item, const LayoutConfiguration& conf, Rest::LayoutData* ldata)
    {
        const_cast<Rest*>(item)->checkDots();
        double visibleX = item->symWidthNoLedgerLines(ldata) + conf.styleMM(Sid::dotNoteDistance) * item->mag();
        double visibleDX = conf.styleMM(Sid::dotDotDistance) * item->mag();
        double invisibleX = item->symWidthNoLedgerLines(ldata);
        double y = item->dotLine() * item->spatium() * .5;
        for (NoteDot* dot : item->dotList()) {
            NoteDot::LayoutData* dotldata = dot->mutldata();
            TLayout::layoutNoteDot(dot, dotldata);
            if (dot->visible()) {
                dotldata->setPos(visibleX, y);
                visibleX += visibleDX;
            } else {
                invisibleX +=  0.1 * item->spatium();
                dotldata->setPos(invisibleX, y);
                invisibleX += item->symWidth(SymId::augmentationDot) * dot->mag();
            }
        }
    };

    layoutRestDots(item, ctx.conf(), ldata);
}

void TLayout::layoutShadowNote(ShadowNote* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    if (!item->isValid()) {
        item->setbbox(RectF());
        return;
    }

    double _spatium = item->spatium();
    double mag = item->mag();
    RectF noteheadBbox = item->symBbox(item->noteheadSymbol());
    bool up = item->computeUp();

    // Layout dots
    double dotWidth = 0.0;
    if (item->duration().dots() > 0) {
        double noteheadWidth = noteheadBbox.width();
        double d  = ctx.conf().styleMM(Sid::dotNoteDistance) * mag;
        double dd = ctx.conf().styleMM(Sid::dotDotDistance) * mag;
        dotWidth = (noteheadWidth + d);
        if (item->hasFlag() && up) {
            dotWidth = std::max(dotWidth, noteheadWidth + item->symBbox(item->flagSym()).right());
        }
        for (int i = 0; i < item->duration().dots(); i++) {
            dotWidth += dd * i;
        }
    }

    RectF newBbox;
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

    int lineIdx = item->lineIndex();

    // Layout ledger lines if needed
    if (item->ledgerLinesVisible()) {
        double extraLen = ctx.conf().styleMM(Sid::ledgerLineLength) * mag;
        double step = 0.5 * _spatium * item->staffType()->lineDistance().val();
        double yOffset = item->staffOffsetY();
        double x = noteheadBbox.x() - extraLen;
        double w = noteheadBbox.width() + 2 * extraLen;

        double lw = ctx.conf().styleMM(Sid::ledgerLineWidth);

        RectF r(x, -lw * .5, w, lw);
        const int topLine = -2 + yOffset / step;
        for (int i = topLine; i >= lineIdx; i -= 2) {
            newBbox |= r.translated(PointF(0, step * (i - lineIdx)));
        }
        int l = item->staffType()->lines() * 2 + yOffset / step; // first ledger line below staff
        for (int i = l; i <= lineIdx; i += 2) {
            newBbox |= r.translated(PointF(0, step * (i - lineIdx)));
        }
    }

    // Layout accidental
    SymId acc = Accidental::subtype2symbol(item->accidentalType());
    if (acc != SymId::noSym) {
        RectF symRect = item->symBbox(acc);
        double accWidth = symRect.width() + ctx.conf().styleMM(Sid::accidentalNoteDistance) * mag;
        double dh = 0.0;

        if (symRect.y() < newBbox.y()) {
            dh = symRect.height() - (newBbox.y() - symRect.y());
            newBbox.setY(symRect.y());
        } else if (symRect.y() > newBbox.y()) {
            dh = newBbox.height() - (symRect.y() - newBbox.y());
        }

        newBbox.setX(newBbox.x() - accWidth);
        newBbox.setWidth(newBbox.width() + accWidth);
        newBbox.setHeight(newBbox.height() + dh);
    }

    const std::set<SymId>& articulationIds = item->articulationIds();
    if (articulationIds.empty()) {
        item->setbbox(newBbox);
        return;
    }

    // Layout articulations
    double articulationsTop = -_spatium * .5 * lineIdx + item->segmentSkylineTopY();
    RectF rectWithArticulations = RectF(PointF(newBbox.x(), articulationsTop), newBbox.bottomRight());

    for (const SymId& artic: item->articulationIds()) {
        bool isMarcato = Articulation::symId2ArticulationName(artic).contains(u"marcato");
        bool isLv = artic == SymId::articLaissezVibrerAbove || artic == SymId::articLaissezVibrerBelow;
        double symH = item->symHeight(artic);

        if (!up || isMarcato) {
            double topY = rectWithArticulations.y();
            if (topY > 0.0) {
                topY = 0.0;
            }

            rectWithArticulations.setTop(topY - symH - _spatium);
        } else {
            rectWithArticulations.setHeight(rectWithArticulations.height() + symH + _spatium);
        }
        if (isLv) {
            const double width = item->symWidth(artic) - item->symWidth(item->noteheadSymbol()) / 2;
            rectWithArticulations.setWidth(rectWithArticulations.width() + width);
        }
    }

    newBbox.unite(rectWithArticulations);

    item->setbbox(newBbox);
}

void TLayout::layoutLine(SLine* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    item->computeStartElement();
    item->computeEndElement();

    System* s1;
    System* s2;
    PointF p1(item->linePos(Grip::START, &s1));
    PointF p2(item->linePos(Grip::END,   &s2));

    const std::vector<System*>& systems = ctx.dom().systems();
    system_idx_t sysIdx1 = muse::indexOf(systems, s1);
    system_idx_t sysIdx2 = muse::indexOf(systems, s2);
    int segmentsNeeded = 0;

    if (sysIdx1 == muse::nidx || sysIdx2 == muse::nidx) {
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
        layoutLineSegment(lineSegm, ctx);
    }
}

void TLayout::layoutNoteAnchoredLine(SLine* item, EngravingItem::LayoutData* ldata, LayoutContext& ctx)
{
    double _spatium = item->spatium();
    Note* startAnchor = toNote(item->startElement());
    Note* endAnchor = toNote(item->endElement());
    Chord* startChord = startAnchor->chord();
    Chord* endChord = endAnchor->chord();
    LineSegment* startSeg = toLineSegment(item->frontSegment());
    LineSegment* endSeg = toLineSegment(item->backSegment());

    // Note: line segments are defined by
    // initial point: ipos() (relative to system origin)
    // ending point:  pos2() (relative to initial point)

    // LINE ENDING POINTS TO NOTEHEAD CENTRES

    // assume line goes from centre of initial note centre to centre of ending note:
    // move first segment origin and last segment ending point from notehead origin to notehead centre
    // For TAB: begin at the right-edge of initial note rather than centre
    PointF startOffset = (startChord->staff()->isTabStaff(startChord->tick()))
                         ? PointF(startAnchor->ldata()->bbox().right(), 0.0)
                         : PointF(startAnchor->headWidth() * 0.5, 0.0);

    PointF endOffset = PointF(endAnchor->headWidth() * 0.5, 0.0);

    // AVOID HORIZONTAL LINES

    // for microtonality read tuning, or check note accidental
    double startTune = startAnchor->tuning();
    double endTune = endAnchor->tuning();
    AccidentalType startAcc = startAnchor->accidentalType();
    AccidentalType endAcc = endAnchor->accidentalType();
    if (muse::RealIsNull(startTune) && Accidental::isMicrotonal(startAcc)) {
        startTune = Accidental::subtype2centOffset(startAcc);
    }
    if (muse::RealIsNull(endTune) && Accidental::isMicrotonal(endAcc)) {
        endTune = Accidental::subtype2centOffset(endAcc);
    }

    int upDown = (0 < (endAnchor->ppitch() - startAnchor->ppitch())) - ((endAnchor->ppitch() - startAnchor->ppitch()) < 0);
    // same note, so compare tunings
    if (upDown == 0) {
        upDown = (0 < (endTune - startTune)) - ((endTune - startTune) < 0);
    }

    // on TAB's, glissando are by necessity on the same string, this gives an horizontal glissando line;
    // make bottom end point lower and top ending point higher
    if (startChord->staff()->isTabStaff(startChord->tick())) {
        double yOff = startChord->staff()->lineDistance(startChord->tick()) * 0.4 * _spatium;
        startOffset.ry() += yOff * upDown;
        endOffset.ry() -= yOff * upDown;
    }
    // if not TAB, angle glissando between notes on the same line
    else {
        if (startAnchor->line() == endAnchor->line()) {
            startOffset.ry() += _spatium * 0.25 * upDown;
            endOffset.ry() -= _spatium * 0.25 * upDown;
        }
    }

    // move initial point of first segment and adjust its length accordingly
    startSeg->setPos(startSeg->ldata()->pos() + startOffset);
    startSeg->setPos2(startSeg->ipos2() - startOffset);
    // adjust ending point of last segment
    endSeg->setPos2(endSeg->ipos2() + endOffset);

    // INTERPOLATION OF INTERMEDIATE POINTS

    // get total x-width and total y-height of all segments
    double xTot = 0.0;
    for (SpannerSegment* segm : item->spannerSegments()) {
        xTot += segm->ipos2().x();
    }
    double startY   = startSeg->ldata()->pos().y();
    double yTot = endSeg->ldata()->pos().y() + endSeg->ipos2().y() - startY;
    yTot -= yStaffDifference(endSeg->system(), startSeg->system(), track2staff(item->track2()));
    double ratio = muse::divide(yTot, xTot, 1.0);
    // interpolate y-coord of intermediate points across total width and height
    double xCurr = 0.0;
    double yCurr;
    for (unsigned i = 0; i + 1 < item->spannerSegments().size(); i++) {
        SpannerSegment* segm = item->segmentAt(i);
        xCurr += segm->ipos2().x();
        yCurr = startY + ratio * xCurr;
        segm->rypos2() = yCurr - segm->ldata()->pos().y();           // position segm. end point at yCurr
        // next segment shall start where this segment stopped
        SpannerSegment* nextSeg = item->segmentAt(i + 1);
        yCurr += yStaffDifference(nextSeg->system(), segm->system(), track2staff(item->track2()));
        segm = nextSeg;
        segm->rypos2() += segm->ldata()->pos().y() - yCurr;          // adjust next segm. vertical length
        segm->mutldata()->setPosY(yCurr);                                // position next segm. start point at yCurr
    }

    // KEEP CLEAR OF ALL ELEMENTS OF THE CHORD
    // Remove offset already applied
    startOffset  *= -1.0;
    endOffset *= -1.0;
    // Look at chord shapes (but don't consider lyrics)
    Shape startCRShape = startChord->shape();
    startCRShape.remove_if([](ShapeElement& s) {
        if (!s.item() || s.item()->isLyrics()) {
            return true;
        } else {
            return false;
        }
    });

    double startYAbove = startAnchor->ldata()->pos().y() + startAnchor->ldata()->bbox().topRight().y();
    double startYBelow = startYAbove + startAnchor->ldata()->bbox().height();
    startOffset.rx() += startCRShape.rightMostEdgeAtHeight(startYAbove, startYBelow) - startAnchor->pos().x();
    if (!endChord->staff()->isTabStaff(endChord->tick())) {
        double endYAbove = endAnchor->ldata()->pos().y() + endAnchor->ldata()->bbox().topLeft().y();
        double endYBelow = endYAbove + endAnchor->ldata()->bbox().height();
        double noteMiddle = endYAbove + endAnchor->ldata()->bbox().height() / 2;
        if (upDown != 0) {
            int llWidth = ctx.conf().styleS(Sid::ledgerLineWidth).val() * _spatium;
            // Only check top/bottom half of note depending on line approach direction
            // to avoid clearing acidentals the line won't collide with
            endYAbove = upDown == 1 ? noteMiddle - llWidth : endYAbove;
            endYBelow = upDown == 1 ? endYBelow : noteMiddle + llWidth;
        }

        endOffset.rx() -= endAnchor->pos().x() - endChord->shape().leftMostEdgeAtHeight(endYAbove, endYBelow);
    }
    // Add note distance
    const double lineNoteDist = 0.25 * item->spatium(); // TODO: style
    startOffset.rx() += lineNoteDist;
    endOffset.rx() -= lineNoteDist;

    // apply offsets: shorten first segment by x1 (and proportionally y) and adjust its length accordingly
    startOffset.ry() = startSeg->ipos2().y() * muse::divide(startOffset.x(), startSeg->ipos2().x(), 1.0);
    startSeg->setPos(startSeg->ldata()->pos() + startOffset);
    startSeg->setPos2(startSeg->ipos2() - startOffset);
    // adjust last segment length by x2 (and proportionally y)
    endOffset.ry() = endSeg->ipos2().y() * muse::divide(endOffset.x(), endSeg->ipos2().x(), 1.0);
    endSeg->setPos2(endSeg->ipos2() + endOffset);

    for (SpannerSegment* segm : item->spannerSegments()) {
        TLayout::layoutItem(segm, ctx);
    }

    // compute line bbox as the bbox of the last segment, relative to the end anchor note
    PointF endAnchorPagePos = endAnchor->pagePos();
    PointF endSystemPagePos;
    IF_ASSERT_FAILED(endChord->segment()->system()) {
        endSystemPagePos = endSeg->pos();
    } else {
        endSystemPagePos = endChord->segment()->system()->pagePos();
    }

    PointF endAnchorSystPos = endAnchorPagePos - endSystemPagePos;
    RectF r = RectF(endAnchorSystPos - endSeg->pos(), endAnchorSystPos - endSeg->pos() - endSeg->pos2()).normalized();
    double lw = item->absoluteFromSpatium(item->lineWidth()) * .5;
    ldata->setBbox(r.adjusted(-lw, -lw, lw, lw));
}

void TLayout::layoutNoteLine(NoteLine* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    TLayout::layoutLine(item, ctx);

    if (item->lineEndPlacement() == NoteLineEndPlacement::OFFSET_ENDS) {
        layoutNoteAnchoredLine(item, item->mutldata(), ctx);
        ChordLayout::addLineAttachPoints(item);
    }
}

void TLayout::layoutNoteLineSegment(NoteLineSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);

    NoteLineSegment::LayoutData* ldata = item->mutldata();
    layoutTextLineBaseSegment(item, ctx);

    if (item->staff()) {
        ldata->setMag(item->staff()->staffMag(item->tick()));
    }
    Shape sh = textLineBaseSegmentShape(item);
    ldata->setShape(sh);
}

void TLayout::layoutSlur(Slur* item, LayoutContext& ctx)
{
    SlurTieLayout::createSlurSegments(item, ctx);
}

void TLayout::layoutSpacer(Spacer* item, LayoutContext&)
{
    LAYOUT_CALL_ITEM(item);
    Spacer::LayoutData* ldata = item->mutldata();

    double spatium = item->spatium();

    PainterPath path = PainterPath();
    double w = spatium;
    double b = w * .5;
    double h = item->explicitParent() ? item->absoluteGap() : std::min(item->gap(), Spatium(4.0)).toMM(spatium).val();       // limit length for palette

    switch (item->spacerType()) {
    case SpacerType::DOWN:
        path.lineTo(w, 0.0);
        path.moveTo(b, 0.0);
        path.lineTo(b, h);
        path.lineTo(0.0, h - b);
        path.moveTo(b, h);
        path.lineTo(w, h - b);
        break;
    case SpacerType::UP:
        path.moveTo(b, 0.0);
        path.lineTo(0.0, b);
        path.moveTo(b, 0.0);
        path.lineTo(w, b);
        path.moveTo(b, 0.0);
        path.lineTo(b, h);
        path.moveTo(0.0, h);
        path.lineTo(w, h);
        break;
    case SpacerType::FIXED:
        path.lineTo(w, 0.0);
        path.moveTo(b, 0.0);
        path.lineTo(b, h);
        path.moveTo(0.0, h);
        path.lineTo(w, h);
        break;
    }
    ldata->path = path;
    double lw = spatium * 0.4;
    RectF bb(0, 0, w, h);
    bb.adjust(-lw, -lw, lw, lw);
    ldata->setBbox(bb);

    item->setZ(0.0);
}

void TLayout::layoutSpanner(Spanner* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    layoutItem(item, ctx);
}

void TLayout::layoutStaffLines(StaffLines* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    layoutForWidth(item, item->measure()->width(), ctx);
}

void TLayout::layoutForWidth(StaffLines* item, double w, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    StaffLines::LayoutData* ldata = item->mutldata();
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
        item->setColor(item->configuration()->defaultColor());
    }
    item->setLw(ctx.conf().styleS(Sid::staffLineWidth).val() * _spatium);
    double x1 = item->pos().x();
    double x2 = x1 + w;
    double y  = item->pos().y();
    ldata->setBbox(x1, -item->lw() * .5 + y, w, (_lines - 1) * dist + item->lw());

    std::vector<LineF> ll;
    for (int i = 0; i < _lines; ++i) {
        ll.push_back(LineF(x1, y, x2, y));
        y += dist;
    }
    item->setLines(ll);

    if (s && s->staffType(item->measure()->tick())->isTabStaff()) {
        MaskLayout::maskTABStringLinesForFrets(item, ctx);
    }
}

void TLayout::layoutStaffState(const StaffState* item, StaffState::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    if (ldata->isValid()) {
        return;
    }

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

void TLayout::layoutStaffText(const StaffText* item, StaffText::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    layoutBaseTextBase(item, ldata);

    if (item->autoplace()) {
        const Segment* s = toSegment(item->explicitParent());
        const Measure* m = s->measure();
        LD_CONDITION(ldata->isSetPos());
        LD_CONDITION(m->ldata()->isSetPos());
        LD_CONDITION(s->ldata()->isSetPos());
    }

    Autoplace::autoplaceSegmentElement(item, ldata);

    if (SoundFlag* flag = item->soundFlag()) {
        layoutSoundFlag(flag, flag->mutldata());
    }
}

void TLayout::layoutStaffTypeChange(const StaffTypeChange* item, StaffTypeChange::LayoutData* ldata, const LayoutConfiguration& conf)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    double spatium = conf.spatium();
    ldata->setBbox(RectF(-item->lw() * .5, -item->lw() * .5, spatium * 2.5 + item->lw(), spatium * 2.5 + item->lw()));
    if (item->measure()) {
        double y = -1.5 * spatium - ldata->bbox().height() + item->measure()->system()->staff(item->staffIdx())->y();
        ldata->setPos(spatium * .8, y);
    } else {
        ldata->setPos(0.0, 0.0);
    }
}

void TLayout::layoutStem(const Stem* item, Stem::LayoutData* ldata, const LayoutConfiguration& conf)
{
    LAYOUT_CALL_ITEM(item);

    const bool up = item->up();
    const double _up = up ? -1.0 : 1.0;

    //Note* note = up ? item->chord()->downNote() : item->chord()->upNote();
    //! NOTE A lot of spam
    // LD_CONDITION(note->ldata()->mirror.has_value());

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
            if ((up && !note->ldata()->mirror.value(LD_ACCESS::BAD)) || (!up && note->ldata()->mirror.value(LD_ACCESS::BAD))) {
                y1 = note->stemUpSE().y();
            } else {
                y1 = note->stemDownNW().y();
            }

            ldata->setPosY(note->ldata()->pos().y());
        }

        const Hook* hook = item->chord()->hook();
        const Beam* beam = item->chord()->beam();
        if (hook && !beam) {
            y2 += hook->smuflAnchor().y();
        }

        if (beam) {
            y2 -= _up * beam->absoluteFromSpatium(conf.styleS(Sid::beamWidth)) * .5 * beam->mag();
        }
    }

    double lineWidthCorrection = item->lineWidthMag() * 0.5;
    double lineX = isTabStaff ? 0.0 : _up * lineWidthCorrection;

    LineF line = LineF(lineX, y1, lineX, y2);
    ldata->line = line;

    // compute line and bounding rectangle
    RectF rect(line.p1(), line.p2() + PointF(0.0, ldata->beamCorrection));
    ldata->setBbox(rect.normalized().adjusted(-lineWidthCorrection, 0, lineWidthCorrection, 0));
}

void TLayout::layoutStemSlash(const StemSlash* item, StemSlash::LayoutData* ldata, const LayoutConfiguration& conf)
{
    LAYOUT_CALL_ITEM(item);
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    const Chord* c = item->chord();
    const Stem* stem = c->stem();
    const Hook* hook = c->hook();
    const Beam* beam = c->beam();

    LD_CONDITION(stem->ldata()->isSetPos());
    LD_CONDITION(stem->ldata()->isSetBbox());
    //  Notes without a stem (including whole notes) don't draw a slash.

    static constexpr double heightReduction = 0.66;
    static constexpr double angleIncrease = 1.2;
    static constexpr double lengthIncrease = 1.1;
    const double mag = c->mag();

    double up = c->up() ? -1 : 1;
    double stemTipY = c->up()
                      ? stem->ldata()->bbox().translated(stem->pos()).top()
                      : stem->ldata()->bbox().translated(stem->pos()).bottom();
    double leftHang = conf.noteHeadWidth() * mag / 2;
    double angle = conf.styleD(Sid::stemSlashAngle) * M_PI / 180; // converting to radians
    bool straight = conf.styleB(Sid::useStraightNoteFlags);
    double graceNoteMag = mag;

    double startX = stem->ldata()->bbox().translated(stem->pos()).right() - leftHang;
    double startY = stemTipY - up * graceNoteMag * conf.styleMM(Sid::stemSlashPosition)
                    * (straight || !hook ? heightReduction : 1);

    double endX = 0;
    double endY = 0;

    if (hook) {
        auto musicFont = conf.styleSt(Sid::musicalSymbolFont);
        // HACK: adjust slash angle for fonts with "fat" hooks. In future, we must use smufl cutOut
        if (c->beams() >= 2 && !straight
            && (musicFont == "Bravura" || musicFont == "Finale Maestro" || musicFont == "Gonville")) {
            angle *= angleIncrease;
        }
        endX = hook->ldata()->bbox().translated(hook->ldata()->pos()).right(); // always ends at the right bbox margin of the hook
        endY = startY + up * (endX - startX) * tan(angle);
    } else if (beam) {
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
    } else {
        double rightHang = (conf.noteHeadWidth() * mag / 2) - stem->width(); //  subtract the stem width so the slash is optically centered on the stem
        endX = stem->ldata()->bbox().translated(stem->pos()).right() + rightHang;
        endY = startY + up * (endX - startX) * tan(angle);
    }

    ldata->line = LineF(PointF(startX, startY), PointF(endX, endY));
    ldata->stemWidth = conf.styleMM(Sid::stemSlashThickness) * graceNoteMag;

    RectF bbox = RectF(ldata->line.p1(), ldata->line.p2()).normalized();
    bbox = bbox.adjusted(-ldata->stemWidth / 2, -ldata->stemWidth / 2, ldata->stemWidth, ldata->stemWidth);
    ldata->setBbox(bbox);
}

void TLayout::layoutSticking(const Sticking* item, Sticking::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    layoutBaseTextBase(item, ldata);

    AlignH itemAlign =  item->align().horizontal;
    if (itemAlign != AlignH::LEFT) {
        const Segment* seg = item->segment();
        const Chord* chord = nullptr;
        track_idx_t sTrack = trackZeroVoice(item->track());
        track_idx_t eTrack = sTrack + VOICES;
        for (track_idx_t track = sTrack; track < eTrack; ++track) {
            EngravingItem* el = seg->element(track);
            if (el && el->isChord()) {
                chord = toChord(el);
                break;
            }
        }

        if (chord) {
            const Note* refNote = item->placeAbove() ? chord->upNote() : chord->downNote();
            double noteWidth = refNote->ldata()->bbox().width();
            ldata->moveX(itemAlign == AlignH::HCENTER ? 0.5 * noteWidth : noteWidth);
        }
    }

    if (item->autoplace() && item->explicitParent()) {
        const Segment* s = toSegment(item->explicitParent());
        const Measure* m = s->measure();
        LD_CONDITION(ldata->isSetPos());
        LD_CONDITION(m->ldata()->isSetPos());
        LD_CONDITION(s->ldata()->isSetPos());
    }

    Autoplace::autoplaceSegmentElement(item, ldata);
}

void TLayout::layoutStringTunings(StringTunings* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    item->updateText();

    TLayout::layoutBaseTextBase(item, ctx);

    if (item->noStringVisible()) {
        double spatium = item->spatium();
        Font font(item->font());

        RectF rect;
        rect.setTopLeft({ 0, item->ldata()->bbox().y() - font.weight() - spatium * .15 });
        rect.setSize({ font.weight() - spatium, (font.weight() - spatium * .35) * 1.5 });

        item->setbbox(rect);
    }

    for (TextBlock& block : item->mutldata()->blocks) {
        for (TextFragment& fragment : block.fragments()) {
            Font font = fragment.font(item);
            if (font.type() == Font::Type::MusicSymbol) {
                // HACK: the music symbol doesn't have a good baseline
                // to go with text so we correct it here
                const double baselineAdjustment = 0.35 * font.pointSizeF();
                fragment.pos.setY(fragment.pos.y() + baselineAdjustment);
            }
        }
    }

    double secondStringXAlign = 0.0;
    for (const TextFragment& fragment : item->fragmentList()) {
        if (fragment.font(item).type() == Font::Type::MusicSymbol) {
            secondStringXAlign = std::max(secondStringXAlign, fragment.pos.x());
        }
    }

    for (TextBlock& block : item->mutldata()->blocks) {
        double xMove = 0.0;
        for (TextFragment& fragment : block.fragments()) {
            if (block.fragments().front() == fragment) {  // skip first
                continue;
            }

            if (fragment.font(item).type() == Font::Type::MusicSymbol) {
                xMove = secondStringXAlign - fragment.pos.x();
            }
            fragment.pos.setX(fragment.pos.x() + xMove);
        }
    }

    Segment* parentSegment = item->segment();
    item->move(PointF(-parentSegment->x() + item->spatium(), 0.0));

    Autoplace::autoplaceSegmentElement(item, item->mutldata());
}

void TLayout::layoutSoundFlag(const SoundFlag* item, SoundFlag::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);

    const Score* score = item->score();
    if (score && !score->showSoundFlags()) {
        return;
    }

    const EngravingItem* parent = toStaffText(item->parentItem());
    if (!parent) {
        return;
    }

    double spatium = item->spatium();
    double iconHeight = spatium * 3.0;

    RectF parentBbox = parent->ldata()->bbox();
    RectF iconBBox = RectF(parentBbox.x(), parentBbox.y(), iconHeight, iconHeight);

    iconBBox.moveCenter(parentBbox.center());

    // <icon><space><text>
    double space = spatium / 2.0;
    iconBBox.setX(parentBbox.x() - iconBBox.width() - space);

    ldata->setBbox(iconBBox);
}

void TLayout::layoutSymbol(const Symbol* item, Symbol::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    LD_INDEPENDENT;

    if (ldata->isValid()) {
        return;
    }

    if (item->parentItem()->isNote()) {
        double parenScale
            = (item->onTabStaff()
               && (item->sym() == SymId::noteheadParenthesisLeft || item->sym() == SymId::noteheadParenthesisRight)) ? 0.8 : 1;
        ldata->setMag(item->parentItem()->mag() * parenScale);
    } else if (item->staff()) {
        ldata->setMag(item->staff()->staffMag(item->tick()));
    }
    ldata->setBbox(item->scoreFont()
                   ? item->scoreFont()->bbox(item->sym(), item->magS() * item->symbolsSize())
                   : item->symBbox(item->sym()));
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

    // see BSymbol::add
    for (EngravingItem* e : item->leafs()) {
        switch (e->type()) {
        case ElementType::SYMBOL: {
            Symbol* s = item_cast<Symbol*>(e);
            layoutSymbol(s, s->mutldata(), ctx);
        } break;
        case ElementType::IMAGE: {
            Image* im = item_cast<Image*>(e);
            layoutImage(im, im->mutldata());
        } break;
        default:
            UNREACHABLE;
            break;
        }
    }
}

void TLayout::layoutFSymbol(const FSymbol* item, FSymbol::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;
    if (ldata->isValid()) {
        return;
    }

    ldata->setBbox(FontMetrics::boundingRect(item->font(), item->toString()));
}

void TLayout::layoutSystemDivider(const SystemDivider* item, SystemDivider::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;
    if (ldata->isValid()) {
        return;
    }

    layoutSymbol(item, ldata, ctx);
}

void TLayout::layoutSystemText(const SystemText* item, SystemText::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    layoutBaseTextBase(item, ldata);

    if (item->autoplace() && item->explicitParent()) {
        const Segment* s = toSegment(item->explicitParent());
        const Measure* m = s->measure();
        LD_CONDITION(ldata->isSetPos());
        LD_CONDITION(m->ldata()->isSetPos());
        LD_CONDITION(s->ldata()->isSetPos());
    }

    Autoplace::autoplaceSegmentElement(item, ldata);
}

void TLayout::layoutTabDurationSymbol(const TabDurationSymbol* item, TabDurationSymbol::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    static constexpr double TAB_RESTSYMBDISPL = 2.0;

    if (!item->tab()) {
        ldata->setBbox(RectF());
        return;
    }
    double spatium    = item->spatium();
    double hbb, wbb, xbb, ybb;     // bbox sizes
    double xpos, ypos;           // position coords

    ldata->beamGrid = TabBeamGrid::NONE;
    Chord* chord = item->explicitParent() && item->explicitParent()->isChord() ? toChord(item->explicitParent()) : nullptr;
// if no chord (shouldn't happens...) or not a special beam mode, layout regular symbol
    if (!chord || !chord->isChord()
        || (chord->beamMode() != BeamMode::BEGIN && chord->beamMode() != BeamMode::MID
            && chord->beamMode() != BeamMode::END)) {
        FontMetrics fm(item->tab()->durationFont());
        hbb   = item->tab()->durationBoxH();
        wbb   = fm.width(item->text());
        xbb   = 0.0;
        xpos  = 0.0;
        ypos  = item->tab()->durationFontYOffset();
        ybb   = item->tab()->durationBoxY() - ypos;
        // with rests, move symbol down by half its displacement from staff
        if (item->explicitParent() && item->explicitParent()->isRest()) {
            ybb  += TAB_RESTSYMBDISPL * spatium;
            ypos += TAB_RESTSYMBDISPL * spatium;
        }
    }
// if on a chord with special beam mode, layout an 'English'-style duration grid
    else {
        const TablatureDurationFont& font = item->tab()->tabDurationFont();
        hbb   = font.gridStemHeight * spatium;         // bbox height is stem height
        wbb   = font.gridStemWidth * spatium;          // bbox width is stem width
        xbb   = -wbb * 0.5;                             // bbox is half at left and half at right of stem centre
        ybb   = -hbb;                                   // bbox top is at top of stem height
        xpos  = 0.75 * spatium;                        // conventional centring of stem on fret marks
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

void TLayout::layoutTapping(Tapping* item, Tapping::LayoutData* ldata, LayoutContext& ctx)
{
    TappingLayout::layoutTapping(item, ldata, ctx);
}

void TLayout::layoutTappingHalfSlur(TappingHalfSlur* item)
{
    UNUSED(item)
}

void TLayout::layoutTempoText(const TempoText* item, TempoText::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    ldata->disconnectSnappedItems();

    layoutBaseTextBase(item, ldata);

    if (!item->autoplace()) {
        return;
    }

    // tempo text on first chordrest of measure should align over time sig if present, unless time sig is above staff
    Segment* s = item->segment();

    RehearsalMark* rehearsalMark = toRehearsalMark(s->findAnnotation(ElementType::REHEARSAL_MARK, item->track(), item->track()));
    RectF rehearsMarkBbox = rehearsalMark ? rehearsalMark->ldata()->bbox().translated(rehearsalMark->pos()) : RectF();
    RectF thisBbox = ldata->bbox().translated(item->pos());

    if (rehearsalMark && rehearsMarkBbox.bottom() > thisBbox.top()
        && item->getProperty(Pid::TEMPO_ALIGN_RIGHT_OF_REHEARSAL_MARK).toBool()) {
        double rightEdge = rehearsMarkBbox.right();
        const double padding = 0.5 * item->fontMetrics().xHeight();
        double curX = ldata->pos().x();
        ldata->setPosX(std::max(curX, rightEdge + padding));
    } else if (s->rtick().isZero()) {
        Segment* p = item->segment()->prev(SegmentType::TimeSig);
        if (p && !p->allElementsInvisible()) {
            ldata->moveX(-(s->x() - p->x()));
            EngravingItem* e = p->element(item->staffIdx() * VOICES);
            if (e) {
                ldata->moveX(p->hasTimeSigAboveStaves() ? e->x() + e->width() + e->spatium() : e->x());
            }
        }
    }
    Autoplace::autoplaceSegmentElement(item, ldata);
}

void TLayout::layoutTextBase(TextBase* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    layoutItem(item, ctx);
}

void TLayout::layoutBaseTextBase(const TextBase* item, TextBase::LayoutData* ldata)
{
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    ldata->setPos(PointF());

    if (item->placeBelow()) {
        ldata->setPosY(item->staff() ? item->staff()->staffHeight(item->tick()) : 0.0);
    }

    layoutBaseTextBase1(item, ldata);
}

void TLayout::layoutBaseTextBase(TextBase* item, LayoutContext&)
{
    layoutBaseTextBase(item, item->mutldata());
}

void TLayout::layoutBaseTextBase1(const TextBase* item, TextBase::LayoutData* ldata)
{
    if (item->explicitParent() && item->layoutToParentWidth()) {
        LD_CONDITION(item->parentItem()->ldata()->isSetBbox());
    }

    if (ldata->layoutInvalid) {
        item->createBlocks(ldata);
    }

    Shape shape;
    double y = 0.0;

    // adjust the bounding box for the text item
    for (size_t i = 0; i < ldata->blocks.size(); ++i) {
        TextBlock& t = ldata->blocks[i];
        t.layout(item);
        y += t.lineSpacing();
        t.setY(y);
        shape.add(t.shape().translated(PointF(0.0, y)));
    }

    RectF bb = shape.bbox();

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

    shape.translateY(yoff);
    ldata->setShape(shape);

    if (item->hasFrame()) {
        item->layoutFrame(ldata);
    }

    if (!item->isDynamic() && !(item->explicitParent() && item->parent()->isBox())) {
        computeTextHighResShape(item, ldata);
    }
}

void TLayout::computeTextHighResShape(const TextBase* item, TextBase::LayoutData* ldata)
{
    Shape& shape = ldata->highResShape.mut_value();
    shape.clear();
    shape.elements().reserve(item->xmlText().size());

    for (const TextBlock& block : ldata->blocks) {
        double y = block.y();
        for (const TextFragment& fragment : block.fragments()) {
            FontMetrics fontMetrics = FontMetrics(fragment.font(item));
            double x = fragment.pos.x();
            size_t textSize = fragment.text.size();
            for (size_t i = 0; i < textSize; ++i) {
                Char character = fragment.text.at(i);
                RectF characterBoundingRect = fontMetrics.tightBoundingRect(fragment.text.at(i));
                characterBoundingRect.translate(x, y);
                shape.add(characterBoundingRect);
                if (i + 1 < textSize) {
                    x += fontMetrics.horizontalAdvance(character);
                }
            }
        }
    }

    ldata->highResShape = shape;
}

void TLayout::layoutBaseTextBase1(TextBase* item, const LayoutContext&)
{
    layoutBaseTextBase1(item, item->mutldata());
}

Shape TLayout::textLineBaseSegmentShape(const TextLineBaseSegment* item)
{
    LAYOUT_CALL_ITEM(item);
    Shape shape;
    if (!item->text()->empty()) {
        shape.add(item->text()->ldata()->bbox().translated(item->text()->pos()), item->text());
    }
    if (!item->endText()->empty()) {
        shape.add(item->endText()->ldata()->bbox().translated(item->endText()->pos()), item->endText());
    }
    double lw2 = 0.5 * item->absoluteFromSpatium(item->lineWidth());
    bool isDottedLine = item->textLineBase()->lineStyle() == LineType::DOTTED;
    if (item->twoLines()) {     // hairpins
        shape.add(item->boundingBoxOfLine(item->points()[0], item->points()[1], lw2, isDottedLine), item);
        shape.add(item->boundingBoxOfLine(item->points()[2], item->points()[3], lw2, isDottedLine), item);
    } else {
        for (int i = 0; i < item->npoints() - 1; ++i) {
            shape.add(item->boundingBoxOfLine(item->points()[i], item->points()[i + 1], lw2, isDottedLine), item,
                      !item->textLineBase()->lineVisible());
        }
    }
    return shape;
}

void TLayout::layoutText(const Text* item, Text::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    if (item->explicitParent() && item->layoutToParentWidth()) {
        LD_CONDITION(item->parentItem()->ldata()->isSetBbox());
    }

    layoutBaseTextBase(item, ldata);
}

void TLayout::layoutTextLine(TextLine* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    layoutLine(item, ctx);
}

void TLayout::layoutTextLineSegment(TextLineSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    TextLineSegment::LayoutData* ldata = item->mutldata();
    layoutTextLineBaseSegment(item, ctx);
    if (item->isStyled(Pid::OFFSET)) {
        item->roffset() = item->textLine()->propertyDefault(Pid::OFFSET).value<PointF>();
    }

    Shape sh = textLineBaseSegmentShape(item);
    ldata->setShape(sh);

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
    TextLineBaseSegment::LayoutData* ldata = item->mutldata();
    item->npointsRef() = 0;
    TextLineBase* tl = item->textLineBase();
    const double _spatium = tl->spatium();
    const bool isSingleOrBegin = item->isSingleBeginType();

    if (item->spanner()->placeBelow()) {
        ldata->setPosY(item->staff() ? item->staff()->staffHeight(item->tick()) : 0.0);
    }

    // adjust Y pos to staffType offset
    ldata->moveY(item->staffOffsetY());

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

    if (isSingleOrBegin) {
        item->text()->setXmlText(tl->beginText());
        item->text()->setFamily(tl->beginFontFamily());
        item->text()->setSize(tl->beginFontSize());
        item->text()->setOffset(tl->beginTextOffset() * item->mag());
        item->text()->setAlign(tl->beginTextAlign());
        item->text()->setFontStyle(tl->beginFontStyle());
    } else {
        item->text()->setXmlText(tl->continueText());
        item->text()->setFamily(tl->continueFontFamily());
        item->text()->setSize(tl->continueFontSize());
        item->text()->setOffset(tl->continueTextOffset() * item->mag());
        item->text()->setAlign(tl->continueTextAlign());
        item->text()->setFontStyle(tl->continueFontStyle());
    }
    item->text()->setPlacement(PlacementV::ABOVE);
    item->text()->setTrack(item->track());
    item->text()->setColor(tl->lineColor());
    layoutText(item->text(), item->text()->mutldata());

    if ((item->isSingleType() || item->isEndType())) {
        item->endText()->setXmlText(tl->endText());
        item->endText()->setFamily(tl->endFontFamily());
        item->endText()->setSize(tl->endFontSize());
        item->endText()->setOffset(tl->endTextOffset());
        item->endText()->setAlign(tl->endTextAlign());
        item->endText()->setFontStyle(tl->endFontStyle());
        item->endText()->setPlacement(PlacementV::ABOVE);
        item->endText()->setTrack(item->track());
        item->endText()->setColor(tl->lineColor());
        layoutText(item->endText(), item->endText()->mutldata());
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
        && (!isSingleOrBegin || tl->beginHookType() == HookType::NONE)
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
    const double y0 = -tl->absoluteFromSpatium(tl->lineWidth());
    double y1 = std::min(0.0, pp2.y()) + y0;
    double y2 = std::max(0.0, pp2.y()) - y0;

    double l1 = 0.0;
    double l2 = 0.0;
    const double gapBetweenTextAndLine = _spatium * tl->gapBetweenTextAndLine().val();

    const bool alignBeginText = tl->beginTextPlace() == TextPlace::LEFT || tl->beginTextPlace() == TextPlace::AUTO;
    const bool alignContinueText = tl->continueTextPlace() == TextPlace::LEFT || tl->continueTextPlace() == TextPlace::AUTO;
    const bool alignEndText = tl->endTextPlace() == TextPlace::LEFT || tl->endTextPlace() == TextPlace::AUTO;
    const bool hasBeginText = !item->text()->empty() && isSingleOrBegin;
    const bool hasContinueText = !item->text()->empty() && !isSingleOrBegin;
    const bool hasEndText = !item->endText()->empty() && item->isSingleEndType();

    if (!item->text()->empty()) {
        if ((isSingleOrBegin && alignBeginText) || (!isSingleOrBegin && alignContinueText)) {
            l1 = gapBetweenTextAndLine;
            switch (item->text()->align().horizontal) {
            case AlignH::LEFT:
                l1 += item->text()->ldata()->bbox().width();
                break;
            case AlignH::HCENTER:
                l1 += item->text()->ldata()->bbox().width() / 2;
                break;
            default:
                break;
            }
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
        ldata->addBbox(item->text()->ldata()->bbox().translated(item->text()->pos()));      // DEBUG
    }
    // set end text position and extend bbox
    if (!item->endText()->empty()) {
        if (alignEndText) {
            l2 = gapBetweenTextAndLine;
            switch (item->endText()->align().horizontal) {
            case AlignH::RIGHT:
                l2 += item->endText()->ldata()->bbox().width();
                break;
            case AlignH::HCENTER:
                l2 += item->endText()->ldata()->bbox().width() / 2;
                break;
            default:
                break;
            }
        }
        item->endText()->mutldata()->moveX(ldata->bbox().right());
        ldata->addBbox(item->endText()->ldata()->bbox().translated(item->endText()->pos()));
    }

    if (tl->lineVisible() || !ctx.conf().isPrintingMode()) {
        pp1 = PointF(l1, 0.0);
        pp2.rx() -= l2;

        // Make sure baseline of text and line are properly aligned (accounting for line thickness)
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

        if (isSingleOrBegin && tl->beginHookType() != HookType::NONE) {
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
                    // For dashes lines, we extend the lines somewhat,
                    // so that the corner between them gets filled
                    bool checkAngle = tl->endHookType() == HookType::HOOK_45 || tl->diagonal();
                    extendLines(pp1, pp22, endHookStartpoint, endHookEndpoint, tl->absoluteFromSpatium(tl->lineWidth()), checkAngle);
                }
            }

            item->pointsRef()[item->npointsRef()++] = endHookEndpoint;
        }

        item->setLineLength(sqrt(PointF::dotProduct(pp22 - pp1, pp22 - pp1)));
    }
}

void TLayout::layoutTie(Tie* item, LayoutContext&)
{
    UNUSED(item);
}

void TLayout::layoutTimeSig(const TimeSig* item, TimeSig::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    ldata->setPos(0.0, 0.0);
    double spatium = item->spatium();

    ldata->setBbox(RectF());                    // prepare for an empty time signature

    ldata->pointLargeLeftParen = PointF();
    ldata->pz = PointF();
    ldata->pn = PointF();
    ldata->pointLargeRightParen = PointF();

    const MStyle& style = item->style();
    TimeSigPlacement timeSigPlacement = style.styleV(Sid::timeSigPlacement).value<TimeSigPlacement>();
    if (timeSigPlacement == TimeSigPlacement::NORMAL) {
        const_cast<TimeSig*>(item)->setSystemFlag(false);
    } else {
        const_cast<TimeSig*>(item)->setSystemFlag(true);
    }

    double lineDist = 0.0;
    int numOfLines = 0;
    TimeSigType sigType = item->timeSigType();
    const Staff* staff = item->staff();
    const Segment* seg = item->segment();
    const Measure* meas = seg ? seg->measure() : nullptr;
    const Fraction tick = meas ? meas->tick() : item->tick();

    if (staff) {
        // if staff is without time sig, format as if no text at all
        if (!staff->staffTypeForElement(item)->genTimesig()) {
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
        numOfLines  = staff->lines(tick);
        lineDist    = staff->lineDistance(tick);
    } else {
        // assume dimensions of a standard staff
        lineDist = 1.0;
        numOfLines = 5;
    }

    // if some symbol
    // compute vert. displacement to center in the staff height
    // determine middle staff position:

    double yoff = spatium * (numOfLines - 1) * .5 * lineDist;

    // C and Ccut are placed at the middle of the staff: use yoff directly
    IEngravingFontPtr font = ctx.engravingFont();
    ScaleF scale = item->scale();
    SizeF mag(item->magS() * scale);
    TimeSigStyle timeSigStyle = item->timeSigStyle();

    double numDist = item->numDist() * scale.height();

    Shape shape;
    if (sigType == TimeSigType::FOUR_FOUR) {
        ldata->pz = PointF(0.0, yoff);
        SymId sym = timeSigStyle == TimeSigStyle::LARGE ? SymId::timeSigCommonLarge
                    : timeSigStyle == TimeSigStyle::NARROW ? SymId::timeSigCommonNarrow
                    : SymId::timeSigCommon;
        RectF bbox = font->bbox(sym, mag);
        shape.add(bbox.translated(ldata->pz), item);
        ldata->ns.clear();
        ldata->ns.push_back(sym);
        ldata->ds.clear();
    } else if (sigType == TimeSigType::ALLA_BREVE) {
        ldata->pz = PointF(0.0, yoff);
        SymId sym = timeSigStyle == TimeSigStyle::LARGE ? SymId::timeSigCutCommonLarge
                    : timeSigStyle == TimeSigStyle::NARROW ? SymId::timeSigCutCommonNarrow
                    : SymId::timeSigCutCommon;
        RectF bbox = font->bbox(sym, mag);
        shape.add(bbox.translated(ldata->pz), item);
        ldata->ns.clear();
        ldata->ns.push_back(sym);
        ldata->ds.clear();
    } else if (sigType == TimeSigType::CUT_BACH) {
        ldata->pz = PointF(0.0, yoff);
        SymId sym = timeSigStyle == TimeSigStyle::LARGE ? SymId::timeSigCut2Large
                    : timeSigStyle == TimeSigStyle::NARROW ? SymId::timeSigCut2Narrow
                    : SymId::timeSigCut2;
        RectF bbox = font->bbox(sym, mag);
        shape.add(bbox.translated(ldata->pz), item);
        ldata->ns.clear();
        ldata->ns.push_back(sym);
        ldata->ds.clear();
    } else if (sigType == TimeSigType::CUT_TRIPLE) {
        ldata->pz = PointF(0.0, yoff);
        SymId sym = timeSigStyle == TimeSigStyle::LARGE ? SymId::timeSigCut3Large
                    : timeSigStyle == TimeSigStyle::NARROW ? SymId::timeSigCut3Narrow
                    : SymId::timeSigCut3;
        RectF bbox = font->bbox(sym, mag);
        shape.add(bbox.translated(ldata->pz), item);
        ldata->ns.clear();
        ldata->ns.push_back(sym);
        ldata->ds.clear();
    } else {
        if (item->numeratorString().isEmpty()) {
            ldata->ns = timeSigSymIdsFromString(item->numeratorString().isEmpty()
                                                ? String::number(item->sig().numerator())
                                                : item->numeratorString(),
                                                timeSigStyle);

            ldata->ds = timeSigSymIdsFromString(item->denominatorString().isEmpty()
                                                ? String::number(item->sig().denominator())
                                                : item->denominatorString(),
                                                timeSigStyle);
        } else {
            ldata->ns = timeSigSymIdsFromString(item->numeratorString(), timeSigStyle);
            ldata->ds = timeSigSymIdsFromString(item->denominatorString(), timeSigStyle);
        }

        RectF numRect = font->bbox(ldata->ns, mag);
        RectF denRect = font->bbox(ldata->ds, mag);

        double pzY = yoff - (denRect.width() < 0.01 ? 0.0 : 0.5 * (numRect.height() + numDist));
        double pnY = yoff + 0.5 * (denRect.height() + numDist);

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
        int centerY = yoff / 2 + spatium;
        int widestPortion = numRect.width() > denRect.width() ? numRect.width() : denRect.width();
        ldata->pointLargeLeftParen = PointF(-spatium, centerY);
        ldata->pointLargeRightParen = PointF(widestPortion + spatium, centerY);

        shape.add(numRect.translated(ldata->pz), item);       // translate bounding boxes to actual string positions
        shape.add(denRect.translated(ldata->pn), item);
        if (item->largeParentheses()) {
            shape.add(RectF(ldata->pointLargeLeftParen.x(), ldata->pointLargeLeftParen.y() - denRect.height(), spatium / 2,
                            numRect.height() + denRect.height()), item);
            shape.add(RectF(ldata->pointLargeRightParen.x(), ldata->pointLargeRightParen.y() - denRect.height(),  spatium / 2,
                            numRect.height() + denRect.height()), item);
        }
    }

    ldata->setShape(shape);

    ldata->setPosX(-shape.bbox().left());

    if (item->isAboveStaves()) {
        ldata->setPosY(-2 * spatium * (1 + scale.height()) - 0.5 * numDist);
    } else if (item->isAcrossStaves()) {
        double top = ldata->bbox().top();
        ldata->setPosY(-top);
    }

    ldata->moveY(item->yPos());
}

void TLayout::layoutTimeTickAnchor(TimeTickAnchor* item, LayoutContext&)
{
    TimeTickAnchor::LayoutData* ldata = item->mutldata();
    const StaffType* st = item->staffType();
    ldata->setPos(0.0, 0.0);
    double width = item->segment()->width();
    double height = item->staff()->staffHeight();
    if (st && st->lines() == 1) {
        // Special case for single line staves
        const double lineDist = st->lineDistance().val() * item->spatium();
        ldata->setPosY(-lineDist);
        height = lineDist * 2;
    }
    ldata->setBbox(0.0, 0.0, width, height);
}

void TLayout::layoutTremoloSingle(TremoloSingleChord* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    TremoloLayout::layout(item, ctx);
}

void TLayout::layoutTremoloTwo(TremoloTwoChord* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    TremoloLayout::layout(item, ctx);
}

void TLayout::layoutTremoloBar(const TremoloBar* item, TremoloBar::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    LD_INDEPENDENT;

    double spatium = item->spatium();

    ldata->setPos(0.0, -spatium * 3.0);

    /* we place the tremolo bars starting slightly before the
     *  notehead, and end it slightly after, drawing above the
     *  note. The values specified in Guitar Pro are very large, too
     *  large for the scale used in Musescore. We used the
     *  timeFactor and pitchFactor below to reduce these values down
     *  consistently to values that make sense to draw with the
     *  Musescore scale. */

    double timeFactor  = item->userMag() / 1.0;
    double pitchFactor = -spatium * .02;

    PolygonF polygon;
    for (const PitchValue& v : item->points()) {
        polygon << PointF(v.time * timeFactor, v.pitch * pitchFactor);
    }
    ldata->polygon = polygon;

    const double w = item->absoluteFromSpatium(item->lineWidth());
    ldata->setBbox(ldata->polygon.boundingRect().adjusted(-w, -w, w, w));
}

void TLayout::layoutTrillSegment(TrillSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    TrillSegment::LayoutData* ldata = item->mutldata();
    Trill* trill = item->trill();
    EngravingItem* startItem = trill->startElement();
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
        ldata->setPosY(item->staff() ? item->staff()->staffHeight(item->tick()) : 0.0);
    }
    const double yOff = item->staffOffsetY();
    ldata->moveY(yOff);

    bool accidentalGoesBelow = trill->trillType() == TrillType::DOWNPRALL_LINE;
    Ornament* ornament = trill->ornament();
    if (ornament) {
        if (item->isSingleBeginType()) {
            TLayout::layoutOrnament(ornament, ornament->mutldata(), ctx.conf());
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
        switch (trill->trillType()) {
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
        Accidental* a = trill->accidental();
        if (a) {
            double vertMargin = 0.35 * item->spatium();
            RectF box = item->symBbox(item->symbols().front());
            double x = 0;
            double y = 0;
            x = 0.5 * (box.width() - a->width());
            double minVertDist = accidentalGoesBelow
                                 ? Shape(box).minVerticalDistance(a->shape())
                                 : a->shape().minVerticalDistance(Shape(box));
            y = (accidentalGoesBelow ? minVertDist + vertMargin : -minVertDist - vertMargin) + yOff;
            a->setPos(x, y);
            a->setParent(item);
        }
    } else {
        switch (trill->trillType()) {
        case TrillType::TRILL_LINE:
        case TrillType::PRALLPRALL_LINE:
            item->symbolLine(SymId::wiggleTrill, SymId::wiggleTrill);
            break;
        case TrillType::UPPRALL_LINE:
        case TrillType::DOWNPRALL_LINE:
            item->symbolLine(SymId::ornamentZigZagLineNoRightEnd,
                             SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
            break;
        }
    }

    if (item->isStyled(Pid::OFFSET)) {
        item->roffset() = trill->propertyDefault(Pid::OFFSET).value<PointF>();
    }

    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::fillTrillSegmentShape(const TrillSegment* item, TrillSegment::LayoutData* ldata, const LayoutConfiguration& conf)
{
    LAYOUT_CALL_ITEM(item);
    std::shared_ptr<const IEngravingFont> font = conf.engravingFont();
    Shape s = font->shape(item->symbols(), item->magS());
    Accidental* accidental = item->trill()->accidental();
    if (accidental && accidental->visible() && item->isSingleBeginType()) {
        s.add(accidental->shape().translate(accidental->pos()));
    }

    ldata->setShape(s);
}

void TLayout::layoutTripletFeel(const TripletFeel* item, TripletFeel::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    layoutSystemText(item, ldata);
}

void TLayout::layoutTrill(Trill* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    layoutLine(static_cast<SLine*>(item), ctx);
}

void TLayout::layoutTuplet(Tuplet* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    TupletLayout::layout(item, ctx);
}

void TLayout::fillTupletShape(const Tuplet* item, Tuplet::LayoutData* ldata)
{
    LAYOUT_CALL_ITEM(item);
    Shape s;
    if (item->hasBracket()) {
        auto tupletRect = [](const PointF& p1, const PointF& p2, double height, const Tuplet* item) {
            Shape shape;
            double xLength = p2.x() - p1.x();
            int nSteps = 2 * std::ceil(xLength / item->spatium());
            double xStep = xLength / nSteps;
            double yStep = (p2.y() - p1.y()) / nSteps;
            double rectHeight = (item->isUp() ? height : -height) - yStep;
            double xCur = p1.x();
            double yCur = p1.y() + yStep;
            for (int i = 0; i < nSteps; ++i) {
                shape.add(RectF(xCur, yCur, xStep, rectHeight).normalized(), item);
                xCur += xStep;
                yCur += yStep;
            }
            return shape;
        };

        double height = item->style().styleMM(Sid::tupletBracketHookHeight);
        s.add(tupletRect(item->bracketL[0], item->bracketL[1], height, item));
        s.add(tupletRect(item->bracketL[1], item->bracketL[2], height, item));
        if (item->number()) {
            s.add(tupletRect(item->bracketR[0], item->bracketR[1], height, item));
            s.add(tupletRect(item->bracketR[1], item->bracketR[2], height, item));
        } else {
            s.add(tupletRect(item->bracketL[2], item->bracketL[3], height, item));
        }
    }
    if (item->number()) {
        s.add(item->number()->ldata()->bbox().translated(item->number()->pos()));
    }

    ldata->setShape(s);
}

void TLayout::layoutVibratoSegment(VibratoSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    VibratoSegment::LayoutData* ldata = item->mutldata();
    if (item->staff()) {
        ldata->setMag(item->staff()->staffMag(item->tick()));
    }
    if (item->spanner()->placeBelow()) {
        ldata->setPosY(item->staff() ? item->staff()->staffHeight(item->tick()) : 0.0);
    }

    ldata->moveY(item->staffOffsetY());

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

void TLayout::layoutVibrato(Vibrato* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    layoutLine(static_cast<SLine*>(item), ctx);

    if (ctx.conf().isPaletteMode()) {
        return;
    }
    if (item->spannerSegments().empty()) {
        LOGD("Vibrato: no segments");
        return;
    }
}

void TLayout::layoutVolta(Volta* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    layoutLine(item, ctx);
}

void TLayout::layoutVoltaSegment(VoltaSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    VoltaSegment::LayoutData* ldata = item->mutldata();
    layoutTextLineBaseSegment(item, ctx);
    Shape sh = textLineBaseSegmentShape(item);
    ldata->setShape(sh);
    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

void TLayout::layoutWhammyBarSegment(WhammyBarSegment* item, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(item);
    WhammyBarSegment::LayoutData* ldata = item->mutldata();
    layoutTextLineBaseSegment(item, ctx);

    Shape sh = textLineBaseSegmentShape(item);
    ldata->setShape(sh);

    Autoplace::autoplaceSpannerSegment(item, ldata, ctx.conf().spatium());
}

using LayoutSystemTypes = rtti::TypeList<LyricsLine, Slur, HammerOnPullOff, TappingHalfSlur, Volta>;

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
    LAYOUT_CALL_ITEM(item);
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
        if (!ss->system() || ss->isTappingHalfSlurSegment()) {
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
    LAYOUT_CALL_ITEM(line);
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
        if (line->tick2() < etick) {
            sst = SpannerSegmentType::SINGLE;
        } else if (line->tick2() == etick) {
            if (line->isPedal() && toPedal(line)->connect45HookToNext()) {
                sst = SpannerSegmentType::BEGIN;
            } else {
                sst = SpannerSegmentType::SINGLE;
            }
        } else {
            sst = SpannerSegmentType::BEGIN;
        }
        if (line->hasVoiceAssignmentProperties()) {
            line->setPlacementBasedOnVoiceAssignment(ctx.conf().styleV(Sid::dynamicsHairpinVoiceBasedPlacement).value<DirectionV>());
        }
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
    lineSegm->setPlacement(line->placement());

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

    layoutLineSegment(lineSegm, ctx);

    return lineSegm;
}

SpannerSegment* TLayout::layoutSystem(LyricsLine* line, System* system, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(line);
    if (!line->isPartialLyricsLine() && !line->lyrics()) {
        return nullptr;
    }

    Fraction stick = system->firstMeasure()->tick();
    Fraction etick = system->lastMeasure()->endTick();

    LyricsLineSegment* lineSegm = toLyricsLineSegment(TLayout::getNextLayoutSystemSegment(line, system, [line](System* parent) {
        return line->createLineSegment(parent);
    }));

    SpannerSegmentType sst;
    if (line->tick() >= stick) {
        TLayout::layoutLyricsLine(line, ctx);
        if (line->ticks().isZero() && line->isEndMelisma()) {
            return nullptr;
        }
        line->computeStartElement();
        line->computeEndElement();
        sst = line->tick2() <= etick ? SpannerSegmentType::SINGLE : SpannerSegmentType::BEGIN;
    } else if (line->tick() < stick && line->tick2() > etick) {
        sst = SpannerSegmentType::MIDDLE;
    } else {
        sst = SpannerSegmentType::END;
    }
    lineSegm->setSpannerSegmentType(sst);

    TLayout::layoutLyricsLineSegment(lineSegm, ctx);
    if (!line->isPartialLyricsLine() && !line->lyrics()) {
        // this line could have been removed in the process of laying out surrounding lyrics
        return nullptr;
    }

    return lineSegm;
}

SpannerSegment* TLayout::layoutSystem(Volta* line, System* system, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(line);
    SpannerSegment* voltaSegment = layoutSystemSLine(line, system, ctx);

    // we need set tempo in layout because all tempos of score is set in layout
    // so fermata in seconda volta works correct because fermata apply itself tempo during layouting
    line->setTempo();

    return voltaSegment;
}

SpannerSegment* TLayout::layoutSystem(Slur* line, System* system, LayoutContext& ctx)
{
    LAYOUT_CALL_ITEM(line);
    return SlurTieLayout::layoutSystem(line, system, ctx);
}

// Called after layout of all systems is done so precise
// number of systems for this spanner becomes available.
void TLayout::layoutSystemsDone(Spanner* item)
{
    LAYOUT_CALL_ITEM(item);
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
