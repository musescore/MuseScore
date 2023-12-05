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
#include "chordlayout.h"

#include "containers.h"

#include "dom/factory.h"

#include "dom/accidental.h"
#include "dom/arpeggio.h"

#include "dom/beam.h"
#include "dom/chord.h"
#include "dom/fingering.h"
#include "dom/glissando.h"
#include "dom/guitarbend.h"
#include "dom/hook.h"

#include "dom/ledgerline.h"
#include "dom/lyrics.h"

#include "dom/measure.h"

#include "dom/navigate.h"
#include "dom/note.h"

#include "dom/ornament.h"
#include "dom/part.h"
#include "dom/rest.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/staff.h"
#include "dom/stem.h"
#include "dom/stemslash.h"
#include "dom/stretchedbend.h"
#include "dom/system.h"
#include "dom/tie.h"
#include "dom/slur.h"

#include "dom/undo.h"
#include "dom/utils.h"

#include "tlayout.h"
#include "slurtielayout.h"
#include "beamlayout.h"
#include "autoplace.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::stable;

void ChordLayout::layout(Chord* item, LayoutContext& ctx)
{
    if (item->notes().empty()) {
        return;
    }

    int gi = 0;
    for (Chord* c : item->graceNotes()) {
        // HACK: graceIndex is not well-maintained on add & remove
        // so rebuild now
        c->setGraceIndex(gi++);
    }

    if (item->onTabStaff()) {
        layoutTablature(item, ctx);
    } else {
        layoutPitched(item, ctx);
    }

    fillShape(item, item->mutldata(), ctx.conf());
}

void ChordLayout::layoutPitched(Chord* item, LayoutContext& ctx)
{
    for (Chord* c : item->graceNotes()) {
        layoutPitched(c, ctx);
    }

    double mag_             = item->staff() ? item->staff()->staffMag(item) : 1.0;      // palette elements do not have a staff
    double dotNoteDistance  = ctx.conf().styleMM(Sid::dotNoteDistance) * mag_;

    double chordX           = (item->noteType() == NoteType::NORMAL) ? item->ldata()->pos().x() : 0.0;

    while (item->ledgerLines()) {
        LedgerLine* l = item->ledgerLines()->next();
        delete item->ledgerLines();
        item->setLedgerLine(l);
    }

    double lll    = 0.0;           // space to leave at left of chord
    double rrr    = 0.0;           // space to leave at right of chord
    double lhead  = 0.0;           // amount of notehead to left of chord origin
    Note* upnote = item->upNote();

    delete item->tabDur();     // no TAB? no duration symbol! (may happen when converting a TAB into PITCHED)
    item->setTabDur(nullptr);

    //-----------------------------------------
    //  process notes
    //-----------------------------------------

    // Keeps track if there are any accidentals in this chord.
    // Used to remove excess space in front of arpeggios.
    // See GitHub issue #8970 for more details.
    // https://github.com/musescore/MuseScore/issues/8970
    std::vector<Accidental*> chordAccidentals;

    for (Note* note : item->notes()) {
        TLayout::layoutNote(note, note->mutldata());

        double x1 = note->pos().x() + chordX;
        double x2 = x1 + note->headWidth();
        lll      = std::max(lll, -x1);
        rrr      = std::max(rrr, x2);
        // track amount of space due to notehead only
        lhead    = std::max(lhead, -x1);

        Accidental* accidental = note->accidental();
        if (accidental && accidental->visible()) {
            chordAccidentals.push_back(accidental);
        }
        if (accidental && accidental->addToSkyline() && !note->fixed()) {
            // convert x position of accidental to segment coordinate system
            double x = accidental->pos().x() + note->pos().x() + chordX;
            // distance from accidental to note already taken into account
            // but here perhaps we create more padding in *front* of accidental?
            x -= ctx.conf().styleMM(Sid::accidentalDistance) * mag_;
            lll = std::max(lll, -x);
        }

        // clear layout for note-based fingerings
        for (EngravingItem* e : note->el()) {
            if (e->isFingering()) {
                Fingering* f = toFingering(e);
                if (f->layoutType() == ElementType::NOTE) {
                    f->setPos(PointF());
                    f->setbbox(RectF());
                }
            }
        }
    }

    //-----------------------------------------
    //  create ledger lines
    //-----------------------------------------

    item->addLedgerLines();

    if (item->arpeggio()) {
        TLayout::layoutArpeggio(item->arpeggio(), item->arpeggio()->mutldata(), ctx.conf());

        double arpeggioNoteDistance = ctx.conf().styleMM(Sid::ArpeggioNoteDistance) * mag_;

        double gapSize = arpeggioNoteDistance;

        if (chordAccidentals.size()) {
            double arpeggioAccidentalDistance = ctx.conf().styleMM(Sid::ArpeggioAccidentalDistance) * mag_;
            double accidentalDistance = ctx.conf().styleMM(Sid::accidentalDistance) * mag_;
            gapSize = arpeggioAccidentalDistance - accidentalDistance;
//            gapSize -= ArpeggioLayout::insetDistance(chordAccidentals, mag_, item);
        }

        double extraX = item->arpeggio()->width() + gapSize + chordX;

        double y1   = upnote->pos().y() - upnote->headHeight() * .5;
        item->arpeggio()->setPos(-(lll + extraX), y1);
        if (item->arpeggio()->visible()) {
            lll += extraX;
        }
        // _arpeggio->layout() called in layoutArpeggio2()

        // handle the special case of _arpeggio->span() > 1
        // in layoutArpeggio2() after page layout has done so we
        // know the y position of the next staves
    }

    if (item->dots()) {
        double x = item->dotPosX() + dotNoteDistance
                   + double(item->dots() - 1) * ctx.conf().styleMM(Sid::dotDotDistance) * mag_;
        x += item->symWidth(SymId::augmentationDot);
        rrr = std::max(rrr, x);
    }

    if (item->hook()) {
        if (item->beam()) {
            ctx.mutDom().undoRemoveElement(item->hook());
        } else {
            TLayout::layoutHook(item->hook(), item->hook()->mutldata());
            if (item->up() && item->stem()) {
                // hook position is not set yet
                double x = item->hook()->ldata()->bbox().right() + item->stem()->flagPosition().x() + chordX;
                rrr = std::max(rrr, x);
            }
        }
    }

    item->setSpaceLw(lll);
    item->setSpaceRw(rrr);

    for (Note* note : item->notes()) {
        layoutNote2(note, ctx);
    }

    for (EngravingItem* e : item->el()) {
        if (e->type() == ElementType::SLUR) {       // we cannot at this time as chordpositions are not fixed
            continue;
        }
        TLayout::layoutItem(e, ctx);
        if (e->type() == ElementType::CHORDLINE) {
            RectF tbbox = e->ldata()->bbox().translated(e->pos());
            double lx = tbbox.left() + chordX;
            double rx = tbbox.right() + chordX;
            if (-lx > item->spaceLw()) {
                item->setSpaceLw(item->spaceLw() - lx);
            }
            if (rx > item->spaceRw()) {
                item->setSpaceRw(rx);
            }
        }
    }

    // align note-based fingerings
    std::vector<Fingering*> alignNote;
    double xNote = 10000.0;
    for (Note* note : item->notes()) {
        bool leftFound = false;
        for (EngravingItem* e : note->el()) {
            if (e->isFingering() && e->autoplace()) {
                Fingering* f = toFingering(e);
                if (f->layoutType() == ElementType::NOTE && f->textStyleType() == TextStyleType::LH_GUITAR_FINGERING) {
                    alignNote.push_back(f);
                    if (!leftFound) {
                        leftFound = true;
                        double xf = f->ldata()->pos().x();
                        xNote = std::min(xNote, xf);
                    }
                }
            }
        }
    }
    for (Fingering* f : alignNote) {
        f->mutldata()->setPosX(xNote);
    }
}

void ChordLayout::layoutTablature(Chord* item, LayoutContext& ctx)
{
    double _spatium          = item->spatium();
    double mag_ = item->staff() ? item->staff()->staffMag(item) : 1.0;    // palette elements do not have a staff
    double dotNoteDistance = ctx.conf().styleMM(Sid::dotNoteDistance) * mag_;
    double minNoteDistance = ctx.conf().styleMM(Sid::minNoteDistance) * mag_;
    double minTieLength = ctx.conf().styleMM(Sid::MinTieLength) * mag_;

    for (Chord* c : item->graceNotes()) {
        layoutTablature(c, ctx);
    }

    while (item->ledgerLines()) {
        LedgerLine* l = item->ledgerLines()->next();
        delete item->ledgerLines();
        item->setLedgerLine(l);
    }

    double lll         = 0.0;                    // space to leave at left of chord
    double rrr         = 0.0;                    // space to leave at right of chord
    Note* upnote       = item->upNote();
    double headWidth   = item->symWidth(SymId::noteheadBlack);
    const Staff* st    = item->staff();
    const StaffType* tab = st->staffTypeForElement(item);
    double lineDist    = tab->lineDistance().val() * _spatium;
    double stemX       = tab->chordStemPosX(item) * _spatium;
    int ledgerLines = 0;
    double llY         = 0.0;

    size_t numOfNotes  = item->notes().size();
    double minY        = 1000.0;                 // just a very large value
    for (size_t i = 0; i < numOfNotes; ++i) {
        Note* note = item->notes().at(i);
        note->updateFrettingForTiesAndBends();
        TLayout::layoutNote(note, note->mutldata());
        // set headWidth to max fret text width
        double fretWidth = note->ldata()->bbox().width();
        if (headWidth < fretWidth) {
            headWidth = fretWidth;
        }
        // centre fret string on stem
        double x = stemX - fretWidth * 0.5;
        double y = note->fixed() ? note->line() * lineDist / 2 : tab->physStringToYOffset(note->string()) * _spatium;
        note->setPos(x, y);
        if (y < minY) {
            minY  = y;
        }
        int currLedgerLines   = tab->numOfTabLedgerLines(note->string());
        if (currLedgerLines > ledgerLines) {
            ledgerLines = currLedgerLines;
            llY         = y;
        }

        // allow extra space for shortened ties; this code must be kept synchronized
        // with the tie positioning code in Tie::slurPos()
        // but the allocation of space needs to be performed here
        Tie* tie;
        tie = note->tieBack();
        if (tie) {
            tie->calculateDirection();
            double overlap = 0.0;                // how much tie can overlap start and end notes
            bool shortStart = false;            // whether tie should clear start note or not
            Note* startNote = tie->startNote();
            Chord* startChord = startNote->chord();
            if (startChord && startChord->measure() == item->measure() && startChord == prevChordRest(item)) {
                double startNoteWidth = startNote->width();
                // overlap into start chord?
                // if in start chord, there are several notes or stem and tie in same direction
                if (startChord->notes().size() > 1 || (startChord->stem() && startChord->up() == tie->up())) {
                    // clear start note (1/8 of fret mark width)
                    shortStart = true;
                    overlap -= startNoteWidth * 0.125;
                } else {            // overlap start note (by ca. 1/3 of fret mark width)
                    overlap += startNoteWidth * 0.35;
                }
                // overlap into end chord (this)?
                // if several notes or neither stem or tie are up
                if (item->notes().size() > 1 || (item->stem() && !item->up() && !tie->up())) {
                    // for positive offset:
                    //    use available space
                    // for negative x offset:
                    //    space is allocated elsewhere, so don't re-allocate here
                    if (note->ldata()->pos().x() != 0.0) {                      // this probably does not work for TAB, as
                        overlap += std::abs(note->ldata()->pos().x());              // _pos is used to centre the fret on the stem
                    } else {
                        overlap -= fretWidth * 0.125;
                    }
                } else {
                    if (shortStart) {
                        overlap += fretWidth * 0.15;
                    } else {
                        overlap += fretWidth * 0.35;
                    }
                }
                double d = std::max(minTieLength - overlap, 0.0);
                lll = std::max(lll, d);
            }
        }
    }

    // create ledger lines, if required (in some historic styles)
    if (ledgerLines > 0) {
// there seems to be no need for widening 'ledger lines' beyond fret mark widths; more 'on the field'
// tests and usage will show if this depends on the metrics of the specific fonts used or not.
//            double extraLen    = style().styleS(Sid::ledgerLineLength).val() * _spatium;
        double extraLen    = 0;
        double llX         = stemX - (headWidth + extraLen) * 0.5;
        for (int i = 0; i < ledgerLines; i++) {
            LedgerLine* ldgLin = new LedgerLine(ctx.mutDom().dummyParent());
            ldgLin->setParent(item);
            ldgLin->setTrack(item->track());
            ldgLin->setVisible(item->visible());
            ldgLin->setLen(headWidth + extraLen);
            ldgLin->setPos(llX, llY);
            ldgLin->setNext(item->ledgerLines());
            item->setLedgerLine(ldgLin);
            TLayout::layoutLedgerLine(ldgLin, ctx);
            llY += lineDist / ledgerLines;
        }
        headWidth += extraLen;            // include ledger lines extra width in chord width
    }

    // horiz. spacing: leave half width at each side of the (potential) stem
    double halfHeadWidth = headWidth * 0.5;
    if (lll < stemX - halfHeadWidth) {
        lll = stemX - halfHeadWidth;
    }
    if (rrr < stemX + halfHeadWidth) {
        rrr = stemX + halfHeadWidth;
    }
    // align dots to the widest fret mark (not needed in all TAB styles, but harmless anyway)
    item->setDotPosX(headWidth);

    if (item->shouldHaveStem()) {
        // if stem is required but missing, add it;
        // set stem position (stem length is set in Chord:layoutStem() )
        if (!item->stem()) {
            Stem* stem = Factory::createStem(item);
            stem->setParent(item);
            stem->setGenerated(true);
            ctx.mutDom().undo(new AddElement(stem));
        }
        item->stem()->setPos(tab->chordStemPos(item) * _spatium);
        if (item->hook()) {
            if (item->beam()) {
                ctx.mutDom().undoRemoveElement(item->hook());
            } else {
                if (rrr < stemX + item->hook()->width()) {
                    rrr = stemX + item->hook()->width();
                }

                item->hook()->setPos(item->stem()->flagPosition());
            }
        }
    } else {
        if (item->stem()) {
            ctx.mutDom().undo(new RemoveElement(item->stem()));
            item->remove(item->stem());
        }
        if (item->hook()) {
            ctx.mutDom().undo(new RemoveElement(item->hook()));
            item->remove(item->hook());
        }
        if (item->beam()) {
            ctx.mutDom().undo(new RemoveElement(item->beam()));
            item->remove(item->beam());
        }
    }

    if (!tab->genDurations()                           // if tab is not set for duration symbols
        || track2voice(item->track())                        // or not in first voice
        || (item->isGrace()                                  // no tab duration symbols if grace notes
            && item->beamMode() == BeamMode::AUTO)) {      // and beammode == AUTO
        //
        delete item->tabDur();       // delete an existing duration symbol
        item->setTabDur(nullptr);
    } else {
        //
        // tab duration symbols
        //
        // if no previous CR
        // OR symbol repeat set to ALWAYS
        // OR symbol repeat condition is triggered
        // OR duration type and/or number of dots is different from current CR
        // OR chord beam mode not AUTO
        // OR previous CR is a rest
        // AND no not-stem
        // set a duration symbol (trying to re-use existing symbols where existing to minimize
        // symbol creation and deletion)
        bool needTabDur = false;
        bool repeat = false;
        if (!item->noStem()) {
            // check duration of prev. CR segm
            ChordRest* prevCR = prevChordRest(item);
            if (prevCR == 0) {
                needTabDur = true;
            } else if (item->beamMode() != BeamMode::AUTO
                       || prevCR->durationType().type() != item->durationType().type()
                       || prevCR->dots() != item->dots()
                       || prevCR->tuplet() != item->tuplet()
                       || prevCR->type() == ElementType::REST) {
                needTabDur = true;
            } else if (tab->symRepeat() == TablatureSymbolRepeat::ALWAYS
                       || ((tab->symRepeat() == TablatureSymbolRepeat::MEASURE
                            || tab->symRepeat() == TablatureSymbolRepeat::SYSTEM)
                           && item->measure() != prevCR->measure())) {
                needTabDur = true;
                repeat = true;
            }
        }
        if (needTabDur) {
            // symbol needed; if not exist, create; if exists, update duration
            if (!item->tabDur()) {
                item->setTabDur(new TabDurationSymbol(item, tab, item->durationType().type(), item->dots()));
            } else {
                item->tabDur()->setDuration(item->durationType().type(), item->dots(), tab);
            }
            item->tabDur()->setParent(item);
            item->tabDur()->setRepeat(repeat);
//                  _tabDur->setMag(mag());           // useless to set grace mag: graces have no dur. symbol
            TLayout::layoutTabDurationSymbol(item->tabDur(), item->tabDur()->mutldata());
            if (minY < 0) {                           // if some fret extends above tab body (like bass strings)
                TabDurationSymbol::LayoutData* tdladata = item->tabDur()->mutldata();
                tdladata->moveY(minY);             // raise duration symbol
                tdladata->setBbox(tdladata->bbox().translated(0, minY));
            }
        } else {                                // symbol not needed: if exists, delete
            delete item->tabDur();
            item->setTabDur(nullptr);
        }
    }                                     // end of if(duration_symbols)

    if (item->arpeggio()) {
        double y = upnote->pos().y() - upnote->headHeight() * .5;
        TLayout::layoutArpeggio(item->arpeggio(), item->arpeggio()->mutldata(), ctx.conf());
        lll += item->arpeggio()->width() + _spatium * .5;
        item->arpeggio()->setPos(-lll, y);

        // handle the special case of _arpeggio->span() > 1
        // in layoutArpeggio2() after page layout has done so we
        // know the y position of the next staves
    }

    // allocate enough room for glissandi
    if (item->endsGlissandoOrGuitarBend()) {
        if (!item->rtick().isZero()) {                          // if not at beginning of measure
            lll += _spatium * 0.5 + minTieLength;
        }
        // special case of system-initial glissando final note is handled in Glissando::layout() itself
    }

    if (item->hook()) {
        if (item->beam()) {
            ctx.mutDom().undoRemoveElement(item->hook());
        } else if (tab == 0) {
            TLayout::layoutHook(item->hook(), item->hook()->mutldata());
            if (item->up()) {
                // hook position is not set yet
                double x = item->hook()->ldata()->bbox().right() + item->stem()->flagPosition().x();
                rrr = std::max(rrr, x);
            }
        }
    }

    if (item->dots()) {
        double x = 0.0;
        // if stems are beside staff, dots are placed near to stem
        if (!tab->stemThrough()) {
            // if there is an unbeamed hook, dots should start after the hook
            if (item->hook() && !item->beam()) {
                x = item->hook()->width() + dotNoteDistance;
            }
            // if not, dots should start at a fixed distance right after the stem
            else {
                x = STAFFTYPE_TAB_DEFAULTDOTDIST_X * _spatium;
            }
            item->setDotPosX(x);
        }
        // if stems are through staff, use dot position computed above on fret mark widths
        else {
            x = item->dotPosX() + dotNoteDistance
                + (item->dots() - 1) * ctx.conf().styleS(Sid::dotDotDistance).val() * _spatium;
        }
        x += item->symWidth(SymId::augmentationDot);
        rrr = std::max(rrr, x);
    }

    item->setSpaceLw(lll);
    item->setSpaceRw(rrr);

    double graceMag = ctx.conf().styleD(Sid::graceNoteMag);

    std::vector<Chord*> graceNotesBefore = item->Chord::graceNotesBefore();
    size_t nb = graceNotesBefore.size();
    if (nb) {
        double xl = -(item->spaceLw() + minNoteDistance);
        for (int i = static_cast<int>(nb) - 1; i >= 0; --i) {
            Chord* c = graceNotesBefore.at(i);
            xl -= c->spaceRw() /* * 1.2*/;
            c->setPos(xl, 0);
            xl -= c->spaceLw() + minNoteDistance * graceMag;
        }
        if (-xl > item->spaceLw()) {
            item->setSpaceLw(item->spaceLw() - xl);
        }
    }
    std::vector<Chord*> gna = item->graceNotesAfter();
    size_t na = gna.size();
    if (na) {
        // get factor for start distance after main note. Values found by testing.
        double fc;
        switch (item->durationType().type()) {
        case DurationType::V_LONG:    fc = 3.8;
            break;
        case DurationType::V_BREVE:   fc = 3.8;
            break;
        case DurationType::V_WHOLE:   fc = 3.8;
            break;
        case DurationType::V_HALF:    fc = 3.6;
            break;
        case DurationType::V_QUARTER: fc = 2.1;
            break;
        case DurationType::V_EIGHTH:  fc = 1.4;
            break;
        case DurationType::V_16TH:    fc = 1.2;
            break;
        default: fc = 1;
        }
        double xr = fc * (item->spaceRw() + minNoteDistance);
        for (int i = 0; i <= static_cast<int>(na) - 1; i++) {
            Chord* c = gna.at(i);
            xr += c->spaceLw() * (i == 0 ? 1.3 : 1);
            c->setPos(xr, 0);
            xr += c->spaceRw() + minNoteDistance * graceMag;
        }
        if (xr > item->spaceRw()) {
            item->setSpaceRw(xr);
        }
    }
    for (EngravingItem* e : item->el()) {
        TLayout::layoutItem(e, ctx);
        if (e->type() == ElementType::CHORDLINE) {
            RectF tbbox = e->ldata()->bbox().translated(e->pos());
            double lx = tbbox.left();
            double rx = tbbox.right();
            if (-lx > item->spaceLw()) {
                item->setSpaceLw(item->spaceLw() - lx);
            }
            if (rx > item->spaceRw()) {
                item->setSpaceRw(rx);
            }
        }
    }

    for (size_t i = 0; i < numOfNotes; ++i) {
        layoutNote2(item->notes().at(i), ctx);
    }

    if (item->stemSlash()) {
        TLayout::layoutStemSlash(item->stemSlash(), item->stemSlash()->mutldata(), ctx.conf());
    }
}

//---------------------------------------------------------
//   layoutNotesSpanners
//---------------------------------------------------------

void ChordLayout::layoutSpanners(Chord* item, LayoutContext& ctx)
{
    for (const Note* n : item->notes()) {
        Tie* tie = n->tieFor();
        if (tie) {
            TLayout::layoutTie(tie, ctx);
        }
        for (Spanner* sp : n->spannerBack()) {
            if (sp->isGuitarBend()) {
                continue;
            }
            TLayout::layoutSpanner(sp, ctx);
        }
    }
}

//---------------------------------------------------------
//   layoutArticulations
//    layout tenuto and staccato
//    called before layouting slurs
//---------------------------------------------------------

void ChordLayout::layoutArticulations(Chord* item, LayoutContext& ctx)
{
    for (Chord* gc : item->graceNotes()) {
        layoutArticulations(gc, ctx);
    }

    if (item->articulations().empty()) {
        return;
    }
    const Staff* st = item->staff();
    const StaffType* staffType = st->staffTypeForElement(item);
    double mag            = (staffType->isSmall() ? ctx.conf().styleD(Sid::smallStaffMag) : 1.0) * staffType->userMag();
    double _spatium       = ctx.conf().spatium() * mag;
    double _lineDist       = _spatium * staffType->lineDistance().val() / 2;
    const double minDist = ctx.conf().styleMM(Sid::articulationMinDistance);
    const ArticulationStemSideAlign articulationHAlign = ctx.conf().styleV(Sid::articulationStemHAlign).value<ArticulationStemSideAlign>();
    const bool keepArticsTogether = ctx.conf().styleB(Sid::articulationKeepTogether);
    const double stemSideDistance = ctx.conf().styleMM(Sid::propertyDistanceStem);

    int numCloseArtics = 0;
    bool hasStaffArticsUp = false;
    bool hasStaffArticsDown = false;
    if (keepArticsTogether) {
        // find out how many close-to-note artics there are, and whether there is a staff-anchored artic to align to
        for (Articulation* a : item->articulations()) {
            if (a->layoutCloseToNote()) {
                ++numCloseArtics;
            } else {
                if (a->up()) {
                    hasStaffArticsUp = true;
                } else {
                    hasStaffArticsDown = true;
                }
                if (hasStaffArticsDown && hasStaffArticsUp) {
                    break;           // nothing more to be learned now
                }
            }
        }
    }

    //
    //    determine direction
    //    place tenuto and staccato
    //

    Articulation* prevArticulation = nullptr;
    for (Articulation* a : item->articulations()) {
        if (item->measure()->hasVoices(a->staffIdx(), item->tick(), item->actualTicks()) && a->anchor() == ArticulationAnchor::AUTO) {
            a->setUp(item->up());         // if there are voices place articulation at stem
        } else if (a->anchor() == ArticulationAnchor::AUTO) {
            if (a->isOrnament()) {
                bool oddVoice = item->track() % 2;
                a->setUp(!oddVoice);
            } else if (a->symId() >= SymId::articMarcatoAbove && a->symId() <= SymId::articMarcatoTenutoBelow) {
                a->setUp(true);         // Gould, p. 117: strong accents above staff
            } else {
                a->setUp(!item->up());         // place articulation at note head
            }
        } else {
            a->setUp(a->anchor() == ArticulationAnchor::TOP);
        }

        if (!a->layoutCloseToNote()) {
            continue;
        }

        bool bottom = !a->up();      // true: articulation is below chord;  false: articulation is above chord
        TLayout::layoutItem(a, ctx);     // must be done after assigning direction, or else symId is not reliable

        bool headSide = bottom == item->up();
        double y = 0.0;
        double x;
        if ((!headSide || !a->isBasicArticulation()) && item->stem()) {
            switch (articulationHAlign) {
            case ArticulationStemSideAlign::STEM:
                x = item->stem()->width() * .5;
                break;
            case ArticulationStemSideAlign::NOTEHEAD:
                x = item->up() ? item->downNote()->noteheadCenterX() : item->centerX();
                break;
            case ArticulationStemSideAlign::AVERAGE:
            default:
                x = item->up() ? (item->stem()->width() * .5 + item->downNote()->noteheadCenterX()) * .5
                    : (item->stem()->width() * .5 + item->centerX()) * .5;
                break;
            }
            if (item->up()) {
                x = item->downNote()->pos().x() + item->downNote()->bboxRightPos() - x;
            }
        } else {
            x = item->centerX();
        }

        if (bottom) {
            if (!headSide && item->stem()) {
                double stemBottom = item->stem()->ldata()->bbox().translated(item->stem()->pos()).bottom();
                // Check if there's a hook, because the tip of the hook always extends slightly past the end of the stem
                if (item->hook()) {
                    y = item->hook()->ldata()->bbox().translated(item->hook()->pos()).bottom();
                } else {
                    y = item->stem()->ldata()->bbox().translated(item->stem()->pos()).bottom();
                }
                int line   = round((y + _lineDist) / _lineDist);
                int lines = 2 * (staffType->lines() - 1);
                if (line < lines && !(line % 2)) {
                    line += 1;
                }
                double dist = (line * _lineDist) - stemBottom;
                bool hasBeam = item->beam() || (item->tremolo() && item->tremolo()->twoNotes());
                if (line < lines && hasBeam && dist < stemSideDistance) {
                    // beams can give stems weird unpredictable lengths, so we should enforce min
                    // distance even inside the staff

                    // adjust downwards a space but stop at the bottom of the staff
                    line = std::min(line + 2, lines);
                }
                if (line < lines) {        // align between staff lines
                    y = line * _lineDist;
                    y -= a->height() * .5;
                } else if (line == lines) {
                    y = stemSideDistance + (lines * _lineDist);
                } else {
                    y += stemSideDistance;
                }
            } else {
                x = item->centerX();
                int lines = (staffType->lines() - 1) * 2;
                int line = std::max(item->downLine(), -1);
                bool adjustArtic = (a->up() && hasStaffArticsUp) || (!a->up() && hasStaffArticsDown);
                if (keepArticsTogether && adjustArtic && numCloseArtics > 0) {
                    line = std::max(line, lines - (3 + ((numCloseArtics - 1) * 2)));
                }
                if (line < lines - 1) {
                    y = ((line & ~1) + 3) * _lineDist;
                    y -= a->height() * .5;
                } else {
                    y = item->downPos() + 0.5 * item->downNote()->headHeight() + ctx.conf().styleMM(Sid::propertyDistanceHead);
                }
            }
            if (prevArticulation && (prevArticulation->up() == a->up())) {
                int staffBottom = (staffType->lines() - 2) * 2;
                if ((headSide && item->downLine() < staffBottom) || (!headSide && !RealIsEqualOrMore(y, (staffBottom + 1) * _lineDist))) {
                    y += _spatium;
                } else {
                    y += prevArticulation->height() + minDist;
                }
            }
            // center symbol
        } else { // topside
            if (!headSide && item->stem()) {
                double stemTop = item->stem()->ldata()->bbox().translated(item->stem()->pos()).top();
                // Check if there's a hook, because the tip of the hook always extends slightly past the end of the stem
                if (item->hook()) {
                    y = item->hook()->ldata()->bbox().translated(item->hook()->pos()).top();
                } else {
                    y = item->stem()->ldata()->bbox().translated(item->stem()->pos()).top();
                }
                int line = round((y - _lineDist) / _lineDist);
                if (line > 0 && !(line % 2)) {
                    line -= 1;
                }
                double dist = stemTop - (line * _lineDist);
                bool hasBeam = item->beam() || (item->tremolo() && item->tremolo()->twoNotes());
                if (line > 0 && hasBeam && dist < stemSideDistance) {
                    // beams can give stems weird unpredictable lengths, so we should enforce min
                    // distance even inside the staff

                    // adjust upwards a space but stop at the top of the staff
                    line = std::max(line - 2, 0);
                }

                if (line > 0) {        // align between staff lines
                    y = line * _lineDist;
                    y += a->height() * .5;
                } else if (line == 0) {
                    y = -stemSideDistance;
                } else {
                    y -= stemSideDistance;
                }
            } else {
                x = item->centerX();
                int lines = (staffType->lines() - 1) * 2;
                int line = std::min(item->upLine(), lines + 1);
                bool adjustArtic = (a->up() && hasStaffArticsUp) || (!a->up() && hasStaffArticsDown);
                if (keepArticsTogether && adjustArtic && numCloseArtics > 0) {
                    line = std::min(line, 3 + ((numCloseArtics - 1) * 2));
                }
                if (line > 1) {
                    y = (((line + 1) & ~1) - 3) * _lineDist;
                    y += a->height() * .5;
                } else {
                    y = item->upPos() - 0.5 * item->downNote()->headHeight() - ctx.conf().styleMM(Sid::propertyDistanceHead);
                }
            }
            if (prevArticulation && (prevArticulation->up() == a->up())) {
                if ((headSide && item->upLine() > 2) || (!headSide && !RealIsEqualOrLess(y, 0.0))) {
                    y -= item->spatium();
                } else {
                    y -= prevArticulation->height() + minDist;
                }
            }
        }
        a->setPos(x, y);
        prevArticulation = a;
//            measure()->system()->staff(a->staffIdx())->skyline().add(a->shape().translated(a->pos() + segment()->pos() + measure()->pos()));
    }
}

//---------------------------------------------------------
//   layoutArticulations2
//    Called after layouting systems
//    Tentatively layout all articulations
//    To be finished after laying out slurs
//---------------------------------------------------------

void ChordLayout::layoutArticulations2(Chord* item, LayoutContext& ctx, bool layoutOnCrossBeamSide)
{
    ArticulationStemSideAlign articulationHAlign = ctx.conf().styleV(Sid::articulationStemHAlign).value<ArticulationStemSideAlign>();
    for (Chord* gc : item->graceNotes()) {
        layoutArticulations2(gc, ctx);
    }

    if (item->articulations().empty()) {
        return;
    }
    double headSideX = item->centerX();
    double stemSideX = headSideX;
    if (item->stem()) {
        switch (articulationHAlign) {
        case ArticulationStemSideAlign::STEM:
            stemSideX = item->stem()->width() * .5;
            break;
        case ArticulationStemSideAlign::NOTEHEAD:
            stemSideX = item->up() ? item->downNote()->noteheadCenterX() : item->centerX();
            break;
        case ArticulationStemSideAlign::AVERAGE:
        default:
            stemSideX = item->up() ? (item->stem()->width() * .5 + item->downNote()->noteheadCenterX()) * .5
                        : (item->stem()->width() * .5 + item->centerX()) * .5;
            break;
        }
        if (item->up()) {
            stemSideX = item->downNote()->pos().x() + item->downNote()->bboxRightPos() - stemSideX;
        }
    }

    double stacAccentKern = 0.2 * item->spatium();
    double minDist = ctx.conf().styleMM(Sid::articulationMinDistance);
    double staffDist = ctx.conf().styleMM(Sid::propertyDistance);
    double stemDist = ctx.conf().styleMM(Sid::propertyDistanceStem);
    double noteDist = ctx.conf().styleMM(Sid::propertyDistanceHead);

    double chordTopY = item->upPos() - 0.5 * item->upNote()->headHeight();      // note position of highest note
    double chordBotY = item->downPos() + 0.5 * item->upNote()->headHeight();    // note position of lowest note

    double staffTopY = -staffDist;
    double staffBotY = item->staff()->staffHeight() + staffDist;

    // avoid collisions of staff articulations with chord notes:
    // gap between note and staff articulation is distance0 + 0.5 spatium

    if (item->stem()) {
        // Check if there's a hook, because the tip of the hook always extends slightly past the end of the stem
        if (item->up()) {
            double tip = item->hook()
                         ? item->hook()->ldata()->bbox().translated(item->hook()->pos()).top()
                         : item->stem()->ldata()->bbox().translated(item->stem()->pos()).top();

            chordTopY = tip;
        } else {
            double tip = item->hook()
                         ? item->hook()->ldata()->bbox().translated(item->hook()->pos()).bottom()
                         : item->stem()->ldata()->bbox().translated(item->stem()->pos()).bottom();

            chordBotY = tip;
        }
    }

    //
    //    place all articulations with anchor at chord/rest
    //
    chordTopY -= item->up() ? stemDist : noteDist;
    chordBotY += item->up() ? noteDist : stemDist;
    // add space for staccato and tenuto marks which may have been previously laid out
    for (Articulation* a : item->articulations()) {
        ArticulationAnchor aa = a->anchor();
        if (a->layoutCloseToNote() && a->visible() && aa == ArticulationAnchor::AUTO) {
            if (a->up()) {
                chordTopY = a->y() - a->height() - minDist;
            } else {
                chordBotY = a->y() + a->height() + minDist;
            }
        }
    }

    for (Articulation* a : item->articulations()) {
        if (layoutOnCrossBeamSide && !a->isOnCrossBeamSide()) {
            continue;
        }
        if (!a->layoutCloseToNote()) {
            continue;
        }
        if (a->up()) {
            if (a->visible()) {
                chordTopY = a->y() - a->height() - minDist;
            }
        } else {
            if (a->visible()) {
                chordBotY = a->y() + a->height() + minDist;
            }
        }
    }
    //
    //    now place all articulations with staff top or bottom anchor, or chord anchor for artics that don't layout close to note
    //
    staffTopY = std::min(staffTopY, chordTopY);
    staffBotY = std::max(staffBotY, chordBotY);
    Articulation* stacc = nullptr;
    for (Articulation* a : item->articulations()) {
        double kearnHeight = 0.0;
        if (layoutOnCrossBeamSide && !a->isOnCrossBeamSide()) {
            continue;
        }
        if (a->isStaccato()) {
            stacc = a;
        } else if (stacc && a->isAccent() && stacc->up() == a->up()
                   && (RealIsEqualOrLess(stacc->ldata()->pos().y(), 0.0)
                       || RealIsEqualOrMore(stacc->ldata()->pos().y(), item->staff()->staffHeight()))) {
            // obviously, the accent doesn't have a cutout, so this value just artificially moves the stacc
            // and accent closer to each other to simulate some kind of kerning. Looks great using all musescore fonts,
            // though there is a possibility that a different font which has vertically-asymmetrical accents
            // could make this look bad.
            kearnHeight = stacAccentKern;
            stacc = nullptr;
        } else {
            stacc = nullptr;
        }
        if (!a->layoutCloseToNote()) {
            TLayout::layoutItem(a, ctx);
            if (a->up()) {
                a->setPos(!item->up() || !a->isBasicArticulation() ? headSideX : stemSideX, staffTopY + kearnHeight);
                if (a->visible()) {
                    staffTopY = a->y() - a->height() - minDist;
                }
            } else {
                a->setPos(item->up() || !a->isBasicArticulation() ? headSideX : stemSideX, staffBotY - kearnHeight);
                if (a->visible()) {
                    staffBotY = a->y() + a->height() + minDist;
                }
            }
            Autoplace::doAutoplace(a, a->mutldata());
        }
    }

    for (Articulation* a : item->articulations()) {
        if (a->addToSkyline() && !a->isOnCrossBeamSide()) {
            // the segment shape has already been calculated
            // so measure width and spacing is already determined
            // in line mode, we cannot add to segment shape without throwing this off
            // but adding to skyline is always good
            Segment* s = item->segment();
            Measure* m = s->measure();
            Shape sh = a->shape().translate(a->pos() + item->pos());
            // TODO: limit to width of chord
            // this avoids "staircase" effect due to space not having been allocated already
            // ANOTHER alternative is to allocate the space in layoutPitched() / layoutTablature()
            //double w = std::min(r.width(), width());
            //r.translate((r.width() - w) * 0.5, 0.0);
            //r.setWidth(w);
            if (!ctx.conf().isLineMode()) {
                s->staffShape(item->staffIdx()).add(sh);
            }
            sh.translate(s->pos() + m->pos());
            m->system()->staff(item->vStaffIdx())->skyline().add(sh);
        }
    }
}

//---------------------------------------------------------
//   layoutArticulations3
//    Called after layouting slurs
//    Fix up articulations that need to go outside the slur
//---------------------------------------------------------

void ChordLayout::layoutArticulations3(Chord* item, Slur* slur, LayoutContext& ctx)
{
    SlurSegment* ss;
    if (item == slur->startCR()) {
        ss = slur->frontSegment();
    } else if (item == slur->endCR()) {
        ss = slur->backSegment();
    } else {
        return;
    }
    Segment* s = item->segment();
    Measure* m = item->measure();
    SysStaff* sstaff = m->system() ? m->system()->staff(item->vStaffIdx()) : nullptr;
    for (auto iter = item->articulations().begin(); iter != item->articulations().end(); ++iter) {
        Articulation* a = *iter;
        if (a->layoutCloseToNote() || !a->autoplace() || !slur->addToSkyline()) {
            continue;
        }
        Shape aShape = a->shape().translate(a->pos() + item->pos() + s->pos() + m->pos());
        Shape sShape = ss->shape().translate(ss->pos());
        if (aShape.intersects(sShape)) {
            double d = ctx.conf().styleS(Sid::articulationMinDistance).val() * item->spatium();
            d += slur->up()
                 ? std::max(aShape.minVerticalDistance(sShape), 0.0)
                 : std::max(sShape.minVerticalDistance(aShape), 0.0);
            d *= slur->up() ? -1 : 1;
            for (auto iter2 = iter; iter2 != item->articulations().end(); ++iter2) {
                Articulation* aa = *iter2;
                aa->mutldata()->moveY(d);
                Shape aaShape = aa->shape().translated(aa->pos() + item->pos() + s->pos() + m->pos());
                if (sstaff && aa->addToSkyline()) {
                    sstaff->skyline().add(aaShape);
                    s->staffShape(item->staffIdx()).add(aaShape);
                }
            }
        }
    }
}

//! May be called again when the chord is added to or removed from a beam.
void ChordLayout::layoutStem(Chord* item, LayoutContext& ctx)
{
    // Stem needs to know hook's bbox and SMuFL anchors.
    // This is done before calcDefaultStemLength because the presence or absence of a hook affects stem length
    if (item->shouldHaveHook()) {
        layoutHook(item, ctx);
    } else {
        ctx.mutDom().undoRemoveElement(item->hook());
    }

    // we should calculate default stem length for this chord even if it doesn't have a stem
    // because this length is used for tremolos or other things that attach to where the stem WOULD be
    item->setDefaultStemLength(item->calcDefaultStemLength());

    if (!item->shouldHaveStem()) {
        item->removeStem();
        return;
    }

    if (!item->stem()) {
        item->createStem();
    }

    item->stem()->mutldata()->setPosX(item->stemPosX());

    // This calls _stem->layout()
    item->stem()->setBaseLength(Millimetre(item->defaultStemLength()));

    // And now we need to set the position of the flag.
    if (item->hook()) {
        item->hook()->setPos(item->stem()->flagPosition());
    }

    // Add Stem slash
    if ((item->noteType() == NoteType::ACCIACCATURA) && !(item->beam() && item->beam()->elements().front() != item)) {
        if (!item->stemSlash()) {
            item->add(Factory::createStemSlash(item));
        }
        TLayout::layoutStemSlash(item->stemSlash(), item->stemSlash()->mutldata(), ctx.conf());
    } else if (item->stemSlash()) {
        item->remove(item->stemSlash());
    }
}

void ChordLayout::layoutHook(Chord* item, LayoutContext& ctx)
{
    if (!item->hook()) {
        item->createHook();
        computeUp(item, ctx);
    }
    item->hook()->setHookType(item->up() ? item->durationType().hooks() : -item->durationType().hooks());
    TLayout::layoutHook(item->hook(), item->hook()->mutldata());
}

void ChordLayout::computeUp(Chord* item, LayoutContext& ctx)
{
    assert(!item->notes().empty());

    const StaffType* tab = item->staff() ? item->staff()->staffTypeForElement(item) : 0;
    bool isTabStaff = tab && tab->isTabStaff();
    if (isTabStaff) {
        if (tab->stemless()) {
            item->setUp(false);
            return;
        }
        if (!tab->stemThrough()) {
            bool staffHasMultipleVoices = item->measure()->hasVoices(item->staffIdx(), item->tick(), item->actualTicks());
            if (staffHasMultipleVoices) {
                bool isTrackEven = item->track() % 2 == 0;
                item->setUp(isTrackEven);
                return;
            }
            item->setUp(!tab->stemsDown());
            return;
        }
    }

    if (item->stemDirection() != DirectionV::AUTO && !item->beam() && !(item->tremolo() && item->tremolo()->twoNotes())) {
        item->setUp(item->stemDirection() == DirectionV::UP);
        return;
    }

    if (item->isUiItem()) {
        item->setUp(true);
        return;
    }

    if (item->beam()) {
        bool mixedDirection = false;
        bool cross = false;
        ChordRest* firstCr = item->beam()->elements().front();
        ChordRest* lastCr = item->beam()->elements().back();
        Chord* firstChord = nullptr;
        Chord* lastChord = nullptr;
        for (ChordRest* currCr : item->beam()->elements()) {
            if (!currCr->isChord()) {
                continue;
            }
            if (!firstChord) {
                firstChord = toChord(currCr);
            }
            lastChord = toChord(currCr);
        }
        DirectionV stemDirections = DirectionV::AUTO;
        for (ChordRest* cr : item->beam()->elements()) {
            if (!item->beam()->userModified() && !mixedDirection && cr->isChord() && toChord(cr)->stemDirection() != DirectionV::AUTO) {
                // on an unmodified beam, if all of the elements on that beam are explicitly set in one direction
                // (or AUTO), use that as the direction. This is necessary because the beam has not been laid out yet.
                if (stemDirections == DirectionV::AUTO) {
                    stemDirections = toChord(cr)->stemDirection();
                } else if (stemDirections != toChord(cr)->stemDirection()) {
                    mixedDirection = true;
                }
            }
            if (cr->isChord() && toChord(cr)->staffMove() != 0) {
                cross = true;
                if (!item->beam()->userModified()) { // if the beam is user-modified _up must be decided later down
                    int move = toChord(cr)->staffMove();
                    // we have to determine the first and last chord direction for the beam
                    // so that we can calculate the beam anchor points
                    if (move > 0) {
                        item->setUp(item->staffMove() > 0);
                        firstCr->setUp(firstCr->staffMove() > 0);
                        lastCr->setUp(lastCr->staffMove() > 0);
                    } else {
                        item->setUp(item->staffMove() >= 0);
                        firstCr->setUp(firstCr->staffMove() >= 0);
                        lastCr->setUp(lastCr->staffMove() >= 0);
                    }
                }
                break;
            }
        }
        Measure* measure = item->findMeasure();
        if (!cross) {
            if (!mixedDirection && stemDirections != DirectionV::AUTO) {
                item->setUp(stemDirections == DirectionV::UP);
            } else if (!item->beam()->userModified()) {
                item->setUp(item->beam()->up());
            }
        }
        if (!measure->explicitParent()) {
            // this method will be called later (from Measure::layoutCrossStaff) after the
            // system is completely laid out.
            // this is necessary because otherwise there's no way to deal with cross-staff beams
            // because we don't know how far apart the staves actually are
            return;
        }
        if (item->beam()->userModified()) {
            if (cross && item == firstCr) {
                // necessary because this beam was never laid out before, so its position isn't known
                // and the first chord would calculate wrong stem direction
                TLayout::layoutBeam(item->beam(), ctx);
            } else {
                // otherwise we can use stale layout data; the only reason we would need to lay out here is if
                // it's literally never been laid out before which due to the insane nature of our layout system
                // is actually a possible thing
                BeamLayout::layoutIfNeed(item->beam(), ctx);
            }
            PointF base = item->beam()->pagePos();
            Note* baseNote = item->up() ? item->downNote() : item->upNote();
            double noteY = baseNote->pagePos().y();
            double noteX = item->stemPosX() + item->pagePos().x() - base.x();
            PointF startAnchor = PointF();
            PointF endAnchor = PointF();
            startAnchor = BeamLayout::chordBeamAnchor(item->beam(), firstChord, ChordBeamAnchorType::Start);
            endAnchor = BeamLayout::chordBeamAnchor(item->beam(), lastChord, ChordBeamAnchorType::End);

            if (item == item->beam()->elements().front()) {
                item->setUp(noteY > startAnchor.y());
            } else if (item == item->beam()->elements().back()) {
                item->setUp(noteY > endAnchor.y());
            } else {
                double proportionAlongX = (noteX - startAnchor.x()) / (endAnchor.x() - startAnchor.x());
                double desiredY = proportionAlongX * (endAnchor.y() - startAnchor.y()) + startAnchor.y();
                item->setUp(noteY > desiredY);
            }
        }

        TLayout::layoutBeam(item->beam(), ctx);
        if (cross && item->tremolo() && item->tremolo()->twoNotes() && item->tremolo()->chord1() == item
            && item->tremolo()->chord1()->beam() == item->tremolo()->chord2()->beam()) {
            // beam-infixed two-note trems have to be laid out here
            TLayout::layoutTremolo(item->tremolo(), ctx);
        }
        if (!cross && !item->beam()->userModified()) {
            item->setUp(item->beam()->up());
        }
        return;
    } else if (item->tremolo() && item->tremolo()->twoNotes()) {
        Chord* c1 = item->tremolo()->chord1();
        Chord* c2 = item->tremolo()->chord2();
        bool cross = c1->staffMove() != c2->staffMove();
        if (item == c1) {
            // we have to lay out the tremolo because it hasn't been laid out at all yet, and we need its direction
            TLayout::layoutTremolo(item->tremolo(), ctx);
        }
        Measure* measure = item->findMeasure();
        if (!cross && !item->tremolo()->userModified()) {
            item->setUp(item->tremolo()->up());
        }
        if (!measure->explicitParent()) {
            // this method will be called later (from Measure::layoutCrossStaff) after the
            // system is completely laid out.
            // this is necessary because otherwise there's no way to deal with cross-staff beams
            // because we don't know how far apart the staves actually are
            return;
        }
        if (item->tremolo()->userModified()) {
            Note* baseNote = item->up() ? item->downNote() : item->upNote();
            double tremY = item->tremolo()->chordBeamAnchor(item, ChordBeamAnchorType::Middle).y();
            double noteY = baseNote->pagePos().y();
            item->setUp(noteY > tremY);
        } else if (cross) {
            // unmodified cross-staff trem, should be one note per staff
            if (item->staffMove() != 0) {
                item->setUp(item->staffMove() > 0);
            } else {
                int otherStaffMove = item->staffMove() == c1->staffMove() ? c2->staffMove() : c1->staffMove();
                item->setUp(otherStaffMove < 0);
            }
        }
        if (!cross && !item->tremolo()->userModified()) {
            item->setUp(item->tremolo()->up());
        }
        return;
    }

    bool staffHasMultipleVoices = item->measure()->hasVoices(item->staffIdx(), item->tick(), item->actualTicks());
    if (staffHasMultipleVoices) {
        bool isTrackEven = item->track() % 2 == 0;
        item->setUp(isTrackEven);
        return;
    }

    bool isGraceNote = item->noteType() != NoteType::NORMAL;
    if (isGraceNote) {
        item->setUp(true);
        return;
    }

    bool chordIsCrossStaff = item->staffMove() != 0;
    if (chordIsCrossStaff) {
        item->setUp(item->staffMove() > 0);
        return;
    }

    std::vector<int> distances = item->noteDistances();
    int direction = ChordLayout::computeAutoStemDirection(distances);
    item->setUp(direction > 0);
}

void ChordLayout::computeUp(ChordRest* item, LayoutContext& ctx)
{
    if (item->isChord()) {
        computeUp(static_cast<Chord*>(item), ctx);
    } else {
        // base ChordRest
        item->setUp(true);
    }
}

// return 1 means up, 0 means in the middle, -1 means down
int ChordLayout::computeAutoStemDirection(const std::vector<int>& noteDistances)
{
    int left = 0;
    int right = static_cast<int>(noteDistances.size()) - 1;

    while (left <= right) {
        int leftNote = noteDistances.at(left);
        int rightNote = noteDistances.at(right);
        int netDirecting = leftNote + rightNote;
        if (netDirecting == 0) {
            left++;
            right--;
            continue;
        }
        return netDirecting > 0 ? 1 : -1;
    }
    return 0;
}

//---------------------------------------------------------
//   layoutSegmentElements
//---------------------------------------------------------

static void layoutSegmentElements(Segment* segment, track_idx_t startTrack, track_idx_t endTrack, staff_idx_t staffIdx, LayoutContext& ctx)
{
    for (track_idx_t track = startTrack; track < endTrack; ++track) {
        if (EngravingItem* e = segment->element(track)) {
            if (e->vStaffIdx() == staffIdx) {
                TLayout::layoutItem(e, ctx);
            }
        }
    }
}

void ChordLayout::skipAccidentals(Segment* segment, track_idx_t startTrack, track_idx_t endTrack)
{
    for (track_idx_t track = startTrack; track < endTrack; ++track) {
        EngravingItem* item = segment->elementAt(track);
        if (item && item->isChord()) {
            for (Note* note : toChord(item)->notes()) {
                Accidental* acc = note->accidental();
                if (acc) {
                    acc->mutldata()->setIsSkipDraw(true);
                }
            }
        }
    }
}

//---------------------------------------------------------
//   layoutChords1
//    - layout upstem and downstem chords
//    - offset as necessary to avoid conflict
//---------------------------------------------------------

void ChordLayout::layoutChords1(LayoutContext& ctx, Segment* segment, staff_idx_t staffIdx)
{
    const Staff* staff = ctx.dom().staff(staffIdx);
    const bool isTab = staff->isTabStaff(segment->tick());
    const track_idx_t startTrack = staffIdx * VOICES;
    const track_idx_t endTrack   = startTrack + VOICES;
    const Fraction tick = segment->tick();

    // we need to check all the notes in all the staves of the part so that we don't get weird collisions
    // between accidentals etc with moved notes
    const Part* part = staff->part();
    const track_idx_t partStartTrack = part ? part->startTrack() : startTrack;
    const track_idx_t partEndTrack = part ? part->endTrack() : endTrack;

    if (isTab) {
        skipAccidentals(segment, startTrack, endTrack);
    }

    if (staff && staff->isTabStaff(tick) && (!staff->staffType() || !staff->staffType()->stemThrough())) {
        layoutSegmentElements(segment, startTrack, endTrack, staffIdx, ctx);
        return;
    }

    std::vector<Chord*> chords;
    std::vector<Note*> upStemNotes;
    std::vector<Note*> downStemNotes;
    int upVoices       = 0;
    int downVoices     = 0;
    double nominalWidth = ctx.conf().noteHeadWidth() * staff->staffMag(tick);
    double maxUpWidth   = 0.0;
    double maxDownWidth = 0.0;
    double maxUpMag     = 0.0;
    double maxDownMag   = 0.0;

    // dots and hooks can affect layout of notes as well as vice versa
    int upDots         = 0;
    int downDots       = 0;
    bool upHooks       = false;
    bool downHooks     = false;

    // also check for grace notes
    bool upGrace       = false;
    bool downGrace     = false;

    for (track_idx_t track = partStartTrack; track < partEndTrack; ++track) {
        EngravingItem* e = segment->element(track);
        if (e && e->isChord() && toChord(e)->vStaffIdx() == staffIdx) {
            Chord* chord = toChord(e);
            chords.push_back(chord);
            bool hasGraceBefore = false;
            for (Chord* c : chord->graceNotes()) {
                if (c->isGraceBefore()) {
                    hasGraceBefore = true;
                }
                layoutChords2(c->notes(), c->up(), ctx); // layout grace note noteheads
                layoutChords3(ctx.conf().style(), { c }, c->notes(), staff, ctx); // layout grace note chords
            }
            if (chord->up()) {
                ++upVoices;
                upStemNotes.insert(upStemNotes.end(), chord->notes().begin(), chord->notes().end());
                upDots   = std::max(upDots, chord->dots());
                maxUpMag = std::max(maxUpMag, chord->mag());
                if (!upHooks && !chord->beam()) {
                    upHooks = chord->hook();
                }
                if (hasGraceBefore) {
                    upGrace = true;
                }
            } else {
                ++downVoices;
                downStemNotes.insert(downStemNotes.end(), chord->notes().begin(), chord->notes().end());
                downDots = std::max(downDots, chord->dots());
                maxDownMag = std::max(maxDownMag, chord->mag());
                if (!downHooks && !chord->beam()) {
                    downHooks = chord->hook();
                }
                if (hasGraceBefore) {
                    downGrace = true;
                }
            }
        }
    }

    if (upVoices + downVoices && !isTab) {
        // TODO: use track as secondary sort criteria?
        // otherwise there might be issues with unisons between voices
        // in some corner cases

        maxUpWidth   = nominalWidth * maxUpMag;
        maxDownWidth = nominalWidth * maxDownMag;

        // layout upstem noteheads
        if (upVoices > 1) {
            std::sort(upStemNotes.begin(), upStemNotes.end(),
                      [](Note* n1, const Note* n2) ->bool { return n1->line() > n2->line(); });
        }
        if (upVoices) {
            double hw = layoutChords2(upStemNotes, true, ctx);
            maxUpWidth = std::max(maxUpWidth, hw);
        }

        // layout downstem noteheads
        if (downVoices > 1) {
            std::sort(downStemNotes.begin(), downStemNotes.end(),
                      [](Note* n1, const Note* n2) ->bool { return n1->line() > n2->line(); });
        }
        if (downVoices) {
            double hw = layoutChords2(downStemNotes, false, ctx);
            maxDownWidth = std::max(maxDownWidth, hw);
        }

        double sp                 = staff->spatium(tick);
        double upOffset           = 0.0;          // offset to apply to upstem chords
        double downOffset         = 0.0;          // offset to apply to downstem chords
        double dotAdjust          = 0.0;          // additional chord offset to account for dots
        double dotAdjustThreshold = 0.0;          // if it exceeds this amount

        // centering adjustments for whole note, breve, and small chords
        double centerUp          = 0.0;          // offset to apply in order to center upstem chords
        double oversizeUp        = 0.0;          // adjustment to oversized upstem chord needed if laid out to the right
        double centerDown        = 0.0;          // offset to apply in order to center downstem chords
        double centerAdjustUp    = 0.0;          // adjustment to upstem chord needed after centering donwstem chord
        double centerAdjustDown  = 0.0;          // adjustment to downstem chord needed after centering upstem chord

        // only center chords if they differ from nominal by at least this amount
        // this avoids unnecessary centering on differences due only to floating point roundoff
        // it also allows for the possibility of disabling centering
        // for notes only "slightly" larger than nominal, like half notes
        // but this will result in them not being aligned with each other between voices
        // unless you change to left alignment as described in the comments below
        double centerThreshold   = 0.01 * sp;

        // amount by which actual width exceeds nominal, adjusted for staff mag() only
        double headDiff = maxUpWidth - nominalWidth;
        // amount by which actual width exceeds nominal, adjusted for staff & chord/note mag()
        double headDiff2 = maxUpWidth - nominalWidth * (maxUpMag / staff->staffMag(tick));
        if (headDiff > centerThreshold) {
            // larger than nominal
            centerUp = headDiff * -0.5;
            // maxUpWidth is true width, but we no longer will care about that
            // instead, we care only about portion to right of origin
            maxUpWidth += centerUp;
            // to left align rather than center, delete both of the above
            if (headDiff2 > centerThreshold) {
                // if max notehead is wider than nominal with chord/note mag() applied
                // then noteheads extend to left of origin
                // because stemPosX() is based on nominal width
                // so we need to correct for that too
                centerUp += headDiff2;
                oversizeUp = headDiff2;
            }
        } else if (-headDiff > centerThreshold) {
            // smaller than nominal
            centerUp = -headDiff * 0.5;
            if (headDiff2 > centerThreshold) {
                // max notehead is wider than nominal with chord/note mag() applied
                // perform same adjustment as above
                centerUp += headDiff2;
                oversizeUp = headDiff2;
            }
            centerAdjustDown = centerUp;
        }

        headDiff = maxDownWidth - nominalWidth;
        if (headDiff > centerThreshold) {
            // larger than nominal
            centerDown = headDiff * -0.5;
            // to left align rather than center, change the above to
            //centerAdjustUp = headDiff;
            maxDownWidth = nominalWidth - centerDown;
        } else if (-headDiff > centerThreshold) {
            // smaller than nominal
            centerDown = -headDiff * 0.5;
            centerAdjustUp = centerDown;
        }

        // handle conflict between upstem and downstem chords

        if (upVoices && downVoices) {
            Note* bottomUpNote = upStemNotes.front();
            Note* topDownNote  = downStemNotes.back();
            int separation = topDownNote->line() - bottomUpNote->line();

            std::vector<Note*> overlapNotes;
            overlapNotes.reserve(8);

            if (separation == 1) {
                // second
                if (upDots && !downDots) {
                    upOffset = maxDownWidth + 0.1 * sp;
                } else {
                    downOffset = maxUpWidth;
                    // align stems if present
                    if (topDownNote->chord()->stem() && bottomUpNote->chord()->stem()) {
                        downOffset -= topDownNote->chord()->stem()->lineWidth();
                    } else if (topDownNote->chord()->durationType().headType() != NoteHeadType::HEAD_BREVIS
                               && bottomUpNote->chord()->durationType().headType() != NoteHeadType::HEAD_BREVIS) {
                        // stemless notes should be aligned as is they were stemmed
                        // (except in case of brevis, cause the notehead has the side bars)
                        downOffset -= ctx.conf().styleMM(Sid::stemWidth) * topDownNote->chord()->mag();
                    }
                }
            } else if (separation < 1) {
                // overlap (possibly unison)

                // build list of overlapping notes
                for (size_t i = 0, n = upStemNotes.size(); i < n; ++i) {
                    if (upStemNotes[i]->line() >= topDownNote->line() - 1) {
                        overlapNotes.push_back(upStemNotes[i]);
                    } else {
                        break;
                    }
                }
                for (size_t i = downStemNotes.size(); i > 0; --i) {         // loop most probably needs to be in this reverse order
                    if (downStemNotes[i - 1]->line() <= bottomUpNote->line() + 1) {
                        overlapNotes.push_back(downStemNotes[i - 1]);
                    } else {
                        break;
                    }
                }
                std::sort(overlapNotes.begin(), overlapNotes.end(),
                          [](Note* n1, const Note* n2) ->bool { return n1->line() > n2->line(); });

                // determine nature of overlap
                bool shareHeads = true;               // can all overlapping notes share heads?
                bool matchPending = false;            // looking for a unison match
                bool conflictUnison = false;          // unison found
                bool conflictSecondUpHigher = false;              // second found
                bool conflictSecondDownHigher = false;            // second found
                int lastLine = 1000;
                Note* p = overlapNotes[0];
                for (size_t i = 0, count = overlapNotes.size(); i < count; ++i) {
                    Note* n = overlapNotes[i];
                    NoteHeadType nHeadType;
                    NoteHeadType pHeadType;
                    Chord* nchord = n->chord();
                    Chord* pchord = p->chord();
                    if (n->ldata()->mirror()) {
                        if (separation < 0) {
                            // don't try to share heads if there is any mirroring
                            shareHeads = false;
                            // don't worry about conflicts involving mirrored notes
                            continue;
                        }
                    }
                    int line = n->line();
                    int d = lastLine - line;
                    switch (d) {
                    case 0:
                        // unison
                        conflictUnison = true;
                        matchPending = false;
                        nHeadType = (n->headType() == NoteHeadType::HEAD_AUTO) ? n->chord()->durationType().headType() : n->headType();
                        pHeadType = (p->headType() == NoteHeadType::HEAD_AUTO) ? p->chord()->durationType().headType() : p->headType();
                        // the most important rules for sharing noteheads on unisons between voices are
                        // that notes must be one same line with same tpc
                        // noteheads must be unmirrored and of same group
                        // and chords must be same size (or else sharing code won't work)
                        if (n->headGroup() != p->headGroup() || n->tpc() != p->tpc() || n->ldata()->mirror()
                            || p->ldata()->mirror()
                            || nchord->isSmall() != pchord->isSmall()) {
                            shareHeads = false;
                        } else {
                            // noteheads are potentially shareable
                            // it is more subjective at this point
                            // current default is to require *either* of the following:
                            //    1) both chords have same number of dots, both have stems, and both noteheads are same type and are full size (automatic match)
                            // or 2) one or more of the noteheads is not of type AUTO, but is explicitly set to match the other (user-forced match)
                            // or 3) exactly one of the noteheads is invisible (user-forced match)
                            // thus user can force notes to be shared despite differing number of dots or either being stemless
                            // by setting one of the notehead types to match the other or by making one notehead invisible
                            // TODO: consider adding a style option, staff properties, or note property to control sharing
                            if ((nchord->dots() != pchord->dots() || !nchord->stem() || !pchord->stem() || nHeadType != pHeadType
                                 || n->isSmall() || p->isSmall())
                                && ((n->headType() == NoteHeadType::HEAD_AUTO && p->headType() == NoteHeadType::HEAD_AUTO)
                                    || nHeadType != pHeadType)
                                && (n->visible() == p->visible())) {
                                shareHeads = false;
                            }
                        }
                        break;
                    case 1:
                        // second
                        // trust that this won't be a problem for single unison
                        if (separation < 0) {
                            if (n->chord()->up()) {
                                conflictSecondUpHigher = true;
                            } else {
                                conflictSecondDownHigher = true;
                            }
                            shareHeads = false;
                        }
                        break;
                    default:
                        // no conflict
                        if (matchPending) {
                            shareHeads = false;
                        }
                        matchPending = true;
                    }
                    p = n;
                    lastLine = line;
                }
                if (matchPending) {
                    shareHeads = false;
                }

                bool conflict = conflictUnison || conflictSecondDownHigher || conflictSecondUpHigher;
                bool ledgerOverlapAbove = false;
                bool ledgerOverlapBelow = false;

                double ledgerGap = 0.15 * sp;
                double ledgerLen = ctx.conf().styleS(Sid::ledgerLineLength).val() * sp;
                int firstLedgerBelow = staff->lines(bottomUpNote->tick()) * 2;
                int topDownStemLen = 0;
                if (!conflictUnison && topDownNote->chord()->stem()) {
                    topDownStemLen = std::round(topDownNote->chord()->stem()->ldata()->bbox().height() / sp * 2);
                    if (bottomUpNote->line() > firstLedgerBelow - 1 && topDownNote->line() < bottomUpNote->line()
                        && topDownNote->line() + topDownStemLen >= firstLedgerBelow) {
                        ledgerOverlapBelow = true;
                    }
                }

                int firstLedgerAbove = -2;
                int bottomUpStemLen = 0;
                if (!conflictUnison && bottomUpNote->chord()->stem()) {
                    bottomUpStemLen = std::round(bottomUpNote->chord()->stem()->ldata()->bbox().height() / sp * 2);
                    if (topDownNote->line() < -1 && topDownNote->line() < bottomUpNote->line()
                        && bottomUpNote->line() - bottomUpStemLen <= firstLedgerAbove) {
                        ledgerOverlapAbove = true;
                    }
                }

                // calculate offsets
                if (shareHeads) {
                    for (int i = static_cast<int>(overlapNotes.size()) - 1; i >= 1; i -= 2) {
                        Note* previousNote = overlapNotes[i - 1];
                        Note* n = overlapNotes[i];
                        if (!(previousNote->chord()->isNudged() || n->chord()->isNudged())) {
                            if (previousNote->chord()->dots() == n->chord()->dots()) {
                                // hide one set dots
                                bool onLine = !(previousNote->line() & 1);
                                if (onLine) {
                                    // hide dots for lower voice
                                    if (previousNote->voice() & 1) {
                                        previousNote->setDotsHidden(true);
                                    } else {
                                        n->setDotsHidden(true);
                                    }
                                } else {
                                    // hide dots for upper voice
                                    if (!(previousNote->voice() & 1)) {
                                        previousNote->setDotsHidden(true);
                                    } else {
                                        n->setDotsHidden(true);
                                    }
                                }
                            }
                            // formerly we hid noteheads in an effort to fix playback
                            // but this doesn't work for cases where noteheads cannot be shared
                            // so better to solve the problem elsewhere
                        }
                    }
                } else if (conflict && (upDots && !downDots)) {
                    upOffset = maxDownWidth + 0.1 * sp;
                } else if (conflictUnison && separation == 0 && (!downGrace || upGrace)) {
                    downOffset = maxUpWidth + 0.15 * sp;
                } else if (conflictUnison) {
                    upOffset = maxDownWidth + 0.15 * sp;
                } else if (conflictSecondUpHigher) {
                    upOffset = maxDownWidth + 0.15 * sp;
                } else if ((downHooks && !upHooks) && !(upDots && !downDots)) {
                    // Shift by ledger line length if ledger line conflict or just 0.3sp if no ledger lines
                    double adjSpace = (ledgerOverlapAbove || ledgerOverlapBelow) ? ledgerGap + ledgerLen : 0.3 * sp;
                    downOffset = maxUpWidth + adjSpace;
                } else if (conflictSecondDownHigher) {
                    if (downDots && !upDots) {
                        double adjSpace = (ledgerOverlapAbove || ledgerOverlapBelow) ? ledgerGap + ledgerLen : 0.2 * sp;
                        downOffset = maxUpWidth + adjSpace;
                    } else {
                        // Prevent ledger line & notehead collision
                        double adjSpace
                            = (topDownNote->line() <= firstLedgerAbove
                               || bottomUpNote->line() >= firstLedgerBelow) ? ledgerLen - ledgerGap - 0.2 * sp : -0.2 * sp;
                        upOffset = maxDownWidth + adjSpace;
                        if (downHooks) {
                            bool needsHookSpace = (ledgerOverlapBelow || ledgerOverlapAbove);
                            double hookSpace = topDownNote->chord()->hook()->width();
                            upOffset = needsHookSpace ? hookSpace + ledgerLen + ledgerGap : upOffset + 0.3 * sp;
                        }
                    }
                } else {
                    // no direct conflict, so parts can overlap (downstem on left)
                    // just be sure that stems clear opposing noteheads and ledger lines
                    double clearLeft = 0.0, clearRight = 0.0;
                    if (topDownNote->chord()->stem()) {
                        if (ledgerOverlapBelow) {
                            // Create space between stem and ledger line below staff
                            clearLeft = ledgerLen + ledgerGap + topDownNote->chord()->stem()->lineWidth();
                        } else {
                            clearLeft = topDownNote->chord()->stem()->lineWidth() + 0.3 * sp;
                        }
                    }
                    if (bottomUpNote->chord()->stem()) {
                        if (ledgerOverlapAbove) {
                            // Create space between stem and ledger line above staff
                            clearRight = maxDownWidth + ledgerLen + ledgerGap - maxUpWidth + bottomUpNote->chord()->stem()->lineWidth();
                        } else {
                            clearRight = bottomUpNote->chord()->stem()->lineWidth() + std::max(maxDownWidth - maxUpWidth, 0.0) + 0.3 * sp;
                        }
                    } else {
                        downDots = 0;             // no need to adjust for dots in this case
                    }
                    upOffset = std::max(clearLeft, clearRight);
                    // Check if there's enough space to tuck under a flag
                    Note* topUpNote = upStemNotes.back();
                    // Move notes out of the way of straight flags
                    int pad = ctx.conf().styleB(Sid::useStraightNoteFlags) ? 2 : 1;
                    bool overlapsFlag = topDownNote->line() + topDownStemLen + pad > topUpNote->line();
                    if (downHooks && (ledgerOverlapBelow || overlapsFlag)) {
                        // we will need more space to avoid collision with hook
                        // but we won't need as much dot adjustment
                        if (ledgerOverlapBelow) {
                            double hookWidth = topDownNote->chord()->hook()->width();
                            upOffset = hookWidth + ledgerLen + ledgerGap;
                        }
                        upOffset = std::max(upOffset, maxDownWidth + 0.1 * sp);
                        dotAdjustThreshold = maxUpWidth - 0.3 * sp;
                    }
                    // if downstem chord is small, don't center
                    // and we might not need as much dot adjustment either
                    if (centerDown > 0.0) {
                        centerDown = 0.0;
                        centerAdjustUp = 0.0;
                        dotAdjustThreshold = (upOffset - maxDownWidth) + maxUpWidth - 0.3 * sp;
                    }
                }
            }

            // adjust for dots
            if ((upDots && !downDots) || (downDots && !upDots)) {
                // only one sets of dots
                // place between chords
                int dots;
                double mag;
                if (upDots) {
                    dots = upDots;
                    mag = maxUpMag;
                } else {
                    dots = downDots;
                    mag = maxDownMag;
                }
                double dotWidth = segment->symWidth(SymId::augmentationDot);
                // first dot
                dotAdjust = ctx.conf().styleMM(Sid::dotNoteDistance) + dotWidth;
                // additional dots
                if (dots > 1) {
                    dotAdjust += ctx.conf().styleMM(Sid::dotDotDistance).val() * (dots - 1);
                }
                dotAdjust *= mag;
                // only by amount over threshold
                dotAdjust = std::max(dotAdjust - dotAdjustThreshold, 0.0);
            }
            if (separation == 1) {
                dotAdjust += 0.1 * sp;
            }
        }

        // apply chord offsets
        for (track_idx_t track = partStartTrack; track < partEndTrack; ++track) {
            EngravingItem* e = segment->element(track);
            if (e && e->isChord() && toChord(e)->vStaffIdx() == staffIdx) {
                Chord* chord = toChord(e);
                Chord::LayoutData* chordLdata = chord->mutldata();
                if (chord->up()) {
                    if (!RealIsNull(upOffset)) {
                        chordLdata->moveX(upOffset + centerAdjustUp + oversizeUp);
                        if (downDots && !upDots) {
                            chordLdata->moveX(dotAdjust);
                        }
                    } else {
                        chordLdata->moveX(centerUp);
                    }
                } else {
                    if (!RealIsNull(downOffset)) {
                        chordLdata->moveX(downOffset + centerAdjustDown);
                        if (upDots && !downDots) {
                            chordLdata->moveX(dotAdjust);
                        }
                    } else {
                        chordLdata->moveX(centerDown);
                    }
                }
            }
        }

        // layout chords
        std::vector<Note*> notes;
        if (upVoices) {
            notes.insert(notes.end(), upStemNotes.begin(), upStemNotes.end());
        }
        if (downVoices) {
            notes.insert(notes.end(), downStemNotes.begin(), downStemNotes.end());
        }
        if (upVoices + downVoices > 1) {
            std::sort(notes.begin(), notes.end(),
                      [](Note* n1, const Note* n2) ->bool { return n1->line() > n2->line(); });
        }
        layoutChords3(ctx.conf().style(), chords, notes, staff, ctx);
    }

    layoutSegmentElements(segment, partStartTrack, partEndTrack, staffIdx, ctx);
    for (Chord* chord : chords) {
        Ornament* ornament = chord->findOrnament();
        if (ornament && ornament->showCueNote()) {
            TLayout::layoutOrnamentCueNote(ornament, ctx);
        }
    }
}

//---------------------------------------------------------
//   layoutChords2
//    - determine which notes need mirroring
//    - this is called once for each stem direction
//      eg, once for voices 1&3, once for 2&4
//      with all notes combined and sorted to resemble one chord
//    - return maximum non-mirrored notehead width
//---------------------------------------------------------

double ChordLayout::layoutChords2(std::vector<Note*>& notes, bool up, LayoutContext& ctx)
{
    int startIdx, endIdx, incIdx;
    double maxWidth = 0.0;

    // loop in correct direction so that first encountered notehead wins conflict
    if (up) {
        // loop bottom up
        startIdx = 0;
        endIdx = int(notes.size());
        incIdx = 1;
    } else {
        // loop top down
        startIdx = int(notes.size()) - 1;
        endIdx = -1;
        incIdx = -1;
    }

    int ll        = 1000;           // line of previous notehead
                                    // hack: start high so first note won't show as conflict
    bool lvisible = false;          // was last note visible?
    bool mirror   = false;          // should current notehead be mirrored?
                                    // value is retained and may be used on next iteration
                                    // to track mirror status of previous note
    bool isLeft   = notes[startIdx]->chord()->up();               // is notehead on left?
    staff_idx_t lStaffIdx = notes[startIdx]->chord()->vStaffIdx();        // staff of last note (including staffMove)

    for (int idx = startIdx; idx != endIdx; idx += incIdx) {
        Note* note    = notes[idx];                         // current note
        int line      = note->line();                       // line of current note
        Chord* chord  = note->chord();
        staff_idx_t staffIdx  = chord->vStaffIdx();                 // staff of current note

        // there is a conflict
        // if this is same or adjacent line as previous note (and chords are on same staff!)
        // but no need to do anything about it if either note is invisible
        bool conflict = (std::abs(ll - line) < 2) && (lStaffIdx == staffIdx) && note->visible() && lvisible;

        // this note is on opposite side of stem as previous note
        // if there is a conflict
        // or if this the first note *after* a conflict
        if (conflict || (chord->up() != isLeft)) {
            isLeft = !isLeft;
        }

        // determine if we would need to mirror current note
        // to get it to the correct side
        // this would be needed to get a note to left or downstem or right of upstem
        // whether or not we actually do this is determined later (based on user mirror property)
        bool nmirror = (chord->up() != isLeft);

        // by default, notes and dots are not hidden
        // this may be changed later to allow unisons to share noteheads
        note->setHidden(false);
        note->setDotsHidden(false);

        // be sure chord position is initialized
        // chord may be moved to the right later
        // if there are conflicts between voices
        chord->mutldata()->setPosX(0.0);

        // let user mirror property override the default we calculated
        if (note->userMirror() == DirectionH::AUTO) {
            mirror = nmirror;
        } else {
            mirror = note->chord()->up();
            if (note->userMirror() == DirectionH::LEFT) {
                mirror = !mirror;
            }
        }
        note->mutldata()->mirror.set_value(mirror);
        if (chord->stem()) {
            TLayout::layoutStem(chord->stem(), chord->stem()->mutldata(), ctx.conf()); // needed because mirroring can cause stem position to change
        }

        // accumulate return value
        if (!mirror) {
            maxWidth = std::max(maxWidth, note->bboxRightPos());
        }

        // prepare for next iteration
        lvisible = note->visible();
        lStaffIdx = staffIdx;
        ll       = line;
    }

    return maxWidth;
}

//---------------------------------------------------------
//   AcEl
//---------------------------------------------------------

struct AcEl {
    Note* note;
    double x;            // actual x position of this accidental relative to origin
    double top;          // top of accidental bbox relative to staff
    double bottom;       // bottom of accidental bbox relative to staff
    int line;           // line of note
    int next;           // index of next accidental of same pitch class (ascending list)
    double width;        // width of accidental
    double ascent;       // amount (in sp) vertical strokes extend above body
    double descent;      // amount (in sp) vertical strokes extend below body
    double rightClear;   // amount (in sp) to right of last vertical stroke above body
    double leftClear;    // amount (in sp) to left of last vertical stroke below body
};

//---------------------------------------------------------
//   resolveAccidentals
//    lx = calculated position of rightmost edge of left accidental relative to origin
//---------------------------------------------------------

static bool resolveAccidentals(AcEl* left, AcEl* right, double& lx, double pd, double sp)
{
    AcEl* upper;
    AcEl* lower;
    if (left->line >= right->line) {
        upper = right;
        lower = left;
    } else {
        upper = left;
        lower = right;
    }

    double gap = lower->top - upper->bottom;

    // no conflict at all if there is sufficient vertical gap between accidentals
    // the arrangement of accidentals into columns assumes accidentals an octave apart *do* clear
    if (gap >= pd || lower->line - upper->line >= 7) {
        return false;
    }

    double allowableOverlap = std::max(upper->descent, lower->ascent) - pd;

    // accidentals that are "close" (small gap or even slight overlap)
    if (std::abs(gap) <= 0.33 * sp) {
        // acceptable with slight offset
        // if one of the accidentals can subsume the overlap
        // and both accidentals allow it
        if (-gap <= allowableOverlap && std::min(upper->descent, lower->ascent) > 0.0) {
            double align = std::min(left->width, right->width);
            lx = std::min(lx, right->x + align - pd);
            return true;
        }
    }

    // amount by which overlapping accidentals will be separated
    // for example, the vertical stems of two flat signs
    // these need more space than we would need between non-overlapping accidentals
    double overlapShift = pd * 1.41;

    // accidentals with more significant overlap
    // acceptable if one accidental can subsume overlap
    if (left == lower && -gap <= allowableOverlap) {
        double offset = std::max(left->rightClear, right->leftClear);
        offset = std::min(offset, left->width) - overlapShift;
        lx = std::min(lx, right->x + offset);
        return true;
    }

    // accidentals with even more overlap
    // can work if both accidentals can subsume overlap
    if (left == lower && -gap <= upper->descent + lower->ascent - pd) {
        double offset = std::min(left->rightClear, right->leftClear) - overlapShift;
        if (offset > 0.0) {
            lx = std::min(lx, right->x + offset);
            return true;
        }
    }

    // otherwise, there is real conflict
    lx = std::min(lx, right->x - pd);
    return true;
}

//---------------------------------------------------------
//   layoutAccidental
//---------------------------------------------------------

static std::pair<double, double> layoutAccidental(const MStyle& style, AcEl* me, AcEl* above, AcEl* below, double colOffset,
                                                  std::vector<Note*>& leftNotes, double pnd,
                                                  double pd, double sp)
{
    double lx = colOffset;
    Accidental* acc = me->note->accidental();
    double mag = acc->mag();
    pnd *= mag;
    pd *= mag;

    Chord* chord = me->note->chord();
    Staff* staff = chord->staff();
    Fraction tick = chord->tick();

    // extra space for ledger lines
    double ledgerAdjust = 0.0;
    double ledgerVerticalClear = 0.0;
    bool ledgerAbove = chord->upNote()->line() <= -2;
    bool ledgerBelow = chord->downNote()->line() >= staff->lines(tick) * 2;
    if (ledgerAbove || ledgerBelow) {
        // ledger lines are present
        // check for collision with lines above & below staff
        // note that on 1-line staff, both collisions are possible at once
        // TODO: account for cutouts in accidental
        double lds = staff->lineDistance(tick) * sp;
        if ((ledgerAbove && me->top + lds <= pnd) || (ledgerBelow && staff->lines(tick) * lds - me->bottom <= pnd)) {
            ledgerAdjust = -style.styleS(Sid::ledgerLineLength).val() * sp;
            ledgerVerticalClear = style.styleS(Sid::ledgerLineWidth).val() * 0.5 * sp;
            lx = std::min(lx, ledgerAdjust);
        }
    }

    // clear left notes
    size_t lns = leftNotes.size();
    for (size_t i = 0; i < lns; ++i) {
        Note* ln = leftNotes[i];
        int lnLine = ln->line();
        double lnTop = (lnLine - 1) * 0.5 * sp;
        double lnBottom = lnTop + sp;
        if (me->top - lnBottom <= pnd && lnTop - me->bottom <= pnd) {
            double lnLedgerAdjust = 0.0;
            if (lnLine <= -2 || lnLine >= staff->lines(tick) * 2) {
                // left note has a ledger line we probably need to clear horizontally as well
                // except for accidentals that clear the last extended ledger line vertically
                // in these cases, the accidental may tuck closer
                Note* lastLnNote = lnLine < 0 ? leftNotes[0] : leftNotes[lns - 1];
                int lastLnLine = lastLnNote->line();
                double ledgerY = (lastLnLine / 2) * sp;
                if (me->line < 0 && ledgerY - me->bottom < ledgerVerticalClear) {
                    lnLedgerAdjust = ledgerAdjust;
                } else if (me->line > 0 && me->top - ledgerY < ledgerVerticalClear) {
                    lnLedgerAdjust = ledgerAdjust;
                }
            }
            // undercut note above if possible
            if (lnBottom - me->top <= me->ascent - pnd) {
                lx = std::min(lx, ln->x() + lnLedgerAdjust + me->rightClear);
            } else {
                lx = std::min(lx, ln->x() + lnLedgerAdjust);
            }
        } else if (lnTop > me->bottom) {
            break;
        }
    }

    // clear other accidentals
    bool conflictAbove = false;
    bool conflictBelow = false;

    if (above) {
        conflictAbove = resolveAccidentals(me, above, lx, pd, sp);
    }
    if (below) {
        conflictBelow = resolveAccidentals(me, below, lx, pd, sp);
    }
    if (conflictAbove || conflictBelow) {
        me->x = lx - acc->width() - acc->ldata()->bbox().x();
    } else if (colOffset != 0.0) {
        me->x = lx - pd - acc->width() - acc->ldata()->bbox().x();
    } else {
        me->x = lx - pnd - acc->width() - acc->ldata()->bbox().x();
    }

    return std::pair<double, double>(me->x, me->x + me->width);
}

//---------------------------------------------------------
//   placeDots
//---------------------------------------------------------

void ChordLayout::placeDots(const std::vector<Chord*>& chords, const std::vector<Note*>& notes)
{
    Chord* chord = nullptr;
    for (Chord* c : chords) {
        if (c->dots() > 0) {
            chord = c;
            break;
        } else {
            for (Note* note : c->notes()) {
                note->setDotRelativeLine(0); // this manages the deletion of dots
            }
        }
    }
    if (!chord || chord->staff()->isTabStaff(chord->tick())) {
        return;
    }
    std::vector<Note*> topDownNotes;
    std::vector<Note*> bottomUpNotes;
    std::vector<int> anchoredDots;
    // construct combined chords using the notes from overlapping chords
    getNoteListForDots(chord, topDownNotes, bottomUpNotes, anchoredDots);

    for (Note* note : notes) {
        bool onLine = !(note->line() & 1);
        if (onLine) {
            std::unordered_map<int, Note*> alreadyAdded;
            bool finished = false;
            for (Note* otherNote : bottomUpNotes) {
                int dotMove = otherNote->dotPosition() == DirectionV::UP ? -1 : 1;
                int otherDotLoc = otherNote->line() + dotMove;
                bool added = alreadyAdded.count(otherDotLoc);
                if (!added && mu::contains(anchoredDots, otherDotLoc)) {
                    dotMove = -dotMove; // if the desired space is taken, adjust opposite
                } else if (added && alreadyAdded[otherDotLoc] != otherNote) {
                    dotMove = -dotMove;
                }
                // set y for this note
                if (note == otherNote) {
                    note->setDotRelativeLine(dotMove);
                    finished = true;
                    anchoredDots.push_back(note->line() + dotMove);
                    alreadyAdded[otherNote->line() + dotMove] = otherNote;
                    break;
                }
            }
            if (!finished) {
                alreadyAdded.clear();
                for (Note* otherNote : topDownNotes) {
                    int dotMove = otherNote->dotPosition() == DirectionV::DOWN ? 1 : -1;
                    int otherDotLoc = otherNote->line() + dotMove;
                    bool added = alreadyAdded.count(otherDotLoc);
                    if (!added && mu::contains(anchoredDots, otherDotLoc)) {
                        dotMove = -dotMove;
                    } else if (added && alreadyAdded[otherDotLoc] != otherNote) {
                        dotMove = -dotMove;
                    }
                    // set y for this note
                    if (note == otherNote) {
                        note->setDotRelativeLine(dotMove);
                        finished = true;
                        anchoredDots.push_back(note->line() + dotMove);
                        break;
                    }
                    if (!added) {
                        alreadyAdded[otherNote->line() + dotMove] = otherNote;
                    }
                }
            }
            IF_ASSERT_FAILED(finished) {
                // this should never happen
                // the note is on a line and topDownNotes and bottomUpNotes are all of the lined notes
                LOGI() << "tick: " << note->tick().toString();
                note->setDotRelativeLine(0);
            }
        } else {
            // on a space; usually this means the dot is on this same line, but there is an exception
            // for a unison within the same chord.
            for (Note* otherNote : note->chord()->notes()) {
                if (note == otherNote) {
                    note->setDotRelativeLine(0); // same space as notehead
                    break;
                }
                if (note->line() == otherNote->line()) {
                    bool adjustDown = (note->chord()->voice() & 1) && !note->chord()->up();
                    note->setDotRelativeLine(adjustDown ? 2 : -2);
                    break;
                }
            }
        }
    }
}

//---------------------------------------------------------
//   layoutChords3
//    - calculate positions of notes, accidentals, dots
//---------------------------------------------------------

void ChordLayout::layoutChords3(const MStyle& style, const std::vector<Chord*>& chords,
                                const std::vector<Note*>& notes, const Staff* staff, LayoutContext& ctx)
{
    //---------------------------------------------------
    //    layout accidentals
    //    find column for dots
    //---------------------------------------------------

    std::vector<Note*> leftNotes;   // notes to left of origin
    leftNotes.reserve(8);
    std::vector<AcEl> aclist;         // accidentals
    aclist.reserve(8);

    // track columns of octave-separated accidentals
    int columnBottom[7] = { -1, -1, -1, -1, -1, -1, -1 };

    Fraction tick      =  notes.front()->chord()->segment()->tick();
    double sp           = staff->spatium(tick);
    double stepDistance = sp * staff->lineDistance(tick) * .5;
    int stepOffset     = staff->staffType(tick)->stepOffset();

    double lx           = 10000.0;    // leftmost notehead position
    double upDotPosX    = 0.0;
    double downDotPosX  = 0.0;

    int nNotes = int(notes.size());
    int nAcc = 0;
    int prevSubtype = 0;
    int prevLine = std::numeric_limits<int>::min();

    for (int i = nNotes - 1; i >= 0; --i) {
        Note* note     = notes[i];
        Accidental* ac = note->accidental();
        if (ac && ac->subtype() == prevSubtype && note->line() == prevLine) {
            // we shouldn't have two of the same accidental on the same line.
            // if we find one that is identical to the one before it, don't lay it out
            ac->setbbox(RectF());
            ac->setPos(PointF());
        } else if (ac) {
            prevLine = note->line();
            prevSubtype = ac->subtype();
            ac->computeMag();
            TLayout::layoutItem(ac, ctx);
            if (!ac->visible() || note->fixed()) {
                ac->setPos(ac->ldata()->bbox().x() - ac->width(), 0.0);
            } else {
                AcEl acel;
                acel.note   = note;
                int line    = note->line();
                acel.line   = line;
                acel.x      = 0.0;
                acel.top    = line * 0.5 * sp + ac->ldata()->bbox().top();
                acel.bottom = line * 0.5 * sp + ac->ldata()->bbox().bottom();
                acel.width  = ac->width();
                PointF bboxNE = ac->symBbox(ac->symId()).topRight();
                PointF bboxSW = ac->symBbox(ac->symId()).bottomLeft();
                PointF cutOutNE = ac->symSmuflAnchor(ac->symId(), SmuflAnchorId::cutOutNE);
                PointF cutOutSW = ac->symSmuflAnchor(ac->symId(), SmuflAnchorId::cutOutSW);
                if (!cutOutNE.isNull()) {
                    acel.ascent     = cutOutNE.y() - bboxNE.y();
                    acel.rightClear = bboxNE.x() - cutOutNE.x();
                } else {
                    acel.ascent     = 0.0;
                    acel.rightClear = 0.0;
                }

                if (!cutOutSW.isNull()) {
                    acel.descent   = bboxSW.y() - cutOutSW.y();
                    acel.leftClear = cutOutSW.x() - bboxSW.x();
                } else {
                    acel.descent   = 0.0;
                    acel.leftClear = 0.0;
                }

                int pitchClass = (line + 700) % 7;
                acel.next = columnBottom[pitchClass];
                columnBottom[pitchClass] = nAcc;
                aclist.push_back(acel);
                ++nAcc;
            }
        }

        Chord* chord = note->chord();
        bool _up     = chord->up();

        if (chord->stemSlash()) {
            TLayout::layoutStemSlash(chord->stemSlash(), chord->stemSlash()->mutldata(), ctx.conf());
        }

        double overlapMirror;
        Stem* stem = chord->stem();
        if (stem) {
            overlapMirror = stem->lineWidth() * chord->mag();
        } else if (chord->durationType().headType() == NoteHeadType::HEAD_WHOLE) {
            overlapMirror = style.styleMM(Sid::stemWidth) * chord->mag();
        } else {
            overlapMirror = 0.0;
        }

        double x = 0.0;
        if (note->ldata()->mirror()) {
            if (_up) {
                x = chord->stemPosX() - overlapMirror;
            } else {
                x = -note->headBodyWidth() + overlapMirror;
            }
        } else if (_up) {
            x = chord->stemPosX() - note->headBodyWidth();
        }

        double ny = (note->line() + stepOffset) * stepDistance;
        if (note->ldata()->pos().y() != ny) {
            note->mutldata()->setPosY(ny);
            if (chord->stem()) {
                TLayout::layoutStem(chord->stem(), chord->stem()->mutldata(), ctx.conf());
                if (chord->hook()) {
                    chord->hook()->mutldata()->setPosY(chord->stem()->flagPosition().y());
                }
            }
        }
        note->mutldata()->setPosX(x);

        // find leftmost non-mirrored note to set as X origin for accidental layout
        // a mirrored note that extends to left of segment X origin
        // will displace accidentals only if there is conflict
        double sx = x + chord->x();     // segment-relative X position of note
        if (note->ldata()->mirror() && !chord->up() && sx < -note->headBodyWidth() / 2) {
            leftNotes.push_back(note);
        } else if (sx < lx) {
            lx = sx;
        }

        double xx = x + note->headBodyWidth() + chord->pos().x();

        //---------------------------------------------------
        //    layout dots simply
        //     we will check for conflicts after all the notes have been processed
        //---------------------------------------------------

        DirectionV dotPosition = note->userDotPosition();
        if (chord->dots()) {
            if (chord->up()) {
                upDotPosX = std::max(upDotPosX, xx);
            } else {
                downDotPosX = std::max(downDotPosX, xx);
            }

            if (dotPosition == DirectionV::AUTO && nNotes > 1 && note->visible() && !note->dotsHidden()) {
                // resolve dot conflicts
                int line = note->line();
                Note* above = (i < nNotes - 1) ? notes[i + 1] : 0;
                if (above && (!above->visible() || above->dotsHidden() || above->chord()->dots() == 0)) {
                    above = 0;
                }
                int intervalAbove = above ? line - above->line() : 1000;
                Note* below = (i > 0) ? notes[i - 1] : 0;
                if (below && (!below->visible() || below->dotsHidden() || below->chord()->dots() == 0)) {
                    below = 0;
                }
                int intervalBelow = below ? below->line() - line : 1000;
                if ((line & 1) == 0) {
                    // line
                    if (intervalAbove == 1 && intervalBelow != 1) {
                        dotPosition = DirectionV::DOWN;
                    } else if (intervalBelow == 1 && intervalAbove != 1) {
                        dotPosition = DirectionV::UP;
                    } else if (intervalAbove == 0 || intervalBelow == 0) {
                        // unison
                        dotPosition = DirectionV::AUTO; // unison conflicts taken care of later
                    }
                } else {
                    // space
                    if (intervalAbove == 0 && above->chord()->dots()) {
                        // unison
                        if (!(note->voice() & 1)) {
                            dotPosition = DirectionV::UP; // space, doesn't matter
                        } else {
                            if (!(above->voice() & 1)) {
                                above->setDotPosition(DirectionV::UP);
                            } else {
                                dotPosition = DirectionV::DOWN;
                            }
                        }
                    }
                }
            }
        }
        if (dotPosition == DirectionV::AUTO) {
            dotPosition = note->voice() & 1 ? DirectionV::DOWN : DirectionV::UP;
        }
        note->setDotPosition(dotPosition);
    }
    // Now, we can resolve note conflicts as a superchord
    placeDots(chords, notes);

    // if there are no non-mirrored notes in a downstem chord,
    // then use the stem X position as X origin for accidental layout
    if (nNotes && static_cast<int>(leftNotes.size()) == nNotes) {
        lx = notes.front()->chord()->stemPosX();
    }

    // Look for conflicts in up-stem and down-stemmed chords. If conflicts, all dots are aligned
    // to the same vertical line. If no conflicts, each chords aligns the dots individually.
    bool conflict = false;
    for (Chord* chord1 : chords) {
        for (Chord* chord2 : chords) {
            if ((chord1 != chord2)
                && ((chord1->up() && !chord2->up() && chord2->upNote()->line() - chord1->downNote()->line() < 2)
                    || (!chord1->up() && chord2->up() && chord1->upNote()->line() - chord2->downNote()->line() < 2))) {
                conflict = true;
                break;
            }
        }
        if (conflict) {
            break;
        }
    }
    if (conflict) {
        double maxPosX = std::max(upDotPosX, downDotPosX);
        for (Chord* chord : chords) {
            chord->setDotPosX(maxPosX);
        }
    } else {
        for (Chord* chord : chords) {
            if (chord->up()) {
                chord->setDotPosX(upDotPosX);
            } else {
                chord->setDotPosX(downDotPosX);
            }
        }
    }

    if (nAcc == 0) {
        return;
    }

    std::vector<int> umi;
    double pd  = style.styleMM(Sid::accidentalDistance);
    double pnd = style.styleMM(Sid::accidentalNoteDistance);
    double colOffset = 0.0;

    if (nAcc >= 2 && aclist[nAcc - 1].line - aclist[0].line >= 7) {
        // accidentals spread over an octave or more
        // set up columns for accidentals with octave matches
        // these will start at right and work to the left
        // unmatched accidentals will use zig zag approach (see below)
        // starting to the left of the octave columns

        int columnTop[7] = { -1, -1, -1, -1, -1, -1, -1 };

        // find columns of octaves
        for (int pc = 0; pc < 7; ++pc) {
            if (columnBottom[pc] == -1) {
                continue;
            }
            // calculate column height
            for (int j = columnBottom[pc]; j != -1; j = aclist[j].next) {
                columnTop[pc] = j;
            }
        }

        // compute reasonable column order
        // use zig zag
        std::vector<int> column;
        std::vector<int> unmatched;
        int n = nAcc - 1;
        for (int i = 0; i <= n; ++i, --n) {
            int pc = (aclist[i].line + 700) % 7;
            if (aclist[columnTop[pc]].line != aclist[columnBottom[pc]].line) {
                if (!mu::contains(column, pc)) {
                    column.push_back(pc);
                }
            } else {
                unmatched.push_back(i);
            }
            if (i == n) {
                break;
            }
            pc = (aclist[n].line + 700) % 7;
            if (aclist[columnTop[pc]].line != aclist[columnBottom[pc]].line) {
                if (!mu::contains(column, pc)) {
                    column.push_back(pc);
                }
            } else {
                unmatched.push_back(n);
            }
        }
        size_t nColumns = column.size();
        size_t nUnmatched = unmatched.size();

        // handle unmatched accidentals
        for (size_t i = 0; i < nUnmatched; ++i) {
            // first try to slot it into an existing column
            AcEl* me = &aclist[unmatched[i]];
            // find column
            bool found = false;
            for (size_t j = 0; j < nColumns; ++j) {
                int pc = column[j];
                int above = -1;
                int below = -1;
                // find slot within column
                for (int k = columnBottom[pc]; k != -1; k = aclist[k].next) {
                    if (aclist[k].line < me->line) {
                        above = k;
                        break;
                    }
                    below = k;
                }
                // check to see if accidental can fit in slot
                double myPd = pd * me->note->accidental()->mag();
                bool conflict2 = false;
                if (above != -1 && me->top - aclist[above].bottom < myPd) {
                    conflict2 = true;
                } else if (below != -1 && aclist[below].top - me->bottom < myPd) {
                    conflict2 = true;
                }
                if (!conflict2) {
                    // insert into column
                    found = true;
                    me->next = above;
                    if (above == -1) {
                        columnTop[pc] = unmatched[i];
                    }
                    if (below != -1) {
                        aclist[below].next = unmatched[i];
                    } else {
                        columnBottom[pc] = unmatched[i];
                    }
                    break;
                }
            }
            // if no slot found, then add to list of unmatched accidental indices
            if (!found) {
                umi.push_back(unmatched[i]);
            }
        }
        nAcc = static_cast<int>(umi.size());
        if (nAcc > 1) {
            std::sort(umi.begin(), umi.end());
        }

        bool alignLeft = style.styleB(Sid::alignAccidentalsLeft);

        // through columns
        for (size_t i = 0; i < nColumns; ++i) {
            // column index
            const int pc = column[i];

            double minX = 0.0;
            double maxX = 0.0;
            AcEl* below = 0;
            // through accidentals in this column
            for (int j = columnBottom[pc]; j != -1; j = aclist[j].next) {
                std::pair<double, double> x = layoutAccidental(style, &aclist[j], 0, below, colOffset, leftNotes, pnd, pd, sp);
                minX = std::min(minX, x.first);
                maxX = std::min(maxX, x.second);
                below = &aclist[j];
            }

            // align
            int next = -1;
            for (int j = columnBottom[pc]; j != -1; j = next) {
                AcEl* current = &aclist[j];
                next = current->next;
                if (next != -1 && current->line == aclist[next].line) {
                    continue;
                }

                if (alignLeft) {
                    current->x = minX;
                } else {
                    current->x = maxX - current->width;
                }
            }
            colOffset = minX;
        }
    } else {
        for (int i = 0; i < nAcc; ++i) {
            umi.push_back(i);
        }
    }

    if (nAcc) {
        // for accidentals with no octave matches, use zig zag approach
        // layout right to left in pairs, (next) highest then lowest

        AcEl* me = &aclist[umi[0]];
        AcEl* above = 0;
        AcEl* below = 0;

        // layout top accidental
        layoutAccidental(style, me, above, below, colOffset, leftNotes, pnd, pd, sp);

        // layout bottom accidental
        int n = nAcc - 1;
        if (n > 0) {
            above = me;
            me = &aclist[umi[n]];
            layoutAccidental(style, me, above, below, colOffset, leftNotes, pnd, pd, sp);
        }

        // layout middle accidentals
        if (n > 1) {
            for (int i = 1; i < n; ++i, --n) {
                // next highest
                below = me;
                me = &aclist[umi[i]];
                layoutAccidental(style, me, above, below, colOffset, leftNotes, pnd, pd, sp);
                if (i == n - 1) {
                    break;
                }
                // next lowest
                above = me;
                me = &aclist[umi[n - 1]];
                layoutAccidental(style, me, above, below, colOffset, leftNotes, pnd, pd, sp);
            }
        }
    }

    for (const AcEl& e : aclist) {
        // even though we initially calculate accidental position relative to segment
        // we must record pos for accidental relative to note,
        // since pos is always interpreted relative to parent
        Note* note = e.note;
        double x    = e.x + lx - (note->x() + note->chord()->x());
        note->accidental()->setPos(x, 0);
    }
}

//---------------------------------------------------------
//   getNoteListForDots
//      This method populates three lists: one for chord notes that need to be checked from the top down,
//      one for chords from the bottom up, and one for spaces (where the dot will be in that space)
//---------------------------------------------------------

void ChordLayout::getNoteListForDots(Chord* c, std::vector<Note*>& topDownNotes, std::vector<Note*>& bottomUpNotes,
                                     std::vector<int>& anchoredDots)
{
    Measure* measure = c->measure();
    bool hasVoices = measure->hasVoices(c->vStaffIdx(), c->tick(), c->ticks());
    bool hasUpperCrossNotes = false;
    bool hasLowerCrossNotes = false;
    staff_idx_t partTopStaff = c->part()->startTrack() / VOICES;
    staff_idx_t partBottomStaff = c->part()->endTrack() / VOICES;
    track_idx_t startVoice = c->track() - c->voice();
    // Get the last track we need to check for cross staff notes.
    // Either 1 stave away from the stave we are laying out or the bottom staff of the part
    track_idx_t lastVoice = std::min(c->vStaffIdx() + 2, partBottomStaff) * VOICES;

    // Check for cross staff notes on staff above without dots
    if (partTopStaff != c->vStaffIdx()) {
        for (size_t i = partTopStaff * VOICES; i < (partTopStaff + 1) * VOICES; ++i) {
            if (Chord* voiceChord = measure->findChord(c->tick(), i)) {
                if (voiceChord->vStaffIdx() == c->vStaffIdx()) {
                    hasUpperCrossNotes = true;
                    startVoice = i;
                    break;
                }
            }
        }
    }

    // Check for cross staff notes on stave below
    if (partBottomStaff != c->vStaffIdx()) {
        for (size_t i = (c->vStaffIdx() + 1) * VOICES; i < lastVoice; ++i) {
            if (Chord* voiceChord = measure->findChord(c->tick(), i)) {
                if (voiceChord->vStaffIdx() == c->vStaffIdx()) {
                    hasLowerCrossNotes = true;
                    break;
                }
            }
        }
    }

    if (!hasVoices && !(hasUpperCrossNotes || hasLowerCrossNotes)) {
        // only this voice, so topDownNotes is just the notes in the chord
        for (Note* note : c->notes()) {
            if (note->line() & 1) {
                int newOffset = 0;
                bool adjustDown = (c->voice() & 1) && !c->up();
                if (!anchoredDots.empty() && anchoredDots.back() == note->line()) {
                    if (anchoredDots.size() >= 2 && anchoredDots[anchoredDots.size() - 2] == note->line() + (adjustDown ? 2 : -2)) {
                        newOffset = adjustDown ? -2 : 2;
                    } else {
                        newOffset = adjustDown ? 2 : -2;
                    }
                }
                anchoredDots.push_back(note->line() + newOffset);
            } else {
                topDownNotes.push_back(note);
            }
        }
    } else {
        // Get a list of notes in this staff that adjust dots from top down,
        // bottom up, and also start our locked-in dot list by adding all lines where dots are
        // guaranteed
        lastVoice = hasLowerCrossNotes ? lastVoice : (c->vStaffIdx() + 1) * VOICES;
        // Check staves above and below for moved chords (If staff below is available)
        for (size_t i = startVoice; i < lastVoice; ++i) {
            if (Chord* voiceChord = measure->findChord(c->tick(), i)) {
                // Skip chords on adjacent staves which have not been moved to this staff
                if (voiceChord->vStaffIdx() != c->vStaffIdx()) {
                    continue;
                }
                bool startFromTop = !((voiceChord->voice() & 1) && !voiceChord->up()) && voiceChord->staffMove() != -1;
                if (startFromTop) {
                    for (Note* note : voiceChord->notes()) {
                        if (note->line() & 1) {
                            anchoredDots.push_back(note->line());
                        } else {
                            topDownNotes.push_back(note);
                        }
                    }
                } else {
                    for (Note* note : voiceChord->notes()) {
                        if (note->line() & 1) {
                            anchoredDots.push_back(note->line());
                        } else {
                            bottomUpNotes.push_back(note);
                        }
                    }
                }
            }
        }
    }
    // our two lists now contain only notes that are on lines
    std::sort(topDownNotes.begin(), topDownNotes.end(),
              [](Note* n1, Note* n2) { return n1->line() < n2->line(); });
    std::sort(bottomUpNotes.begin(), bottomUpNotes.end(),
              [](Note* n1, Note* n2) { return n1->line() > n2->line(); });
}

/* updateGraceNotes()
 * Processes a full measure, making sure that all grace notes are
 * attacched to the correct segment. Has to be performed after
 * all the segments are known.
 * */
void ChordLayout::updateGraceNotes(Measure* measure, LayoutContext& ctx)
{
    // Clean everything
    for (Segment& s : measure->segments()) {
        for (unsigned track = 0; track < ctx.dom().ntracks(); ++track) {
            EngravingItem* e = s.preAppendedItem(track);
            if (e && e->isGraceNotesGroup()) {
                s.clearPreAppended(track);
                std::set<staff_idx_t> stavesToReShape;
                for (Chord* grace : *toGraceNotesGroup(e)) {
                    stavesToReShape.insert(grace->staffIdx());
                    stavesToReShape.insert(grace->vStaffIdx());
                }
                for (staff_idx_t staffToReshape : stavesToReShape) {
                    s.createShape(staffToReshape);
                }
            }
        }
    }
    // Append grace notes to appropriate segment
    for (Segment& s : measure->segments()) {
        if (!s.isChordRestType()) {
            continue;
        }
        for (auto el : s.elist()) {
            if (el && el->isChord() && !toChord(el)->graceNotes().empty()) {
                appendGraceNotes(toChord(el));
            }
        }
    }
    // Layout grace note groups
    for (Segment& s : measure->segments()) {
        for (unsigned track = 0; track < ctx.dom().ntracks(); ++track) {
            EngravingItem* e = s.preAppendedItem(track);
            if (e && e->isGraceNotesGroup()) {
                GraceNotesGroup* gng = toGraceNotesGroup(e);
                TLayout::layoutGraceNotesGroup(gng, ctx);
                gng->addToShape();
            }
        }
    }
}

void ChordLayout::appendGraceNotes(Chord* chord)
{
    Segment* segment = chord->segment();
    Measure* measure = chord->measure();
    track_idx_t track = chord->track();
    staff_idx_t staffIdx = chord->staffIdx();
    GraceNotesGroup& gnb = chord->graceNotesBefore();
    GraceNotesGroup& gna = chord->graceNotesAfter();

    //Attach graceNotesBefore of this chord to *this* segment
    if (!gnb.empty()) {
        // If this segment already contains grace notes in the same voice (could happen if a
        // previous chord has appended grace-notes-after here) put them in the same vector.
        EngravingItem* item = segment->preAppendedItem(static_cast<int>(track));
        if (item && item->isGraceNotesGroup()) {
            GraceNotesGroup* gng = toGraceNotesGroup(item);
            gng->insert(gng->end(), gnb.begin(), gnb.end());
        } else {
            gnb.setAppendedSegment(segment);
            segment->preAppend(&gnb, static_cast<int>(track));
        }
    }

    //Attach graceNotesAfter of this chord to the *following* segment
    if (!gna.empty()) {
        Segment* followingSeg = measure->tick2segment(segment->tick() + chord->actualTicks(), SegmentType::All);
        while (followingSeg && !followingSeg->hasElements(staff2track(staffIdx), staff2track(staffIdx) + 3)) {
            // If there is nothing on this staff, go to next segment
            followingSeg = followingSeg->next();
        }
        if (followingSeg) {
            gna.setAppendedSegment(followingSeg);
            followingSeg->preAppend(&gna, static_cast<int>(track));
        }
    }
}

/* Grace-notes-after have the special property of belonging to
*  a segment but being pre-appended to another. This repositioning
*  is needed and must be called AFTER horizontal spacing is calculated. */
void ChordLayout::repositionGraceNotesAfter(Segment* segment, size_t tracks)
{
    for (size_t track = 0; track < tracks; track++) {
        EngravingItem* item = segment->preAppendedItem(static_cast<int>(track));
        if (!item || !item->isGraceNotesGroup()) {
            continue;
        }
        GraceNotesGroup* gng = toGraceNotesGroup(item);
        for (Chord* chord : *gng) {
            double offset = segment->ldata()->pos().x() - chord->parentItem()->parentItem()->ldata()->pos().x();
            // Difference between the segment they "belong" and the segment they are "appended" to.
            chord->setPos(chord->ldata()->pos().x() + offset, 0.0);
        }
    }
}

void ChordLayout::clearLineAttachPoints(Measure* measure)
{
    for (Segment& s : measure->segments()) {
        if (!s.isChordRestType()) {
            continue;
        }
        for (EngravingItem* e : s.elist()) {
            if (!e || !e->isChord()) {
                continue;
            }
            Chord* c = toChord(e);
            for (Note* n : c->notes()) {
                n->lineAttachPoints().clear();
            }
            for (Chord* ch : c->graceNotes()) {
                for (Note* n : ch->notes()) {
                    n->lineAttachPoints().clear();
                }
            }
        }
    }
}

/* We perform a pre-layout of ties and glissandi to obtain the attach points and attach them to
 * the notes of the chord. Will be needed for spacing calculation, particularly to
 * enforce minTieLength. The true layout of ties and glissandi is done much later. */
void ChordLayout::updateLineAttachPoints(Chord* chord, bool isFirstInMeasure, LayoutContext& ctx)
{
    if (chord->endsGlissandoOrGuitarBend()) {
        for (Note* note : chord->notes()) {
            for (Spanner* sp : note->spannerBack()) {
                if (sp->isGlissando()) {
                    TLayout::layoutGlissando(toGlissando(sp), ctx);
                } else if (sp->isGuitarBend()) {
                    TLayout::layoutGuitarBend(toGuitarBend(sp), ctx);
                }
            }
        }
    }
    if (isFirstInMeasure) {
        for (Note* note : chord->notes()) {
            Tie* tieBack = note->tieBack();
            if (tieBack && tieBack->startNote()->findMeasure() != note->findMeasure()) {
                SlurTieLayout::tieLayoutBack(tieBack, note->findMeasure()->system());
            }
        }
    }
    for (Note* note : chord->notes()) {
        Tie* tie = note->tieFor();
        if (tie) {
            Note* endNote = tie->endNote();
            if (endNote && endNote->findMeasure() == note->findMeasure()) {
                SlurTieLayout::tieLayoutFor(tie, note->findMeasure()->system());  // line attach points are updated here
            }
        }
    }
}

void ChordLayout::resolveVerticalRestConflicts(LayoutContext& ctx, Segment* segment, staff_idx_t staffIdx)
{
    std::vector<Rest*> rests;
    std::vector<Chord*> chords;

    collectChordsAndRest(segment, staffIdx, chords, rests);

    if (rests.empty()) {
        return;
    }

    collectChordsOverlappingRests(segment, staffIdx, chords);

    for (Rest* rest : rests) {
        rest->verticalClearance().reset();
    }

    const Staff* staff = ctx.dom().staff(staffIdx);
    if (!chords.empty()) {
        resolveRestVSChord(rests, chords, staff, segment);
    }

    if (rests.size() < 2) {
        return;
    }

    resolveRestVSRest(rests, staff, segment, ctx);
}

void ChordLayout::resolveRestVSChord(std::vector<Rest*>& rests, std::vector<Chord*>& chords, const Staff* staff, Segment* segment)
{
    Fraction tick = segment->tick();
    int lines = staff->lines(tick);
    double spatium = staff->spatium(tick);
    double lineDistance = staff->lineDistance(tick) * spatium;
    double minRestToChordClearance = 0.35 * spatium;

    for (Rest* rest : rests) {
        if (!rest->visible() || !rest->autoplace()) {
            continue;
        }
        RestVerticalClearance& restVerticalClearance = rest->verticalClearance();
        for (Chord* chord : chords) {
            if (!chord->visible() || !chord->autoplace()) {
                continue;
            }

            bool restAbove = rest->voice() < chord->voice() || (chord->slash() && !(rest->voice() % 2));
            int upSign = restAbove ? -1 : 1;
            double restYOffset = rest->offset().y();
            bool ignoreYOffset = (restAbove && restYOffset > 0) || (!restAbove && restYOffset < 0);
            PointF offset = ignoreYOffset ? PointF(0, restYOffset) : PointF(0, 0);

            Shape chordShape = chord->shape().translated(chord->pos());
            chordShape.removeInvisibles();
            if (chordShape.empty()) {
                continue;
            }

            double clearance = 0.0;
            Shape restShape = rest->shape().translated(rest->pos() - offset);
            if (chord->segment() == rest->segment()) {
                clearance = restAbove
                            ? restShape.verticalClearance(chordShape)
                            : chordShape.verticalClearance(restShape);
            } else {
                Note* limitNote = restAbove ? chord->upNote() : chord->downNote();
                Shape noteShape = limitNote->shape().translate(limitNote->pos());
                clearance = restAbove ? noteShape.top() - restShape.bottom() : restShape.top() - noteShape.bottom();
                minRestToChordClearance = 0.0;
            }

            double margin = clearance - minRestToChordClearance;
            int marginInSteps = floor(margin / lineDistance);
            if (restAbove) {
                restVerticalClearance.setBelow(marginInSteps);
            } else {
                restVerticalClearance.setAbove(marginInSteps);
            }
            if (margin > 0) {
                continue;
            }

            rest->verticalClearance().setLocked(true);
            bool isWholeOrHalf = rest->isWholeRest() || rest->durationType() == DurationType::V_HALF;
            bool outAboveStaff = restAbove && restShape.bottom() + margin < minRestToChordClearance;
            bool outBelowStaff = !restAbove && restShape.top() - margin > (lines - 1) * lineDistance - minRestToChordClearance;
            bool useHalfSpaceSteps = (outAboveStaff || outBelowStaff) && !isWholeOrHalf;
            double yMove;
            if (useHalfSpaceSteps) {
                int steps = ceil(abs(margin) / (lineDistance / 2));
                yMove = steps * lineDistance / 2 * upSign;
                rest->mutldata()->moveY(yMove);
            } else {
                int steps = ceil(abs(margin) / lineDistance);
                yMove = steps * lineDistance * upSign;
                rest->mutldata()->moveY(yMove);
            }
            for (Rest* mergedRest : rest->ldata()->mergedRests) {
                mergedRest->mutldata()->moveY(yMove);
            }
            if (isWholeOrHalf) {
                double y = rest->pos().y();
                int line = y < 0 ? floor(y / lineDistance) : floor(y / lineDistance);
                rest->updateSymbol(line, lines, rest->mutldata()); // Because it may need to use the symbol with ledger line now
            }
        }
    }
}

void ChordLayout::resolveRestVSRest(std::vector<Rest*>& rests, const Staff* staff,
                                    Segment* segment, LayoutContext& ctx,
                                    bool considerBeams)
{
    if (rests.empty()) {
        return;
    }

    Fraction tick = segment->tick();
    double spatium = staff->spatium(tick);
    double lineDistance = staff->lineDistance(tick) * spatium;
    int lines = staff->lines(tick);
    const double minRestToRestClearance = 0.35 * spatium;

    for (size_t i = 0; i < rests.size() - 1; ++i) {
        Rest* rest1 = rests[i];
        if (!rest1->visible() || !rest1->autoplace()) {
            continue;
        }

        RestVerticalClearance& rest1Clearance = rest1->verticalClearance();
        Shape shape1 = rest1->shape().translated(rest1->pos() - rest1->offset());

        Rest* rest2 = rests[i + 1];
        if (!rest2->visible() || !rest2->autoplace()) {
            continue;
        }

        if (mu::contains(rest1->ldata()->mergedRests, rest2) || mu::contains(rest2->ldata()->mergedRests, rest1)) {
            continue;
        }

        Shape shape2 = rest2->shape().translated(rest2->pos() - rest2->offset());
        RestVerticalClearance& rest2Clearance = rest2->verticalClearance();

        double clearance;
        bool firstAbove = rest1->voice() < rest2->voice();
        if (firstAbove) {
            clearance = shape1.verticalClearance(shape2);
        } else {
            clearance = shape2.verticalClearance(shape1);
        }
        double margin = clearance - minRestToRestClearance;
        int marginInSteps = floor(margin / lineDistance);
        if (firstAbove) {
            rest1Clearance.setBelow(marginInSteps);
            rest2Clearance.setAbove(marginInSteps);
        } else {
            rest1Clearance.setAbove(marginInSteps);
            rest2Clearance.setBelow(marginInSteps);
        }

        if (margin > 0) {
            continue;
        }

        int steps = ceil(abs(margin) / lineDistance);
        // Move the two rests away from each other
        int step1 = floor(double(steps) / 2);
        int step2 = ceil(double(steps) / 2);
        int maxStep1 = firstAbove ? rest1Clearance.above() : rest1Clearance.below();
        int maxStep2 = firstAbove ? rest2Clearance.below() : rest2Clearance.above();
        maxStep1 = std::max(maxStep1, 0);
        maxStep2 = std::max(maxStep2, 0);
        if (step1 > maxStep1) {
            step2 += step1 - maxStep1; // First rest is locked, try move the second more
        }
        if (step2 > maxStep2) {
            step1 += step2 - maxStep2; // Second rest is locked, try move the first more
        }
        step1 = std::min(step1, maxStep1);
        step2 = std::min(step2, maxStep2);
        rest1->mutldata()->moveY(step1 * lineDistance * (firstAbove ? -1 : 1));
        rest2->mutldata()->moveY(step2 * lineDistance * (firstAbove ? 1 : -1));

        Beam* beam1 = rest1->beam();
        Beam* beam2 = rest2->beam();
        if (beam1 && beam2 && considerBeams) {
            shape1 = rest1->shape().translated(rest1->pos() - rest1->offset());
            shape2 = rest2->shape().translated(rest2->pos() - rest2->offset());

            ChordRest* beam1Start = beam1->elements().front();
            ChordRest* beam1End = beam1->elements().back();
            double y1Start = BeamLayout::chordBeamAnchorY(beam1, beam1Start) - beam1Start->pagePos().y();
            double y1End = BeamLayout::chordBeamAnchorY(beam1, beam1End) - beam1End->pagePos().y();
            double beam1Ymid = 0.5 * (y1Start + y1End);

            ChordRest* beam2Start = beam2->elements().front();
            ChordRest* beam2End = beam2->elements().back();
            double y2Start = BeamLayout::chordBeamAnchorY(beam2, beam2Start) - beam2Start->pagePos().y();
            double y2End = BeamLayout::chordBeamAnchorY(beam2, beam2End) - beam2End->pagePos().y();
            double beam2Ymid = 0.5 * (y2Start + y2End);

            double centerY = 0.5 * (beam1Ymid + beam2Ymid);

            double upperBound = shape1.bottom();
            double lowerBound = shape2.top();
            int steps2 = 0;
            if (centerY < upperBound) {
                steps2 = floor((centerY - upperBound) / lineDistance);
            } else if (centerY > lowerBound) {
                steps2 = ceil((centerY - lowerBound) / lineDistance);
            }
            double moveY = steps2 * lineDistance;
            rest1->mutldata()->moveY(moveY);
            rest2->mutldata()->moveY(moveY);
            shape1.translate(PointF(0.0, moveY));
            shape2.translate(PointF(0.0, moveY));

            double halfLineDistance = 0.5 * lineDistance;
            if (shape1.bottom() < -halfLineDistance) {
                rest1->mutldata()->moveY(halfLineDistance);
            } else if (centerY >= (lines - 1) * lineDistance + halfLineDistance) {
                rest2->mutldata()->moveY(-halfLineDistance);
            }

            rest1->verticalClearance().setLocked(true);
            rest2->verticalClearance().setLocked(true);
            TLayout::layoutBeam(beam1, ctx);
            TLayout::layoutBeam(beam2, ctx);
        }

        bool rest1IsWholeOrHalf = rest1->isWholeRest() || rest1->durationType() == DurationType::V_HALF;
        bool rest2IsWholeOrHalf = rest2->isWholeRest() || rest2->durationType() == DurationType::V_HALF;
        double y = 0.0;
        int line = 0;

        if (rest1IsWholeOrHalf) {
            Rest::LayoutData* rest1LayoutData = rest1->mutldata();
            y = rest1->pos().y();
            line = y < 0 ? floor(y / lineDistance) : floor(y / lineDistance);
            rest1->updateSymbol(line, lines, rest1LayoutData);
        }
        if (rest2IsWholeOrHalf) {
            Rest::LayoutData* rest2LayoutData = rest2->mutldata();
            y = rest2->pos().y();
            line = y < 0 ? floor(y / lineDistance) : floor(y / lineDistance);
            rest2->updateSymbol(line, lines, rest2LayoutData);
        }
    }
}

void ChordLayout::layoutChordBaseFingering(Chord* chord, System* system, LayoutContext&)
{
    std::set<staff_idx_t> shapesToRecreate;
    std::list<Note*> notes;
    Segment* segment = chord->segment();
    for (auto gc : chord->graceNotes()) {
        for (auto n : gc->notes()) {
            notes.push_back(n);
        }
    }
    for (auto n : chord->notes()) {
        notes.push_back(n);
    }
    std::list<Fingering*> fingerings;
    for (Note* note : notes) {
        for (EngravingItem* el : note->el()) {
            if (el->isFingering()) {
                Fingering* f = toFingering(el);
                if (f->layoutType() == ElementType::CHORD && !f->isOnCrossBeamSide()) {
                    // Fingering on top of cross-staff beams must be laid out later
                    if (f->placeAbove()) {
                        fingerings.push_back(f);
                    } else {
                        fingerings.push_front(f);
                    }
                }
            }
        }
    }
    for (Fingering* f : fingerings) {
        TLayout::layoutFingering(f, f->mutldata());
        if (f->addToSkyline()) {
            Note* n = f->note();
            RectF r
                = f->ldata()->bbox().translated(f->pos() + n->pos() + n->chord()->pos() + segment->pos() + segment->measure()->pos());
            system->staff(f->note()->chord()->vStaffIdx())->skyline().add(r);
        }
        shapesToRecreate.insert(f->staffIdx());
    }
    for (staff_idx_t staffIdx : shapesToRecreate) {
        segment->createShape(staffIdx);
    }
}

void ChordLayout::layoutStretchedBends(Chord* chord, LayoutContext& ctx)
{
    if (!Note::engravingConfiguration()->guitarProImportExperimental()) {
        return;
    }

    for (EngravingItem* item : chord->el()) {
        if (item && item->isStretchedBend()) {
            toStretchedBend(item)->adjustBendInChord();
        }
    }

    for (EngravingItem* item : chord->el()) {
        if (item && item->isStretchedBend()) {
            TLayout::layoutStretched(toStretchedBend(item), ctx);
        }
    }
}

void ChordLayout::crossMeasureSetup(Chord* chord, bool on, LayoutContext& ctx)
{
    if (!on) {
        if (chord->crossMeasure() != CrossMeasure::UNKNOWN) {
            chord->setCrossMeasure(CrossMeasure::UNKNOWN);
            layoutStem(chord, ctx);
        }
        return;
    }
    if (chord->crossMeasure() == CrossMeasure::UNKNOWN) {
        CrossMeasure tempCross = CrossMeasure::NONE;      // assume no cross-measure modification
        // if chord has only one note and note is tied forward
        if (chord->notes().size() == 1 && chord->notes().at(0)->tieFor()) {
            Chord* tiedChord = chord->notes().at(0)->tieFor()->endNote()->chord();
            // if tied note belongs to another measure and to a single-note chord
            if (tiedChord->measure() != chord->measure() && tiedChord->notes().size() == 1) {
                // get total duration
                std::vector<TDuration> durList = toDurationList(
                    chord->actualDurationType().fraction()
                    + tiedChord->actualDurationType().fraction(), true);
                // if duration can be expressed as a single duration
                // apply cross-measure modification
                if (durList.size() == 1) {
                    tempCross = CrossMeasure::FIRST;
                    chord->setCrossMeasure(tempCross);
                    chord->setCrossMeasureDurationType(durList[0]);
                    layoutStem(chord, ctx);
                }
            }
            chord->setCrossMeasure(tempCross);
            tiedChord->setCrossMeasure(tempCross == CrossMeasure::FIRST
                                       ? CrossMeasure::SECOND : CrossMeasure::NONE);
        }
    }
}

// called after final position of note is set
void ChordLayout::layoutNote2(Note* item, LayoutContext& ctx)
{
    const Staff* staff = item->staff();
    if (!staff) {
        return;
    }

    Note::LayoutData* ldata = item->mutldata();
    const StaffType* staffType = staff->staffTypeForElement(item);
    // for standard staves this is done in Score::layoutChords3()
    // so that the results are available there
    bool isTabStaff = staffType && staffType->isTabStaff();
    // First, for tab staves that have show back-tied fret marks option, we add parentheses to the tied note if
    // the tie spans a system boundary. This can't be done in layout as the system of each note is not decided yet
    bool useParens = isTabStaff && !staffType->showBackTied() && !item->fixed();
    if (useParens
        && item->tieBack()
        && (
            item->chord()->measure()->system() != item->tieBack()->startNote()->chord()->measure()->system()
            || !item->el().empty()
            )) {
        if (!item->fretString().startsWith(u'(')) { // Hack: don't add parentheses if already added
            item->setFretString(String(u"(%1)").arg(item->fretString()));
        }
        double w = item->tabHeadWidth(staffType);     // !! use _fretString
        ldata->setBbox(0, staffType->fretBoxY() * item->magS(), w, staffType->fretBoxH() * item->magS());
    }
    int dots = item->chord()->dots();
    if (dots && !item->dots().empty()) {
        // if chords have notes with different mag, dots must still  align
        double correctMag = item->chord()->notes().size() > 1 ? item->chord()->mag() : item->mag();
        double d  = ctx.conf().point(ctx.conf().styleS(Sid::dotNoteDistance)) * correctMag;
        double dd = ctx.conf().point(ctx.conf().styleS(Sid::dotDotDistance)) * correctMag;
        double x  = item->chord()->dotPosX() - item->pos().x() - item->chord()->pos().x();
        // in case of dots with different size, center-align them
        if (item->mag() != item->chord()->mag() && item->chord()->notes().size() > 1) {
            double relativeMag = item->mag() / item->chord()->mag();
            double centerAlignOffset = item->dot(0)->width() * (1 / relativeMag - 1) / 2;
            x += centerAlignOffset;
        }
        // adjust dot distance for hooks
        if (item->chord()->hook() && item->chord()->up()) {
            double hookRight = item->chord()->hook()->width() + item->chord()->hook()->x() + item->chord()->pos().x();
            double hookBottom = item->chord()->hook()->height() + item->chord()->hook()->y() + item->chord()->pos().y()
                                + (0.25 * item->spatium());
            // the top dot in the chord, not the dot for this particular note:
            double dotY = item->chord()->notes().back()->y() + item->chord()->notes().back()->dots().front()->pos().y();
            if (item->chord()->dotPosX() < hookRight && dotY < hookBottom) {
                d = item->chord()->hook()->width();
            }
        }
        // if TAB and stems through staff
        if (isTabStaff && staffType->stemThrough()) {
            // with TAB's, dot Y is not calculated during layoutChords3(),
            // as layoutChords3() is not even called for TAB's;
            // setDotRelativeLine() actually also manages creation/deletion of NoteDot's
            item->setDotRelativeLine(0);

            // use TAB default note-to-dot spacing
            dd = STAFFTYPE_TAB_DEFAULTDOTDIST_X * item->spatium();
            d = dd * 0.5;
        }
        // apply to dots
        double xx = x + d;
        for (NoteDot* dot : item->dots()) {
            dot->mutldata()->setPosX(xx);
            xx += dd;
        }
    }

    // layout elements attached to note
    for (EngravingItem* e : item->el()) {
        if (e->isSymbol()) {
            e->mutldata()->setMag(item->mag());
            Shape noteShape = item->shape();
            noteShape.remove_if([e](ShapeElement& s) { return s.item() == e || s.item()->isBend(); });
            LedgerLine* ledger = item->line() < -1 || item->line() > item->staff()->lines(item->tick())
                                 ? item->chord()->ledgerLines() : nullptr;
            if (ledger) {
                noteShape.add(ledger->shape().translate(ledger->pos() - item->pos()));
            }
            double right = noteShape.right();
            double left = noteShape.left();
            Symbol* sym = toSymbol(e);
            TLayout::layoutItem(e, ctx);
            double parenthesisPadding = ctx.conf().styleMM(Sid::bracketedAccidentalPadding) * item->mag();
            if (sym->sym() == SymId::noteheadParenthesisRight) {
                if (isTabStaff) {
                    const Staff* st = item->staff();
                    const StaffType* tab = st->staffTypeForElement(item);
                    right = item->tabHeadWidth(tab);
                }

                if (Note::engravingConfiguration()->tablatureParenthesesZIndexWorkaround() && item->staff()->isTabStaff(e->tick())) {
                    e->mutldata()->moveX(right + item->symWidth(SymId::noteheadParenthesisRight));
                } else {
                    e->mutldata()->setPosX(right + parenthesisPadding);
                }
            } else if (sym->sym() == SymId::noteheadParenthesisLeft) {
                if (!Note::engravingConfiguration()->tablatureParenthesesZIndexWorkaround() || !item->staff()->isTabStaff(e->tick())) {
                    e->mutldata()->setPosX(-left - e->width() - parenthesisPadding);
                }
            }
        } else if (e->isFingering()) {
            // don't set mag; fingerings should not scale with note
            Fingering* f = toFingering(e);
            if (f->propertyFlags(Pid::PLACEMENT) == PropertyFlags::STYLED) {
                f->setPlacement(f->calculatePlacement());
            }
            // layout fingerings that are placed relative to notehead
            // fingerings placed relative to chord will be laid out later
            if (f->layoutType() == ElementType::NOTE) {
                TLayout::layoutFingering(f, f->mutldata());
            }
        } else {
            e->mutldata()->setMag(item->mag());
            TLayout::layoutItem(e, ctx);
        }
    }
}

void ChordLayout::checkStartEndSlurs(Chord* chord, LayoutContext& ctx)
{
    chord->startEndSlurs().reset();
    for (Spanner* spanner : chord->startingSpanners()) {
        if (!spanner->isSlur()) {
            continue;
        }
        Slur* slur = toSlur(spanner);
        SlurTieLayout::computeUp(slur, ctx);
        if (slur->up()) {
            chord->startEndSlurs().startUp = true;
        } else {
            chord->startEndSlurs().startDown = true;
        }
        // Check if end chord has been connected to this slur. If not, connect it.
        if (!slur->endChord()) {
            continue;
        }
        slur->endChord()->addEndingSpanner(slur);
    }
    for (Spanner* spanner : chord->endingSpanners()) {
        if (!spanner->isSlur()) {
            continue;
        }
        if (toSlur(spanner)->up()) {
            chord->startEndSlurs().endUp = true;
        } else {
            chord->startEndSlurs().endDown = true;
        }
    }
}

void ChordLayout::checkAndFillShape(const ChordRest* item, ChordRest::LayoutData* ldata, const LayoutConfiguration& conf)
{
#ifdef MUE_ENABLE_ENGRAVING_LD_ACCESS
    Shape origin = ldata->shape(LD_ACCESS::PASS);
#endif

    fillShape(item, ldata, conf);

#ifdef MUE_ENABLE_ENGRAVING_LD_ACCESS
    Shape fixed = ldata->shape(LD_ACCESS::PASS);
    if (!origin.equal(fixed)) {
        LOGE() << "Shape not actual for item: " << item->typeName();
    }
#endif
}

void ChordLayout::fillShape(const ChordRest* item, Chord::LayoutData* ldata, const LayoutConfiguration& conf)
{
    switch (item->type()) {
    case ElementType::CHORD:
        fillShape(static_cast<const Chord*>(item), static_cast<Chord::LayoutData*>(ldata), conf);
        break;
    case ElementType::REST:
        fillShape(static_cast<const Rest*>(item), static_cast<Rest::LayoutData*>(ldata), conf);
        break;
    case ElementType::MEASURE_REPEAT:
        fillShape(static_cast<const MeasureRepeat*>(item), static_cast<MeasureRepeat::LayoutData*>(ldata), conf);
        break;
    case ElementType::MMREST:
        fillShape(static_cast<const MMRest*>(item), static_cast<MMRest::LayoutData*>(ldata), conf);
        break;
    default:
        DO_ASSERT(false);
        break;
    }
}

Shape ChordLayout::chordRestShape(const ChordRest* item, const LayoutConfiguration& conf)
{
    Shape shape;
    {
        double x1 = 1000000.0;
        double x2 = -1000000.0;
        for (Lyrics* l : item->lyrics()) {
            if (!l || !l->addToSkyline()) {
                continue;
            }
            double lmargin = conf.styleS(Sid::lyricsMinDistance).val() * item->spatium() * 0.5;
            double rmargin = lmargin;
            LyricsSyllabic syl = l->syllabic();
            if ((syl == LyricsSyllabic::BEGIN || syl == LyricsSyllabic::MIDDLE) && conf.styleB(Sid::lyricsDashForce)) {
                rmargin = std::max(rmargin, conf.styleMM(Sid::lyricsDashMinLength).val());
            }
            // for horizontal spacing we only need the lyrics width:
            x1 = std::min(x1, l->ldata()->bbox().x() - lmargin + l->pos().x());
            x2 = std::max(x2, l->ldata()->bbox().x() + l->ldata()->bbox().width() + rmargin + l->pos().x());
            if (l->ticks() == Fraction::fromTicks(Lyrics::TEMP_MELISMA_TICKS)) {
                x2 += item->spatium();
            }
            shape.addHorizontalSpacing(l, x1, x2);
        }
    }

    if (item->isMelismaEnd()) {
        double right = item->rightEdge();
        shape.addHorizontalSpacing(nullptr, right, right);
    }

    return shape;
}

void ChordLayout::fillShape(const Chord* item, ChordRest::LayoutData* ldata, const LayoutConfiguration& conf)
{
    Shape shape(Shape::Type::Composite);

    Hook* hook = item->hook();
    if (hook) {
        LD_CONDITION(hook->ldata()->isSetShape());
    }

    Stem* stem = item->stem();
    if (stem) {
        LD_CONDITION(stem->ldata()->isSetShape());
    }

    StemSlash* stemSlash = item->stemSlash();
    if (stemSlash) {
        LD_CONDITION(stemSlash->ldata()->isSetShape());
    }

    Arpeggio* arpeggio = item->arpeggio();
    if (arpeggio) {
        LD_CONDITION(arpeggio->ldata()->isSetShape());
    }

    BeamSegment* beamlet = item->beamlet();

    if (hook && hook->addToSkyline()) {
        shape.add(hook->shape().translate(hook->pos()));
    }

    if (stem && stem->addToSkyline()) {
        shape.add(stem->shape().translate(stem->pos()));
    }

    if (stemSlash && stemSlash->addToSkyline()) {
        shape.add(stemSlash->shape().translate(stemSlash->pos()));
    }

    if (arpeggio && arpeggio->addToSkyline()) {
        shape.add(arpeggio->shape().translate(arpeggio->pos()));
    }
//      if (_tremolo)
//            shape.add(_tremolo->shape().translated(_tremolo->pos()));
    for (Note* note : item->notes()) {
        shape.add(note->shape().translate(note->pos()));
    }

    for (EngravingItem* e : item->el()) {
        if (e->addToSkyline()) {
            shape.add(e->shape().translate(e->pos()));
        }
    }

    shape.add(chordRestShape(item, conf));      // add lyrics

    for (const LedgerLine* l = item->ledgerLines(); l; l = l->next()) {
        shape.add(l->shape().translate(l->pos()));
    }

    if (beamlet && stem) {
        double xPos = beamlet->line.p1().x() - stem->ldata()->pos().x();
        if (beamlet->isBefore && !item->up()) {
            xPos -= stem->width();
        } else if (!beamlet->isBefore && item->up()) {
            xPos += stem->width();
        }
        shape.add(beamlet->shape().translated(PointF(-xPos, 0.0)));
    }

    ldata->setShape(shape);
}

void ChordLayout::fillShape(const Rest* item, Rest::LayoutData* ldata, const LayoutConfiguration& conf)
{
    Shape shape(Shape::Type::Composite);

    if (!item->isGap()) {
        shape.add(chordRestShape(item, conf));
        shape.add(item->symBbox(ldata->sym), item);
        for (const NoteDot* dot : item->dotList()) {
            shape.add(item->symBbox(SymId::augmentationDot).translated(dot->pos()), dot);
        }
    }

    for (const EngravingItem* e : item->el()) {
        if (e->addToSkyline()) {
            shape.add(e->shape().translate(e->pos()));
        }
    }

    ldata->setShape(shape);
}

void ChordLayout::fillShape(const MeasureRepeat* item, MeasureRepeat::LayoutData* ldata, const LayoutConfiguration&)
{
    Shape shape(Shape::Type::Composite);

    shape.add(item->numberRect());
    shape.add(item->symBbox(ldata->symId));

    ldata->setShape(shape);
}

void ChordLayout::fillShape(const MMRest* item, MMRest::LayoutData* ldata, const LayoutConfiguration& conf)
{
    Shape shape(Shape::Type::Composite);

    double vStrokeHeight = conf.styleMM(Sid::mmRestHBarVStrokeHeight);
    shape.add(RectF(0.0, -(vStrokeHeight * .5), ldata->restWidth, vStrokeHeight));
    if (item->numberVisible()) {
        shape.add(item->numberRect());
    }

    ldata->setShape(shape);
}
