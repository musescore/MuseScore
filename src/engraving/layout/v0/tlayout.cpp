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

#include "tlayout.h"

#include "global/realfn.h"
#include "draw/fontmetrics.h"

#include "../iengravingconfiguration.h"
#include "../infrastructure/rtti.h"

#include "../iengravingfont.h"
#include "../types/typesconv.h"
#include "../types/symnames.h"
#include "../libmscore/score.h"
#include "../libmscore/utils.h"

#include "../libmscore/accidental.h"
#include "../libmscore/actionicon.h"
#include "../libmscore/ambitus.h"
#include "../libmscore/arpeggio.h"
#include "../libmscore/articulation.h"

#include "../libmscore/bagpembell.h"
#include "../libmscore/barline.h"
#include "../libmscore/beam.h"
#include "../libmscore/bend.h"
#include "../libmscore/box.h"
#include "../libmscore/bracket.h"
#include "../libmscore/breath.h"

#include "../libmscore/chord.h"
#include "../libmscore/chordline.h"
#include "../libmscore/clef.h"
#include "../libmscore/capo.h"

#include "../libmscore/deadslapped.h"
#include "../libmscore/dynamic.h"

#include "../libmscore/expression.h"

#include "../libmscore/fermata.h"
#include "../libmscore/figuredbass.h"
#include "../libmscore/fingering.h"
#include "../libmscore/fret.h"
#include "../libmscore/fretcircle.h"

#include "../libmscore/glissando.h"
#include "../libmscore/gradualtempochange.h"

#include "../libmscore/hairpin.h"
#include "../libmscore/harppedaldiagram.h"
#include "../libmscore/harmonicmark.h"
#include "../libmscore/harmony.h"
#include "../libmscore/hook.h"

#include "../libmscore/image.h"
#include "../libmscore/instrchange.h"
#include "../libmscore/instrumentname.h"

#include "../libmscore/jump.h"

#include "../libmscore/keysig.h"

#include "../libmscore/layoutbreak.h"
#include "../libmscore/ledgerline.h"
#include "../libmscore/letring.h"
#include "../libmscore/line.h"
#include "../libmscore/lyrics.h"

#include "../libmscore/marker.h"
#include "../libmscore/measurebase.h"
#include "../libmscore/measurenumber.h"
#include "../libmscore/measurenumberbase.h"
#include "../libmscore/measurerepeat.h"
#include "../libmscore/mmrest.h"
#include "../libmscore/mmrestrange.h"

#include "../libmscore/note.h"
#include "../libmscore/notedot.h"

#include "../libmscore/ornament.h"
#include "../libmscore/ottava.h"

#include "../libmscore/page.h"
#include "../libmscore/palmmute.h"
#include "../libmscore/part.h"
#include "../libmscore/pedal.h"
#include "../libmscore/pickscrape.h"
#include "../libmscore/playtechannotation.h"

#include "../libmscore/rasgueado.h"
#include "../libmscore/rehearsalmark.h"
#include "../libmscore/rest.h"

#include "../libmscore/shadownote.h"
#include "../libmscore/slur.h"
#include "../libmscore/spacer.h"
#include "../libmscore/staff.h"
#include "../libmscore/stafflines.h"
#include "../libmscore/staffstate.h"
#include "../libmscore/stafftext.h"
#include "../libmscore/stafftype.h"
#include "../libmscore/stafftypechange.h"
#include "../libmscore/stem.h"
#include "../libmscore/stemslash.h"
#include "../libmscore/sticking.h"
#include "../libmscore/stretchedbend.h"
#include "../libmscore/bsymbol.h"
#include "../libmscore/symbol.h"
#include "../libmscore/system.h"
#include "../libmscore/systemdivider.h"
#include "../libmscore/systemtext.h"

#include "../libmscore/tempotext.h"
#include "../libmscore/text.h"
#include "../libmscore/textframe.h"
#include "../libmscore/textline.h"
#include "../libmscore/tie.h"
#include "../libmscore/timesig.h"
#include "../libmscore/tremolo.h"
#include "../libmscore/tremolobar.h"
#include "../libmscore/trill.h"
#include "../libmscore/tripletfeel.h"
#include "../libmscore/tuplet.h"

#include "../libmscore/vibrato.h"
#include "../libmscore/volta.h"

#include "../libmscore/whammybar.h"

#include "arpeggiolayout.h"
#include "beamlayout.h"
#include "chordlayout.h"
#include "lyricslayout.h"
#include "slurtielayout.h"
#include "tremololayout.h"
#include "tupletlayout.h"

using namespace mu::draw;
using namespace mu::engraving;
using namespace mu::engraving::layout::v0;

using LayoutTypes = rtti::TypeList<Accidental, ActionIcon, Ambitus, Arpeggio, Articulation,
                                   BagpipeEmbellishment, BarLine, Beam, Bend, StretchedBend,
                                   HBox, VBox, FBox, TBox, Bracket, Breath,
                                   Chord, ChordLine, Clef, Capo,
                                   Dynamic, Expression,
                                   Fermata, FiguredBass, Fingering, FretDiagram,
                                   Glissando, GlissandoSegment, GradualTempoChange, GradualTempoChangeSegment,
                                   Hairpin, HairpinSegment, HarpPedalDiagram, Harmony, HarmonicMarkSegment, Hook,
                                   Image, InstrumentChange,
                                   Jump,
                                   KeySig,
                                   LayoutBreak, LetRing, LetRingSegment, LedgerLine, Lyrics, LyricsLineSegment,
                                   Marker, MeasureNumber, MeasureRepeat, MMRest, MMRestRange,
                                   Note, NoteDot, NoteHead,
                                   Ornament,
                                   Ottava, OttavaSegment,
                                   PalmMute, PalmMuteSegment, Pedal, PedalSegment, PlayTechAnnotation,
                                   RasgueadoSegment, RehearsalMark, Rest,
                                   ShadowNote, Slur, Spacer, StaffState, StaffText, StaffTypeChange, Stem, StemSlash, Sticking,
                                   Symbol, FSymbol, SystemDivider, SystemText,
                                   TempoText, Text, TextLine, TextLineSegment, Tie, TimeSig,
                                   Tremolo, TremoloBar, Trill, TrillSegment, TripletFeel, Tuplet,
                                   Vibrato, VibratoSegment, Volta, VoltaSegment,
                                   WhammyBarSegment>;

class LayoutVisitor : public rtti::Visitor<LayoutVisitor>
{
public:
    template<typename T>
    static bool doVisit(EngravingItem* item, LayoutContext& ctx)
    {
        if (T::classof(item)) {
            TLayout::layout(static_cast<T*>(item), ctx);
            return true;
        }
        return false;
    }
};

void TLayout::layoutItem(EngravingItem* item, LayoutContext& ctx)
{
    bool found = LayoutVisitor::visit(LayoutTypes {}, item, ctx);
    if (!found) {
        LOGE() << "not found in lyaout types item: " << item->typeName();
        DO_ASSERT(found);
    }
}

void TLayout::layout(Accidental* item, LayoutContext& ctx)
{
    item->clearElements();

    // TODO: remove Accidental in layout
    // don't show accidentals for tab or slash notation
    if (item->onTabStaff() || (item->note() && item->note()->fixed())) {
        item->setbbox(RectF());
        return;
    }

    // if the accidental is standard (doubleflat, flat, natural, sharp or double sharp)
    // and it has either no bracket or parentheses, then we have glyphs straight from smufl.
    if (item->bracket() == AccidentalBracket::NONE
        || (item->bracket() == AccidentalBracket::PARENTHESIS
            && (item->accidentalType() == AccidentalType::FLAT
                || item->accidentalType() == AccidentalType::NATURAL
                || item->accidentalType() == AccidentalType::SHARP
                || item->accidentalType() == AccidentalType::SHARP2
                || item->accidentalType() == AccidentalType::FLAT2))) {
        layoutSingleGlyphAccidental(item, ctx);
    } else {
        layoutMultiGlyphAccidental(item, ctx);
    }
}

void TLayout::layoutSingleGlyphAccidental(Accidental* item, LayoutContext& ctx)
{
    RectF r;

    SymId s = item->symbol();
    if (item->bracket() == AccidentalBracket::PARENTHESIS && !item->parentNoteHasParentheses()) {
        switch (item->accidentalType()) {
        case AccidentalType::FLAT2:
            s = SymId::accidentalDoubleFlatParens;
            break;
        case AccidentalType::FLAT:
            s = SymId::accidentalFlatParens;
            break;
        case AccidentalType::NATURAL:
            s = SymId::accidentalNaturalParens;
            break;
        case AccidentalType::SHARP:
            s = SymId::accidentalSharpParens;
            break;
        case AccidentalType::SHARP2:
            s = SymId::accidentalDoubleSharpParens;
            break;
        default:
            break;
        }
        if (!ctx.engravingFont()->isValid(s)) {
            layoutMultiGlyphAccidental(item, ctx);
            return;
        }
    }

    SymElement e(s, 0.0, 0.0);
    item->addElement(e);
    r.unite(item->symBbox(s));
    item->setbbox(r);
}

void TLayout::layoutMultiGlyphAccidental(Accidental* item, LayoutContext& ctx)
{
    double margin = ctx.style().styleMM(Sid::bracketedAccidentalPadding);
    RectF r;
    double x = 0.0;

    // should always be true
    if (item->bracket() != AccidentalBracket::NONE && !item->parentNoteHasParentheses()) {
        SymId id = SymId::noSym;
        switch (item->bracket()) {
        case AccidentalBracket::PARENTHESIS:
            id = SymId::accidentalParensLeft;
            break;
        case AccidentalBracket::BRACKET:
            id = SymId::accidentalBracketLeft;
            break;
        case AccidentalBracket::BRACE:
            id = SymId::accidentalCombiningOpenCurlyBrace;
            break;
        case AccidentalBracket::NONE: // can't happen
            break;
        }
        SymElement se(id, 0.0, item->bracket() == AccidentalBracket::BRACE ? item->spatium() * 0.4 : 0.0);
        item->addElement(se);
        r.unite(item->symBbox(id));
        x += item->symAdvance(id) + margin;
    }

    SymId s = item->symbol();
    SymElement e(s, x, 0.0);
    item->addElement(e);
    r.unite(item->symBbox(s).translated(x, 0.0));

    // should always be true
    if (item->bracket() != AccidentalBracket::NONE && !item->parentNoteHasParentheses()) {
        x += item->symAdvance(s) + margin;
        SymId id = SymId::noSym;
        switch (item->bracket()) {
        case AccidentalBracket::PARENTHESIS:
            id = SymId::accidentalParensRight;
            break;
        case AccidentalBracket::BRACKET:
            id = SymId::accidentalBracketRight;
            break;
        case AccidentalBracket::BRACE:
            id = SymId::accidentalCombiningCloseCurlyBrace;
            break;
        case AccidentalBracket::NONE: // can't happen
            break;
        }
        SymElement se(id, x, item->bracket() == AccidentalBracket::BRACE ? item->spatium() * 0.4 : 0.0);
        item->addElement(se);
        r.unite(item->symBbox(id).translated(x, 0.0));
    }
    item->setbbox(r);
}

void TLayout::layout(ActionIcon* item, LayoutContext&)
{
    FontMetrics fontMetrics(item->iconFont());
    item->setbbox(fontMetrics.boundingRect(Char(item->icon())));
}

void TLayout::layout(Ambitus* item, LayoutContext& ctx)
{
    int bottomLine, topLine;
    ClefType clf;
    double headWdt = item->headWidth();
    Key key;
    double lineDist;
    int numOfLines;
    Segment* segm = item->segment();
    double _spatium = item->spatium();
    const Staff* stf = nullptr;
    if (segm && item->track() != mu::nidx) {
        Fraction tick    = segm->tick();
        stf         = ctx.dom().staff(item->staffIdx());
        lineDist    = stf->lineDistance(tick) * _spatium;
        numOfLines  = stf->lines(tick);
        clf         = stf->clef(tick);
    } else {                              // for use in palettes
        lineDist    = _spatium;
        numOfLines  = 3;
        clf         = ClefType::G;
    }

    //
    // NOTEHEADS Y POS
    //
    // if pitch == INVALID_PITCH or tpc == Tpc::TPC_INVALID, set to some default:
    // for use in palettes and when actual range cannot be calculated (new ambitus or no notes in staff)
    //
    double xAccidOffTop    = 0;
    double xAccidOffBottom = 0;
    if (stf) {
        key = stf->key(segm->tick());
    } else {
        key = Key::C;
    }

    // top notehead
    if (item->topPitch() == INVALID_PITCH || item->topTpc() == Tpc::TPC_INVALID) {
        item->setTopPosY(0.0);  // if uninitialized, set to top staff line
    } else {
        topLine  = absStep(item->topTpc(), item->topPitch());
        topLine  = relStep(topLine, clf);
        item->setTopPosY(topLine * lineDist * 0.5);
        // compute accidental
        AccidentalType accidType;
        // if (13 <= (tpc - key) <= 19) there is no accidental)
        if (item->topTpc() - int(key) >= 13 && item->topTpc() - int(key) <= 19) {
            accidType = AccidentalType::NONE;
        } else {
            AccidentalVal accidVal = tpc2alter(item->topTpc());
            accidType = Accidental::value2subtype(accidVal);
            if (accidType == AccidentalType::NONE) {
                accidType = AccidentalType::NATURAL;
            }
        }
        item->topAccidental()->setAccidentalType(accidType);
        if (accidType != AccidentalType::NONE) {
            layout(item->topAccidental(), ctx);
        } else {
            item->topAccidental()->setbbox(RectF());
        }
        item->topAccidental()->setPosY(item->topPos().y());
    }

    // bottom notehead
    if (item->bottomPitch() == INVALID_PITCH || item->bottomTpc() == Tpc::TPC_INVALID) {
        item->setBottomPosY((numOfLines - 1) * lineDist);             // if uninitialized, set to last staff line
    } else {
        bottomLine  = absStep(item->bottomTpc(), item->bottomPitch());
        bottomLine  = relStep(bottomLine, clf);
        item->setBottomPosY(bottomLine * lineDist * 0.5);
        // compute accidental
        AccidentalType accidType;
        if (item->bottomTpc() - int(key) >= 13 && item->bottomTpc() - int(key) <= 19) {
            accidType = AccidentalType::NONE;
        } else {
            AccidentalVal accidVal = tpc2alter(item->bottomTpc());
            accidType = Accidental::value2subtype(accidVal);
            if (accidType == AccidentalType::NONE) {
                accidType = AccidentalType::NATURAL;
            }
        }
        item->bottomAccidental()->setAccidentalType(accidType);
        if (accidType != AccidentalType::NONE) {
            layout(item->bottomAccidental(), ctx);
        } else {
            item->bottomAccidental()->setbbox(RectF());
        }
        item->bottomAccidental()->setPosY(item->bottomPos().y());
    }

    //
    // NOTEHEAD X POS
    //
    // Note: manages colliding accidentals
    //
    double accNoteDist = item->point(ctx.style().styleS(Sid::accidentalNoteDistance));
    xAccidOffTop      = item->topAccidental()->width() + accNoteDist;
    xAccidOffBottom   = item->bottomAccidental()->width() + accNoteDist;

    // if top accidental extends down more than bottom accidental extends up,
    // AND ambitus is not leaning right, bottom accidental needs to be displaced
    bool collision
        =(item->topAccidental()->ipos().y() + item->topAccidental()->bbox().y() + item->topAccidental()->height()
          > item->bottomAccidental()->ipos().y() + item->bottomAccidental()->bbox().y())
          && item->direction() != DirectionH::RIGHT;
    if (collision) {
        // displace bottom accidental (also attempting to 'undercut' flats)
        xAccidOffBottom = xAccidOffTop
                          + ((item->bottomAccidental()->accidentalType() == AccidentalType::FLAT
                              || item->bottomAccidental()->accidentalType() == AccidentalType::FLAT2
                              || item->bottomAccidental()->accidentalType() == AccidentalType::NATURAL)
                             ? item->bottomAccidental()->width() * 0.5 : item->bottomAccidental()->width());
    }

    switch (item->direction()) {
    case DirectionH::AUTO:                       // noteheads one above the other
        // left align noteheads and right align accidentals 'hanging' on the left
        item->setTopPosX(0.0);
        item->setBottomPosX(0.0);
        item->topAccidental()->setPosX(-xAccidOffTop);
        item->bottomAccidental()->setPosX(-xAccidOffBottom);
        break;
    case DirectionH::LEFT:                       // top notehead at the left of bottom notehead
        // place top notehead at left margin; bottom notehead at right of top head;
        // top accid. 'hanging' on left of top head and bottom accid. 'hanging' at left of bottom head
        item->setTopPosX(0.0);
        item->setBottomPosX(headWdt);
        item->topAccidental()->setPosX(-xAccidOffTop);
        item->bottomAccidental()->setPosX(collision ? -xAccidOffBottom : headWdt - xAccidOffBottom);
        break;
    case DirectionH::RIGHT:                      // top notehead at the right of bottom notehead
        // bottom notehead at left margin; top notehead at right of bottomnotehead
        // top accid. 'hanging' on left of top head and bottom accid. 'hanging' at left of bottom head
        item->setBottomPosX(0.0);
        item->setTopPosX(headWdt);
        item->bottomAccidental()->setPosX(-xAccidOffBottom);
        item->topAccidental()->setPosX(headWdt - xAccidOffTop);
        break;
    }

    // compute line from top note centre to bottom note centre
    LineF fullLine(item->topPos().x() + headWdt * 0.5,
                   item->topPos().y(),
                   item->bottomPos().x() + headWdt * 0.5,
                   item->bottomPos().y());
    // shorten line on each side by offsets
    double yDelta = item->bottomPos().y() - item->topPos().y();
    if (yDelta != 0.0) {
        double off = _spatium * Ambitus::LINEOFFSET_DEFAULT;
        PointF p1 = fullLine.pointAt(off / yDelta);
        PointF p2 = fullLine.pointAt(1 - (off / yDelta));
        item->setLine(LineF(p1, p2));
    } else {
        item->setLine(fullLine);
    }

    RectF headRect(0, -0.5 * _spatium, headWdt, 1 * _spatium);
    item->setbbox(headRect.translated(item->topPos()).united(headRect.translated(item->bottomPos()))
                  .united(item->topAccidental()->bbox().translated(item->topAccidental()->ipos()))
                  .united(item->bottomAccidental()->bbox().translated(item->bottomAccidental()->ipos()))
                  );
}

void TLayout::layout(Arpeggio* item, LayoutContext& ctx)
{
    ArpeggioLayout::layout(item, ctx);
}

void TLayout::layout(Articulation* item, LayoutContext& ctx)
{
    item->setSkipDraw(false);
    if (item->isHiddenOnTabStaff()) {
        item->setSkipDraw(true);
        return;
    }

    RectF bRect;

    if (item->textType() != ArticulationTextType::NO_TEXT) {
        mu::draw::Font scaledFont(item->font());
        scaledFont.setPointSizeF(item->font().pointSizeF() * item->magS());
        mu::draw::FontMetrics fm(scaledFont);

        bRect = fm.boundingRect(scaledFont, TConv::text(item->textType()));
    } else {
        bRect = item->symBbox(item->symId());
    }

    item->setbbox(bRect.translated(-0.5 * bRect.width(), 0.0));

    if (item->isOrnament()) {
        layout(toOrnament(item), ctx);
    }
}

void TLayout::layout(BagpipeEmbellishment* item, LayoutContext& ctx)
{
    /*
    if (_embelType == 0 || _embelType == 8 || _embelType == 9) {
          LOGD("BagpipeEmbellishment::layout st %d", _embelType);
          }
     */
    SymId headsym = SymId::noteheadBlack;
    SymId flagsym = SymId::flag32ndUp;

    noteList nl = item->getNoteList();
    BagpipeEmbellishment::BEDrawingDataX dx(headsym, flagsym, item->magS(), ctx.spatium(), static_cast<int>(nl.size()));

    item->setbbox(RectF());
    /*
    if (_embelType == 0 || _embelType == 8 || _embelType == 9) {
          symMetrics("headsym", headsym);
          symMetrics("flagsym", flagsym);
          LOGD("mags %f headw %f headp %f spatium %f xl %f",
                 dx.mags, dx.headw, dx.headp, dx.spatium, dx.xl);
          }
     */

    bool drawFlag = nl.size() == 1;

    // draw the notes including stem, (optional) flag and (optional) ledger line
    double x = dx.xl;
    for (int note : nl) {
        int line = BagpipeEmbellishment::BagpipeNoteInfoList[note].line;
        BagpipeEmbellishment::BEDrawingDataY dy(line, ctx.spatium());

        // head
        RectF headBBox = ctx.engravingFont()->bbox(headsym, dx.mags);
        item->addbbox(headBBox.translated(PointF(x - dx.lw * .5 - dx.headw, dy.y2)));
        /*
        if (_embelType == 0 || _embelType == 8 || _embelType == 9) {
              printBBox(" notehead", bbox());
              }
         */

        // stem
        // highest top of stems actually used is y1b
        item->addbbox(RectF(x - dx.lw * .5 - dx.headw, dy.y1b, dx.lw, dy.y2 - dy.y1b));
        /*
        if (_embelType == 0 || _embelType == 8 || _embelType == 9) {
              printBBox(" notehead + stem", bbox());
              }
         */

        // flag
        if (drawFlag) {
            RectF flagBBox = ctx.engravingFont()->bbox(flagsym, dx.mags);
            item->addbbox(flagBBox.translated(PointF(x - dx.lw * .5 + dx.xcorr, dy.y1f + dy.ycorr)));
            // printBBox(" notehead + stem + flag", bbox());
        }

        // draw the ledger line for high A
        if (line == -2) {
            item->addbbox(RectF(x - dx.headw * 1.5 - dx.lw * .5, dy.y2 - dx.lw * 2, dx.headw * 2, dx.lw));
            /*
            if (_embelType == 8) {
                  printBBox(" notehead + stem + ledger line", bbox());
                  }
             */
        }

        // move x to next note x position
        x += dx.headp;
    }
}

void TLayout::layout(BarLine* item, LayoutContext& ctx)
{
    item->setPos(PointF());
    // barlines hidden on this staff
    if (item->staff() && item->segment()) {
        if ((!item->staff()->staffTypeForElement(item)->showBarlines() && item->segment()->segmentType() == SegmentType::EndBarLine)
            || (item->staff()->hideSystemBarLine() && item->segment()->segmentType() == SegmentType::BeginBarLine)) {
            item->setbbox(RectF());
            return;
        }
    }

    item->setMag(ctx.style().styleB(Sid::scaleBarlines) && item->staff() ? item->staff()->staffMag(item->tick()) : 1.0);
    // Note: the true values of y1 and y2 are computed in layout2() (can be done only
    // after staff distances are known). This is a temporary layout.
    double _spatium = item->spatium();
    item->setY1(_spatium * .5 * item->spanFrom());
    if (RealIsEqual(item->y2(), 0.0)) {
        item->setY2(_spatium * .5 * (8.0 + item->spanTo()));
    }

    double w = layoutWidth(item, ctx) * item->mag();
    RectF r(0.0, item->y1(), w, item->y2() - item->y1());

    if (ctx.style().styleB(Sid::repeatBarTips)) {
        switch (item->barLineType()) {
        case BarLineType::START_REPEAT:
            r.unite(item->symBbox(SymId::bracketTop).translated(0, item->y1()));
            // r |= symBbox(SymId::bracketBottom).translated(0, y2);
            break;
        case BarLineType::END_REPEAT: {
            double w1 = 0.0;               //symBbox(SymId::reversedBracketTop).width();
            r.unite(item->symBbox(SymId::reversedBracketTop).translated(-w1, item->y1()));
            // r |= symBbox(SymId::reversedBracketBottom).translated(0, y2);
        }
        break;
        case BarLineType::END_START_REPEAT: {
            double w1 = 0.0;               //symBbox(SymId::reversedBracketTop).width();
            r.unite(item->symBbox(SymId::reversedBracketTop).translated(-w1, item->y1()));
            r.unite(item->symBbox(SymId::bracketTop).translated(0, item->y1()));
            // r |= symBbox(SymId::reversedBracketBottom).translated(0, y2);
        }
        break;
        default:
            break;
        }
    }
    item->setbbox(r);

    for (EngravingItem* e : *item->el()) {
        layoutItem(e, ctx);
        if (e->isArticulationFamily()) {
            Articulation* a  = toArticulation(e);
            DirectionV dir    = a->direction();
            double distance   = 0.5 * item->spatium();
            double x          = item->width() * .5;
            if (dir == DirectionV::DOWN) {
                double botY = item->y2() + distance;
                a->setPos(PointF(x, botY));
            } else {
                double topY = item->y1() - distance;
                a->setPos(PointF(x, topY));
            }
        }
    }
}

double TLayout::layoutWidth(const BarLine* item, LayoutContext& ctx)
{
    const double dotWidth = item->symWidth(SymId::repeatDot);

    double w { 0.0 };
    switch (item->barLineType()) {
    case BarLineType::DOUBLE:
        w = ctx.style().styleMM(Sid::doubleBarWidth) * 2.0
            + ctx.style().styleMM(Sid::doubleBarDistance);
        break;
    case BarLineType::DOUBLE_HEAVY:
        w = ctx.style().styleMM(Sid::endBarWidth) * 2.0
            + ctx.style().styleMM(Sid::endBarDistance);
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
}

RectF TLayout::layoutRect(const BarLine* item, LayoutContext& ctx)
{
    RectF bb = item->bbox();
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
        if (ctx.style().styleB(Sid::repeatBarTips)) {
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
    // barlines hidden on this staff
    if (item->staff() && item->segment()) {
        if ((!item->staff()->staffTypeForElement(item)->showBarlines() && item->segment()->segmentType() == SegmentType::EndBarLine)
            || (item->staff()->hideSystemBarLine() && item->segment()->segmentType() == SegmentType::BeginBarLine)) {
            item->setbbox(RectF());
            return;
        }
    }

    item->calcY();
    item->bbox().setTop(item->y1());
    item->bbox().setBottom(item->y2());

    if (ctx.style().styleB(Sid::repeatBarTips)) {
        switch (item->barLineType()) {
        case BarLineType::START_REPEAT:
            item->bbox().unite(item->symBbox(SymId::bracketTop).translated(0, item->y1()));
            item->bbox().unite(item->symBbox(SymId::bracketBottom).translated(0, item->y2()));
            break;
        case BarLineType::END_REPEAT:
        {
            double w1 = 0.0;               //symBbox(SymId::reversedBracketTop).width();
            item->bbox().unite(item->symBbox(SymId::reversedBracketTop).translated(-w1, item->y1()));
            item->bbox().unite(item->symBbox(SymId::reversedBracketBottom).translated(-w1, item->y2()));
            break;
        }
        case BarLineType::END_START_REPEAT:
        {
            double w1 = 0.0;               //symBbox(SymId::reversedBracketTop).width();
            item->bbox().unite(item->symBbox(SymId::reversedBracketTop).translated(-w1, item->y1()));
            item->bbox().unite(item->symBbox(SymId::reversedBracketBottom).translated(-w1, item->y2()));
            item->bbox().unite(item->symBbox(SymId::bracketTop).translated(0, item->y1()));
            item->bbox().unite(item->symBbox(SymId::bracketBottom).translated(0, item->y2()));
            break;
        }
        default:
            break;
        }
    }
}

void TLayout::layout(Beam* item, LayoutContext& ctx)
{
    BeamLayout::layout(item, ctx);
}

void TLayout::layout1(Beam* item, LayoutContext& ctx)
{
    BeamLayout::layout1(item, ctx);
}

void TLayout::layout(Bend* item, LayoutContext& ctx)
{
    // during mtest, there may be no score. If so, exit.
    if (!ctx.isValid()) {
        return;
    }

    double _spatium = item->spatium();

    if (item->staff() && !item->staff()->isTabStaff(item->tick())) {
        if (!item->explicitParent()) {
            item->setNoteWidth(-_spatium * 2);
            item->setNotePos(PointF(0.0, _spatium * 3));
        }
    }

    double _lw = item->lineWidth();
    Note* note = toNote(item->explicitParent());
    if (note == 0) {
        item->setNoteWidth(0.0);
        item->setNotePos(PointF());
    } else {
        PointF notePos = note->pos();
        notePos.ry() = std::max(notePos.y(), 0.0);

        item->setNoteWidth(note->width());
        item->setNotePos(notePos);
    }
    RectF bb;

    mu::draw::FontMetrics fm(item->font(_spatium));

    size_t n   = item->points().size();
    double x = item->noteWidth();
    double y = -_spatium * .8;
    double x2, y2;

    double aw = _spatium * .5;
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
            y2 = -item->notePos().y() - _spatium * 2;
            x2 = x;
            bb.unite(RectF(x, y, x2 - x, y2 - y));

            bb.unite(arrowUp.translated(x2, y2 + _spatium * .2).boundingRect());

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
            x2 = x + _spatium;
            y2 = y;
            bb.unite(RectF(x, y, x2 - x, y2 - y));
        } else if (pitch < item->points().at(pt + 1).pitch) {
            // up
            x2 = x + _spatium * .5;
            y2 = -item->notePos().y() - _spatium * 2;
            double dx = x2 - x;
            double dy = y2 - y;

            PainterPath path;
            path.moveTo(x, y);
            path.cubicTo(x + dx / 2, y, x2, y + dy / 4, x2, y2);
            bb.unite(path.boundingRect());
            bb.unite(arrowUp.translated(x2, y2 + _spatium * .2).boundingRect());

            int idx = (item->points().at(pt + 1).pitch + 12) / 25;
            const char* l = Bend::label[idx];
            bb.unite(fm.boundingRect(RectF(x2, y2, 0, 0),
                                     draw::AlignHCenter | draw::AlignBottom | draw::TextDontClip,
                                     String::fromAscii(l)));
        } else {
            // down
            x2 = x + _spatium * .5;
            y2 = y + _spatium * 3;
            double dx = x2 - x;
            double dy = y2 - y;

            PainterPath path;
            path.moveTo(x, y);
            path.cubicTo(x + dx / 2, y, x2, y + dy / 4, x2, y2);
            bb.unite(path.boundingRect());

            bb.unite(arrowDown.translated(x2, y2 - _spatium * .2).boundingRect());
        }
        x = x2;
        y = y2;
    }
    bb.adjust(-_lw, -_lw, _lw, _lw);
    item->setbbox(bb);
    item->setPos(0.0, 0.0);
}

using BoxTypes = rtti::TypeList<HBox, VBox, FBox, TBox>;

class BoxVisitor : public rtti::Visitor<BoxVisitor>
{
public:
    template<typename T>
    static bool doVisit(EngravingItem* item, LayoutContext& ctx)
    {
        if (T::classof(item)) {
            TLayout::layout(static_cast<T*>(item), ctx);
            return true;
        }
        return false;
    }
};

void TLayout::layout(Box* item, LayoutContext& ctx)
{
    BoxVisitor::visit(BoxVisitor::ShouldBeFound, BoxTypes {}, item, ctx);
}

void TLayout::layoutBox(Box* item, LayoutContext& ctx)
{
    layoutMeasureBase(item, ctx);
    for (EngravingItem* e : item->el()) {
        if (!e->isLayoutBreak()) {
            layoutItem(e, ctx);
        }
    }
}

void TLayout::layout(HBox* item, LayoutContext& ctx)
{
    if (item->explicitParent() && item->explicitParent()->isVBox()) {
        VBox* vb = toVBox(item->explicitParent());
        double x = vb->leftMargin() * DPMM;
        double y = vb->topMargin() * DPMM;
        double w = item->point(item->boxWidth());
        double h = vb->height() - (vb->topMargin() + vb->bottomMargin()) * DPMM;
        item->setPos(x, y);
        item->bbox().setRect(0.0, 0.0, w, h);
    } else if (item->system()) {
        item->bbox().setRect(0.0, 0.0, item->point(item->boxWidth()), item->system()->height());
    } else {
        item->bbox().setRect(0.0, 0.0, 50, 50);
    }
    layoutBox(static_cast<Box*>(item), ctx);
}

void TLayout::layout2(HBox* item, LayoutContext& ctx)
{
    layoutBox(item, ctx);
}

void TLayout::layout(VBox* item, LayoutContext& ctx)
{
    item->setPos(PointF());

    if (item->system()) {
        item->bbox().setRect(0.0, 0.0, item->system()->width(), item->point(item->boxHeight()));
    } else {
        item->bbox().setRect(0.0, 0.0, 50, 50);
    }

    for (EngravingItem* e : item->el()) {
        if (!e->isLayoutBreak()) {
            layoutItem(e, ctx);
        }
    }

    if (item->getProperty(Pid::BOX_AUTOSIZE).toBool()) {
        double contentHeight = item->contentRect().height();

        if (contentHeight < item->minHeight()) {
            contentHeight = item->minHeight();
        }

        item->setHeight(contentHeight);
    }

    layoutMeasureBase(item, ctx);

    if (MScore::noImages) {
        adjustLayoutWithoutImages(item, ctx);
    }
}

void TLayout::adjustLayoutWithoutImages(VBox* item, LayoutContext& ctx)
{
    double calculatedVBoxHeight = 0;
    const int padding = ctx.spatium();
    auto elementList = item->el();

    for (auto pElement : elementList) {
        if (pElement->isText()) {
            Text* txt = toText(pElement);
            txt->bbox().moveTop(0);
            calculatedVBoxHeight += txt->height() + padding;
        }
    }

    item->setHeight(calculatedVBoxHeight);
    layoutBox(static_cast<Box*>(item), ctx);
}

void TLayout::layout(FBox* item, LayoutContext& ctx)
{
    item->bbox().setRect(0.0, 0.0, item->system()->width(), item->point(item->boxHeight()));
    layoutBox(static_cast<Box*>(item), ctx);
}

void TLayout::layout(TBox* item, LayoutContext& ctx)
{
    item->setPos(PointF());        // !?
    item->bbox().setRect(0.0, 0.0, item->system()->width(), 0);
    layout(item->text(), ctx);

    double h = 0.;
    if (item->text()->empty()) {
        h = mu::draw::FontMetrics::ascent(item->text()->font());
    } else {
        h = item->text()->height();
    }
    double y = item->topMargin() * DPMM;
    item->text()->setPos(item->leftMargin() * DPMM, y);
    h += item->topMargin() * DPMM + item->bottomMargin() * DPMM;
    item->bbox().setRect(0.0, 0.0, item->system()->width(), h);

    layoutMeasureBase(item, ctx);   // layout LayoutBreak's
}

void TLayout::layout(Bracket* item, LayoutContext& ctx)
{
    if (RealIsNull(item->h2())) {
        return;
    }

    PainterPath path;
    Shape shape;

    item->setVisible(item->bi()->visible());

    switch (item->bracketType()) {
    case BracketType::BRACE: {
        String musicalSymbolFont = ctx.style().styleSt(Sid::MusicalSymbolFont);
        if (musicalSymbolFont == "Emmentaler" || musicalSymbolFont == "Gonville") {
            item->setBraceSymbol(SymId::noSym);
            double w = ctx.style().styleMM(Sid::akkoladeWidth);

#define XM(a) (a + 700) * w / 700
#define YM(a) (a + 7100) * item->h2() / 7100

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
            item->setbbox(path.boundingRect());
            shape.add(item->bbox());
        } else {
            if (item->braceSymbol() == SymId::noSym) {
                item->setBraceSymbol(SymId::brace);
            }
            double h = item->h2() * 2;
            double w = item->symWidth(item->braceSymbol()) * item->magx();
            item->bbox().setRect(0, 0, w, h);
            shape.add(item->bbox());
        }
    }
    break;
    case BracketType::NORMAL: {
        double _spatium = item->spatium();
        double w = ctx.style().styleMM(Sid::bracketWidth) * .5;
        double x = -w;

        double bd   = (ctx.style().styleSt(Sid::MusicalSymbolFont) == "Leland") ? _spatium * .5 : _spatium * .25;
        shape.add(RectF(x, -bd, w * 2, 2 * (item->h2() + bd)));
        shape.add(item->symBbox(SymId::bracketTop).translated(PointF(-w, -bd)));
        shape.add(item->symBbox(SymId::bracketBottom).translated(PointF(-w, bd + 2 * item->h2())));

        w      += item->symWidth(SymId::bracketTop);
        double y = -item->symHeight(SymId::bracketTop) - bd;
        double h = (-y + item->h2()) * 2;
        item->bbox().setRect(x, y, w, h);
    }
    break;
    case BracketType::SQUARE: {
        double w = ctx.style().styleMM(Sid::staffLineWidth) * .5;
        double x = -w;
        double y = -w;
        double h = (item->h2() + w) * 2;
        w      += (.5 * item->spatium() + 3 * w);
        item->bbox().setRect(x, y, w, h);
        shape.add(item->bbox());
    }
    break;
    case BracketType::LINE: {
        double _spatium = item->spatium();
        double w = 0.67 * ctx.style().styleMM(Sid::bracketWidth) * .5;
        double x = -w;
        double bd = _spatium * .25;
        double y = -bd;
        double h = (-y + item->h2()) * 2;
        item->bbox().setRect(x, y, w, h);
        shape.add(item->bbox());
    }
    break;
    case BracketType::NO_BRACKET:
        break;
    }

    item->setPath(path);
    item->setShape(shape);
}

void TLayout::layout(Breath* item, LayoutContext& ctx)
{
    bool palette = (!item->staff() || item->track() == mu::nidx);
    if (!palette) {
        int voiceOffset = item->placeBelow() * (item->staff()->lines(item->tick()) - 1) * item->spatium();
        if (item->isCaesura()) {
            item->setPos(item->xpos(), item->spatium() + voiceOffset);
        } else if ((ctx.style().styleSt(Sid::MusicalSymbolFont) == "Emmentaler")
                   && (item->symId() == SymId::breathMarkComma)) {
            item->setPos(item->xpos(), 0.5 * item->spatium() + voiceOffset);
        } else {
            item->setPos(item->xpos(), -0.5 * item->spatium() + voiceOffset);
        }
    }
    item->setbbox(item->symBbox(item->symId()));
}

void TLayout::layout(Chord* item, LayoutContext& ctx)
{
    ChordLayout::layout(item, ctx);
}

void TLayout::layout(ChordLine* item, LayoutContext& ctx)
{
    item->setMag(item->chord() ? item->chord()->mag() : 1);
    if (!item->modified()) {
        double x2 = 0;
        double y2 = 0;
        double baseLength = item->spatium() * (item->chord() ? item->chord()->intrinsicMag() : 1);
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
            item->setPath(path);
        }
    }

    if (item->explicitParent()) {
        Note* note = nullptr;

        if (item->note()) {
            note = item->chord()->findNote(item->note()->pitch());
        }

        if (!note) {
            note = item->chord()->upNote();
        }

        double x = 0.0;
        double y = note->pos().y();
        double horOffset = 0.33 * item->spatium();     // one third of a space away from the note
        double vertOffset = 0.25 * item->spatium();     // one quarter of a space from the center line
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

        item->setPos(x, y);
    } else {
        item->setPos(0.0, 0.0);
    }

    if (!item->isWavy()) {
        RectF r = item->path().boundingRect();
        int x1 = 0, y1 = 0, width = 0, height = 0;

        x1 = r.x();
        y1 = r.y();
        width = r.width();
        height = r.height();
        item->bbox().setRect(x1, y1, width, height);
    } else {
        RectF r = ctx.engravingFont()->bbox(ChordLine::WAVE_SYMBOLS, item->magS());
        double angle = ChordLine::WAVE_ANGEL * M_PI / 180;

        r.setHeight(r.height() + r.width() * sin(angle));

        /// TODO: calculate properly the rect for wavy type
        if (item->chordLineType() == ChordLineType::DOIT) {
            r.setY(item->y() - r.height() * (item->onTabStaff() ? 1.25 : 1));
        }

        item->setbbox(r);
    }
}

void TLayout::layout(Clef* item, LayoutContext&)
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
                item->setClefType(item->staff()->clefType(Fraction(0, 1)));
            }
        }

        // if clef not to show or not compatible with staff group
        if (!show) {
            item->setbbox(RectF());
            item->setSymId(SymId::noSym);
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
        item->setSymId(ClefInfo::symId(item->clefType()));
        yoff = lineDist * (5 - ClefInfo::line(item->clefType()));
    } else {
        item->setSymId(SymId::noSym);
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
    RectF r(item->symBbox(item->symId()));
    double x = item->segment() && item->segment()->rtick().isNotZero() ? -r.right() : 0.0;
    item->setPos(x, yoff * _spatium + (stepOffset * 0.5 * _spatium));

    item->setbbox(r);
}

void TLayout::layout(Capo* item, LayoutContext& ctx)
{
    if (item->shouldAutomaticallyGenerateText() || item->empty()) {
        if (const Part* part = item->part()) {
            if (const Instrument* instrument = part->instrument(item->tick())) {
                if (const StringData* stringData = instrument->stringData()) {
                    String text = item->generateText(stringData->strings());
                    item->setXmlText(text);
                }
            }
        }
    }

    layoutTextBase(item, ctx);
    item->autoplaceSegmentElement();
}

void TLayout::layout(DeadSlapped* item, LayoutContext&)
{
    const double deadSlappedWidth = item->spatium() * 2;
    RectF rect = RectF(0, 0, deadSlappedWidth, item->staff()->height());
    item->setbbox(rect);

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

        item->setPath1(path1);
        item->setPath2(path2);
    }
}

void TLayout::layout(Dynamic* item, LayoutContext& ctx)
{
    item->setSnappedExpression(nullptr); // Here we reset it. It will become known again when we layout expression

    const StaffType* stType = item->staffType();

    item->setSkipDraw(false);
    if (stType && stType->isHiddenElementOnTab(ctx.style(), Sid::dynamicsShowTabCommon, Sid::dynamicsShowTabSimple)) {
        item->setSkipDraw(true);
        return;
    }

    layoutTextBase(item, ctx);

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
        item->movePosX(itemToAlign->width() * 0.5);
        return;
    }

    Chord* chord = toChord(itemToAlign);
    bool centerOnNote = item->centerOnNotehead() || (!item->centerOnNotehead() && item->align().horizontal == AlignH::HCENTER);

    // Move to center of notehead width
    Note* note = chord->notes().at(0);
    double noteHeadWidth = note->headWidth();
    item->movePosX(noteHeadWidth * (centerOnNote ? 0.5 : 1));

    if (!item->centerOnNotehead()) {
        return;
    }

    // Use Smufl optical center for dynamic if available
    SymId symId = TConv::symId(item->dynamicType());
    double opticalCenter = item->symSmuflAnchor(symId, SmuflAnchorId::opticalCenter).x();
    if (symId != SymId::noSym && opticalCenter) {
        double symWidth = item->symBbox(symId).width();
        double offset = symWidth / 2 - opticalCenter + item->symBbox(symId).left();
        double spatiumScaling = item->spatium() / ctx.spatium();
        offset *= spatiumScaling;
        item->movePosX(offset);
    }

    // If the dynamic contains custom text, keep it aligned
    item->movePosX(-item->customTextOffset());
}

void TLayout::layout(Expression* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx);

    Segment* segment = item->explicitParent() ? toSegment(item->explicitParent()) : nullptr;
    if (!segment) {
        return;
    }

    if (item->align().horizontal != AlignH::LEFT) {
        Chord* chordToAlign = nullptr;
        // Look for chord in this staff
        track_idx_t startTrack = track2staff(item->staffIdx());
        track_idx_t endTrack = startTrack + VOICES;
        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            EngravingItem* engravingItem = segment->elementAt(track);
            if (engravingItem && engravingItem->isChord()) {
                chordToAlign = toChord(item);
                break;
            }
        }

        if (chordToAlign) {
            Note* note = chordToAlign->notes().at(0);
            double headWidth = note->headWidth();
            bool center = item->align().horizontal == AlignH::HCENTER;
            item->movePosX(headWidth * (center ? 0.5 : 1));
        }
    }

    item->setSnappedDynamic(nullptr);

    if (!item->autoplace() || !item->snapToDynamics()) {
        return;
    }

    Dynamic* dynamic = toDynamic(segment->findAnnotation(ElementType::DYNAMIC, item->track(), item->track()));
    if (!dynamic || dynamic->placeAbove() != item->placeAbove()) {
        item->autoplaceSegmentElement();
        return;
    }

    item->setSnappedDynamic(dynamic);
    dynamic->setSnappedExpression(item);

    // If there is a dynamic on same segment and track, lock this expression to it
    double padding = item->computeDynamicExpressionDistance();
    double dynamicRight = dynamic->shape().translate(dynamic->pos()).right();
    double expressionLeft = item->bbox().translated(item->pos()).left();
    double difference = expressionLeft - dynamicRight - padding;
    item->movePosX(-difference);

    // Keep expression and dynamic vertically aligned
    item->autoplaceSegmentElement();
    bool above = item->placeAbove();
    double yExpression = item->pos().y();
    double yDynamic = dynamic->pos().y();
    bool expressionIsOuter = above ? yExpression < yDynamic : yExpression > yDynamic;
    if (expressionIsOuter) {
        dynamic->movePosY((yExpression - yDynamic));
    } else {
        item->movePosY((yDynamic - yExpression));
    }
}

void TLayout::layout(Fermata* item, LayoutContext& ctx)
{
    const StaffType* stType = item->staffType();

    item->setSkipDraw(false);
    if (stType && stType->isHiddenElementOnTab(ctx.style(), Sid::fermataShowTabCommon, Sid::fermataShowTabSimple)) {
        item->setSkipDraw(true);
        return;
    }

    Segment* s = item->segment();
    item->setPos(PointF());
    if (!s) {            // for use in palette
        item->setOffset(0.0, 0.0);
        RectF b(item->symBbox(item->symId()));
        item->setbbox(b.translated(-0.5 * b.width(), 0.0));
        return;
    }

    if (item->isStyled(Pid::OFFSET)) {
        item->setOffset(item->propertyDefault(Pid::OFFSET).value<PointF>());
    }
    EngravingItem* e = s->element(item->track());
    if (e) {
        if (e->isChord()) {
            Chord* chord = toChord(e);
            Note* note = chord->up() ? chord->downNote() : chord->upNote();
            double offset = chord->xpos() + note->xpos() + note->headWidth() / 2;
            item->movePosX(offset);
        } else {
            item->movePosX(e->x() - e->shape().left() + e->width() * item->staff()->staffMag(Fraction(0, 1)) * .5);
        }
    }

    String name = String::fromAscii(SymNames::nameForSymId(item->symId()).ascii());
    if (item->placeAbove()) {
        if (name.endsWith(u"Below")) {
            item->setSymId(SymNames::symIdByName(name.left(name.size() - 5) + u"Above"));
        }
    } else {
        item->movePosY(item->staff()->height());
        if (name.endsWith(u"Above")) {
            item->setSymId(SymNames::symIdByName(name.left(name.size() - 5) + u"Below"));
        }
    }
    RectF b(item->symBbox(item->symId()));
    item->setbbox(b.translated(-0.5 * b.width(), 0.0));
    item->autoplaceSegmentElement();
}

//---------------------------------------------------------
//   FiguredBassItem layout
//    creates the display text (set as element text) and computes
//    the horiz. offset needed to align the right part as well as the vert. offset
//---------------------------------------------------------

void TLayout::layout(FiguredBassItem* item, LayoutContext& ctx)
{
    double h, w, x, x1, x2, y;

    // construct font metrics
    int fontIdx = 0;
    mu::draw::Font f(FiguredBass::FBFonts().at(fontIdx).family, draw::Font::Type::Tablature);

    // font size in pixels, scaled according to spatium()
    // (use the same font selection as used in draw() below)
    double m = ctx.style().styleD(Sid::figuredBassFontSize) * item->spatium() / SPATIUM20;
    f.setPointSizeF(m);
    mu::draw::FontMetrics fm(f);

    String str;
    x  = item->symWidth(SymId::noteheadBlack) * .5;
    x1 = x2 = 0.0;

    // create display text
    int font = 0;
    int style = ctx.style().styleI(Sid::figuredBassStyle);

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

    item->setDisplayText(str);                  // this text will be displayed

    if (str.size()) {                     // if some text
        x = x - (x1 + x2) * 0.5;          // position the text so that [x1<-->x2] is centered below the note
    } else {                              // if no text (but possibly a line)
        x = 0;                            // start at note left margin
    }
    // vertical position
    h = fm.lineSpacing();
    h *= ctx.style().styleD(Sid::figuredBassLineHeight);
    if (ctx.style().styleI(Sid::figuredBassAlignment) == 0) {          // top alignment: stack down from first item
        y = h * item->ord();
    } else {                                                      // bottom alignment: stack up from last item
        y = -h * (item->figuredBass()->itemsCount() - item->ord());
    }
    item->setPos(x, y);
    // determine bbox from text width
//      w = fm.width(str);
    w = fm.width(str);
    item->setTextWidth(w);
    // if there is a cont.line, extend width to cover the whole FB element duration line
    int lineLen;
    if (item->contLine() != FiguredBassItem::ContLine::NONE && (lineLen = item->figuredBass()->lineLength(0)) > w) {
        w = lineLen;
    }
    item->bbox().setRect(0, 0, w, h);
}

void TLayout::layout(FiguredBass* item, LayoutContext& ctx)
{
    // VERTICAL POSITION:
    const double y = ctx.style().styleD(Sid::figuredBassYOffset) * item->spatium();
    item->setPos(PointF(0.0, y));

    // BOUNDING BOX and individual item layout (if required)
    layout1TextBase(item, ctx);  // prepare structs and data expected by Text methods
    // if element could be parsed into items, layout each element
    // Items list will be empty in edit mode (see FiguredBass::startEdit).
    // TODO: consider disabling specific layout in case text style is changed (tid() != TextStyleName::FIGURED_BASS).
    if (item->items().size() > 0) {
        layoutLines(item, ctx);
        item->bbox().setRect(0, 0, item->lineLength(0), 0);
        // layout each item and enlarge bbox to include items bboxes
        for (FiguredBassItem* fit : item->items()) {
            layout(fit, ctx);
            item->addbbox(fit->bbox().translated(fit->pos()));
        }
    }
}

//    lays out the duration indicator line(s), filling the _lineLengths array
//    and the length of printed lines (used by continuation lines)

void TLayout::layoutLines(FiguredBass* item, LayoutContext& ctx)
{
    std::vector<double> lineLengths = item->lineLengths();
    if (item->ticks() <= Fraction(0, 1) || !item->segment()) {
        lineLengths.resize(1);                             // be sure to always have
        lineLengths[0] = 0;                                // at least 1 item in array
        item->setLineLengths(lineLengths);
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
        item->setLineLengths(lineLengths);
        return;
    }

    // get length of printed lines from horiz. page position of lastCR
    // (enter a bit 'into' the ChordRest for clarity)
    double printedLineLength = lastCR ? lastCR->pageX() - item->pageX() + 1.5 * item->spatium() : 3 * item->spatium();
    item->setPrintedLineLength(printedLineLength);

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

    item->setLineLengths(lineLengths);
}

void TLayout::layout(Fingering* item, LayoutContext& ctx)
{
    if (item->explicitParent()) {
        Fraction tick = item->parentItem()->tick();
        const Staff* st = item->staff();
        item->setSkipDraw(false);
        if (st && st->isTabStaff(tick)
            && (!st->staffType(tick)->showTabFingering() || item->textStyleType() == TextStyleType::STRING_NUMBER)) {
            item->setSkipDraw(true);
            return;
        }
    }

    layoutTextBase(item, ctx);
    item->setPosY(0.0);      // handle placement below

    if (item->autoplace() && item->note()) {
        Note* n      = item->note();
        Chord* chord = n->chord();
        bool voices  = chord->measure()->hasVoices(chord->staffIdx(), chord->tick(), chord->actualTicks());
        bool tight   = voices && chord->notes().size() == 1 && !chord->beam() && item->textStyleType() != TextStyleType::STRING_NUMBER;

        double headWidth = n->bboxRightPos();

        // update offset after drag
        double rebase = 0.0;
        if (item->offsetChanged() != OffsetChange::NONE && !tight) {
            rebase = item->rebaseOffset();
        }

        // temporarily exclude self from chord shape
        item->setAutoplace(false);

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
                item->movePosX(-n->ipos().x());
            }
            item->movePosX(headWidth * .5);
            if (above) {
                if (tight) {
                    if (chord->stem()) {
                        item->movePosX(-0.8 * sp);
                    }
                    item->movePosY(-1.5 * sp);
                } else {
                    RectF r = item->bbox().translated(m->pos() + s->pos() + chord->pos() + n->pos() + item->pos());
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
                        top = stem->y() + stem->bbox().top();
                    } else {
                        Note* un = chord->upNote();
                        top = std::min(0.0, un->y() + un->bbox().top());
                    }
                    top -= md;
                    double diff = (item->bbox().bottom() + item->ipos().y() + yd + n->y()) - top;
                    if (diff > 0.0) {
                        yd -= diff;
                    }
                    if (item->offsetChanged() != OffsetChange::NONE) {
                        // user moved element within the skyline
                        // we may need to adjust minDistance, yd, and/or offset
                        bool inStaff = above ? r.bottom() + rebase > 0.0 : r.top() + rebase < item->staff()->height();
                        item->rebaseMinDistance(md, yd, sp, rebase, above, inStaff);
                    }
                    item->movePosY(yd);
                }
            } else {
                if (tight) {
                    if (chord->stem()) {
                        item->movePosX(0.8 * sp);
                    }
                    item->movePosY(1.5 * sp);
                } else {
                    RectF r = item->bbox().translated(m->pos() + s->pos() + chord->pos() + n->pos() + item->pos());
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
                        bottom = stem->y() + stem->bbox().bottom();
                    } else {
                        Note* dn = chord->downNote();
                        bottom = std::max(vStaff->height(), dn->y() + dn->bbox().bottom());
                    }
                    bottom += md;
                    double diff = bottom - (item->bbox().top() + item->ipos().y() + yd + n->y());
                    if (diff > 0.0) {
                        yd += diff;
                    }
                    if (item->offsetChanged() != OffsetChange::NONE) {
                        // user moved element within the skyline
                        // we may need to adjust minDistance, yd, and/or offset
                        bool inStaff = above ? r.bottom() + rebase > 0.0 : r.top() + rebase < item->staff()->height();
                        item->rebaseMinDistance(md, yd, sp, rebase, above, inStaff);
                    }
                    item->movePosY(yd);
                }
            }
        } else if (item->textStyleType() == TextStyleType::LH_GUITAR_FINGERING) {
            // place to left of note
            double left = n->shape().left();
            if (left - n->x() > 0.0) {
                item->movePosX(-left);
            } else {
                item->movePosX(-n->x());
            }
        }
        // for other fingering styles, do not autoplace

        // restore autoplace
        item->setAutoplace(true);
    } else if (item->offsetChanged() != OffsetChange::NONE) {
        // rebase horizontally too, as autoplace may have adjusted it
        item->rebaseOffset(false);
    }
    item->setOffsetChanged(false);
}

void TLayout::layout(FretDiagram* item, LayoutContext& ctx)
{
    double _spatium  = item->spatium() * item->userMag();
    item->setStringLw(_spatium * 0.08);
    item->setNutLw((item->fretOffset() || !item->showNut()) ? item->stringLw() : _spatium * 0.2);
    item->setStringDist(ctx.style().styleMM(Sid::fretStringSpacing) * item->userMag());
    item->setFretDist(ctx.style().styleMM(Sid::fretFretSpacing) * item->userMag());
    item->setMarkerSize(item->stringDist() * .8);

    double w    = item->stringDist() * (item->strings() - 1) + item->markerSize();
    double h    = (item->frets() + 1) * item->fretDist() + item->markerSize();
    double y    = -(item->markerSize() * .5 + item->fretDist());
    double x    = -(item->markerSize() * .5);

    // Allocate space for fret offset number
    if (item->fretOffset() > 0) {
        mu::draw::Font scaledFont(item->font());
        scaledFont.setPointSizeF(item->font().pointSizeF() * item->userMag());

        double fretNumMag = ctx.style().styleD(Sid::fretNumMag);
        scaledFont.setPointSizeF(scaledFont.pointSizeF() * fretNumMag);
        mu::draw::FontMetrics fm2(scaledFont);
        double numw = fm2.width(String::number(item->fretOffset() + 1));
        double xdiff = numw + item->stringDist() * .4;
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
    item->bbox().setRect(x, y, w, h);

    if (!item->explicitParent() || !item->explicitParent()->isSegment()) {
        item->setPos(PointF());
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
                noteheadWidth = item->symWidth(r->sym());
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
        mainWidth = item->stringDist() * (item->strings() - 1);
    } else if (item->orientation() == Orientation::HORIZONTAL) {
        mainWidth = item->fretDist() * (item->frets() + 0.5);
    }
    item->setPos((noteheadWidth - mainWidth) / 2, -(h + item->styleP(Sid::fretY)));

    item->autoplaceSegmentElement();

    // don't display harmony in palette
    if (!item->explicitParent()) {
        return;
    }

    Harmony* harmony = item->harmony();
    if (harmony) {
        layout(harmony, ctx);
    }

    if (harmony && harmony->autoplace() && harmony->explicitParent()) {
        Segment* s = toSegment(item->explicitParent());
        Measure* m = s->measure();
        staff_idx_t si = item->staffIdx();

        SysStaff* ss = m->system()->staff(si);
        RectF r = harmony->bbox().translated(m->pos() + s->pos() + item->pos() + harmony->pos());

        double minDistance = harmony->minDistance().val() * item->spatium();
        SkylineLine sk(false);
        sk.add(r.x(), r.bottom(), r.width());
        double d = sk.minDistance(ss->skyline().north());
        if (d > -minDistance) {
            double yd = d + minDistance;
            yd *= -1.0;
            harmony->movePosY(yd);
            r.translate(PointF(0.0, yd));
        }
        if (harmony->addToSkyline()) {
            ss->skyline().add(r);
        }
    }
}

void TLayout::layout(FretCircle* item, LayoutContext&)
{
    item->setSkipDraw(false);
    if (!item->tabEllipseEnabled()) {
        item->setSkipDraw(true);
        item->setbbox(RectF());
        return;
    }

    double lw = item->spatium() * FretCircle::CIRCLE_WIDTH / 2;
    item->setRect(item->ellipseRect());

    RectF chordRect;
    double minWidth = item->chord()->upNote()->width();
    for (const Note* note : item->chord()->notes()) {
        chordRect |= note->bbox();
        minWidth = std::min(minWidth, note->width());
    }

    double offsetFromUpNote = (item->rect().height() - chordRect.height()
                               - (item->chord()->downNote()->pos().y() - item->chord()->upNote()->pos().y())
                               ) / 2;
    item->setOffsetFromUpNote(offsetFromUpNote);
    item->setSideOffset((item->rect().width() - minWidth) / 2);

    item->setbbox(item->rect().adjusted(-lw, -lw, lw, lw));
}

void TLayout::layout(Glissando* item, LayoutContext& ctx)
{
    double _spatium = item->spatium();

    if (ctx.isPaletteMode() || !item->startElement() || !item->endElement()) {    // for use in palettes or while dragging
        if (item->spannerSegments().empty()) {
            item->add(item->createLineSegment(ctx.mutDom().dummyParent()->system()));
        }
        LineSegment* s = item->frontSegment();
        s->setPos(PointF(-_spatium * Glissando::GLISS_PALETTE_WIDTH / 2, _spatium * Glissando::GLISS_PALETTE_HEIGHT / 2));
        s->setPos2(PointF(_spatium * Glissando::GLISS_PALETTE_WIDTH, -_spatium * Glissando::GLISS_PALETTE_HEIGHT));
        layout(s, ctx);
        return;
    }
    layoutLine(item, ctx);
    if (item->spannerSegments().empty()) {
        LOGD("no segments");
        return;
    }
    item->setPos(0.0, 0.0);

    Note* anchor1     = toNote(item->startElement());
    Note* anchor2     = toNote(item->endElement());
    Chord* cr1         = anchor1->chord();
    Chord* cr2         = anchor2->chord();
    GlissandoSegment* segm1 = toGlissandoSegment(item->frontSegment());
    GlissandoSegment* segm2 = toGlissandoSegment(item->backSegment());

    // Note: line segments are defined by
    // initial point: ipos() (relative to system origin)
    // ending point:  pos2() (relative to initial point)

    // LINE ENDING POINTS TO NOTEHEAD CENTRES

    // assume gliss. line goes from centre of initial note centre to centre of ending note:
    // move first segment origin and last segment ending point from notehead origin to notehead centre
    // For TAB: begin at the right-edge of initial note rather than centre
    PointF offs1 = (cr1->staff()->isTabStaff(cr1->tick()))
                   ? PointF(anchor1->bbox().right(), 0.0)
                   : PointF(anchor1->headWidth() * 0.5, 0.0);

    PointF offs2 = PointF(anchor2->headWidth() * 0.5, 0.0);

    // AVOID HORIZONTAL LINES

    int upDown = (0 < (anchor2->pitch() - anchor1->pitch())) - ((anchor2->pitch() - anchor1->pitch()) < 0);
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
    segm1->setPos(segm1->ipos() + offs1);
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
    double y0   = segm1->ipos().y();
    double yTot = segm2->ipos().y() + segm2->ipos2().y() - y0;
    yTot -= yStaffDifference(segm2->system(), segm2->staffIdx(), segm1->system(), segm1->staffIdx());
    double ratio = yTot / xTot;
    // interpolate y-coord of intermediate points across total width and height
    double xCurr = 0.0;
    double yCurr;
    for (unsigned i = 0; i + 1 < item->spannerSegments().size(); i++) {
        SpannerSegment* segm = item->segmentAt(i);
        xCurr += segm->ipos2().x();
        yCurr = y0 + ratio * xCurr;
        segm->rypos2() = yCurr - segm->ipos().y();           // position segm. end point at yCurr
        // next segment shall start where this segment stopped, corrected for the staff y-difference
        SpannerSegment* nextSeg = item->segmentAt(i + 1);
        yCurr += yStaffDifference(nextSeg->system(), nextSeg->staffIdx(), segm->system(), segm->staffIdx());
        segm = nextSeg;
        segm->rypos2() += segm->ipos().y() - yCurr;          // adjust next segm. vertical length
        segm->setPosY(yCurr);                                // position next segm. start point at yCurr
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
    segm1->setPos(segm1->ipos() + offs1);
    segm1->setPos2(segm1->ipos2() - offs1);
    // adjust last segment length by x2 (and proportionally y)
    offs2.ry() = segm2->ipos2().y() * offs2.x() / segm2->ipos2().x();
    segm2->setPos2(segm2->ipos2() + offs2);

    for (SpannerSegment* segm : item->spannerSegments()) {
        layoutItem(segm, ctx);
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
    item->setbbox(r.adjusted(-lw, -lw, lw, lw));

    item->addLineAttachPoints();
}

void TLayout::layout(GlissandoSegment* item, LayoutContext&)
{
    if (item->pos2().x() <= 0) {
        item->setbbox(RectF());
        return;
    }

    if (item->staff()) {
        item->setMag(item->staff()->staffMag(item->tick()));
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
        double xpos = offset - item->parent()->rxoffset() - item->parent()->xpos();
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
    xPos = std::min(xPos, -double(ctx.style().styleMM(Sid::graceToMainNoteDist) + firstGN->notes().front()->headWidth() / 2));
    item->setPos(xPos, 0.0);
}

void TLayout::layout(GradualTempoChangeSegment* item, LayoutContext& ctx)
{
    layoutTextLineBaseSegment(item, ctx);

    if (item->isStyled(Pid::OFFSET)) {
        item->roffset() = item->tempoChange()->propertyDefault(Pid::OFFSET).value<PointF>();
    }
    item->autoplaceSpannerSegment();
}

void TLayout::layout(GradualTempoChange* item, LayoutContext& ctx)
{
    layoutLine(item, ctx);
}

void TLayout::layout(HairpinSegment* item, LayoutContext& ctx)
{
    const StaffType* stType = item->staffType();

    item->setSkipDraw(false);
    if (stType && stType->isHiddenElementOnTab(ctx.style(), Sid::hairpinShowTabCommon, Sid::hairpinShowTabSimple)) {
        item->setSkipDraw(true);
        return;
    }

    const double _spatium = item->spatium();
    const track_idx_t _trck = item->track();
    Dynamic* sd = nullptr;
    Dynamic* ed = nullptr;
    double dymax = item->hairpin()->placeBelow() ? -10000.0 : 10000.0;
    if (item->autoplace() && !ctx.isPaletteMode()) {
        Segment* start = item->hairpin()->startSegment();
        Segment* end = item->hairpin()->endSegment();
        // Try to fit between adjacent dynamics
        double minDynamicsDistance = ctx.style().styleMM(Sid::autoplaceHairpinDynamicsDistance) * item->staff()->staffMag(item->tick());
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
                double sdRight = sd->pos().x() + segmentXPos + sd->bbox().right();
                if (sd->snappedExpression()) {
                    Expression* expression = sd->snappedExpression();
                    double exprRight = expression->pos().x() + segmentXPos + expression->bbox().right();
                    sdRight = std::max(sdRight, exprRight);
                }
                const double dist    = std::max(sdRight - item->pos().x() + minDynamicsDistance, 0.0);
                item->movePosX(dist);
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
                const double edLeft  = ed->bbox().left() + ed->pos().x()
                                       + ed->segment()->pos().x() + ed->measure()->pos().x();
                const double dist    = edLeft - item->pos2().x() - item->pos().x() - minDynamicsDistance;
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

        RectF r = RectF(l1.p1(), l1.p2()).normalized().united(RectF(l2.p1(), l2.p2()).normalized());
        if (!item->text()->empty()) {
            r.unite(item->text()->bbox());
        }
        if (!item->endText()->empty()) {
            r.unite(item->endText()->bbox().translated(x + item->endText()->bbox().width(), 0.0));
        }
        double w  = item->point(ctx.style().styleS(Sid::hairpinLineWidth));
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
    if (item->offsetChanged() != OffsetChange::NONE) {
        rebase = item->rebaseOffset();
    }

    if (item->autoplace()) {
        double ymax = item->pos().y();
        double d;
        double ddiff = item->hairpin()->isLineType() ? 0.0 : _spatium * 0.5;

        double sp = item->spatium();

        // TODO: in the future, there should be a minDistance style setting for hairpinLines as well as hairpins.
        double minDist = item->twoLines() ? item->minDistance().val() : ctx.style().styleS(Sid::dynamicsMinDistance).val();
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
            if (item->offsetChanged() != OffsetChange::NONE) {
                // user moved element within the skyline
                // we may need to adjust minDistance, yd, and/or offset
                double adj = item->pos().y() + rebase;
                bool inStaff = above ? sh.bottom() + adj > 0.0 : sh.top() + adj < item->staff()->height();
                item->rebaseMinDistance(md, yd, sp, rebase, above, inStaff);
            }
            item->movePosY(yd);
        }

        if (item->hairpin()->addToSkyline() && !item->hairpin()->diagonal()) {
            // align dynamics with hairpin
            if (sd && sd->autoplace() && sd->placement() == item->hairpin()->placement()) {
                double ny = item->y() + ddiff - sd->offset().y();
                if (sd->placeAbove()) {
                    ny = std::min(ny, sd->ipos().y());
                } else {
                    ny = std::max(ny, sd->ipos().y());
                }
                if (sd->ipos().y() != ny) {
                    sd->setPosY(ny);
                    if (sd->snappedExpression()) {
                        sd->snappedExpression()->setPosY(ny);
                    }
                    if (sd->addToSkyline()) {
                        Segment* s = sd->segment();
                        Measure* m = s->measure();
                        RectF r = sd->bbox().translated(sd->pos());
                        s->staffShape(sd->staffIdx()).add(r);
                        r = sd->bbox().translated(sd->pos() + s->pos() + m->pos());
                        m->system()->staff(sd->staffIdx())->skyline().add(r);
                    }
                }
            }
            if (ed && ed->autoplace() && ed->placement() == item->hairpin()->placement()) {
                double ny = item->y() + ddiff - ed->offset().y();
                if (ed->placeAbove()) {
                    ny = std::min(ny, ed->ipos().y());
                } else {
                    ny = std::max(ny, ed->ipos().y());
                }
                if (ed->ipos().y() != ny) {
                    ed->setPosY(ny);
                    if (ed->snappedExpression()) {
                        ed->snappedExpression()->setPosY(ny);
                    }
                    if (ed->addToSkyline()) {
                        Segment* s = ed->segment();
                        Measure* m = s->measure();
                        RectF r = ed->bbox().translated(ed->pos());
                        s->staffShape(ed->staffIdx()).add(r);
                        r = ed->bbox().translated(ed->pos() + s->pos() + m->pos());
                        m->system()->staff(ed->staffIdx())->skyline().add(r);
                    }
                }
            }
        }
    }
    item->setOffsetChanged(false);
}

void TLayout::layout(Hairpin* item, LayoutContext& ctx)
{
    item->setPos(0.0, 0.0);
    layoutTextLineBase(item, ctx);
}

void TLayout::layout(HarpPedalDiagram* item, LayoutContext& ctx)
{
    item->updateDiagramText();
    layoutTextBase(item, ctx);
    item->autoplaceSegmentElement();
}

void TLayout::layout(HarmonicMarkSegment* item, LayoutContext& ctx)
{
    const StaffType* stType = item->staffType();

    item->setSkipDraw(false);
    if (stType
        && (!stType->isTabStaff()
            || stType->isHiddenElementOnTab(ctx.style(), Sid::harmonicMarkShowTabCommon, Sid::harmonicMarkShowTabSimple))) {
        item->setSkipDraw(true);
        return;
    }

    layoutTextLineBaseSegment(item, ctx);

    item->autoplaceSpannerSegment();
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

void TLayout::layout1(Harmony* item, LayoutContext& ctx)
{
    if (item->isLayoutInvalid()) {
        item->createLayout();
    }

    if (item->textBlockList().empty()) {
        item->textBlockList().push_back(TextBlock());
    }

    auto positionPoint = calculateBoundingRect(item, ctx);

    if (item->hasFrame()) {
        item->layoutFrame();
    }

    ctx.addRefresh(item->canvasBoundingRect());
    item->setPos(positionPoint);
}

PointF TLayout::calculateBoundingRect(Harmony* item, LayoutContext& ctx)
{
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
            newPosY = item->ypos();
        } else {
            newPosY = ypos - ((item->align() == AlignV::BOTTOM) ? item->harmonyHeight() - item->bbox().height() : 0.0);
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
            newPosY = ypos - yy - ctx.style().styleMM(Sid::harmonyFretDist);
        } else {
            newPosY = ypos;
        }

        for (TextSegment* ts : item->textList()) {
            ts->offset = PointF(xx, yy);
        }

        item->setbbox(bb.translated(xx, yy));
        item->setHarmonyHeight(item->bbox().height());
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
    item->setbbox(item->symBbox(item->sym()));
}

void TLayout::layout(Image* item, LayoutContext&)
{
    item->setPos(0.0, 0.0);
    item->init();

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
            imageSize = item->pixel2size(item->parentItem()->bbox().size());
        }
    }

    item->setSize(imageSize);

    // in any case, adjust position relative to parent
    item->setbbox(RectF(PointF(), item->size2pixel(imageSize)));
}

void TLayout::layout(InstrumentChange* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx);
    item->autoplaceSegmentElement();
}

void TLayout::layout(InstrumentName* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx);
}

void TLayout::layout(Jump* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx);
    item->autoplaceMeasureElement();
}

void TLayout::layout(KeySig* item, LayoutContext& ctx)
{
    double _spatium = item->spatium();
    double step = _spatium * (item->staff() ? item->staff()->staffTypeForElement(item)->lineDistance().val() * 0.5 : 0.5);

    item->setbbox(RectF());

    item->keySymbols().clear();
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
        double accidentalGap = ctx.style().styleS(Sid::keysigAccidentalDistance).val();
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
                if (item->keySymbols().size() > 0) {
                    KeySym& previous = item->keySymbols().back();
                    double previousWidth = item->symWidth(previous.sym) / _spatium;
                    ks.xPos = previous.xPos + previousWidth + accidentalGap;
                } else {
                    ks.xPos = 0;
                }
                // TODO octave metters?
                item->keySymbols().push_back(ks);
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
            if (item->keySymbols().size() > 0) {
                KeySym& previous = item->keySymbols().back();
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
                item->keySymbols().push_back(ks);
                xpos += t1 < 0 ? 0.7 : 1; // flats closer
            }
            // create symbol; natural only if is user defined
            if (sym != SymId::accidentalNatural || sym == cd.sym) {
                KeySym ks;
                ks.sym = sym;
                ks.line = line;
                ks.xPos = xpos;
                item->keySymbols().push_back(ks);
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
            naturalsOn = !newSection && (ctx.style().styleI(Sid::keySigNaturals) != int(KeySigNatural::NONE) || (t1 == 0));
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
                              && (ctx.style().styleI(Sid::keySigNaturals) == int(KeySigNatural::BEFORE)
                                  || t1 * int(t2) < 0);

        // naturals should go AFTER accidentals if they should not go before!
        bool suffixNaturals = naturalsOn && !prefixNaturals;

        const signed char* lines = ClefInfo::lines(clef);

        if (prefixNaturals) {
            for (int i = 0; i < 7; ++i) {
                if (naturals & (1 << i)) {
                    keySigAddLayout(item, ctx, SymId::accidentalNatural, lines[i + coffset]);
                }
            }
        }
        if (abs(t1) <= 7) {
            SymId symbol = t1 > 0 ? SymId::accidentalSharp : SymId::accidentalFlat;
            int lineIndexOffset = t1 > 0 ? 0 : 7;
            for (int i = 0; i < abs(t1); ++i) {
                keySigAddLayout(item, ctx, symbol, lines[lineIndexOffset + i]);
            }
        } else {
            LOGD("illegal t1 key %d", t1);
        }

        // add suffixed naturals, if any
        if (suffixNaturals) {
            for (int i = 0; i < 7; ++i) {
                if (naturals & (1 << i)) {
                    keySigAddLayout(item, ctx, SymId::accidentalNatural, lines[i + coffset]);
                }
            }
        }

        // Follow stepOffset
        if (item->staffType()) {
            item->setPosY(item->staffType()->stepOffset() * 0.5 * _spatium);
        }
    }

    // compute bbox
    for (const KeySym& ks : item->keySymbols()) {
        double x = ks.xPos * _spatium;
        double y = ks.line * step;
        item->addbbox(item->symBbox(ks.sym).translated(x, y));
    }
}

void TLayout::keySigAddLayout(KeySig* item, LayoutContext& ctx, SymId sym, int line)
{
    double _spatium = item->spatium();
    double step = _spatium * (item->staff() ? item->staff()->staffTypeForElement(item)->lineDistance().val() * 0.5 : 0.5);
    KeySym ks;
    ks.sym = sym;
    double x = 0.0;
    if (item->keySymbols().size() > 0) {
        KeySym& previous = item->keySymbols().back();
        double accidentalGap = ctx.style().styleS(Sid::keysigAccidentalDistance).val();
        if (previous.sym != sym) {
            accidentalGap *= 2;
        } else if (previous.sym == SymId::accidentalNatural && sym == SymId::accidentalNatural) {
            accidentalGap = ctx.style().styleS(Sid::keysigNaturalDistance).val();
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
    item->keySymbols().push_back(ks);
}

void TLayout::layout(LayoutBreak* item, LayoutContext&)
{
    UNUSED(item);
}

void TLayout::layout(LedgerLine* item, LayoutContext& ctx)
{
    item->setLineWidth(ctx.style().styleMM(Sid::ledgerLineWidth) * item->chord()->mag());
    if (item->staff()) {
        item->setColor(item->staff()->staffType(item->tick())->color());
    }
    double w2 = item->lineWidth() * .5;

    //Adjust Y position to staffType offset
    if (item->staffType()) {
        item->movePosY(item->staffType()->yoffset().val() * item->spatium());
    }

    if (item->vertical()) {
        item->bbox().setRect(-w2, 0, w2, item->len());
    } else {
        item->bbox().setRect(0, -w2, item->len(), w2);
    }
}

void TLayout::layout(LetRing* item, LayoutContext& ctx)
{
    layoutLine(item, ctx);
}

void TLayout::layout(LetRingSegment* item, LayoutContext& ctx)
{
    const StaffType* stType = item->staffType();

    item->setSkipDraw(false);
    if (stType && stType->isHiddenElementOnTab(ctx.style(), Sid::letRingShowTabCommon, Sid::letRingShowTabSimple)) {
        item->setSkipDraw(true);
        return;
    }

    layoutTextLineBaseSegment(item, ctx);
    item->autoplaceSpannerSegment();
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

void TLayout::layout(Marker* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx);

    // although normally laid out to parent (measure) width,
    // force to center over barline if left-aligned

    if (!ctx.isPaletteMode() && item->layoutToParentWidth() && item->align() == AlignH::LEFT) {
        item->movePosX(-item->width() * 0.5);
    }

    item->autoplaceMeasureElement();
}

void TLayout::layout(MeasureBase* item, LayoutContext& ctx)
{
    layoutItem(item, ctx);
}

void TLayout::layoutMeasureBase(MeasureBase* item, LayoutContext& ctx)
{
    int breakCount = 0;

    for (EngravingItem* element : item->el()) {
        if (element->isLayoutBreak()) {
            double _spatium = item->spatium();
            double x;
            double y;
            if (toLayoutBreak(element)->isNoBreak()) {
                x = item->width() + ctx.style().styleMM(Sid::barWidth) - element->width() * .5;
            } else {
                x = item->width() + ctx.style().styleMM(Sid::barWidth) - element->width()
                    - breakCount * (element->width() + _spatium * .5);
                breakCount++;
            }
            y = -2.5 * _spatium - element->height();
            element->setPos(x, y);
        } else if (element->isMarker() || element->isJump()) {
        } else {
            layoutItem(element, ctx);
        }
    }
}

void TLayout::layout(MeasureNumber* item, LayoutContext& ctx)
{
    layoutMeasureNumberBase(item, ctx);
}

void TLayout::layoutMeasureNumberBase(MeasureNumberBase* item, LayoutContext& ctx)
{
    item->setPos(PointF());
    if (!item->explicitParent()) {
        item->setOffset(0.0, 0.0);
    }

    // TextBase::layout1() needs to be called even if there's no measure attached to it.
    // This happens for example in the palettes.
    layout1TextBase(item, ctx);
    // this could be if (!measure()) but it is the same as current and slower
    // See implementation of MeasureNumberBase::measure().
    if (!item->explicitParent()) {
        return;
    }

    if (item->placeBelow()) {
        double yoff = item->bbox().height();

        // If there is only one line, the barline spans outside the staff lines, so the default position is not correct.
        if (item->staff()->constStaffType(item->measure()->tick())->lines() == 1) {
            yoff += 2.0 * item->spatium();
        } else {
            yoff += item->staff()->height();
        }

        item->setPosY(yoff);
    } else {
        double yoff = 0.0;

        // If there is only one line, the barline spans outside the staff lines, so the default position is not correct.
        if (item->staff()->constStaffType(item->measure()->tick())->lines() == 1) {
            yoff -= 2.0 * item->spatium();
        }

        item->setPosY(yoff);
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

        item->setPosX((x1 + x2) * 0.5);
    } else if (item->hPlacement() == PlacementH::RIGHT) {
        item->setPosX(item->measure()->width());
    }
}

void TLayout::layout(MeasureRepeat* item, LayoutContext& ctx)
{
    for (EngravingItem* e : item->el()) {
        layoutItem(e, ctx);
    }

    switch (item->numMeasures()) {
    case 1:
    {
        item->setSymId(SymId::repeat1Bar);
        if (ctx.style().styleB(Sid::mrNumberSeries) && item->track() != mu::nidx) {
            int placeInSeries = 2; // "1" would be the measure actually being repeated
            staff_idx_t staffIdx = item->staffIdx();
            Measure* m = item->measure();
            while (m && m->isOneMeasureRepeat(staffIdx) && m->prevIsOneMeasureRepeat(staffIdx)) {
                placeInSeries++;
                m = m->prevMeasure();
            }
            if (placeInSeries % ctx.style().styleI(Sid::mrNumberEveryXMeasures) == 0) {
                if (ctx.style().styleB(Sid::mrNumberSeriesWithParentheses)) {
                    item->setNumberSym(String(u"(%1)").arg(placeInSeries));
                } else {
                    item->setNumberSym(placeInSeries);
                }
            } else {
                item->clearNumberSym();
            }
        } else if (ctx.style().styleB(Sid::oneMeasureRepeatShow1)) {
            item->setNumberSym(1);
        } else {
            item->clearNumberSym();
        }
        break;
    }
    case 2:
        item->setSymId(SymId::repeat2Bars);
        item->setNumberSym(item->numMeasures());
        break;
    case 4:
        item->setSymId(SymId::repeat4Bars);
        item->setNumberSym(item->numMeasures());
        break;
    default:
        item->setSymId(SymId::noSym); // should never happen
        item->clearNumberSym();
        break;
    }

    RectF bbox = item->symBbox(item->symId());

    if (item->track() != mu::nidx) { // if this is in score rather than a palette cell
        // For unknown reasons, the symbol has some offset in almost all SMuFL fonts
        // We compensate for it, to make sure the symbol is visually centered around the staff line
        double offset = (-bbox.top() - bbox.bottom()) / 2.0;

        const StaffType* staffType = item->staffType();

        // Only need to set y position here; x position is handled in MeasureLayout::layoutMeasureElements()
        item->setPos(0, std::floor(staffType->middleLine() / 2.0) * staffType->lineDistance().val() * item->spatium() + offset);
    }

    item->setbbox(bbox);

    if (item->track() != mu::nidx && !item->numberSym().empty()) {
        item->addbbox(item->numberRect());
    }
}

void TLayout::layout(MMRest* item, LayoutContext& ctx)
{
    item->setNumber(item->measure()->mmRestCount());
    item->setNumberSym(item->number());

    for (EngravingItem* e : item->el()) {
        layoutItem(e, ctx);
    }

    if (ctx.style().styleB(Sid::oldStyleMultiMeasureRests)) {
        SymIdList restSyms;
        double symsWidth = 0.0;

        int remaining = item->number();
        double spacing = ctx.style().styleMM(Sid::mmRestOldStyleSpacing);
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

        item->setRestSyms(restSyms);
        item->setSymsWidth(symsWidth);

        double symHeight = item->symBbox(item->restSyms().at(0)).height();
        item->setbbox(RectF((item->width() - item->symsWidth()) * .5, -item->spatium(), item->symsWidth(), symHeight));
    } else { // H-bar
        double vStrokeHeight = ctx.style().styleMM(Sid::mmRestHBarVStrokeHeight);
        item->setbbox(RectF(0.0, -(vStrokeHeight * .5), item->width(), vStrokeHeight));
    }

    // Only need to set y position here; x position is handled in MeasureLayout::layoutMeasureElements()
    const StaffType* staffType = item->staffType();
    item->setPos(0, (staffType->middleLine() / 2.0) * staffType->lineDistance().val() * item->spatium());

    if (item->numberVisible()) {
        item->addbbox(item->numberRect());
    }
}

void TLayout::layout(MMRestRange* item, LayoutContext& ctx)
{
    layoutMeasureNumberBase(item, ctx);
}

void TLayout::layout(Note* item, LayoutContext&)
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
            item->setFretString(u"/");
        } else {
            item->setFretString(tab->fretString(fabs(item->fret()), item->string(), item->deadNote()));

            if (item->negativeFretUsed()) {
                item->setFretString(u"-" + item->fretString());
            }

            if (item->displayFret() == Note::DisplayFretOption::ArtificialHarmonic) {
                item->setFretString(String(u"%1 <%2>").arg(item->fretString(), String::number(item->harmonicFret())));
            } else if (item->displayFret() == Note::DisplayFretOption::NaturalHarmonic) {
                item->setFretString(String(u"<%1>").arg(String::number(item->harmonicFret())));
            }
        }

        if ((item->ghost() && !Note::engravingConfiguration()->tablatureParenthesesZIndexWorkaround())) {
            item->setFretString(String(u"(%1)").arg(item->fretString()));
        }

        double w = item->tabHeadWidth(tab);     // !! use _fretString
        item->bbox().setRect(0, tab->fretBoxY() * mags, w, tab->fretBoxH() * mags);

        if (item->ghost() && Note::engravingConfiguration()->tablatureParenthesesZIndexWorkaround()) {
            item->bbox().setWidth(w + item->symWidth(SymId::noteheadParenthesisLeft) + item->symWidth(SymId::noteheadParenthesisRight));
        } else {
            item->bbox().setWidth(w);
        }
    } else {
        if (item->deadNote()) {
            item->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
        } else if (item->harmonic()) {
            item->setHeadGroup(NoteHeadGroup::HEAD_DIAMOND);
        }
        SymId nh = item->noteHead();
        if (Note::engravingConfiguration()->crossNoteHeadAlwaysBlack() && ((nh == SymId::noteheadXHalf) || (nh == SymId::noteheadXWhole))) {
            nh = SymId::noteheadXBlack;
        }

        item->setCachedNoteheadSym(nh);

        if (item->isNoteName()) {
            item->setCachedSymNull(SymId::noteEmptyBlack);
            NoteHeadType ht = item->headType() == NoteHeadType::HEAD_AUTO ? item->chord()->durationType().headType() : item->headType();
            if (ht == NoteHeadType::HEAD_WHOLE) {
                item->setCachedSymNull(SymId::noteEmptyWhole);
            } else if (ht == NoteHeadType::HEAD_HALF) {
                item->setCachedSymNull(SymId::noteEmptyHalf);
            }
        } else {
            item->setCachedSymNull(SymId::noSym);
        }
        item->setbbox(item->symBbox(nh));
    }
}

void TLayout::layout(NoteDot* item, LayoutContext&)
{
    item->setbbox(item->symBbox(SymId::augmentationDot));
}

void TLayout::layout(Ornament* item, LayoutContext& ctx)
{
    double _spatium = item->spatium();
    double vertMargin = 0.35 * _spatium;
    static constexpr double ornamentAccidentalMag = 0.6; // TODO: style?

    if (!item->showCueNote()) {
        for (size_t i = 0; i < item->accidentalsAboveAndBelow().size(); ++i) {
            bool above = (i == 0);
            Accidental* accidental = item->accidentalsAboveAndBelow()[i];
            if (!accidental) {
                continue;
            }
            accidental->computeMag();
            accidental->setMag(accidental->mag() * ornamentAccidentalMag);
            layout(accidental, ctx);
            Shape accidentalShape = accidental->shape();
            double minVertDist = above ? accidentalShape.minVerticalDistance(item->bbox()) : Shape(item->bbox()).minVerticalDistance(
                accidentalShape);
            accidental->setPos(-0.5 * accidental->width(), above ? (-minVertDist - vertMargin) : (minVertDist + vertMargin));
        }
        return;
    }

    if (!item->explicitParent()) {
        return;
    }

    Chord* parentChord = toChord(item->parentItem());
    Chord* cueNoteChord = item->cueNoteChord();

    if (!cueNoteChord) {
        return;
    }

    Note* cueNote = cueNoteChord->notes().front();
    ChordLayout::layoutChords3(ctx.style(), { cueNoteChord }, { cueNote }, item->staff(), ctx);
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
    cueNoteChord->setPosX(minDist);
}

void TLayout::layout(Ottava* item, LayoutContext& ctx)
{
    layoutLine(item, ctx);
}

void TLayout::layout(OttavaSegment* item, LayoutContext& ctx)
{
    layoutTextLineBaseSegment(item, ctx);
    item->autoplaceSpannerSegment();
}

void TLayout::layout(PalmMute* item, LayoutContext& ctx)
{
    layoutLine(item, ctx);
}

void TLayout::layout(PalmMuteSegment* item, LayoutContext& ctx)
{
    const StaffType* stType = item->staffType();

    item->setSkipDraw(false);
    if (stType && stType->isHiddenElementOnTab(ctx.style(), Sid::palmMuteShowTabCommon, Sid::palmMuteShowTabSimple)) {
        item->setSkipDraw(true);
        return;
    }

    layoutTextLineBaseSegment(item, ctx);
    item->autoplaceSpannerSegment();
}

void TLayout::layout(Pedal* item, LayoutContext& ctx)
{
    layoutLine(item, ctx);
}

void TLayout::layout(PedalSegment* item, LayoutContext& ctx)
{
    layoutTextLineBaseSegment(item, ctx);
    if (item->isStyled(Pid::OFFSET)) {
        item->roffset() = item->pedal()->propertyDefault(Pid::OFFSET).value<PointF>();
    }
    item->autoplaceSpannerSegment();
}

void TLayout::layout(PickScrapeSegment* item, LayoutContext& ctx)
{
    layoutTextLineBaseSegment(item, ctx);
    item->autoplaceSpannerSegment();
}

void TLayout::layout(PlayTechAnnotation* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx);
    item->autoplaceSegmentElement();
}

void TLayout::layout(RasgueadoSegment* item, LayoutContext& ctx)
{
    const StaffType* stType = item->staffType();

    item->setSkipDraw(false);
    if (stType && stType->isHiddenElementOnTab(ctx.style(), Sid::rasgueadoShowTabCommon, Sid::rasgueadoShowTabSimple)) {
        item->setSkipDraw(true);
        return;
    }

    layoutTextLineBaseSegment(item, ctx);
    item->autoplaceSpannerSegment();
}

void TLayout::layout(RehearsalMark* item, LayoutContext& ctx)
{
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
                item->setPosX(barlineX);
            } else {
                // header at start of system
                // align to a point just after the header
                EngravingItem* e = header->element(item->track());
                double w = e ? e->width() : header->width();
                item->setPosX(header->x() + w - s->x());

                // special case for right aligned rehearsal marks at start of system
                // left align with start of measure if that is further left
                if (item->align() == AlignH::RIGHT) {
                    item->setPosX(std::min(item->xpos(), measureX + item->width()));
                }
            }
        }
        item->autoplaceSegmentElement();
    }
}

void TLayout::layout(Rest* item, LayoutContext& ctx)
{
    if (item->isGap()) {
        return;
    }
    for (EngravingItem* e : item->el()) {
        layoutItem(e, ctx);
    }

    item->setSkipDraw(false);
    if (item->deadSlapped()) {
        item->setSkipDraw(true);
        return;
    }

    double _spatium = item->spatium();

    item->setPosX(0.0);
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
                item->setTabDur(new TabDurationSymbol(item, stt, type, dots));
            } else {
                item->tabDur()->setDuration(type, dots, stt);
            }
            item->tabDur()->setParent(item);
// needed?        _tabDur->setTrack(track());
            layout(item->tabDur(), ctx);
            item->setbbox(item->tabDur()->bbox());
            item->setPos(0.0, 0.0);                   // no rest is drawn: reset any position might be set for it
            return;
        }
        // if no rests or no duration symbols, delete any dur. symbol and chain into standard staff mngmt
        // this is to ensure horiz space is reserved for rest, even if they are not displayed
        // Rest::draw() will skip their drawing, if not needed
        if (item->tabDur()) {
            delete item->tabDur();
            item->setTabDur(nullptr);
        }
    }

    item->setDotLine(Rest::getDotline(item->durationType().type()));

    double yOff = item->offset().y();
    const Staff* stf = item->staff();
    const StaffType* st = stf ? stf->staffTypeForElement(item) : 0;
    double lineDist = st ? st->lineDistance().val() : 1.0;
    int userLine   = yOff == 0.0 ? 0 : lrint(yOff / (lineDist * _spatium));
    int lines      = st ? st->lines() : 5;

    int naturalLine = item->computeNaturalLine(lines); // Measured in 1sp steps
    int voiceOffset = item->computeVoiceOffset(lines); // Measured in 1sp steps
    int wholeRestOffset = item->computeWholeRestOffset(voiceOffset, lines);
    int finalLine = naturalLine + voiceOffset + wholeRestOffset;

    item->setSym(item->getSymbol(item->durationType().type(), finalLine + userLine, lines));

    item->setPosY(finalLine * lineDist * _spatium);
    if (!item->shouldNotBeDrawn()) {
        item->setbbox(item->symBbox(item->sym()));
    }
    layoutRestDots(item, ctx);
}

void TLayout::layoutRestDots(Rest* item, LayoutContext& ctx)
{
    item->checkDots();
    double x = item->symWidthNoLedgerLines() + ctx.style().styleMM(Sid::dotNoteDistance) * item->mag();
    double dx = ctx.style().styleMM(Sid::dotDotDistance) * item->mag();
    double y = item->dotLine() * item->spatium() * .5;
    for (NoteDot* dot : item->dotList()) {
        layout(dot, ctx);
        dot->setPos(x, y);
        x += dx;
    }
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
        double d  = ctx.style().styleMM(Sid::dotNoteDistance) * item->mag();
        double dd = ctx.style().styleMM(Sid::dotDotDistance) * item->mag();
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

        double stemWidth = ctx.style().styleMM(Sid::stemWidth);
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
        double extraLen = ctx.style().styleMM(Sid::ledgerLineLength) * item->mag();
        double step = 0.5 * _spatium * item->staffType()->lineDistance().val();
        double x = noteheadBbox.x() - extraLen;
        double w = noteheadBbox.width() + 2 * extraLen;

        double lw = ctx.style().styleMM(Sid::ledgerLineWidth);

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
    if (ctx.isPaletteMode() || (item->tick() == Fraction(-1, 1)) || (item->tick2() == Fraction::fromTicks(1))) {
        //
        // when used in a palette or while dragging from palette,
        // SLine has no parent and
        // tick and tick2 has no meaning so no layout is
        // possible and needed
        //
        if (item->spannerSegments().empty()) {
            item->setLen(ctx.spatium() * 7);
        }

        LineSegment* lineSegm = item->frontSegment();
        layout(lineSegm, ctx);
        item->setbbox(lineSegm->bbox());
        return;
    }

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
    const Staff* s = item->staff();
    double _spatium = item->spatium();
    double dist     = _spatium;
    item->setPos(PointF(0.0, 0.0));
    int _lines;
    if (s) {
        item->setMag(s->staffMag(item->measure()->tick()));
        item->setVisible(!s->isLinesInvisible(item->measure()->tick()));
        item->setColor(s->color(item->measure()->tick()));
        const StaffType* st = s->staffType(item->measure()->tick());
        dist         *= st->lineDistance().val();
        _lines        = st->lines();
        item->setPosY(st->yoffset().val() * _spatium);
//            if (_lines == 1)
//                  rypos() = 2 * _spatium;
    } else {
        _lines = 5;
        item->setColor(StaffLines::engravingConfiguration()->defaultColor());
    }
    item->setLw(ctx.style().styleS(Sid::staffLineWidth).val() * _spatium);
    double x1 = item->pos().x();
    double x2 = x1 + w;
    double y  = item->pos().y();
    item->bbox().setRect(x1, -item->lw() * .5 + y, w, (_lines - 1) * dist + item->lw());

    std::vector<mu::LineF> ll;
    for (int i = 0; i < _lines; ++i) {
        ll.push_back(LineF(x1, y, x2, y));
        y += dist;
    }
    item->setLines(ll);
}

void TLayout::layout(StaffState* item, LayoutContext&)
{
    double _spatium = item->spatium();
    item->setLw(_spatium * 0.3);
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

    item->setPath(path);

    RectF bb(0, 0, w, h);
    bb.adjust(-item->lw(), -item->lw(), item->lw(), item->lw());
    item->setbbox(bb);
    item->setPos(0.0, _spatium * -6.0);
}

void TLayout::layout(StaffText* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx);
    item->autoplaceSegmentElement();
}

void TLayout::layout(StaffTypeChange* item, LayoutContext& ctx)
{
    double _spatium = ctx.spatium();
    item->setbbox(RectF(-item->lw() * .5, -item->lw() * .5, _spatium * 2.5 + item->lw(), _spatium * 2.5 + item->lw()));
    if (item->measure()) {
        double y = -1.5 * _spatium - item->height() + item->measure()->system()->staff(item->staffIdx())->y();
        item->setPos(_spatium * .8, y);
    } else {
        item->setPos(0.0, 0.0);
    }
}

void TLayout::layout(Stem* item, LayoutContext& ctx)
{
    const bool up = item->up();
    const double _up = up ? -1.0 : 1.0;

    double y1 = 0.0; // vertical displacement to match note attach point
    double y2 = _up * (item->length());

    bool isTabStaff = false;
    if (item->chord()) {
        item->setMag(item->chord()->mag());

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

            item->setPosY(note->ypos());
        }

        if (item->chord()->hook() && !item->chord()->beam()) {
            y2 += item->chord()->hook()->smuflAnchor().y();
        }

        if (item->chord()->beam()) {
            y2 -= _up * item->point(ctx.style().styleS(Sid::beamWidth)) * .5 * item->chord()->beam()->mag();
        }
    }

    double lineWidthCorrection = item->lineWidthMag() * 0.5;
    double lineX = isTabStaff ? 0.0 : _up * lineWidthCorrection;

    LineF line = LineF(lineX, y1, lineX, y2);
    item->setLine(line);

    // HACK: if there is a beam, extend the bounding box of the stem (NOT the stem itself) by half beam width.
    // This way the bbox of the stem covers also the beam position. Hugely helps with all the collision checks.
    double beamCorrection = (item->chord() && item->chord()->beam()) ? _up * ctx.style().styleMM(Sid::beamWidth) * item->mag() / 2 : 0.0;
    // compute line and bounding rectangle
    RectF rect(line.p1(), line.p2() + PointF(0.0, beamCorrection));
    item->setbbox(rect.normalized().adjusted(-lineWidthCorrection, 0, lineWidthCorrection, 0));
}

void TLayout::layout(StemSlash* item, LayoutContext& ctx)
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

    double up = c->up() ? -1 : 1;
    double stemTipY = c->up() ? stem->bbox().translated(stem->pos()).top() : stem->bbox().translated(stem->pos()).bottom();
    double leftHang = ctx.noteHeadWidth() * ctx.style().styleD(Sid::graceNoteMag) / 2;
    double angle = ctx.style().styleD(Sid::stemSlashAngle) * M_PI / 180; // converting to radians
    bool straight = ctx.style().styleB(Sid::useStraightNoteFlags);
    double graceNoteMag = ctx.style().styleD(Sid::graceNoteMag);

    double startX = stem->bbox().translated(stem->pos()).right() - leftHang;

    double startY;
    if (straight || beam) {
        startY = stemTipY - up * graceNoteMag * ctx.style().styleMM(Sid::stemSlashPosition) * heightReduction;
    } else {
        startY = stemTipY - up * graceNoteMag * ctx.style().styleMM(Sid::stemSlashPosition);
    }

    double endX = 0;
    double endY = 0;

    if (hook) {
        auto musicFont = ctx.style().styleSt(Sid::MusicalSymbolFont);
        // HACK: adjust slash angle for fonts with "fat" hooks. In future, we must use smufl cutOut
        if (c->beams() >= 2 && !straight
            && (musicFont == "Bravura" || musicFont == "Finale Maestro" || musicFont == "Gonville")) {
            angle *= angleIncrease;
        }
        endX = hook->bbox().translated(hook->pos()).right(); // always ends at the right bbox margin of the hook
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

    item->setLine(LineF(PointF(startX, startY), PointF(endX, endY)));
    item->setStemWidth(ctx.style().styleMM(Sid::stemSlashThickness) * graceNoteMag);

    RectF bbox = RectF(item->line().p1(), item->line().p2()).normalized();
    bbox = bbox.adjusted(-item->stemWidth() / 2, -item->stemWidth() / 2, item->stemWidth(), item->stemWidth());
    item->setbbox(bbox);
}

void TLayout::layout(Sticking* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx);
    item->autoplaceSegmentElement();
}

void TLayout::layout(StretchedBend* item, LayoutContext& ctx)
{
    doLayout(item, ctx, false);
}

void TLayout::layoutStretched(StretchedBend* item, LayoutContext& ctx)
{
    doLayout(item, ctx, true);
}

void TLayout::doLayout(StretchedBend* item, LayoutContext&, bool stretchedMode)
{
    item->m_stretchedMode = stretchedMode;

    // preLayout
    {
        Note* note = toNote(item->explicitParent());
        item->m_notePos   = note->pos();
        item->m_noteWidth = note->width();
        item->m_noteHeight = note->height();

        item->fillArrows();
        item->fillSegments();
        item->stretchSegments();
    }

    item->layoutDraw(true);

    // postLayout
    {
        double lw = item->lineWidth();
        RectF& bRect = item->bbox();
        bRect.adjust(-lw, -lw, lw, lw);
        item->setPos(0.0, 0.0);
    }
}

void TLayout::layoutBaseSymbol(BSymbol* item, LayoutContext& ctx)
{
    if (item->explicitParent() && item->parentItem()->isNote()) {
        item->setMag(item->parentItem()->mag());
    } else if (item->staff()) {
        item->setMag(item->staff()->staffMag(item->tick()));
    }

    if (!item->explicitParent()) {
        item->setOffset(.0, .0);
        item->setPos(.0, .0);
    }
    for (EngravingItem* e : item->leafs()) {
        layoutItem(e, ctx);
    }
}

void TLayout::layout(Symbol* item, LayoutContext& ctx)
{
    item->setbbox(item->scoreFont() ? item->scoreFont()->bbox(item->sym(), item->magS()) : item->symBbox(item->sym()));
    double w = item->width();
    PointF p;
    if (item->align() == AlignV::BOTTOM) {
        p.setY(-item->height());
    } else if (item->align() == AlignV::VCENTER) {
        p.setY((-item->height()) * .5);
    } else if (item->align() == AlignV::BASELINE) {
        p.setY(-item->baseLine());
    }
    if (item->align() == AlignH::RIGHT) {
        p.setX(-w);
    } else if (item->align() == AlignH::HCENTER) {
        p.setX(-(w * .5));
    }
    item->setPos(p);
    layoutBaseSymbol(item, ctx);
}

void TLayout::layout(FSymbol* item, LayoutContext&)
{
    item->setbbox(mu::draw::FontMetrics::boundingRect(item->font(), item->toString()));
}

void TLayout::layout(SystemDivider* item, LayoutContext& ctx)
{
    SymId sid;

    if (item->dividerType() == SystemDivider::Type::LEFT) {
        sid = SymNames::symIdByName(ctx.style().styleSt(Sid::dividerLeftSym));
    } else {
        sid = SymNames::symIdByName(ctx.style().styleSt(Sid::dividerRightSym));
    }
    item->setSym(sid, ctx.engravingFont());

    layout(static_cast<Symbol*>(item), ctx);
}

void TLayout::layout(SystemText* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx);
    item->autoplaceSegmentElement();
}

void TLayout::layout(TabDurationSymbol* item, LayoutContext&)
{
    static constexpr double TAB_RESTSYMBDISPL = 2.0;

    if (!item->tab()) {
        item->setbbox(RectF());
        return;
    }
    double _spatium    = item->spatium();
    double hbb, wbb, xbb, ybb;   // bbox sizes
    double xpos, ypos;           // position coords

    item->setBeamGrid(TabBeamGrid::NONE);
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
            item->setBeamGrid(TabBeamGrid::INITIAL);
            item->setBeamLength(0.0);
        } else if (chord->beamMode() == BeamMode::MID || chord->beamMode() == BeamMode::END) {
            item->setBeamLevel(static_cast<int>(chord->durationType().type()) - static_cast<int>(font.zeroBeamLevel));
            item->setBeamGrid(item->beamLevel() < 1 ? TabBeamGrid::INITIAL : TabBeamGrid::MEDIALFINAL);
            // _beamLength and bbox x and width will be set in layout2(),
            // once horiz. positions of chords are known
        }
    }
// set this' mag from parent chord mag (include staff mag)
    double mag = chord != nullptr ? chord->mag() : 1.0;
    item->setMag(mag);
    mag = item->magS();         // local mag * score mag
// set magnified bbox and position
    item->bbox().setRect(xbb * mag, ybb * mag, wbb * mag, hbb * mag);
    item->setPos(xpos * mag, ypos * mag);
}

void TLayout::layout(TempoText* item, LayoutContext& ctx)
{
    layoutTextBase(item, ctx);

    Segment* s = item->segment();
    if (!s) {                       // for use in palette
        return;
    }

    // tempo text on first chordrest of measure should align over time sig if present
    //
    if (item->autoplace() && s->rtick().isZero()) {
        Segment* p = item->segment()->prev(SegmentType::TimeSig);
        if (p) {
            item->movePosX(-(s->x() - p->x()));
            EngravingItem* e = p->element(item->staffIdx() * VOICES);
            if (e) {
                item->movePosX(e->x());
            }
        }
    }
    item->autoplaceSegmentElement();
}

void TLayout::layout(TextBase* item, LayoutContext& ctx)
{
    layoutItem(item, ctx);
}

void TLayout::layoutTextBase(TextBase* item, LayoutContext& ctx)
{
    item->setPos(PointF());
    if (!item->explicitParent()) {
        item->setOffset(0.0, 0.0);
    }
//      else if (isStyled(Pid::OFFSET))                                   // TODO: should be set already
//            setOffset(propertyDefault(Pid::OFFSET).value<PointF>());
    if (item->placeBelow()) {
        item->setPosY(item->staff() ? item->staff()->height() : 0.0);
    }

    layout1(item, ctx);
}

void TLayout::layout1(TextBase* item, LayoutContext& ctx)
{
    if (Harmony::classof(item)) {
        layout1(static_cast<Harmony*>(item), ctx);
    } else {
        layout1TextBase(item, ctx);
    }
}

void TLayout::layout1TextBase(TextBase* item, LayoutContext& ctx)
{
    if (item->layoutInvalid()) {
        item->createLayout();
    }
    if (item->blocksRef().empty()) {
        item->blocksRef().push_back(TextBlock());
    }
    RectF bb;
    double y = 0;

    // adjust the bounding box for the text item
    for (size_t i = 0; i < item->rows(); ++i) {
        TextBlock* t = &item->blocksRef()[i];
        t->layout(item);
        const RectF* r = &t->boundingRect();

        if (r->height() == 0) {
            r = &item->blocksRef()[i - i].boundingRect();
        }
        y += t->lineSpacing();
        t->setY(y);
        bb |= r->translated(0.0, y);
    }
    double yoff = 0;
    double h    = 0;
    if (item->explicitParent()) {
        if (item->layoutToParentWidth()) {
            if (item->explicitParent()->isTBox()) {
                // hack: vertical alignment is always TOP
                item->setAlign(AlignV::TOP);
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
        item->setPos(PointF());
    }

    if (item->align() == AlignV::BOTTOM) {
        yoff += h - bb.bottom();
    } else if (item->align() == AlignV::VCENTER) {
        yoff +=  (h - (bb.top() + bb.bottom())) * .5;
    } else if (item->align() == AlignV::BASELINE) {
        yoff += h * .5 - item->blocksRef().front().lineSpacing();
    } else {
        yoff += -bb.top();
    }

    for (TextBlock& t : item->blocksRef()) {
        t.setY(t.y() + yoff);
    }

    bb.translate(0.0, yoff);

    item->setbbox(bb);
    if (item->hasFrame()) {
        item->layoutFrame();
    }
    ctx.addRefresh(item->canvasBoundingRect());
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
    layoutTextLineBaseSegment(item, ctx);
    if (item->isStyled(Pid::OFFSET)) {
        item->roffset() = item->textLine()->propertyDefault(Pid::OFFSET).value<PointF>();
    }
    item->autoplaceSpannerSegment();
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
    item->npointsRef() = 0;
    TextLineBase* tl = item->textLineBase();
    double _spatium = tl->spatium();

    if (item->spanner()->placeBelow()) {
        item->setPosY(item->staff() ? item->staff()->height() : 0.0);
    }

    // adjust Y pos to staffType offset
    if (const StaffType* st = item->staffType()) {
        item->movePosY(st->yoffset().val() * item->spatium());
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
            l = item->text()->pos().x() + item->text()->bbox().width() + gapBetweenTextAndLine;
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
    item->bbox().setRect(x1, y1, x2 - x1, y2 - y1);
    if (!item->text()->empty()) {
        item->bbox() |= item->text()->bbox().translated(item->text()->pos());      // DEBUG
    }
    // set end text position and extend bbox
    if (!item->endText()->empty()) {
        item->endText()->movePosX(item->bbox().right());
        item->bbox() |= item->endText()->bbox().translated(item->endText()->pos());
    }

    if (!(tl->lineVisible() || ctx.showInvisible())) {
        return;
    }

    if (tl->lineVisible() || !ctx.printingMode()) {
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

void TLayout::layout(TimeSig* item, LayoutContext& ctx)
{
    item->setPos(0.0, 0.0);
    double _spatium = item->spatium();

    item->setbbox(RectF());                    // prepare for an empty time signature

    TimeSig::DrawArgs drawArgs;
    drawArgs.pointLargeLeftParen = PointF();
    drawArgs.pz = PointF();
    drawArgs.pn = PointF();
    drawArgs.pointLargeRightParen = PointF();

    double lineDist;
    int numOfLines;
    TimeSigType sigType = item->timeSigType();
    const Staff* _staff = item->staff();

    if (_staff) {
        // if staff is without time sig, format as if no text at all
        if (!_staff->staffTypeForElement(item)->genTimesig()) {
            // reset position and box sizes to 0
            // LOGD("staff: no time sig");
            drawArgs.pointLargeLeftParen.rx() = 0.0;
            drawArgs.pn.rx() = 0.0;
            drawArgs.pz.rx() = 0.0;
            drawArgs.pointLargeRightParen.rx() = 0.0;
            item->setDrawArgs(drawArgs);
            item->setbbox(RectF());
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
        drawArgs.pz = PointF(0.0, yoff);
        RectF bbox = font->bbox(SymId::timeSigCommon, mag);
        item->setbbox(bbox.translated(drawArgs.pz));
        drawArgs.ns.clear();
        drawArgs.ns.push_back(SymId::timeSigCommon);
        drawArgs.ds.clear();
    } else if (sigType == TimeSigType::ALLA_BREVE) {
        drawArgs.pz = PointF(0.0, yoff);
        RectF bbox = font->bbox(SymId::timeSigCutCommon, mag);
        item->setbbox(bbox.translated(drawArgs.pz));
        drawArgs.ns.clear();
        drawArgs.ns.push_back(SymId::timeSigCutCommon);
        drawArgs.ds.clear();
    } else if (sigType == TimeSigType::CUT_BACH) {
        drawArgs.pz = PointF(0.0, yoff);
        RectF bbox = font->bbox(SymId::timeSigCut2, mag);
        item->setbbox(bbox.translated(drawArgs.pz));
        drawArgs.ns.clear();
        drawArgs.ns.push_back(SymId::timeSigCut2);
        drawArgs.ds.clear();
    } else if (sigType == TimeSigType::CUT_TRIPLE) {
        drawArgs.pz = PointF(0.0, yoff);
        RectF bbox = font->bbox(SymId::timeSigCut3, mag);
        item->setbbox(bbox.translated(drawArgs.pz));
        drawArgs.ns.clear();
        drawArgs.ns.push_back(SymId::timeSigCut3);
        drawArgs.ds.clear();
    } else {
        if (item->numeratorString().isEmpty()) {
            drawArgs.ns = timeSigSymIdsFromString(item->numeratorString().isEmpty()
                                                  ? String::number(item->sig().numerator())
                                                  : item->numeratorString());

            drawArgs.ds = timeSigSymIdsFromString(item->denominatorString().isEmpty()
                                                  ? String::number(item->sig().denominator())
                                                  : item->denominatorString());
        } else {
            drawArgs.ns = timeSigSymIdsFromString(item->numeratorString());
            drawArgs.ds = timeSigSymIdsFromString(item->denominatorString());
        }

        RectF numRect = font->bbox(drawArgs.ns, mag);
        RectF denRect = font->bbox(drawArgs.ds, mag);

        // position numerator and denominator; vertical displacement:
        // number of lines is odd: 0.0 (strings are directly above and below the middle line)
        // number of lines even:   0.05 (strings are moved up/down to leave 1/10sp between them)

        double displ = (numOfLines & 1) ? 0.0 : (0.05 * _spatium);

        //align on the wider
        double pzY = yoff - (denRect.width() < 0.01 ? 0.0 : (displ + numRect.height() * .5));
        double pnY = yoff + displ + denRect.height() * .5;

        if (numRect.width() >= denRect.width()) {
            // numerator: one space above centre line, unless denomin. is empty (if so, directly centre in the middle)
            drawArgs.pz = PointF(0.0, pzY);
            // denominator: horiz: centred around centre of numerator | vert: one space below centre line
            drawArgs.pn = PointF((numRect.width() - denRect.width()) * .5, pnY);
        } else {
            // numerator: one space above centre line, unless denomin. is empty (if so, directly centre in the middle)
            drawArgs.pz = PointF((denRect.width() - numRect.width()) * .5, pzY);
            // denominator: horiz: centred around centre of numerator | vert: one space below centre line
            drawArgs.pn = PointF(0.0, pnY);
        }

        // centering of parenthesis so the middle of the parenthesis is at the divisor marking level
        int centerY = yoff / 2 + _spatium;
        int widestPortion = numRect.width() > denRect.width() ? numRect.width() : denRect.width();
        drawArgs.pointLargeLeftParen = PointF(-_spatium, centerY);
        drawArgs.pointLargeRightParen = PointF(widestPortion + _spatium, centerY);

        item->setbbox(numRect.translated(drawArgs.pz));       // translate bounding boxes to actual string positions
        item->addbbox(denRect.translated(drawArgs.pn));
        if (item->largeParentheses()) {
            item->addbbox(RectF(drawArgs.pointLargeLeftParen.x(), drawArgs.pointLargeLeftParen.y() - denRect.height(), _spatium / 2,
                                numRect.height() + denRect.height()));
            item->addbbox(RectF(drawArgs.pointLargeRightParen.x(), drawArgs.pointLargeRightParen.y() - denRect.height(),  _spatium / 2,
                                numRect.height() + denRect.height()));
        }
    }

    item->setDrawArgs(drawArgs);
}

void TLayout::layout(Tremolo* item, LayoutContext& ctx)
{
    TremoloLayout::layout(item, ctx);
}

void TLayout::layout(TremoloBar* item, LayoutContext&)
{
    double _spatium = item->spatium();
    if (item->explicitParent()) {
        item->setPos(0.0, -_spatium * 3.0);
    } else {
        item->setPos(PointF());
    }

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
    item->setPolygon(polygon);

    double w = item->lineWidth().val();
    item->setbbox(item->polygon().boundingRect().adjusted(-w, -w, w, w));
}

void TLayout::layout(TrillSegment* item, LayoutContext& ctx)
{
    if (item->staff()) {
        item->setMag(item->staff()->staffMag(item->tick()));
    }
    if (item->spanner()->placeBelow()) {
        item->setPosY(item->staff() ? item->staff()->height() : 0.0);
    }

    bool accidentalGoesBelow = item->trill()->trillType() == TrillType::DOWNPRALL_LINE;
    Trill* trill = item->trill();
    Ornament* ornament = trill->ornament();
    if (ornament) {
        if (item->isSingleBeginType()) {
            TLayout::layout(ornament, ctx);
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

    item->autoplaceSpannerSegment();
}

void TLayout::layout(TripletFeel* item, LayoutContext& ctx)
{
    layout(static_cast<SystemText*>(item), ctx);
}

void TLayout::layout(Trill* item, LayoutContext& ctx)
{
    layoutLine(static_cast<SLine*>(item), ctx);

    if (ctx.isPaletteMode()) {
        return;
    }
    if (item->spannerSegments().empty()) {
        return;
    }
    TrillSegment* ls = toTrillSegment(item->frontSegment());
    if (item->spannerSegments().empty()) {
        LOGD("Trill: no segments");
    }
    if (item->accidental()) {
        item->accidental()->setParent(ls);
    }
}

void TLayout::layout(Tuplet* item, LayoutContext& ctx)
{
    TupletLayout::layout(item, ctx);
}

void TLayout::layout(VibratoSegment* item, LayoutContext&)
{
    if (item->staff()) {
        item->setMag(item->staff()->staffMag(item->tick()));
    }
    if (item->spanner()->placeBelow()) {
        item->setPosY(item->staff() ? item->staff()->height() : 0.0);
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

    item->autoplaceSpannerSegment();
}

void TLayout::layout(Vibrato* item, LayoutContext& ctx)
{
    layoutLine(static_cast<SLine*>(item), ctx);

    if (ctx.isPaletteMode()) {
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
    layoutTextLineBaseSegment(item, ctx);
    item->autoplaceSpannerSegment();
}

void TLayout::layout(WhammyBarSegment* item, LayoutContext& ctx)
{
    layoutTextLineBaseSegment(item, ctx);
    item->autoplaceSpannerSegment();
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
