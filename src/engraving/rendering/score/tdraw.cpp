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
#include "tdraw.h"

#include "draw/fontmetrics.h"
#include "draw/svgrenderer.h"

#include "types/typesconv.h"
#include "style/style.h"
#include "style/defaultstyle.h"

#include "dom/accidental.h"
#include "dom/actionicon.h"
#include "dom/ambitus.h"
#include "dom/arpeggio.h"
#include "dom/articulation.h"

#include "dom/bagpembell.h"
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
#include "dom/guitarbend.h"

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

#include "dom/laissezvib.h"
#include "dom/lasso.h"
#include "dom/layoutbreak.h"
#include "dom/ledgerline.h"
#include "dom/letring.h"
#include "dom/lyrics.h"

#include "dom/marker.h"
#include "dom/measurenumber.h"
#include "dom/measurerepeat.h"
#include "dom/mmrest.h"
#include "dom/mmrestrange.h"

#include "dom/navigate.h"
#include "dom/note.h"
#include "dom/notedot.h"
#include "dom/noteline.h"

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

#include "dom/score.h"
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
#include "dom/stretchedbend.h"
#include "dom/symbol.h"
#include "dom/systemdivider.h"
#include "dom/systemtext.h"
#include "dom/systemlock.h"
#include "dom/soundflag.h"

#include "dom/tempotext.h"
#include "dom/text.h"
#include "dom/textbase.h"
#include "dom/textline.h"
#include "dom/textlinebase.h"
#include "dom/tie.h"
#include "dom/timesig.h"
#include "dom/anchors.h"
#include "dom/tremolosinglechord.h"
#include "dom/tremolotwochord.h"
#include "dom/tremolobar.h"
#include "dom/trill.h"
#include "dom/tripletfeel.h"
#include "dom/tuplet.h"

#include "dom/vibrato.h"
#include "dom/volta.h"

#include "dom/whammybar.h"

#include "dom/mscoreview.h"

#include "infrastructure/rtti.h"

// dev
#include "dom/system.h"
#include "dom/measure.h"
#include "dom/segment.h"
#include "dom/chord.h"

using namespace mu::engraving;
using namespace mu::engraving::rtti;
using namespace mu::engraving::rendering::score;
using namespace muse;
using namespace muse::draw;

void TDraw::drawItem(const EngravingItem* item, Painter* painter)
{
    switch (item->type()) {
    case ElementType::ACCIDENTAL:   draw(item_cast<const Accidental*>(item), painter);
        break;
    case ElementType::ACTION_ICON:  draw(item_cast<const ActionIcon*>(item), painter);
        break;
    case ElementType::AMBITUS:      draw(item_cast<const Ambitus*>(item), painter);
        break;
    case ElementType::ARPEGGIO:     draw(item_cast<const Arpeggio*>(item), painter);
        break;
    case ElementType::ARTICULATION: draw(item_cast<const Articulation*>(item), painter);
        break;

    case ElementType::BAGPIPE_EMBELLISHMENT: draw(item_cast<const BagpipeEmbellishment*>(item), painter);
        break;
    case ElementType::BAR_LINE:     draw(item_cast<const BarLine*>(item), painter);
        break;
    case ElementType::BEAM:         draw(item_cast<const Beam*>(item), painter);
        break;
    case ElementType::BEND:         draw(item_cast<const Bend*>(item), painter);
        break;
    case ElementType::HBOX:         draw(item_cast<const HBox*>(item), painter);
        break;
    case ElementType::VBOX:         draw(item_cast<const VBox*>(item), painter);
        break;
    case ElementType::FBOX:         draw(item_cast<const FBox*>(item), painter);
        break;
    case ElementType::TBOX:         draw(item_cast<const TBox*>(item), painter);
        break;
    case ElementType::BRACKET:      draw(item_cast<const Bracket*>(item), painter);
        break;
    case ElementType::BREATH:       draw(item_cast<const Breath*>(item), painter);
        break;

    case ElementType::CHORDLINE:    draw(item_cast<const ChordLine*>(item), painter);
        break;
    case ElementType::CLEF:         draw(item_cast<const Clef*>(item), painter);
        break;
    case ElementType::CAPO:         draw(item_cast<const Capo*>(item), painter);
        break;

    case ElementType::DEAD_SLAPPED: draw(item_cast<const DeadSlapped*>(item), painter);
        break;
    case ElementType::DYNAMIC:      draw(item_cast<const Dynamic*>(item), painter);
        break;

    case ElementType::EXPRESSION:   draw(item_cast<const Expression*>(item), painter);
        break;

    case ElementType::FERMATA:      draw(item_cast<const Fermata*>(item), painter);
        break;
    case ElementType::FIGURED_BASS: draw(item_cast<const FiguredBass*>(item), painter);
        break;
    case ElementType::FINGERING:    draw(item_cast<const Fingering*>(item), painter);
        break;
    case ElementType::FRET_DIAGRAM: draw(item_cast<const FretDiagram*>(item), painter);
        break;
    case ElementType::FRET_CIRCLE:  draw(item_cast<const FretCircle*>(item), painter);
        break;
    case ElementType::FSYMBOL:      draw(item_cast<const FSymbol*>(item), painter);
        break;

    case ElementType::GLISSANDO_SEGMENT: draw(item_cast<const GlissandoSegment*>(item), painter);
        break;
    case ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT: draw(item_cast<const GradualTempoChangeSegment*>(item), painter);
        break;
    case ElementType::GUITAR_BEND_SEGMENT: draw(item_cast<const GuitarBendSegment*>(item), painter);
        break;
    case ElementType::GUITAR_BEND_HOLD_SEGMENT: draw(item_cast<const GuitarBendHoldSegment*>(item), painter);
        break;
    case ElementType::GUITAR_BEND_TEXT: drawTextBase(toTextBase(item), painter);
        break;

    case ElementType::HAIRPIN_SEGMENT: draw(item_cast<const HairpinSegment*>(item), painter);
        break;
    case ElementType::HARP_DIAGRAM: draw(item_cast<const HarpPedalDiagram*>(item), painter);
        break;
    case ElementType::HARMONIC_MARK_SEGMENT: draw(item_cast<const HarmonicMarkSegment*>(item), painter);
        break;
    case ElementType::HARMONY:      draw(item_cast<const Harmony*>(item), painter);
        break;
    case ElementType::HOOK:         draw(item_cast<const Hook*>(item), painter);
        break;

    case ElementType::IMAGE:        draw(item_cast<const Image*>(item), painter);
        break;
    case ElementType::INSTRUMENT_CHANGE: draw(item_cast<const InstrumentChange*>(item), painter);
        break;
    case ElementType::INSTRUMENT_NAME: draw(item_cast<const InstrumentName*>(item), painter);
        break;

    case ElementType::JUMP:         draw(item_cast<const Jump*>(item), painter);
        break;

    case ElementType::KEYSIG:       draw(item_cast<const KeySig*>(item), painter);
        break;
    case ElementType::LAISSEZ_VIB_SEGMENT:  draw(item_cast<const LaissezVibSegment*>(item), painter);
        break;
    case ElementType::LASSO:        draw(item_cast<const Lasso*>(item), painter);
        break;
    case ElementType::LAYOUT_BREAK: draw(item_cast<const LayoutBreak*>(item), painter);
        break;
    case ElementType::LEDGER_LINE:  draw(item_cast<const LedgerLine*>(item), painter);
        break;
    case ElementType::LET_RING_SEGMENT: draw(item_cast<const LetRingSegment*>(item), painter);
        break;
    case ElementType::LYRICS:       draw(item_cast<const Lyrics*>(item), painter);
        break;
    case ElementType::LYRICSLINE_SEGMENT: draw(item_cast<const LyricsLineSegment*>(item), painter);
        break;

    case ElementType::MARKER:       draw(item_cast<const Marker*>(item), painter);
        break;
    case ElementType::MEASURE_NUMBER: draw(item_cast<const MeasureNumber*>(item), painter);
        break;
    case ElementType::MEASURE_REPEAT: draw(item_cast<const MeasureRepeat*>(item), painter);
        break;
    case ElementType::MMREST:       draw(item_cast<const MMRest*>(item), painter);
        break;
    case ElementType::MMREST_RANGE: draw(item_cast<const MMRestRange*>(item), painter);
        break;

    case ElementType::NOTE:         draw(item_cast<const Note*>(item), painter);
        break;
    case ElementType::NOTEDOT:      draw(item_cast<const NoteDot*>(item), painter);
        break;
    case ElementType::NOTEHEAD:     draw(item_cast<const NoteHead*>(item), painter);
        break;
    case ElementType::NOTELINE_SEGMENT: draw(item_cast<const NoteLineSegment*>(item), painter);
        break;

    case ElementType::ORNAMENT:     draw(item_cast<const Ornament*>(item), painter);
        break;
    case ElementType::OTTAVA_SEGMENT:       draw(item_cast<const OttavaSegment*>(item), painter);
        break;

    case ElementType::PAGE:                 draw(item_cast<const Page*>(item), painter);
        break;
    case ElementType::PALM_MUTE_SEGMENT:    draw(item_cast<const PalmMuteSegment*>(item), painter);
        break;
    case ElementType::PEDAL_SEGMENT:        draw(item_cast<const PedalSegment*>(item), painter);
        break;
    case ElementType::PICK_SCRAPE_SEGMENT:  draw(item_cast<const PickScrapeSegment*>(item), painter);
        break;
    case ElementType::PLAYTECH_ANNOTATION:  draw(item_cast<const PlayTechAnnotation*>(item), painter);
        break;

    case ElementType::RASGUEADO_SEGMENT:    draw(item_cast<const RasgueadoSegment*>(item), painter);
        break;
    case ElementType::REHEARSAL_MARK:       draw(item_cast<const RehearsalMark*>(item), painter);
        break;
    case ElementType::REST:                 draw(item_cast<const Rest*>(item), painter);
        break;

    case ElementType::SHADOW_NOTE:          draw(item_cast<const ShadowNote*>(item), painter);
        break;
    case ElementType::SLUR_SEGMENT:         draw(item_cast<const SlurSegment*>(item), painter);
        break;
    case ElementType::SPACER:               draw(item_cast<const Spacer*>(item), painter);
        break;
    case ElementType::STAFF_LINES:          draw(item_cast<const StaffLines*>(item), painter);
        break;
    case ElementType::STAFF_STATE:          draw(item_cast<const StaffState*>(item), painter);
        break;
    case ElementType::STAFF_TEXT:           draw(item_cast<const StaffText*>(item), painter);
        break;
    case ElementType::STAFFTYPE_CHANGE:     draw(item_cast<const StaffTypeChange*>(item), painter);
        break;
    case ElementType::STEM:                 draw(item_cast<const Stem*>(item), painter);
        break;
    case ElementType::STEM_SLASH:           draw(item_cast<const StemSlash*>(item), painter);
        break;
    case ElementType::STICKING:             draw(item_cast<const Sticking*>(item), painter);
        break;
    case ElementType::STRING_TUNINGS:       draw(item_cast<const StringTunings*>(item), painter);
        break;
    case ElementType::STRETCHED_BEND:       draw(item_cast<const StretchedBend*>(item), painter);
        break;
    case ElementType::SYMBOL:               draw(item_cast<const Symbol*>(item), painter);
        break;
    case ElementType::SYSTEM_DIVIDER:       draw(item_cast<const SystemDivider*>(item), painter);
        break;
    case ElementType::SYSTEM_TEXT:          draw(item_cast<const SystemText*>(item), painter);
        break;
    case ElementType::SYSTEM_LOCK_INDICATOR: draw(item_cast<const SystemLockIndicator*>(item), painter);
        break;
    case ElementType::SOUND_FLAG:           draw(item_cast<const SoundFlag*>(item), painter);
        break;

    case ElementType::TAB_DURATION_SYMBOL:  draw(item_cast<const TabDurationSymbol*>(item), painter);
        break;
    case ElementType::TEMPO_TEXT:           draw(item_cast<const TempoText*>(item), painter);
        break;
    case ElementType::TEXT:                 draw(item_cast<const Text*>(item), painter);
        break;
    case ElementType::TEXTLINE_SEGMENT:     draw(item_cast<const TextLineSegment*>(item), painter);
        break;
    case ElementType::TIE_SEGMENT:          draw(item_cast<const TieSegment*>(item), painter);
        break;
    case ElementType::TIMESIG:              draw(item_cast<const TimeSig*>(item), painter);
        break;
    case ElementType::TIME_TICK_ANCHOR:     draw(item_cast<const TimeTickAnchor*>(item), painter);
        break;
    case ElementType::TREMOLO_SINGLECHORD:  draw(item_cast<const TremoloSingleChord*>(item), painter);
        break;
    case ElementType::TREMOLO_TWOCHORD:     draw(item_cast<const TremoloTwoChord*>(item), painter);
        break;
    case ElementType::TREMOLOBAR:           draw(item_cast<const TremoloBar*>(item), painter);
        break;
    case ElementType::TRILL_SEGMENT:        draw(item_cast<const TrillSegment*>(item), painter);
        break;
    case ElementType::TRIPLET_FEEL:         draw(item_cast<const TripletFeel*>(item), painter);
        break;
    case ElementType::TUPLET:               draw(item_cast<const Tuplet*>(item), painter);
        break;

    case ElementType::VIBRATO_SEGMENT:      draw(item_cast<const VibratoSegment*>(item), painter);
        break;
    case ElementType::VOLTA_SEGMENT:        draw(item_cast<const VoltaSegment*>(item), painter);
        break;

    case ElementType::WHAMMY_BAR_SEGMENT:   draw(item_cast<const WhammyBarSegment*>(item), painter);
        break;

    // dev
    case ElementType::SYSTEM:               draw(item_cast<const System*>(item), painter);
        break;
    case ElementType::MEASURE:              draw(item_cast<const Measure*>(item), painter);
        break;
    case ElementType::SEGMENT:              draw(item_cast<const Segment*>(item), painter);
        break;
    case ElementType::CHORD:                draw(item_cast<const Chord*>(item), painter);
        break;
    case ElementType::GRACE_NOTES_GROUP:
        break;
    default:
        NOT_IMPLEMENTED << " type: " << item->typeName();
        UNREACHABLE;
    }
}

void TDraw::draw(const Accidental* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    IF_ASSERT_FAILED(item->ldata()) {
        return;
    }

    painter->setPen(item->curColor());
    for (const Accidental::LayoutData::Sym& e : item->ldata()->syms) {
        item->drawSymbol(e.sym, painter, PointF(e.x, e.y));
    }
}

void TDraw::draw(const ActionIcon* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    const ActionIcon::LayoutData* ldata = item->ldata();
    painter->setFont(item->iconFont());
    painter->drawText(ldata->bbox(), muse::draw::AlignCenter, Char(item->icon()));
}

void TDraw::draw(const Ambitus* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    const Ambitus::LayoutData* ldata = item->ldata();
    IF_ASSERT_FAILED(ldata) {
        return;
    }

    double spatium = item->spatium();
    double lw = item->lineWidth().val() * spatium;
    painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));

    item->drawSymbol(item->noteHead(), painter, ldata->topPos);
    item->drawSymbol(item->noteHead(), painter, ldata->bottomPos);
    if (item->hasLine()) {
        painter->drawLine(ldata->line);
    }

    // draw ledger lines (if not in a palette)
    if (item->segment() && item->track() != muse::nidx) {
        Fraction tick = item->segment()->tick();
        Staff* staff = item->score()->staff(item->staffIdx());
        double lineDist = staff->lineDistance(tick);
        int numOfLines = staff->lines(tick);
        double step = lineDist * spatium;
        double stepTolerance = step * 0.1;
        double ledgerLineLength = item->style().styleS(Sid::ledgerLineLength).val() * spatium;
        double ledgerLineWidth = item->style().styleS(Sid::ledgerLineWidth).val() * spatium;
        painter->setPen(Pen(item->curColor(), ledgerLineWidth, PenStyle::SolidLine, PenCapStyle::FlatCap));

        if (ldata->topPos.y() - stepTolerance <= -step) {
            double xMin = ldata->topPos.x() - ledgerLineLength;
            double xMax = ldata->topPos.x() + item->headWidth() + ledgerLineLength;
            for (double y = -step; y >= ldata->topPos.y() - stepTolerance; y -= step) {
                painter->drawLine(PointF(xMin, y), PointF(xMax, y));
            }
        }

        if (ldata->bottomPos.y() + stepTolerance >= numOfLines * step) {
            double xMin = ldata->bottomPos.x() - ledgerLineLength;
            double xMax = ldata->bottomPos.x() + item->headWidth() + ledgerLineLength;
            for (double y = numOfLines * step; y <= ldata->bottomPos.y() + stepTolerance; y += step) {
                painter->drawLine(PointF(xMin, y), PointF(xMax, y));
            }
        }
    }
}

void TDraw::draw(const Arpeggio* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    const Arpeggio::LayoutData* ldata = item->ldata();
    IF_ASSERT_FAILED(ldata) {
        return;
    }

    const double y1 = ldata->bbox().top();
    const double y2 = ldata->bbox().bottom();
    const double lineWidth = item->style().styleMM(Sid::arpeggioLineWidth);

    painter->setPen(Pen(item->curColor(), lineWidth, PenStyle::SolidLine, PenCapStyle::FlatCap));
    painter->save();

    switch (item->arpeggioType()) {
    case ArpeggioType::NORMAL:
    case ArpeggioType::UP:
    {
        const RectF& r = ldata->symsBBox;
        painter->rotate(-90.0);
        item->drawSymbols(ldata->symbols, painter, PointF(-r.right() - y1, -r.bottom() + r.height()));
    } break;

    case ArpeggioType::DOWN:
    {
        const RectF& r = ldata->symsBBox;
        painter->rotate(90.0);
        item->drawSymbols(ldata->symbols, painter, PointF(-r.left() + y1, -r.top() - r.height()));
    } break;

    case ArpeggioType::UP_STRAIGHT:
    {
        const RectF& r = ldata->symsBBox;
        double x1 = item->spatium() * 0.5;
        item->drawSymbol(SymId::arrowheadBlackUp, painter, PointF(x1 - r.width() * 0.5, y1 - r.top()));
        double ny1 = y1 - r.top() * 0.5;
        painter->drawLine(LineF(x1, ny1, x1, y2));
    } break;

    case ArpeggioType::DOWN_STRAIGHT:
    {
        const RectF& r = ldata->symsBBox;
        double x1 = item->spatium() * 0.5;
        item->drawSymbol(SymId::arrowheadBlackDown, painter, PointF(x1 - r.width() * 0.5, y2 - r.bottom()));
        double ny2 = y2 + r.top() * 0.5;
        painter->drawLine(LineF(x1, y1, x1, ny2));
    } break;

    case ArpeggioType::BRACKET:
    {
        double w = item->style().styleS(Sid::arpeggioHookLen).val() * item->spatium();
        painter->drawLine(LineF(0.0, y1, w, y1));
        painter->drawLine(LineF(0.0, y2, w, y2));
        painter->drawLine(LineF(0.0, y1 - lineWidth / 2, 0.0, y2 + lineWidth / 2));
    } break;
    }
    painter->restore();
}

void TDraw::draw(const Articulation* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    const Articulation::LayoutData* ldata = item->ldata();

    painter->setPen(item->curColor());

    if (item->textType() == ArticulationTextType::NO_TEXT) {
        item->drawSymbol(item->symId(), painter, PointF(-0.5 * item->width(), 0.0));
    } else {
        Font scaledFont(item->font());
        scaledFont.setPointSizeF(scaledFont.pointSizeF() * item->magS() * MScore::pixelRatio);
        painter->setFont(scaledFont);
        painter->drawText(ldata->bbox(), TextDontClip | AlignLeft | AlignTop, TConv::text(item->textType()));
    }
}

void TDraw::draw(const Ornament* item, Painter* painter)
{
    draw(static_cast<const Articulation*>(item), painter);
}

void TDraw::draw(const BagpipeEmbellishment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    const BagpipeEmbellishment::LayoutData* data = item->ldata();
    IF_ASSERT_FAILED(data) {
        return;
    }
    const BagpipeEmbellishment::LayoutData::BeamData& dataBeam = data->beamData;

    Pen pen(item->curColor(), data->stemLineW, PenStyle::SolidLine, PenCapStyle::FlatCap);
    painter->setPen(pen);

    // draw the notes including stem, (optional) flag and (optional) ledger line
    for (const auto& p : data->notesData) {
        const BagpipeEmbellishment::LayoutData::NoteData& noteData = p.second;

        // Draw Grace Note
        {
            // draw head
            item->drawSymbol(data->headsym, painter, noteData.headXY);

            // draw stem
            painter->drawLine(noteData.stemLine);

            if (data->isDrawFlag) {
                // draw flag
                item->drawSymbol(data->flagsym, painter, noteData.flagXY);
            }
        }

        // draw the ledger line for high A
        if (!noteData.ledgerLine.isNull()) {
            painter->drawLine(noteData.ledgerLine);
        }
    }

    if (data->isDrawBeam) {
        Pen beamPen(item->curColor(), dataBeam.width, PenStyle::SolidLine, PenCapStyle::FlatCap);
        painter->setPen(beamPen);
        // draw the beams
        auto drawBeams = [](Painter* painter, const double spatium,
                            const double x1, const double x2, double y)
        {
            // draw the beams
            painter->drawLine(LineF(x1, y, x2, y));
            y += spatium / 1.5;
            painter->drawLine(LineF(x1, y, x2, y));
            y += spatium / 1.5;
            painter->drawLine(LineF(x1, y, x2, y));
        };

        drawBeams(painter, data->spatium, dataBeam.x1, dataBeam.x2, dataBeam.y);
    }
}

static void drawDots(const BarLine* item, Painter* painter, double x)
{
    double spatium = item->spatium();

    double y1l;
    double y2l;
    if (item->explicitParent() == 0) {      // for use in palette (always Bravura)
        //Bravura shifted repeatDot symbol 0.5sp upper in the font itself (1.272)
        y1l = 1.5 * spatium;
        y2l = 2.5 * spatium;
    } else {
        const StaffType* st = item->staffType();

        y1l = st->doty1() * spatium;
        y2l = st->doty2() * spatium;

        //workaround to make Emmentaler, Gonville and MuseJazz font work correctly with repeatDots
        if (item->score()->engravingFont()->name() == "Emmentaler"
            || item->score()->engravingFont()->name() == "Gonville"
            || item->score()->engravingFont()->name() == "MuseJazz") {
            double offset = 0.5 * item->style().spatium() * item->mag();
            y1l += offset;
            y2l += offset;
        }

        //adjust for staffType offset
        double stYOffset = item->staffOffsetY();
        y1l += stYOffset;
        y2l += stYOffset;
    }

    item->drawSymbol(SymId::repeatDot, painter, PointF(x, y1l));
    item->drawSymbol(SymId::repeatDot, painter, PointF(x, y2l));
}

static void drawTips(const BarLine* item, const BarLine::LayoutData* data, Painter* painter, bool reversed, double x)
{
    if (reversed) {
        if (item->isTop()) {
            item->drawSymbol(SymId::reversedBracketTop, painter, PointF(x - item->symWidth(SymId::reversedBracketTop), data->y1));
        }
        if (item->isBottom()) {
            item->drawSymbol(SymId::reversedBracketBottom, painter, PointF(x - item->symWidth(SymId::reversedBracketBottom), data->y2));
        }
    } else {
        if (item->isTop()) {
            item->drawSymbol(SymId::bracketTop, painter, PointF(x, data->y1));
        }
        if (item->isBottom()) {
            item->drawSymbol(SymId::bracketBottom, painter, PointF(x, data->y2));
        }
    }
}

void TDraw::draw(const BarLine* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    const BarLine::LayoutData* data = item->ldata();
    IF_ASSERT_FAILED(data) {
        return;
    }

    switch (item->barLineType()) {
    case BarLineType::NORMAL: {
        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(lw * .5, data->y1, lw * .5, data->y2));
    }
    break;

    case BarLineType::BROKEN: {
        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::DashLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(lw * .5, data->y1, lw * .5, data->y2));
    }
    break;

    case BarLineType::DOTTED: {
        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::DotLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(lw * .5, data->y1, lw * .5, data->y2));
    }
    break;

    case BarLineType::END: {
        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        double x  = lw * .5;
        painter->drawLine(LineF(x, data->y1, x, data->y2));

        double lw2 = item->style().styleMM(Sid::endBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        x += ((lw * .5) + item->style().styleMM(Sid::endBarDistance) + (lw2 * .5)) * item->mag();
        painter->drawLine(LineF(x, data->y1, x, data->y2));
    }
    break;

    case BarLineType::DOUBLE: {
        double lw = item->style().styleMM(Sid::doubleBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        double x = lw * .5;
        painter->drawLine(LineF(x, data->y1, x, data->y2));
        x += ((lw * .5) + item->style().styleMM(Sid::doubleBarDistance) + (lw * .5)) * item->mag();
        painter->drawLine(LineF(x, data->y1, x, data->y2));
    }
    break;

    case BarLineType::REVERSE_END: {
        double lw = item->style().styleMM(Sid::endBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        double x = lw * .5;
        painter->drawLine(LineF(x, data->y1, x, data->y2));

        double lw2 = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        x += ((lw * .5) + item->style().styleMM(Sid::endBarDistance) + (lw2 * .5)) * item->mag();
        painter->drawLine(LineF(x, data->y1, x, data->y2));
    }
    break;

    case BarLineType::HEAVY: {
        double lw = item->style().styleMM(Sid::endBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(lw * .5, data->y1, lw * .5, data->y2));
    }
    break;

    case BarLineType::DOUBLE_HEAVY: {
        double lw2 = item->style().styleMM(Sid::endBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        double x = lw2 * .5;
        painter->drawLine(LineF(x, data->y1, x, data->y2));
        x += ((lw2 * .5) + item->style().styleMM(Sid::endBarDistance) + (lw2 * .5)) * item->mag();
        painter->drawLine(LineF(x, data->y1, x, data->y2));
    }
    break;

    case BarLineType::START_REPEAT: {
        double lw2 = item->style().styleMM(Sid::endBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        double x = lw2 * .5;
        painter->drawLine(LineF(x, data->y1, x, data->y2));

        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        x += ((lw2 * .5) + item->style().styleMM(Sid::endBarDistance) + (lw * .5)) * item->mag();
        painter->drawLine(LineF(x, data->y1, x, data->y2));

        x += ((lw * .5) + item->style().styleMM(Sid::repeatBarlineDotSeparation)) * item->mag();
        drawDots(item, painter, x);

        if (item->style().styleB(Sid::repeatBarTips)) {
            drawTips(item, data, painter, false, 0.0);
        }
    }
    break;

    case BarLineType::END_REPEAT: {
        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));

        double x = 0.0;
        drawDots(item, painter, x);

        x += item->symBbox(SymId::repeatDot).width();
        x += (item->style().styleMM(Sid::repeatBarlineDotSeparation) + (lw * .5)) * item->mag();
        painter->drawLine(LineF(x, data->y1, x, data->y2));

        double lw2 = item->style().styleMM(Sid::endBarWidth) * item->mag();
        x += ((lw * .5) + item->style().styleMM(Sid::endBarDistance) + (lw2 * .5)) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(x, data->y1, x, data->y2));

        if (item->style().styleB(Sid::repeatBarTips)) {
            drawTips(item, data, painter, true, x + lw2 * .5);
        }
    }
    break;
    case BarLineType::END_START_REPEAT: {
        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));

        double x = 0.0;
        drawDots(item, painter, x);

        x += item->symBbox(SymId::repeatDot).width();
        x += (item->style().styleMM(Sid::repeatBarlineDotSeparation) + (lw * .5)) * item->mag();
        painter->drawLine(LineF(x, data->y1, x, data->y2));

        double lw2 = item->style().styleMM(Sid::endBarWidth) * item->mag();
        x += ((lw * .5) + item->style().styleMM(Sid::endBarDistance) + (lw2 * .5)) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(x, data->y1, x, data->y2));

        if (item->style().styleB(Sid::repeatBarTips)) {
            drawTips(item, data, painter, true, x + lw2 * .5);
        }

        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        x  += ((lw2 * .5) + item->style().styleMM(Sid::endBarDistance) + (lw * .5)) * item->mag();
        painter->drawLine(LineF(x, data->y1, x, data->y2));

        x += ((lw * .5) + item->style().styleMM(Sid::repeatBarlineDotSeparation)) * item->mag();
        drawDots(item, painter, x);

        if (item->style().styleB(Sid::repeatBarTips)) {
            drawTips(item, data, painter, false, 0.0);
        }
    }
    break;
    }
    Segment* s = item->segment();
    if (s && s->isEndBarLineType() && !item->score()->printing()) {
        Measure* m = s->measure();
        if (m->isIrregular() && item->score()->markIrregularMeasures() && !m->isMMRest()) {
            painter->setPen(item->configuration()->formattingColor());

            Font f(u"Edwin", Font::Type::Text);
            f.setPointSizeF(12 * item->spatium() / SPATIUM20);
            f.setBold(true);
            Char ch = m->ticks() > m->timesig() ? u'+' : u'-';
            RectF r = FontMetrics(f).boundingRect(ch);

            Font scaledFont(f);
            scaledFont.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
            painter->setFont(scaledFont);

            painter->drawText(-r.width(), 0.0, ch);
        }
    }
}

void TDraw::draw(const Beam* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    if (item->beamSegments().empty()) {
        return;
    }
    painter->setBrush(Brush(item->curColor()));
    painter->setNoPen();

    // make beam thickness independent of slant
    // (expression can be simplified?)

    const LineF bs = item->beamSegments().front()->line;
    double d  = (std::abs(bs.y2() - bs.y1())) / (bs.x2() - bs.x1());
    if (item->beamSegments().size() > 1 && d > M_PI / 6.0) {
        d = M_PI / 6.0;
    }
    double ww = (item->beamWidth() / 2.0) / sin(M_PI_2 - atan(d));

    for (const BeamSegment* bs1 : item->beamSegments()) {
        painter->drawPolygon(
            PolygonF({
            PointF(bs1->line.x1(), bs1->line.y1() - ww),
            PointF(bs1->line.x2(), bs1->line.y2() - ww),
            PointF(bs1->line.x2(), bs1->line.y2() + ww),
            PointF(bs1->line.x1(), bs1->line.y1() + ww),
        }),
            FillRule::OddEvenFill);
    }
}

void TDraw::draw(const Bend* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    const Bend::LayoutData* data = item->ldata();
    IF_ASSERT_FAILED(data) {
        return;
    }

    double spatium = item->spatium();
    double lw = item->absoluteFromSpatium(item->lineWidth());

    Pen pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::RoundCap, PenJoinStyle::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Brush(item->curColor()));

    Font f = item->font(spatium * MScore::pixelRatio);
    painter->setFont(f);

    double x  = data->noteWidth + spatium * .2;
    double y  = -spatium * .8;
    double x2, y2;

    double aw = item->style().styleMM(Sid::bendArrowWidth);
    PolygonF arrowUp;
    arrowUp << PointF(0, 0) << PointF(aw * .5, aw) << PointF(-aw * .5, aw);
    PolygonF arrowDown;
    arrowDown << PointF(0, 0) << PointF(aw * .5, -aw) << PointF(-aw * .5, -aw);

    size_t n = item->points().size();
    for (size_t pt = 0; pt < n - 1; ++pt) {
        int pitch = item->points()[pt].pitch;
        if (pt == 0 && pitch) {
            y2 = -data->notePos.y() - spatium * 2;
            x2 = x;
            painter->drawLine(LineF(x, y, x2, y2));

            painter->setBrush(item->curColor());
            painter->drawPolygon(arrowUp.translated(x2, y2));

            int idx = (pitch + 12) / 25;
            const char* l = item->label[idx];
            painter->drawText(RectF(x2, y2, .0, .0),
                              muse::draw::AlignHCenter | muse::draw::AlignBottom | muse::draw::TextDontClip,
                              String::fromAscii(l));

            y = y2;
        }
        if (pitch == item->points()[pt + 1].pitch) {
            if (pt == (n - 2)) {
                break;
            }
            x2 = x + spatium;
            y2 = y;
            painter->drawLine(LineF(x, y, x2, y2));
        } else if (pitch < item->points()[pt + 1].pitch) {
            // up
            x2 = x + spatium * .5;
            y2 = -data->notePos.y() - spatium * 2;
            double dx = x2 - x;
            double dy = y2 - y;

            PainterPath path;
            path.moveTo(x, y);
            path.cubicTo(x + dx / 2, y, x2, y + dy / 4, x2, y2);
            painter->setBrush(BrushStyle::NoBrush);
            painter->drawPath(path);

            painter->setBrush(item->curColor());
            painter->drawPolygon(arrowUp.translated(x2, y2));

            int idx = (item->points()[pt + 1].pitch + 12) / 25;
            const char* l = item->label[idx];
            double ty = y2;       // - _spatium;
            painter->drawText(RectF(x2, ty, .0, .0),
                              muse::draw::AlignHCenter | muse::draw::AlignBottom | muse::draw::TextDontClip,
                              String::fromAscii(l));
        } else {
            // down
            x2 = x + spatium * .5;
            y2 = y + spatium * 3;
            double dx = x2 - x;
            double dy = y2 - y;

            PainterPath path;
            path.moveTo(x, y);
            path.cubicTo(x + dx / 2, y, x2, y + dy / 4, x2, y2);
            painter->setBrush(BrushStyle::NoBrush);
            painter->drawPath(path);

            painter->setBrush(item->curColor());
            painter->drawPolygon(arrowDown.translated(x2, y2));
        }
        x = x2;
        y = y2;
    }
}

void TDraw::draw(const Box* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    if (item->score() && item->score()->printing()) {
        return;
    }

    const Box::LayoutData* ldata = item->ldata();

    const bool showHighlightedFrame = item->selected() || item->dropTarget();
    const bool showFrame = showHighlightedFrame || (item->score() ? item->score()->showFrames() : false);

    if (showFrame) {
        double lineWidth = item->spatium() * .15;
        Pen pen;
        pen.setWidthF(lineWidth);
        pen.setJoinStyle(PenJoinStyle::MiterJoin);
        pen.setCapStyle(PenCapStyle::SquareCap);
        pen.setColor(showHighlightedFrame
                     ? item->configuration()->selectionColor()
                     : item->configuration()->frameColor());
        pen.setDashPattern({ 1, 3 });

        painter->setBrush(BrushStyle::NoBrush);
        painter->setPen(pen);
        lineWidth *= 0.5;
        painter->drawRect(ldata->bbox().adjusted(lineWidth, lineWidth, -lineWidth, -lineWidth));
    }
}

void TDraw::draw(const HBox* item, Painter* painter)
{
    draw(static_cast<const Box*>(item), painter);
}

void TDraw::draw(const VBox* item, Painter* painter)
{
    draw(static_cast<const Box*>(item), painter);
}

void TDraw::draw(const FBox* item, Painter* painter)
{
    draw(static_cast<const Box*>(item), painter);
}

void TDraw::draw(const TBox* item, Painter* painter)
{
    draw(static_cast<const Box*>(item), painter);
}

void TDraw::draw(const Bracket* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    const Bracket::LayoutData* ldata = item->ldata();
    IF_ASSERT_FAILED(ldata) {
        return;
    }

    switch (item->bracketType()) {
    case BracketType::BRACE: {
        if (ldata->braceSymbol == SymId::noSym) {
            painter->setNoPen();
            painter->setBrush(Brush(item->curColor()));
            painter->drawPath(ldata->path);
        } else {
            double h = ldata->bracketHeight;
            double mag = h / (100 * item->magS());
            painter->setPen(item->curColor());
            painter->save();
            painter->scale(item->magx(), mag);
            item->drawSymbol(ldata->braceSymbol, painter, PointF(0, 100 * item->magS()));
            painter->restore();
        }
    }
    break;
    case BracketType::NORMAL: {
        double h = ldata->bracketHeight;
        double spatium = item->spatium();
        double w = item->style().styleMM(Sid::bracketWidth);
        double bd = (item->style().styleSt(Sid::musicalSymbolFont) == "Leland") ? spatium * .5 : spatium * .25;
        Pen pen(item->curColor(), w, PenStyle::SolidLine, PenCapStyle::FlatCap);
        painter->setPen(pen);
        painter->drawLine(LineF(0.0, -bd - w * .5, 0.0, h + bd + w * .5));
        double x = -w * .5;
        double y1 = -bd;
        double y2 = h + bd;
        item->drawSymbol(SymId::bracketTop, painter, PointF(x, y1));
        item->drawSymbol(SymId::bracketBottom, painter, PointF(x, y2));
    }
    break;
    case BracketType::SQUARE: {
        double h = ldata->bracketHeight;
        double lineW = item->style().styleMM(Sid::staffLineWidth);
        double bracketWidth = ldata->bracketWidth - lineW / 2;
        Pen pen(item->curColor(), lineW, PenStyle::SolidLine, PenCapStyle::FlatCap);
        painter->setPen(pen);
        painter->drawLine(LineF(0.0, 0.0, 0.0, h));
        painter->drawLine(LineF(-lineW / 2, 0.0, lineW / 2 + bracketWidth, 0.0));
        painter->drawLine(LineF(-lineW / 2, h, lineW / 2 + bracketWidth, h));
    }
    break;
    case BracketType::LINE: {
        double h = ldata->bracketHeight;
        double w = 0.67 * item->style().styleMM(Sid::bracketWidth);
        Pen pen(item->curColor(), w, PenStyle::SolidLine, PenCapStyle::FlatCap);
        painter->setPen(pen);
        double bd = item->style().styleMM(Sid::staffLineWidth) * 0.5;
        painter->drawLine(LineF(0.0, -bd, 0.0, h + bd));
    }
    break;
    case BracketType::NO_BRACKET:
        break;
    }
}

void TDraw::draw(const Breath* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    painter->setPen(item->curColor());
    item->drawSymbol(item->symId(), painter);
}

void TDraw::draw(const ChordLine* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    const ChordLine::LayoutData* ldata = item->ldata();
    IF_ASSERT_FAILED(ldata) {
        return;
    }

    painter->setPen(Pen(item->curColor(), item->style().styleMM(Sid::chordlineThickness) * item->mag(), PenStyle::SolidLine));
    painter->setBrush(BrushStyle::NoBrush);
    if (!item->isWavy()) {
        painter->drawPath(ldata->path);
    } else {
        painter->save();
        painter->rotate((item->chordLineType() == ChordLineType::FALL ? 1 : -1));
        item->drawSymbol(item->waveSym(), painter);
        painter->restore();
    }
}

void TDraw::draw(const Clef* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    const Clef::LayoutData* ldata = item->ldata();
    IF_ASSERT_FAILED(ldata) {
        return;
    }

    if (ldata->symId == SymId::noSym || (item->staff() && !const_cast<const Staff*>(item->staff())->staffType(item->tick())->genClef())) {
        return;
    }

    painter->setPen(item->curColor());
    item->drawSymbol(ldata->symId, painter);
}

void TDraw::draw(const Capo* item, Painter* painter)
{
    drawTextBase(item, painter);
}

void TDraw::draw(const DeadSlapped* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    const DeadSlapped::LayoutData* ldata = item->ldata();
    IF_ASSERT_FAILED(ldata) {
        return;
    }

    painter->setPen(PenStyle::NoPen);
    painter->setBrush(item->curColor());
    painter->drawPath(ldata->path1);
    painter->drawPath(ldata->path2);
}

void TDraw::draw(const Dynamic* item, Painter* painter)
{
    drawTextBase(item, painter);
}

void TDraw::draw(const Expression* item, Painter* painter)
{
    drawTextBase(item, painter);
}

void TDraw::draw(const Fermata* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    painter->setPen(item->curColor());
    item->drawSymbol(item->symId(), painter, PointF(-0.5 * item->width(), 0.0));
}

void TDraw::draw(const FiguredBass* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    const FiguredBass::LayoutData* ldata = item->ldata();
    // if not printing, draw duration line(s)
    if (!item->score()->printing() && item->score()->showUnprintable()) {
        for (double len : ldata->lineLengths) {
            if (len > 0) {
                painter->setPen(Pen(item->configuration()->formattingColor(), 3));
                painter->drawLine(0.0, -2, len, -2);              // -2: 2 rast. un. above digits
            }
        }
    }

    if (item->items().size() < 1) {                                 // if not parseable into f.b. items
        drawTextBase(item, painter);                                // draw as standard text
    } else {
        for (FiguredBassItem* fi : item->items()) {               // if parseable into f.b. items
            painter->translate(fi->pos());                // draw each item in its proper position
            draw(fi, painter);
            painter->translate(-fi->pos());
        }
    }
}

void TDraw::draw(const FiguredBassItem* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    const FiguredBassItem::LayoutData* ldata = item->ldata();
    int font = 0;
    double _spatium = item->spatium();
    // set font from general style
    Font f(FiguredBass::FBFonts().at(font).family, Font::Type::Tablature);

    // (use the same font selection as used in layout() above)
    double m = item->style().styleD(Sid::figuredBassFontSize) * item->spatium() / SPATIUM20;
    f.setPointSizeF(m * MScore::pixelRatio);

    painter->setFont(f);
    painter->setBrush(BrushStyle::NoBrush);
    Pen pen(item->figuredBass()->curColor(), FiguredBass::FB_CONTLINE_THICKNESS * _spatium, PenStyle::SolidLine, PenCapStyle::RoundCap);
    painter->setPen(pen);
    painter->drawText(ldata->bbox(), muse::draw::TextDontClip | muse::draw::AlignLeft | muse::draw::AlignTop, ldata->displayText);

    // continuation line
    double lineEndX = 0.0;
    if (item->contLine() != FiguredBassItem::ContLine::NONE) {
        double lineStartX  = ldata->textWidth;                           // by default, line starts right after text
        if (lineStartX > 0.0) {
            lineStartX += _spatium * FiguredBass::FB_CONTLINE_LEFT_PADDING;          // if some text, give some room after it
        }
        lineEndX = item->figuredBass()->ldata()->printedLineLength;            // by default, line ends with item duration
        if (lineEndX - lineStartX < 1.0) {                         // if line length < 1 sp, ignore it
            lineEndX = 0.0;
        }

        // if extended cont.line and no closing parenthesis: look at next FB element
        if (item->contLine() > FiguredBassItem::ContLine::SIMPLE && item->parenth5() == FiguredBassItem::Parenthesis::NONE) {
            FiguredBass* nextFB;
            // if there is a contiguous FB element
            if ((nextFB = item->figuredBass()->nextFiguredBass()) != 0) {
                // retrieve the X position (in page coords) of a possible cont. line of nextFB
                // on the same line of 'this'
                PointF pgPos = item->pagePos();
                double nextContPageX = nextFB->additionalContLineX(pgPos.y());
                // if an additional cont. line has been found, extend up to its initial X coord
                if (nextContPageX > 0) {
                    lineEndX = nextContPageX - pgPos.x() + _spatium * FiguredBass::FB_CONTLINE_OVERLAP;
                }
                // with a little bit of overlap
                else {
                    lineEndX = item->figuredBass()->ldata()->lineLength(0);                  // if none found, draw to the duration end
                }
            }
        }
        // if some line, draw it
        if (lineEndX > 0.0) {
            double h = ldata->bbox().height() * FiguredBass::FB_CONTLINE_HEIGHT;
            painter->drawLine(lineStartX, h, lineEndX - ldata->pos().x(), h);
        }
    }

    // closing cont.line parenthesis
    if (item->parenth5() != FiguredBassItem::Parenthesis::NONE) {
        int x = lineEndX > 0.0 ? lineEndX : ldata->textWidth;
        painter->drawText(RectF(x, 0, ldata->bbox().width(), ldata->bbox().height()),
                          muse::draw::AlignLeft | muse::draw::AlignTop,
                          Char(FiguredBass::FBFonts().at(font).displayParenthesis[int(item->parenth5())].unicode()));
    }
}

void TDraw::draw(const Fingering* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void TDraw::draw(const FretDiagram* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    const FretDiagram::LayoutData* ldata = item->ldata();
    PointF translation = -PointF(ldata->stringDist * (item->strings() - 1), 0);
    if (item->orientation() == Orientation::HORIZONTAL) {
        painter->save();
        painter->rotate(-90);
        painter->translate(translation);
    }

    // Init pen and other values
    Pen pen(item->curColor());
    pen.setCapStyle(PenCapStyle::FlatCap);
    painter->setBrush(Brush(Color(painter->pen().color())));

    // x2 is the x val of the rightmost string
    double x2 = (item->strings() - 1) * ldata->stringDist;

    // Draw the nut
    pen.setWidthF(ldata->nutLineWidth);
    painter->setPen(pen);
    double nutY = ldata->nutY;
    painter->drawLine(LineF(-ldata->stringLineWidth * .5, nutY, x2 + ldata->stringLineWidth * .5, nutY));

    // Draw strings and frets
    pen.setWidthF(ldata->stringLineWidth);
    painter->setPen(pen);

    // y2 is the y val of the bottom fretline
    double y1 = ldata->stringExtendTop;
    double y2 = ldata->fretDist * item->frets() + 0.5 * ldata->stringLineWidth + ldata->stringExtendBottom;
    for (int i = 0; i < item->strings(); ++i) {
        double x = ldata->stringDist * i;
        painter->drawLine(LineF(x, y1, x, y2));
    }
    for (int i = 1; i <= item->frets(); ++i) {
        double y = ldata->fretDist * i;
        painter->drawLine(LineF(0.0, y, x2, y));
    }

    // dotd is the diameter of a dot
    double dotd = ldata->dotDiameter;

    // Draw dots, sym pen is used to draw them (and markers)
    Pen symPen(pen);
    symPen.setCapStyle(PenCapStyle::FlatCap);
    double symPenWidth = ldata->stringLineWidth * 1.2;
    symPen.setWidthF(symPenWidth);

    for (auto const& i : item->dots()) {
        for (auto const& d : i.second) {
            if (!d.exists()) {
                continue;
            }

            int string = i.first;
            int fret = d.fret - 1;

            // Calculate coords of the top left corner of the dot
            double x = ldata->stringDist * string - dotd * .5;
            double y = ldata->fretDist * fret + ldata->fretDist * .5 - dotd * .5;

            // Draw different symbols
            painter->setPen(symPen);
            switch (d.dtype) {
            case FretDotType::CROSS:
                // Give the cross a slightly larger width
                symPen.setWidthF(symPenWidth * 1.5);
                painter->setPen(symPen);
                painter->drawLine(LineF(x, y, x + dotd, y + dotd));
                painter->drawLine(LineF(x + dotd, y, x, y + dotd));
                symPen.setWidthF(symPenWidth);
                break;
            case FretDotType::SQUARE:
                painter->setBrush(BrushStyle::NoBrush);
                painter->drawRect(RectF(x, y, dotd, dotd));
                break;
            case FretDotType::TRIANGLE:
                painter->drawLine(LineF(x, y + dotd, x + .5 * dotd, y));
                painter->drawLine(LineF(x + .5 * dotd, y, x + dotd, y + dotd));
                painter->drawLine(LineF(x + dotd, y + dotd, x, y + dotd));
                break;
            case FretDotType::NORMAL:
            default:
                painter->setBrush(symPen.color());
                painter->setNoPen();
                painter->drawEllipse(RectF(x, y, dotd, dotd));
                break;
            }
        }
    }

    // Draw markers
    symPen.setWidthF(ldata->stringLineWidth * 1.2);
    painter->setBrush(BrushStyle::NoBrush);
    painter->setPen(symPen);
    for (auto const& i : item->markers()) {
        int string = i.first;
        FretItem::Marker marker = i.second;
        if (!marker.exists()) {
            continue;
        }

        double x = ldata->stringDist * string - ldata->markerSize * .5;
        double y = ldata->markerY;
        if (marker.mtype == FretMarkerType::CIRCLE) {
            painter->drawEllipse(RectF(x, y, ldata->markerSize, ldata->markerSize));
        } else if (marker.mtype == FretMarkerType::CROSS) {
            painter->drawLine(PointF(x, y), PointF(x + ldata->markerSize, y + ldata->markerSize));
            painter->drawLine(PointF(x, y + ldata->markerSize), PointF(x + ldata->markerSize, y));
        }
    }

    // Draw barres
    for (auto const& i : item->barres()) {
        int fret        = i.first;
        int startString = i.second.startString;
        int endString   = i.second.endString;

        double x1    = ldata->stringDist * startString;
        double newX2 = endString == -1 ? x2 : ldata->stringDist * endString;
        double y     = ldata->fretDist * (fret - 1) + ldata->fretDist * .5;
        if (item->style().styleB(Sid::barreAppearanceSlur)) {
            pen.setWidthF(0.25 * ldata->stringLineWidth);
            pen.setCapStyle(PenCapStyle::RoundCap);
            pen.setJoinStyle(PenJoinStyle::RoundJoin);
            painter->setPen(pen);
            painter->setBrush(Brush(pen.color()));
            painter->drawPath(ldata->slurPath);
        } else {
            pen.setWidthF(dotd * item->style().styleD(Sid::barreLineWidth));
            pen.setCapStyle(PenCapStyle::RoundCap);
            painter->setPen(pen);
            painter->drawLine(LineF(x1, y, newX2, y));
        }
    }

    // Draw fret offset number
    if (item->fretOffset() > 0) {
        Font scaledFont(item->fretNumFont());
        scaledFont.setPointSizeF(scaledFont.pointSizeF() * (item->spatium() / SPATIUM20) * MScore::pixelRatio);
        painter->setFont(scaledFont);
        String text = ldata->fretText;

        if (item->orientation() == Orientation::VERTICAL) {
            if (item->numPos() == 0) {
                painter->drawText(RectF(-ldata->fretNumPadding, .0, .0, ldata->fretDist),
                                  muse::draw::AlignVCenter | muse::draw::AlignRight | muse::draw::TextDontClip, text);
            } else {
                painter->drawText(RectF(x2 + ldata->fretNumPadding, .0, .0, ldata->fretDist),
                                  muse::draw::AlignVCenter | muse::draw::AlignLeft | muse::draw::TextDontClip,
                                  text);
            }
        } else if (item->orientation() == Orientation::HORIZONTAL) {
            painter->save();
            painter->translate(-translation);
            painter->rotate(90);
            if (item->numPos() == 0) {
                painter->drawText(RectF(.0, ldata->stringDist * (item->strings() - 1), .0, .0),
                                  muse::draw::AlignLeft | muse::draw::TextDontClip, text);
            } else {
                painter->drawText(RectF(.0, .0, .0, .0),
                                  muse::draw::AlignBottom | muse::draw::AlignLeft | muse::draw::TextDontClip, text);
            }
            painter->restore();
        }
        painter->setFont(item->fretNumFont());
    }

    for (const FretDiagram::FingeringItem& fingeringItem : item->ldata()->fingeringItems) {
        painter->save();

        Font scaledFont(item->fingeringFont());
        scaledFont.setPointSizeF(scaledFont.pointSizeF() * (item->spatium() / SPATIUM20) * MScore::pixelRatio);
        painter->setFont(scaledFont);
        if (item->orientation() == Orientation::HORIZONTAL) {
            painter->translate(-translation);
            painter->rotate(90);
        }

        painter->drawText(fingeringItem.pos, fingeringItem.fingerNumber);

        painter->restore();
    }

    if (item->orientation() == Orientation::HORIZONTAL) {
        painter->restore();
    }
}

void TDraw::draw(const FretCircle* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    const FretCircle::LayoutData* ldata = item->ldata();
    painter->save();
    painter->setPen(Pen(item->curColor(), item->spatium() * FretCircle::CIRCLE_WIDTH));
    painter->setBrush(BrushStyle::NoBrush);
    painter->drawEllipse(ldata->rect);
    painter->restore();
}

static void setDashAndGapLen(const SLine* line, double& dash, double& gap, Pen& pen)
{
    static constexpr double DOTTED_DASH_LEN = 0.01;
    static constexpr double DOTTED_GAP_LEN = 1.99;
    switch (line->lineStyle()) {
    case LineType::SOLID:
        break;
    case LineType::DASHED:
        dash = line->dashLineLen(), gap = line->dashGapLen();
        break;
    case LineType::DOTTED:
        dash = DOTTED_DASH_LEN, gap = DOTTED_GAP_LEN;
        pen.setCapStyle(PenCapStyle::RoundCap); // round dots
        break;
    }
}

static std::vector<double> distributedDashPattern(double dash, double gap, double lineLength)
{
    int numPairs = std::max(1.0, lineLength / (dash + gap));
    double newGap = (lineLength - dash * (numPairs + 1)) / numPairs;

    return { dash, newGap };
}

void TDraw::draw(const GlissandoSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    if (item->pos2().x() <= 0) {
        return;
    }

    painter->save();
    double _spatium = item->spatium();
    const Glissando* glissando = item->glissando();

    Pen pen(item->curColor(item->getProperty(Pid::VISIBLE).toBool(), item->getProperty(Pid::COLOR).value<Color>()));
    pen.setWidthF(item->absoluteFromSpatium(item->lineWidth()));
    pen.setCapStyle(PenCapStyle::FlatCap);
    painter->setPen(pen);

    // rotate painter so that the line become horizontal
    double w     = item->pos2().x();
    double h     = item->pos2().y();
    double l     = sqrt(w * w + h * h);
    double wi    = asin(-h / l) * 180.0 / M_PI;
    painter->rotate(-wi);

    if (glissando->glissandoType() == GlissandoType::STRAIGHT) {
        const bool isNonSolid = glissando->lineStyle() != LineType::SOLID;
        if (isNonSolid) {
            double lineWidth = glissando->absoluteFromSpatium(glissando->lineWidth());
            double dash = 0;
            double gap = 0;
            setDashAndGapLen(glissando, dash, gap, pen);
            pen.setDashPattern(distributedDashPattern(dash, gap, l / lineWidth));
            painter->setPen(pen);
        }

        painter->drawLine(LineF(0.0, 0.0, l, 0.0));
    } else if (glissando->glissandoType() == GlissandoType::WAVY) {
        RectF b = item->symBbox(SymId::wiggleTrill);
        double a  = item->symAdvance(SymId::wiggleTrill);
        int n    = static_cast<int>(l / a);          // always round down (truncate) to avoid overlap
        double x  = (l - n * a) * 0.5;     // centre line in available space
        SymIdList ids;
        for (int i = 0; i < n; ++i) {
            ids.push_back(SymId::wiggleTrill);
        }

        item->score()->engravingFont()->draw(ids, painter, item->magS(), PointF(x, -(b.y() + b.height() * 0.5)));
    }

    if (glissando->showText()) {
        Font f(glissando->fontFace(), Font::Type::Unknown);
        f.setPointSizeF(glissando->fontSize() * _spatium / SPATIUM20);
        f.setBold(glissando->fontStyle() & FontStyle::Bold);
        f.setItalic(glissando->fontStyle() & FontStyle::Italic);
        f.setUnderline(glissando->fontStyle() & FontStyle::Underline);
        f.setStrike(glissando->fontStyle() & FontStyle::Strike);
        FontMetrics fm(f);
        RectF r = fm.boundingRect(glissando->text());

        // if text longer than available space, skip it
        if (r.width() < l) {
            double yOffset = r.height() + r.y();             // find text descender height
            // raise text slightly above line and slightly more with WAVY than with STRAIGHT
            yOffset += _spatium * (glissando->glissandoType() == GlissandoType::WAVY ? 0.4 : 0.1);

            Font scaledFont(f);
            scaledFont.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
            painter->setFont(scaledFont);

            double x = (l - r.width()) * 0.5;
            painter->drawText(PointF(x, -yOffset), glissando->text());
        }
    }
    painter->restore();
}

void TDraw::draw(const GuitarBendSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    Pen pen(item->curColor(item->visible()));
    pen.setWidthF(item->lineWidth());
    pen.setCapStyle(PenCapStyle::FlatCap);
    pen.setJoinStyle(PenJoinStyle::MiterJoin);
    pen.setColor(item->uiColor());
    painter->setPen(pen);

    Brush brush;
    brush.setStyle(BrushStyle::NoBrush);
    painter->setBrush(brush);

    painter->drawPath(item->ldata()->path());

    if (item->staff()->isTabStaff(item->tick())) {
        brush.setStyle(BrushStyle::SolidPattern);
        brush.setColor(item->uiColor());
        painter->setBrush(brush);
        painter->setNoPen();
        painter->drawPolygon(item->ldata()->arrow());
    }
}

void TDraw::draw(const GuitarBendHoldSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    Pen pen(item->curColor(item->visible()));
    pen.setWidthF(item->lineWidth());
    double dash = item->dashLength();
    pen.setDashPattern({ dash, dash });
    pen.setCapStyle(PenCapStyle::FlatCap);
    pen.setJoinStyle(PenJoinStyle::MiterJoin);
    painter->setPen(pen);

    painter->drawLine(PointF(), item->pos2());
}

void TDraw::draw(const StretchedBend* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    double sp = item->spatium();
    const Color& color = item->curColor();
    const int textFlags = item->textFlags();

    Pen pen(color, item->absoluteFromSpatium(item->lineWidth()), PenStyle::SolidLine, PenCapStyle::RoundCap, PenJoinStyle::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Brush(color));
    Font f = item->font(sp * MScore::pixelRatio);
    painter->setFont(f);

    bool isTextDrawn = false;

    for (const StretchedBend::BendSegment& bendSegment : item->bendSegmentsStretched()) {
        if (!bendSegment.visible) {
            continue;
        }

        const PointF& src = bendSegment.src;
        const PointF& dest = bendSegment.dest;
        const String& text = item->toneToLabel(bendSegment.tone);

        switch (bendSegment.type) {
        case StretchedBend::BendSegmentType::LINE_UP:
        {
            painter->drawLine(LineF(src, dest));
            painter->setBrush(color);
            painter->drawPolygon(item->arrows().up.translated(dest));
            /// TODO: remove substraction after fixing bRect
            PointF pos = dest - PointF(0, sp * 0.5);
            painter->drawText(RectF(pos.x(), pos.y(), .0, .0), textFlags, text);
            break;
        }

        case StretchedBend::BendSegmentType::CURVE_UP:
        case StretchedBend::BendSegmentType::CURVE_DOWN:
        {
            bool bendUp = (bendSegment.type == StretchedBend::BendSegmentType::CURVE_UP);
            double endY = dest.y() + item->arrows().width * (bendUp ? 1 : -1);

            PainterPath path = item->bendCurveFromPoints(src, PointF(dest.x(), endY));
            const auto& arrowPath = (bendUp ? item->arrows().up : item->arrows().down);

            painter->setBrush(BrushStyle::NoBrush);
            painter->drawPath(path);
            painter->setBrush(color);
            painter->drawPolygon(arrowPath.translated(dest));

            if (bendUp && !isTextDrawn) {
                /// TODO: remove subtraction after fixing bRect
                PointF pos = dest - PointF(0, sp * 0.5);
                painter->drawText(RectF(pos.x(), pos.y(), .0, .0), textFlags, text);
                isTextDrawn = true;
            }

            break;
        }

        case StretchedBend::BendSegmentType::LINE_STROKED:
        {
            PainterPath path;
            path.moveTo(src + PointF(item->arrows().width, 0));
            path.lineTo(dest);
            Pen p(painter->pen());
            p.setStyle(PenStyle::DashLine);
            painter->strokePath(path, p);
            break;
        }

        default:
            break;
        }
    }
}

void TDraw::drawTextBase(const TextBase* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    const TextBase::LayoutData* ldata = item->ldata();
    if (item->hasFrame()) {
        double baseSpatium = DefaultStyle::baseStyle().value(Sid::spatium).toReal();
        if (!RealIsNull(item->frameWidth().val())) {
            Color fColor = item->curColor(item->visible(), item->frameColor());
            double frameWidthVal = item->frameWidth().val() * (item->sizeIsSpatiumDependent() ? item->spatium() : baseSpatium);

            Pen pen(fColor, frameWidthVal, PenStyle::SolidLine, PenCapStyle::SquareCap, PenJoinStyle::MiterJoin);
            painter->setPen(pen);
        } else {
            painter->setNoPen();
        }
        Color bg(item->bgColor());
        painter->setBrush(bg.alpha() ? Brush(bg) : BrushStyle::NoBrush);
        if (item->circle()) {
            painter->drawEllipse(ldata->frame);
        } else {
            double frameRoundFactor = (item->sizeIsSpatiumDependent() ? (item->spatium() / baseSpatium) / 2 : 0.5f);

            int r2 = item->frameRound() * frameRoundFactor;
            if (r2 > 99) {
                r2 = 99;
            }
            painter->drawRoundedRect(ldata->frame, item->frameRound() * frameRoundFactor, r2);
        }
    }
    painter->setBrush(BrushStyle::NoBrush);
    painter->setPen(item->textColor());
    for (const TextBlock& t : ldata->blocks) {
        t.draw(painter, item);
    }
}

void TDraw::drawTextLineBaseSegment(const TextLineBaseSegment* item, Painter* painter)
{
    const TextLineBase* tl = item->textLineBase();

    if (!item->text()->empty()) {
        painter->translate(item->text()->pos());
        item->text()->setVisible(tl->visible());
        draw(item->text(), painter);
        painter->translate(-item->text()->pos());
    }

    if (!item->endText()->empty()) {
        painter->translate(item->endText()->pos());
        item->endText()->setVisible(tl->visible());
        draw(item->endText(), painter);
        painter->translate(-item->endText()->pos());
    }

    if ((item->npoints() == 0)
        || (item->score() && (item->score()->printing() || !item->score()->isShowInvisible()) && !tl->lineVisible())) {
        return;
    }

    // color for line (text color comes from the text properties)
    Color color = item->curColor(tl->visible() && tl->lineVisible(), tl->lineColor());

    double lineWidth = tl->absoluteFromSpatium(tl->lineWidth());

    const Pen solidPen(color, lineWidth, PenStyle::SolidLine, PenCapStyle::FlatCap, PenJoinStyle::MiterJoin);
    Pen pen(solidPen);

    double dash = 0;
    double gap = 0;

    setDashAndGapLen(tl, dash, gap, pen);

    const bool isNonSolid = tl->lineStyle() != LineType::SOLID;

    // Draw lines
    if (item->twoLines()) { // hairpins
        if (isNonSolid) {
            pen.setDashPattern({ dash, gap });
        }

        pen.setJoinStyle(PenJoinStyle::BevelJoin);
        painter->setPen(pen);
        if (!item->joinedHairpin().empty() && !isNonSolid) {
            painter->drawPolyline(item->joinedHairpin());
        } else {
            painter->drawLines(&item->points()[0], 2);
        }
        return;
    }

    int start = 0, end = item->npoints();

    // Draw begin hook, if it needs to be drawn separately
    if (item->isSingleBeginType() && tl->beginHookType() != HookType::NONE) {
        bool isTHook = tl->beginHookType() == HookType::HOOK_90T;

        if (isNonSolid || isTHook) {
            const PointF& p1 = item->points()[start++];
            const PointF& p2 = item->points()[start++];

            if (isTHook) {
                painter->setPen(solidPen);
            } else {
                double hookLength = sqrt(PointF::dotProduct(p2 - p1, p2 - p1));
                pen.setDashPattern(distributedDashPattern(dash, gap, hookLength / lineWidth));
                painter->setPen(pen);
            }

            painter->drawLine(p1, p2);
        }
    }

    // Draw end hook, if it needs to be drawn separately
    if (item->isSingleEndType() && tl->endHookType() != HookType::NONE) {
        bool isTHook = tl->endHookType() == HookType::HOOK_90T;

        if (isNonSolid || isTHook) {
            const PointF& p1 = item->points()[--end];
            const PointF& p2 = item->points()[--end];

            if (isTHook) {
                painter->setPen(solidPen);
            } else {
                double hookLength = sqrt(PointF::dotProduct(p2 - p1, p2 - p1));
                pen.setDashPattern(distributedDashPattern(dash, gap, hookLength / lineWidth));
                painter->setPen(pen);
            }

            painter->drawLine(p1, p2);
        }
    }

    // Draw the rest
    if (isNonSolid) {
        pen.setDashPattern(distributedDashPattern(dash, gap, item->lineLength() / lineWidth));
    }

    painter->setPen(pen);
    painter->drawPolyline(&item->points()[start], end - start);
}

void TDraw::draw(const GradualTempoChangeSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextLineBaseSegment(item, painter);
}

void TDraw::draw(const HairpinSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    drawTextLineBaseSegment(item, painter);

    if (item->drawCircledTip()) {
        Color color = item->curColor(item->hairpin()->visible(), item->hairpin()->lineColor());
        double w = item->absoluteFromSpatium(item->lineWidth());
        if (item->staff()) {
            w *= item->staff()->staffMag(item->hairpin()->tick());
        }

        Pen pen(color, w);
        painter->setPen(pen);
        painter->setBrush(BrushStyle::NoBrush);
        painter->drawEllipse(item->circledTip(), item->circledTipRadius(), item->circledTipRadius());
    }
}

void TDraw::draw(const HarpPedalDiagram* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void TDraw::draw(const HarmonicMarkSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextLineBaseSegment(item, painter);
}

void TDraw::draw(const Harmony* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    const TextBase::LayoutData* ldata = item->ldata();

    if (item->isDrawEditMode()) {
        drawTextBase(item, painter);
        return;
    }

    if (item->textList().empty()) {
        drawTextBase(item, painter);
        return;
    }

    if (item->hasFrame()) {
        if (!RealIsNull(item->frameWidth().val())) {
            Color color = item->frameColor();
            Pen pen(color, item->frameWidth().val() * item->spatium(), PenStyle::SolidLine,
                    PenCapStyle::SquareCap, PenJoinStyle::MiterJoin);
            painter->setPen(pen);
        } else {
            painter->setNoPen();
        }
        Color bg(item->bgColor());
        painter->setBrush(bg.alpha() ? Brush(bg) : BrushStyle::NoBrush);
        if (item->circle()) {
            painter->drawArc(ldata->frame, 0, 5760);
        } else {
            int r2 = item->frameRound();
            if (r2 > 99) {
                r2 = 99;
            }
            painter->drawRoundedRect(ldata->frame, item->frameRound(), r2);
        }
    }
    painter->setBrush(BrushStyle::NoBrush);
    Color color = item->textColor();
    painter->setPen(color);
    for (const TextSegment* ts : item->textList()) {
        Font f(ts->m_font);
        f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
#ifndef Q_OS_MACOS
        TextBase::drawTextWorkaround(painter, f, ts->pos(), ts->text);
#else
        painter->setFont(f);
        painter->drawText(ts->pos(), ts->text);
#endif
    }
}

void TDraw::draw(const Hook* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    // hide if belonging to the second chord of a cross-measure pair
    if (item->chord() && item->chord()->crossMeasure() == CrossMeasure::SECOND) {
        return;
    }

    painter->setPen(item->curColor());
    item->drawSymbol(item->sym(), painter);
}

void TDraw::draw(const Image* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    const Image::LayoutData* ldata = item->ldata();
    bool emptyImage = false;
    if (item->imageType() == ImageType::SVG) {
        if (!item->svgRenderer()) {
            emptyImage = true;
        } else {
            item->svgRenderer()->render(painter, ldata->bbox());
        }
    } else if (item->imageType() == ImageType::RASTER) {
        if (item->rasterImage() == nullptr) {
            emptyImage = true;
        } else {
            painter->save();
            SizeF s;
            if (item->sizeIsSpatium()) {
                s = item->size() * item->spatium();
            } else {
                s = item->size() * DPMM;
            }
            if (item->score() && item->score()->printing() && !MScore::svgPrinting) {
                // use original image size for printing, but not for svg for reasonable file size.
                painter->scale(s.width() / item->rasterImage()->width(), s.height() / item->rasterImage()->height());
                painter->drawPixmap(PointF(0, 0), *item->rasterImage());
            } else {
                Transform t = painter->worldTransform();
                muse::Size ss = muse::Size(s.width() * t.m11(), s.height() * t.m22());
                t.setMatrix(1.0, t.m12(), t.m13(), t.m21(), 1.0, t.m23(), t.m31(), t.m32(), t.m33());
                painter->setWorldTransform(t);
                if ((item->buffer().size() != ss || item->dirty()) && item->rasterImage() && !item->rasterImage()->isNull()) {
                    item->setBuffer(item->imageProvider()->scaled(*item->rasterImage(), ss));
                    item->setDirty(false);
                }
                if (item->buffer().isNull()) {
                    emptyImage = true;
                } else {
                    painter->drawPixmap(PointF(0.0, 0.0), item->buffer());
                }
            }
            painter->restore();
        }
    }

    if (emptyImage) {
        painter->setBrush(BrushStyle::NoBrush);
        painter->setPen(item->configuration()->defaultColor());
        painter->drawRect(ldata->bbox());
        painter->drawLine(0.0, 0.0, ldata->bbox().width(), ldata->bbox().height());
        painter->drawLine(ldata->bbox().width(), 0.0, 0.0, ldata->bbox().height());
    }
    if (item->selected() && !(item->score() && item->score()->printing())) {
        painter->setBrush(BrushStyle::NoBrush);
        painter->setPen(item->configuration()->selectionColor());
        painter->drawRect(ldata->bbox());
    }
}

void TDraw::draw(const InstrumentChange* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void TDraw::draw(const InstrumentName* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void TDraw::draw(const Jump* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void TDraw::draw(const KeySig* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    const KeySig::LayoutData* ldata = item->ldata();

    painter->setPen(item->curColor());
    double _spatium = item->spatium();
    double step = _spatium * (item->staff() ? item->staff()->staffTypeForElement(item)->lineDistance().val() * 0.5 : 0.5);
    int lines = item->staff() ? item->staff()->staffTypeForElement(item)->lines() : 5;
    double ledgerLineWidth = item->style().styleMM(Sid::ledgerLineWidth) * item->mag();
    double ledgerExtraLen = item->style().styleS(Sid::ledgerLineLength).val() * _spatium;
    for (const KeySym& ks : ldata->keySymbols) {
        double x = ks.xPos * _spatium;
        double y = ks.line * step;
        item->drawSymbol(ks.sym, painter, PointF(x, y));
        // ledger lines
        double _symWidth = item->symWidth(ks.sym);
        double x1 = x - ledgerExtraLen;
        double x2 = x + _symWidth + ledgerExtraLen;
        painter->setPen(Pen(item->curColor(), ledgerLineWidth, PenStyle::SolidLine, PenCapStyle::FlatCap));
        for (int i = -2; i >= ks.line; i -= 2) { // above
            y = i * step;
            painter->drawLine(LineF(x1, y, x2, y));
        }
        for (int i = lines * 2; i <= ks.line; i += 2) { // below
            y = i * step;
            painter->drawLine(LineF(x1, y, x2, y));
        }
    }
}

void TDraw::draw(const LaissezVibSegment* item, muse::draw::Painter* painter)
{
    const LaissezVibSegment::LayoutData* ldata = item->ldata();
    if (item->score()->style().styleB(Sid::laissezVibUseSmuflSym)) {
        painter->setPen(item->curColor());
        item->drawSymbol(ldata->symbol, painter);
    } else {
        draw(static_cast<const TieSegment*>(item), painter);
    }
}

void TDraw::draw(const Lasso* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    const Lasso::LayoutData* ldata = item->ldata();
    painter->setBrush(Brush(item->configuration()->lassoColor()));
    // always 2 pixel width
    double w = 2.0 / painter->worldTransform().m11() * item->configuration()->guiScaling();
    painter->setPen(Pen(item->configuration()->selectionColor(), w));
    painter->drawRect(ldata->bbox());
}

void TDraw::draw(const LayoutBreak* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    if (item->score()->printing() || !item->score()->showUnprintable()) {
        return;
    }

    Pen pen(item->selected() ? item->configuration()->selectionColor() : item->configuration()->formattingColor());
    painter->setPen(pen);

    Font f(item->font());
    f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
    painter->setFont(f);

    painter->drawSymbol(PointF(), item->iconCode());
}

void TDraw::draw(const LedgerLine* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    if (item->chord()->crossMeasure() == CrossMeasure::SECOND) {
        return;
    }

    const LedgerLine::LayoutData* ldata = item->ldata();

    painter->setPen(Pen(item->curColor(), ldata->lineWidth, PenStyle::SolidLine, PenCapStyle::FlatCap));
    if (item->vertical()) {
        painter->drawLine(LineF(0.0, 0.0, 0.0, item->len()));
    } else {
        painter->drawLine(LineF(0.0, 0.0, item->len(), 0.0));
    }
}

void TDraw::draw(const LetRingSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextLineBaseSegment(item, painter);
}

void TDraw::draw(const Lyrics* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void TDraw::draw(const LyricsLineSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    Pen pen(item->lyricsLine()->lyrics()->curColor());
    pen.setWidthF(item->absoluteFromSpatium(item->lineWidth()));
    pen.setCapStyle(PenCapStyle::FlatCap);
    painter->setPen(pen);
    for (const LineF& dash : item->ldata()->dashes()) {
        painter->drawLine(dash);
    }
}

void TDraw::draw(const Marker* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void TDraw::draw(const MeasureNumber* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void TDraw::draw(const MeasureRepeat* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    const MeasureRepeat::LayoutData* ldata = item->ldata();

    painter->setPen(item->curColor());
    item->drawSymbol(ldata->symId, painter);

    if (!ldata->numberSym.empty()) {
        PointF numberPos = item->numberPosition(item->symBbox(ldata->numberSym));
        item->drawSymbols(ldata->numberSym, painter, numberPos);
    }

    if (item->style().styleB(Sid::fourMeasureRepeatShowExtenders) && item->numMeasures() == 4) {
        // TODO: add style settings specific to measure repeats
        // for now, using thickness and margin same as mmrests
        double hBarThickness = item->style().styleMM(Sid::mmRestHBarThickness);
        if (hBarThickness) {     // don't draw at all if 0, QPainter interprets 0 pen width differently
            Pen pen(painter->pen());
            pen.setCapStyle(PenCapStyle::FlatCap);
            pen.setWidthF(hBarThickness);
            painter->setPen(pen);

            double twoMeasuresWidth = 2 * item->measure()->width();
            double margin = item->style().styleMM(Sid::multiMeasureRestMargin);
            double xOffset = item->symBbox(ldata->symId).width() * .5;
            double gapDistance = (item->symBbox(ldata->symId).width() + item->spatium()) * .5;
            painter->drawLine(LineF(-twoMeasuresWidth + xOffset + margin, 0.0, xOffset - gapDistance, 0.0));
            painter->drawLine(LineF(xOffset + gapDistance, 0.0, twoMeasuresWidth + xOffset - margin, 0.0));
        }
    }
}

void TDraw::draw(const MMRest* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    if (item->shouldNotBeDrawn() || (item->track() % VOICES)) {     //only on voice 1
        return;
    }

    const MMRest::LayoutData* ldata = item->ldata();

    double _spatium = item->spatium();

    // draw number
    painter->setPen(item->curColor());
    RectF numberBox = item->symBbox(ldata->numberSym);
    PointF numberPos = item->numberPos();
    if (item->shouldShowNumber()) {
        item->drawSymbols(ldata->numberSym, painter, numberPos);
    }

    numberBox.translate(numberPos);

    if (item->isOldStyle()) {
        // draw rest symbols
        double x = (ldata->restWidth - ldata->symsWidth) * 0.5;
        double spacing = item->style().styleMM(Sid::mmRestOldStyleSpacing);
        for (SymId sym : ldata->restSyms) {
            double y = (sym == SymId::restWhole ? -_spatium : 0);
            item->drawSymbol(sym, painter, PointF(x, y));
            x += item->symBbox(sym).width() + spacing;
        }
    } else {
        double mag = item->staff()->staffMag(item->tick());
        Pen pen(painter->pen());
        pen.setCapStyle(PenCapStyle::FlatCap);

        // draw horizontal line
        double hBarThickness = item->style().styleMM(Sid::mmRestHBarThickness) * mag;
        if (hBarThickness) { // don't draw at all if 0, QPainter interprets 0 pen width differently
            pen.setWidthF(hBarThickness);
            painter->setPen(pen);
            double halfHBarThickness = hBarThickness * .5;
            if (item->numberVisible() // avoid painting line through number
                && item->style().styleB(Sid::mmRestNumberMaskHBar)
                && numberBox.bottom() >= -halfHBarThickness
                && numberBox.top() <= halfHBarThickness) {
                double gapDistance = (numberBox.width() + _spatium) * .5;
                double midpoint = ldata->restWidth * .5;
                painter->drawLine(LineF(0.0, 0.0, midpoint - gapDistance, 0.0));
                painter->drawLine(LineF(midpoint + gapDistance, 0.0, ldata->restWidth, 0.0));
            } else {
                painter->drawLine(LineF(0.0, 0.0, ldata->restWidth, 0.0));
            }
        }

        // draw vertical lines
        double vStrokeThickness = item->style().styleMM(Sid::mmRestHBarVStrokeThickness) * mag;
        if (vStrokeThickness) { // don't draw at all if 0, QPainter interprets 0 pen width differently
            pen.setWidthF(vStrokeThickness);
            painter->setPen(pen);
            double halfVStrokeHeight = item->style().styleMM(Sid::mmRestHBarVStrokeHeight) * .5 * mag;
            painter->drawLine(LineF(0.0, -halfVStrokeHeight, 0.0, halfVStrokeHeight));
            painter->drawLine(LineF(ldata->restWidth, -halfVStrokeHeight, ldata->restWidth, halfVStrokeHeight));
        }
    }
}

void TDraw::draw(const MMRestRange* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void TDraw::draw(const Note* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    if (item->hidden()) {
        return;
    }

    const Note::LayoutData* ldata = item->ldata();

    auto config = item->configuration();

    bool negativeFret = item->negativeFretUsed() && item->staff()->isTabStaff(item->tick());

    Color c(negativeFret ? config->criticalColor() : item->curColor());
    painter->setPen(c);
    bool tablature = item->staff() && item->staff()->isTabStaff(item->chord()->tick());

    // tablature
    if (tablature) {
        if (item->displayFret() == Note::DisplayFretOption::Hide || item->shouldHideFret()) {
            return;
        }
        const Staff* st = item->staff();
        const StaffType* tab = st->staffTypeForElement(item);
        // draw background, if required (to hide a segment of string line or to show a fretting conflict)
        if (!tab->linesThrough() || item->fretConflict()) {
            double d  = item->style().styleS(Sid::tabFretPadding).val() * item->spatium();
            RectF bb = RectF(ldata->bbox().x() - d,
                             tab->fretMaskY() * item->magS(),
                             ldata->bbox().width() + 2 * d,
                             tab->fretMaskH() * item->magS()
                             );

            // we do not know which viewer did this draw() call
            // so update all:
            if (!item->score()->getViewer().empty()) {
                for (MuseScoreView* view : item->score()->getViewer()) {
                    view->drawBackground(painter, bb);
                }
            } else {
                painter->fillRect(bb, config->noteBackgroundColor());
            }

            if (item->fretConflict() && !item->score()->printing() && item->score()->showUnprintable()) {                //on fret conflict, draw on red background
                painter->save();
                painter->setPen(config->criticalColor());
                painter->setBrush(config->criticalColor());
                painter->drawRect(bb);
                painter->restore();
            }
        }
        Font f(tab->fretFont());
        f.setPointSizeF(f.pointSizeF() * item->magS() * MScore::pixelRatio);
        painter->setFont(f);
        painter->setPen(c);
        double startPosX = ldata->bbox().x();

        const MStyle& style = item->style();
        double yOffset = tab->fretFontYOffset(style);
        painter->drawText(PointF(startPosX, yOffset * item->magS()), item->fretString());
    }
    // NOT tablature
    else {
        // skip drawing, if second note of a cross-measure value
        if (item->chord() && item->chord()->crossMeasure() == CrossMeasure::SECOND) {
            return;
        }
        // warn if pitch extends usable range of instrument
        // by coloring the notehead
        if (item->chord() && item->chord()->segment() && item->staff()
            && !item->score()->printing() && MScore::warnPitchRange && !item->staff()->isDrumStaff(item->chord()->tick())) {
            const Instrument* in = item->part()->instrument(item->chord()->tick());
            int i = item->ppitch();
            if (i < in->minPitchP() || i > in->maxPitchP()) {
                painter->setPen(
                    item->selected() ? config->criticalSelectedColor() : config->criticalColor());
            } else if (i < in->minPitchA() || i > in->maxPitchA()) {
                painter->setPen(item->selected() ? config->warningSelectedColor() : config->warningColor());
            }
        }
        // Warn if notes are unplayable based on previous harp diagram setting
        if (item->chord() && item->chord()->segment() && item->staff() && !item->score()->printing()
            && !item->staff()->isDrumStaff(item->chord()->tick())) {
            HarpPedalDiagram* prevDiagram = item->part()->currentHarpDiagram(item->chord()->segment()->tick());
            if (prevDiagram && !prevDiagram->isTpcPlayable(item->tpc())) {
                painter->setPen(item->selected() ? config->criticalSelectedColor() : config->criticalColor());
            }
        }
        // draw blank notehead to avoid staff and ledger lines
        if (ldata->cachedSymNull.value() != SymId::noSym) {
            painter->save();
            painter->setPen(config->noteBackgroundColor());
            item->drawSymbol(ldata->cachedSymNull.value(), painter);
            painter->restore();
        }
        item->drawSymbol(ldata->cachedNoteheadSym.value(), painter);
    }
}

void TDraw::draw(const NoteDot* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    if (item->note() && item->note()->dotsHidden()) {     // don't draw dot if note is hidden
        return;
    } else if (item->rest() && item->rest()->isGap()) {  // don't draw dot for gap rests
        return;
    }
    const Note* n = item->note();
    Fraction tick = n ? n->chord()->tick() : item->rest()->tick();
    // always draw dot for non-tab
    // for tab, draw if on a note and stems through staff or on a rest and rests shown
    if (!item->staff()->isTabStaff(tick)
        || (n && item->staff()->staffType(tick)->stemThrough())
        || (!n && item->staff()->staffType(tick)->showRests())) {
        painter->setPen(item->curColor());
        item->drawSymbol(SymId::augmentationDot, painter);
    }
}

void TDraw::draw(const NoteHead* item, Painter* painter)
{
    draw(static_cast<const Symbol*>(item), painter);
}

void TDraw::draw(const NoteLineSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextLineBaseSegment(item, painter);
}

void TDraw::draw(const OttavaSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextLineBaseSegment(item, painter);
}

void TDraw::draw(const Page* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    bool shouldDraw = item->score()->isLayoutMode(LayoutMode::PAGE) || item->score()->isLayoutMode(LayoutMode::FLOAT);
    if (!shouldDraw) {
        return;
    }
    //
    // draw header/footer
    //

    page_idx_t n = item->no() + 1 + item->score()->pageNumberOffset();
    painter->setPen(item->curColor());

    auto drawHeaderFooter = [item](Painter* p, int area, const String& ss)
    {
        Text* text = item->layoutHeaderFooter(area, ss);
        if (!text) {
            return;
        }
        p->translate(text->pos());
        draw(text, p);
        p->translate(-text->pos());
        text->resetExplicitParent();
    };

    String s1, s2, s3;

    if (item->style().styleB(Sid::showHeader) && (item->no() || item->style().styleB(Sid::headerFirstPage))) {
        bool odd = (n & 1) || !item->style().styleB(Sid::headerOddEven);
        if (odd) {
            s1 = item->style().styleSt(Sid::oddHeaderL);
            s2 = item->style().styleSt(Sid::oddHeaderC);
            s3 = item->style().styleSt(Sid::oddHeaderR);
        } else {
            s1 = item->style().styleSt(Sid::evenHeaderL);
            s2 = item->style().styleSt(Sid::evenHeaderC);
            s3 = item->style().styleSt(Sid::evenHeaderR);
        }

        drawHeaderFooter(painter, 0, s1);
        drawHeaderFooter(painter, 1, s2);
        drawHeaderFooter(painter, 2, s3);
    }

    if (item->style().styleB(Sid::showFooter) && (item->no() || item->style().styleB(Sid::footerFirstPage))) {
        bool odd = (n & 1) || !item->style().styleB(Sid::footerOddEven);
        if (odd) {
            s1 = item->style().styleSt(Sid::oddFooterL);
            s2 = item->style().styleSt(Sid::oddFooterC);
            s3 = item->style().styleSt(Sid::oddFooterR);
        } else {
            s1 = item->style().styleSt(Sid::evenFooterL);
            s2 = item->style().styleSt(Sid::evenFooterC);
            s3 = item->style().styleSt(Sid::evenFooterR);
        }

        drawHeaderFooter(painter, 3, s1);
        drawHeaderFooter(painter, 4, s2);
        drawHeaderFooter(painter, 5, s3);
    }
}

void TDraw::draw(const PalmMuteSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextLineBaseSegment(item, painter);
}

void TDraw::draw(const PedalSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextLineBaseSegment(item, painter);
}

void TDraw::draw(const PickScrapeSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextLineBaseSegment(item, painter);
}

void TDraw::draw(const PlayTechAnnotation* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void TDraw::draw(const RasgueadoSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextLineBaseSegment(item, painter);
}

void TDraw::draw(const RehearsalMark* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void TDraw::draw(const Rest* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    if (item->shouldNotBeDrawn()) {
        return;
    }

    const Rest::LayoutData* ldata = item->ldata();

    painter->setPen(item->curColor());
    item->drawSymbol(ldata->sym, painter);
}

//! NOTE May be removed later (should be only single mode)
void TDraw::draw(const ShadowNote* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    if (!item->visible() || !item->isValid()) {
        return;
    }

    PointF ap(item->pagePos());
    painter->translate(ap);
    double lw = item->style().styleMM(Sid::stemWidth) * item->mag();
    Pen pen(item->configuration()->highlightSelectionColor(item->voice()), lw, PenStyle::SolidLine, PenCapStyle::FlatCap);
    painter->setPen(pen);

    bool up = item->computeUp();

    // Draw the accidental
    SymId acc = Accidental::subtype2symbol(item->score()->inputState().accidentalType());
    if (acc != SymId::noSym) {
        PointF posAcc;
        posAcc.rx() -= item->symWidth(acc) + item->style().styleMM(Sid::accidentalNoteDistance) * item->mag();
        item->drawSymbol(acc, painter, posAcc);
    }

    // Draw the notehead
    item->drawSymbol(item->noteheadSymbol(), painter);

    // Draw the dots
    double sp = item->spatium();
    double sp2 = sp / 2;
    double noteheadWidth = item->symWidth(item->noteheadSymbol());

    PointF posDot;
    if (item->duration().dots() > 0) {
        double d  = item->style().styleMM(Sid::dotNoteDistance) * item->mag();
        double dd = item->style().styleMM(Sid::dotDotDistance) * item->mag();
        posDot.rx() += (noteheadWidth + d);

        if (item->isRest()) {
            posDot.ry() += Rest::getDotline(item->duration().type()) * sp2;
        } else {
            posDot.ry() -= (item->lineIndex() % 2 == 0 ? sp2 : 0);
        }

        if (item->hasFlag() && up) {
            posDot.rx() = std::max(posDot.x(), noteheadWidth + item->symBbox(item->flagSym()).right());
        }

        for (int i = 0; i < item->duration().dots(); i++) {
            posDot.rx() += dd * i;
            item->drawSymbol(SymId::augmentationDot, painter, posDot);
            posDot.rx() -= dd * i;
        }
    }

    // Draw stem and flag
    if (item->hasStem()) {
        double x = up ? (noteheadWidth - (lw / 2)) : lw / 2;
        double y1 = item->symSmuflAnchor(item->noteheadSymbol(), up ? SmuflAnchorId::stemUpSE : SmuflAnchorId::stemDownNW).y();
        double y2 = (up ? -3.5 : 3.5) * sp;

        if (item->hasFlag()) {
            SymId flag = item->flagSym();
            item->drawSymbol(flag, painter, PointF(x - (lw / 2), y2));
            y2 += item->symSmuflAnchor(flag, up ? SmuflAnchorId::stemUpNW : SmuflAnchorId::stemDownSW).y();
        }
        painter->drawLine(LineF(x, y1, x, y2));
    }

    // Draw ledger lines if needed
    if (item->ledgerLinesVisible()) {
        double extraLen = item->style().styleS(Sid::ledgerLineLength).val() * sp;
        double x1 = -extraLen;
        double x2 = noteheadWidth + extraLen;
        double yOffset = item->staffOffsetY();
        double step = sp2 * item->staffType()->lineDistance().val();

        lw = item->style().styleMM(Sid::ledgerLineWidth) * item->mag();
        pen.setWidthF(lw);
        painter->setPen(pen);

        const int topLine = -2 + yOffset / step;
        for (int i = topLine; i >= item->lineIndex(); i -= 2) {
            double y = step * (i - item->lineIndex());
            painter->drawLine(LineF(x1, y, x2, y));
        }
        int l = item->staffType()->lines() * 2 + yOffset / step; // first ledger line below staff
        for (int i = l; i <= item->lineIndex(); i += 2) {
            double y = step * (i - item->lineIndex());
            painter->drawLine(LineF(x1, y, x2, y));
        }
    }

    item->drawArticulations(painter);

    painter->translate(-ap);
}

void TDraw::draw(const SlurSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    Pen pen(item->curColor(item->getProperty(Pid::VISIBLE).toBool(), item->getProperty(Pid::COLOR).value<Color>()));
    double mag = item->staff() ? item->staff()->staffMag(item->slur()->tick()) : 1.0;

    //Replace generic Qt dash patterns with improved equivalents to show true dots (keep in sync with tie.cpp)
    std::vector<double> dotted     = { 0.01, 1.99 };   // tighter than Qt PenStyle::DotLine equivalent - would be { 0.01, 2.99 }
    std::vector<double> dashed     = { 3.00, 3.00 };   // Compensating for caps. Qt default PenStyle::DashLine is { 4.0, 2.0 }
    std::vector<double> wideDashed = { 5.00, 6.00 };

    switch (item->slurTie()->styleType()) {
    case SlurStyleType::Solid:
        painter->setBrush(Brush(pen.color()));
        pen.setCapStyle(PenCapStyle::RoundCap);
        pen.setJoinStyle(PenJoinStyle::RoundJoin);
        pen.setWidthF(item->endWidth() * mag);
        break;
    case SlurStyleType::Dotted:
        painter->setBrush(BrushStyle::NoBrush);
        pen.setCapStyle(PenCapStyle::RoundCap);           // round dots
        pen.setDashPattern(dotted);
        pen.setWidthF(item->dottedWidth() * mag);
        break;
    case SlurStyleType::Dashed:
        painter->setBrush(BrushStyle::NoBrush);
        pen.setDashPattern(dashed);
        pen.setWidthF(item->dottedWidth() * mag);
        break;
    case SlurStyleType::WideDashed:
        painter->setBrush(BrushStyle::NoBrush);
        pen.setDashPattern(wideDashed);
        pen.setWidthF(item->dottedWidth() * mag);
        break;
    case SlurStyleType::Undefined:
        break;
    }
    painter->setPen(pen);
    painter->drawPath(item->ldata()->path());
}

void TDraw::draw(const Spacer* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    if (item->score()->printing() || !item->score()->showUnprintable()) {
        return;
    }

    auto conf = item->configuration();

    Pen pen(item->selected() ? conf->selectionColor() : conf->formattingColor(), item->spatium()* 0.3);

    painter->setPen(pen);
    painter->setBrush(BrushStyle::NoBrush);
    painter->drawPath(item->path());
}

void TDraw::draw(const StaffLines* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    painter->setPen(Pen(item->curColor(), item->lw(), PenStyle::SolidLine, PenCapStyle::FlatCap));
    painter->drawLines(item->lines());
}

void TDraw::draw(const StaffState* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    if (item->score()->printing() || !item->score()->showUnprintable()) {
        return;
    }

    const StaffState::LayoutData* ldata = item->ldata();
    auto conf = item->configuration();

    Pen pen(item->selected() ? conf->selectionColor() : conf->formattingColor(),
            ldata->lw, PenStyle::SolidLine, PenCapStyle::RoundCap, PenJoinStyle::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(BrushStyle::NoBrush);
    painter->drawPath(ldata->path);
}

void TDraw::draw(const StaffText* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    drawTextBase(item, painter);

    if (item->hasSoundFlag()) {
        draw(item->soundFlag(), painter);
    }
}

void TDraw::draw(const StaffTypeChange* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    if (item->score()->printing() || !item->score()->showUnprintable()) {
        return;
    }

    auto conf = item->configuration();

    double _spatium = item->style().spatium();
    double h  = _spatium * 2.5;
    double w  = _spatium * 2.5;
    double lineDist = 0.35;           // line distance for the icon 'staff lines'
    // draw icon rectangle
    painter->setPen(Pen(item->selected() ? conf->selectionColor() : conf->formattingColor(),
                        item->lw(), PenStyle::SolidLine, PenCapStyle::SquareCap, PenJoinStyle::MiterJoin));
    painter->setBrush(BrushStyle::NoBrush);
    painter->drawRect(0, 0, w, h);

    // draw icon contents
    int lines = 5;
    if (item->staffType()) {
        if (item->staffType()->stemless()) {       // a single notehead represents a stemless staff
            item->drawSymbol(SymId::noteheadBlack, painter, PointF(w * 0.5 - 0.33 * _spatium, h * 0.5), 0.5);
        }
        if (item->staffType()->invisible()) {      // no lines needed. It's done.
            return;
        }
        // show up to 6 lines
        lines = std::min(item->staffType()->lines(), 6);
    }
    // calculate starting point Y for the lines from half the icon height (2.5) so staff lines appear vertically centered
    double startY = 1.25 - (lines - 1) * lineDist * 0.5;
    painter->setPen(Pen(item->selected() ? conf->selectionColor() : conf->formattingColor(),
                        2.5, PenStyle::SolidLine, PenCapStyle::SquareCap, PenJoinStyle::MiterJoin));
    for (int i=0; i < lines; i++) {
        int y = (startY + i * lineDist) * _spatium;
        painter->drawLine(0, y, w, y);
    }
}

void TDraw::draw(const Stem* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    if (!item->chord()) { // may be need assert?
        return;
    }

    // hide if second chord of a cross-measure pair
    if (item->chord()->crossMeasure() == CrossMeasure::SECOND) {
        return;
    }

    const Stem::LayoutData* ldata = item->ldata();

    const Staff* staff = item->staff();
    const StaffType* staffType = staff ? staff->staffTypeForElement(item->chord()) : nullptr;
    const bool isTablature = staffType && staffType->isTabStaff();

    painter->setPen(Pen(item->curColor(), item->lineWidthMag(), PenStyle::SolidLine, PenCapStyle::FlatCap));
    painter->drawLine(ldata->line);

    if (!isTablature) {
        return;
    }

    // TODO: adjust bounding rectangle in layout() for dots and for slash
    double sp = item->spatium();
    bool isUp = item->up();

    // slashed half note stem
    if (item->chord()->durationType().type() == DurationType::V_HALF
        && staffType->minimStyle() == TablatureMinimStyle::SLASHED) {
        // position slashes onto stem
        double y = isUp ? -item->length() + STAFFTYPE_TAB_SLASH_2STARTY_UP * sp
                   : item->length() - STAFFTYPE_TAB_SLASH_2STARTY_DN * sp;
        // if stems through, try to align slashes within or across lines
        if (staffType->stemThrough()) {
            double halfLineDist = staffType->lineDistance().val() * sp * 0.5;
            double halfSlashHgt = STAFFTYPE_TAB_SLASH_2TOTHEIGHT * sp * 0.5;
            y = lrint((y + halfSlashHgt) / halfLineDist) * halfLineDist - halfSlashHgt;
        }
        // draw slashes
        double hlfWdt= sp * STAFFTYPE_TAB_SLASH_WIDTH * 0.5;
        double sln   = sp * STAFFTYPE_TAB_SLASH_SLANTY;
        double thk   = sp * STAFFTYPE_TAB_SLASH_THICK;
        double displ = sp * STAFFTYPE_TAB_SLASH_DISPL;
        PainterPath path;
        for (int i = 0; i < 2; ++i) {
            path.moveTo(hlfWdt, y);                   // top-right corner
            path.lineTo(hlfWdt, y + thk);             // bottom-right corner
            path.lineTo(-hlfWdt, y + thk + sln);      // bottom-left corner
            path.lineTo(-hlfWdt, y + sln);            // top-left corner
            path.closeSubpath();
            y += displ;
        }
        painter->setBrush(Brush(item->curColor()));
        painter->setNoPen();
        painter->drawPath(path);
    }

    // dots
    // NOT THE BEST PLACE FOR THIS?
    // with tablatures and stems beside staves, dots are not drawn near 'notes', but near stems
    int nDots = item->chord()->dots();
    if (nDots > 0 && !staffType->stemThrough()) {
        double x     = item->chord()->dotPosX();
        double y     = ((STAFFTYPE_TAB_DEFAULTSTEMLEN_DN * 0.2) * sp) * (isUp ? -1.0 : 1.0);
        double step  = item->style().styleS(Sid::dotDotDistance).val() * sp;
        for (int dot = 0; dot < nDots; dot++, x += step) {
            item->drawSymbol(SymId::augmentationDot, painter, PointF(x, y));
        }
    }
}

void TDraw::draw(const StemSlash* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    const StemSlash::LayoutData* ldata = item->ldata();
    painter->setPen(Pen(item->curColor(), ldata->stemWidth, PenStyle::SolidLine, PenCapStyle::FlatCap));
    painter->drawLine(ldata->line);
}

void TDraw::draw(const Sticking* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void TDraw::draw(const StringTunings* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    if (item->noStringVisible()) {
        const TextBase::LayoutData* data = item->ldata();

        double spatium = item->spatium();
        double lineWidth = spatium * .15;

        Pen pen(item->curColor(), lineWidth, PenStyle::SolidLine, PenCapStyle::RoundCap, PenJoinStyle::RoundJoin);
        painter->setPen(pen);
        painter->setBrush(Brush(item->curColor()));

        Font f(item->font());
        painter->setFont(f);

        RectF rect = data->bbox();

        double x = rect.x();
        double y = rect.y();
        double width = rect.width();
        double height = rect.height();
        double topPartHeight = height * .66;
        double cornerRadius = 8.0;

        PainterPath path;
        path.moveTo(x, y);
        path.arcTo(x, y + (topPartHeight - 2 * cornerRadius), 2 * cornerRadius, 2 * cornerRadius, 180.0, 90.0);
        path.arcTo(x + width - 2 * cornerRadius, y + (topPartHeight - 2 * cornerRadius), 2 * cornerRadius, 2 * cornerRadius, 270, 90);
        path.lineTo(x + width, y);
        path.moveTo(x + width / 2, y + topPartHeight);
        path.lineTo(x + width / 2, y + height);

        painter->setBrush(BrushStyle::NoBrush);
        painter->drawPath(path);
    } else {
        drawTextBase(item, painter);
    }
}

void TDraw::draw(const Symbol* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    bool tabStaff = item->staff() ? item->staff()->isTabStaff(item->tick()) : false;
    if (tabStaff && (item->sym() == SymId::noteheadParenthesisLeft || item->sym() == SymId::noteheadParenthesisRight)) {
        // Draw background for parentheses on TAB staves
        auto config = item->configuration();
        const Symbol::LayoutData* ldata = item->ldata();
        double d = item->style().styleS(Sid::tabFretPadding).val() * item->spatium();
        RectF bb = RectF(ldata->bbox().x() - d,
                         ldata->bbox().y() - d,
                         ldata->bbox().width() + 2 * d,
                         ldata->bbox().height() + 2 * d
                         );
        if (!item->score()->getViewer().empty()) {
            for (MuseScoreView* view : item->score()->getViewer()) {
                view->drawBackground(painter, bb);
            }
        } else {
            painter->fillRect(bb, config->noteBackgroundColor());
        }
    }

    if (!item->isNoteDot() || !tabStaff) {
        painter->setPen(item->curColor());
        if (item->scoreFont()) {
            item->scoreFont()->draw(item->sym(), painter, item->magS() * item->symbolsSize(), PointF(), item->symAngle());
        } else {
            item->drawSymbol(item->sym(), painter);
        }
    }
}

void TDraw::draw(const FSymbol* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    Font f(item->font());
    f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
    painter->setFont(f);
    painter->setPen(item->curColor());
    painter->drawText(PointF(0, 0), item->toString());
}

void TDraw::draw(const SystemDivider* item, Painter* painter)
{
    draw(static_cast<const Symbol*>(item), painter);
}

void TDraw::draw(const SystemText* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void TDraw::draw(const SystemLockIndicator* item, muse::draw::Painter* painter)
{
    TRACE_DRAW_ITEM;

    if (item->score()->printing() || !item->score()->showUnprintable()) {
        return;
    }

    Pen pen(item->selected() ? item->configuration()->selectionColor() : item->configuration()->formattingColor());
    painter->setPen(pen);

    Font f(item->font());
    f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
    painter->setFont(f);

    painter->drawSymbol(PointF(), item->iconCode());

    if (item->selected()) {
        Color lockedAreaColor = item->configuration()->selectionColor();
        lockedAreaColor.setAlpha(38);
        Brush brush(lockedAreaColor);
        painter->setBrush(brush);
        painter->setNoPen();
        double radius = 0.5 * item->spatium();

        painter->drawRoundedRect(item->ldata()->rangeRect, radius, radius);
    }
}

void TDraw::draw(const SoundFlag* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    if (item->shouldHide()) {
        return;
    }

    painter->setNoPen();
    painter->setBrush(item->iconBackgroundColor());
    painter->drawEllipse(item->ldata()->bbox());

    Font f(item->iconFont());
    f.setPointSizeF(item->spatium() * 2.0);
    painter->setFont(f);

    painter->setPen(!item->selected() ? item->curColor() : Color::WHITE);
    painter->drawText(item->ldata()->bbox(), muse::draw::AlignCenter, Char(item->iconCode()));
}

void TDraw::draw(const TabDurationSymbol* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    if (!item->tab()) {
        return;
    }

    const TabDurationSymbol::LayoutData* ldata = item->ldata();

    if (item->isRepeat() && (item->tab()->symRepeat() == TablatureSymbolRepeat::SYSTEM)) {
        Chord* chord = toChord(item->explicitParent());
        ChordRest* prevCR = prevChordRest(chord);
        if (prevCR && (chord->measure()->system() == prevCR->measure()->system())) {
            return;
        }
    }

    double mag = item->magS();
    double imag = 1.0 / mag;

    Pen pen(item->curColor());
    painter->setPen(pen);
    painter->scale(mag, mag);
    if (ldata->beamGrid == TabBeamGrid::NONE) {
        // if no beam grid, draw symbol
        Font f(item->tab()->durationFont());
        f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
        painter->setFont(f);
        painter->drawText(PointF(0.0, 0.0), item->text());
    } else {
        // if beam grid, draw stem line
        const TablatureDurationFont& font = item->tab()->tabDurationFont();
        double _spatium = item->spatium();
        pen.setCapStyle(PenCapStyle::FlatCap);
        pen.setWidthF(font.gridStemWidth * _spatium);
        painter->setPen(pen);
        // take stem height from bbox, but de-magnify it, as drawing is already magnified
        double h = ldata->bbox().y() / mag;
        painter->drawLine(PointF(0.0, h), PointF(0.0, 0.0));
        // if beam grid is medial/final, draw beam lines too: lines go from mid of
        // previous stem (delta x stored in _beamLength) to mid of this' stem (0.0)
        if (ldata->beamGrid == TabBeamGrid::MEDIALFINAL) {
            pen.setWidthF(font.gridBeamWidth * _spatium);
            painter->setPen(pen);
            // lower height available to beams by half a beam width,
            // so that top beam upper border aligns with stem top
            h += (font.gridBeamWidth * _spatium) * 0.5;
            // draw beams equally spaced within the stem height (this is
            // different from modern engraving, but common in historic prints)
            double step  = -h / ldata->beamLevel;
            double y     = h;
            for (int i = 0; i < ldata->beamLevel; i++, y += step) {
                painter->drawLine(PointF(ldata->beamLength, y), PointF(0.0, y));
            }
        }
    }
    painter->scale(imag, imag);
}

void TDraw::draw(const TempoText* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void TDraw::draw(const Text* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void TDraw::draw(const TextLineSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextLineBaseSegment(item, painter);
}

void TDraw::draw(const TieSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    // hide tie toward the second chord of a cross-measure value
    if (item->tie()->endNote() && item->tie()->endNote()->chord()->crossMeasure() == CrossMeasure::SECOND) {
        return;
    }

    Pen pen(item->curColor(item->getProperty(Pid::VISIBLE).toBool(), item->getProperty(Pid::COLOR).value<Color>()));
    double mag = item->staff() ? item->staff()->staffMag(item->tie()->tick()) : 1.0;

    //Replace generic Qt dash patterns with improved equivalents to show true dots (keep in sync with slur.cpp)
    std::vector<double> dotted     = { 0.01, 1.99 };   // tighter than Qt PenStyle::DotLine equivalent - would be { 0.01, 2.99 }
    std::vector<double> dashed     = { 3.00, 3.00 };   // Compensating for caps. Qt default PenStyle::DashLine is { 4.0, 2.0 }
    std::vector<double> wideDashed = { 5.00, 6.00 };

    switch (item->slurTie()->styleType()) {
    case SlurStyleType::Solid:
        painter->setBrush(Brush(pen.color()));
        pen.setCapStyle(PenCapStyle::RoundCap);
        pen.setJoinStyle(PenJoinStyle::RoundJoin);
        pen.setWidthF(item->endWidth() * mag);
        break;
    case SlurStyleType::Dotted:
        painter->setBrush(BrushStyle::NoBrush);
        pen.setCapStyle(PenCapStyle::RoundCap);           // True dots
        pen.setDashPattern(dotted);
        pen.setWidthF(item->dottedWidth() * mag);
        break;
    case SlurStyleType::Dashed:
        painter->setBrush(BrushStyle::NoBrush);
        pen.setDashPattern(dashed);
        pen.setWidthF(item->dottedWidth() * mag);
        break;
    case SlurStyleType::WideDashed:
        painter->setBrush(BrushStyle::NoBrush);
        pen.setDashPattern(wideDashed);
        pen.setWidthF(item->dottedWidth() * mag);
        break;
    case SlurStyleType::Undefined:
        break;
    }
    painter->setPen(pen);
    painter->drawPath(item->ldata()->path());
}

void TDraw::draw(const TimeSig* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    if (item->staff() && !const_cast<const Staff*>(item->staff())->staffType(item->tick())->genTimesig()) {
        return;
    }
    painter->setPen(item->curColor());

    const TimeSig::LayoutData* ldata = item->ldata();

    item->drawSymbols(ldata->ns, painter, ldata->pz, item->scale());
    item->drawSymbols(ldata->ds, painter, ldata->pn, item->scale());

    if (item->largeParentheses()) {
        item->drawSymbol(SymId::timeSigParensLeft,  painter, ldata->pointLargeLeftParen,  item->scale().width());
        item->drawSymbol(SymId::timeSigParensRight, painter, ldata->pointLargeRightParen, item->scale().width());
    }
}

void TDraw::draw(const TimeTickAnchor* item, Painter* painter)
{
    TimeTickAnchor::DrawRegion drawRegion = item->drawRegion();

    if (drawRegion == TimeTickAnchor::DrawRegion::OUT_OF_RANGE) {
        return;
    }

    Color voiceColor = item->configuration()->voiceColor(item->voiceIdx());

    static constexpr double TINT_MAIN_DARKER = 0.45;
    static constexpr double TINT_MAIN_LIGHTER = 0.55;
    static constexpr double TINT_EXTENDED_DARKER = 0.75;
    static constexpr double TINT_EXTENDED_LIGHTER = 0.85;

    double tint = drawRegion == TimeTickAnchor::DrawRegion::MAIN_REGION
                  ? item->ldata()->darker() ? TINT_MAIN_DARKER : TINT_MAIN_LIGHTER
                  : item->ldata()->darker() ? TINT_EXTENDED_DARKER : TINT_EXTENDED_LIGHTER;

    voiceColor.applyTint(tint);

    Brush brush;
    brush.setColor(voiceColor);
    brush.setStyle(BrushStyle::SolidPattern);
    painter->setBrush(brush);
    painter->setNoPen();

    painter->drawRect(item->ldata()->bbox());
}

void TDraw::draw(const TremoloSingleChord* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    if (item->isBuzzRoll()) {
        painter->setPen(item->curColor());
        item->drawSymbol(SymId::buzzRoll, painter);
    } else {
        painter->setBrush(Brush(item->curColor()));
        painter->setNoPen();
        painter->drawPath(item->path());
    }
}

void TDraw::draw(const TremoloTwoChord* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    const TremoloTwoChord::LayoutData* ldata = item->ldata();

    if (!item->beamSegments().empty()) {
        // two-note trems act like beams

        // make beam thickness independent of slant
        // (expression can be simplified?)
        const LineF bs = item->beamSegments().front()->line;
        double d = (std::abs(bs.y2() - bs.y1())) / (bs.x2() - bs.x1());
        if (item->beamSegments().size() > 1 && d > M_PI / 6.0) {
            d = M_PI / 6.0;
        }
        double ww = (ldata->beamWidth / 2.0) / sin(M_PI_2 - atan(d));
        painter->setBrush(Brush(item->curColor()));
        painter->setNoPen();
        for (const BeamSegment* bs1 : item->beamSegments()) {
            painter->drawPolygon(
                PolygonF({
                PointF(bs1->line.x1(), bs1->line.y1() - ww),
                PointF(bs1->line.x2(), bs1->line.y2() - ww),
                PointF(bs1->line.x2(), bs1->line.y2() + ww),
                PointF(bs1->line.x1(), bs1->line.y1() + ww),
            }),
                FillRule::OddEvenFill);
        }
    }
}

void TDraw::draw(const TremoloBar* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    const TremoloBar::LayoutData* ldata = item->ldata();
    const double lw = item->absoluteFromSpatium(item->lineWidth());
    Pen pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::RoundCap, PenJoinStyle::RoundJoin);
    painter->setPen(pen);
    painter->drawPolyline(ldata->polygon);
}

void TDraw::draw(const TrillSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    painter->setPen(item->spanner()->curColor());
    item->drawSymbols(item->symbols(), painter);
}

void TDraw::draw(const TripletFeel* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void TDraw::draw(const Tuplet* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    // if in a TAB without stems, tuplets are not shown
    const StaffType* stt = item->staffType();
    if (stt && stt->isTabStaff() && stt->stemless()) {
        return;
    }

    Color color(item->curColor());
    if (item->number()) {
        painter->setPen(color);
        PointF pos(item->number()->pos());
        painter->translate(pos);
        draw(item->number(), painter);
        painter->translate(-pos);
    }
    if (item->hasBracket()) {
        Pen pen(color, item->absoluteFromSpatium(item->bracketWidth()));
        pen.setJoinStyle(PenJoinStyle::MiterJoin);
        pen.setCapStyle(PenCapStyle::FlatCap);
        painter->setPen(pen);
        if (!item->number()) {
            painter->drawPolyline(item->bracketL, 4);
        } else {
            painter->drawPolyline(item->bracketL, 3);
            painter->drawPolyline(item->bracketR, 3);
        }
    }
}

void TDraw::draw(const VibratoSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    painter->setPen(item->spanner()->curColor());
    item->drawSymbols(item->symbols(), painter);
}

void TDraw::draw(const VoltaSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextLineBaseSegment(item, painter);
}

void TDraw::draw(const WhammyBarSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextLineBaseSegment(item, painter);
}

// dev
void TDraw::draw(const System* item, Painter* painter)
{
    UNUSED(item);
    UNUSED(painter);
    //painter->drawRect(item->ldata()->bbox());
}

void TDraw::draw(const Measure* item, Painter* painter)
{
    UNUSED(item);
    UNUSED(painter);
    //painter->drawRect(item->ldata()->bbox());
}

void TDraw::draw(const Segment* item, Painter* painter)
{
    UNUSED(item);
    UNUSED(painter);
    //painter->drawRect(item->ldata()->bbox());
}

void TDraw::draw(const Chord* item, Painter* painter)
{
    UNUSED(item);
    UNUSED(painter);
    //painter->drawRect(item->ldata()->bbox());
}
