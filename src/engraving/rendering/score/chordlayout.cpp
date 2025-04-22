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

#include "chordlayout.h"
#include "accidentalslayout.h"
#include "horizontalspacing.h"
#include "beamtremololayout.h"

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
#include "dom/page.h"
#include "dom/part.h"
#include "dom/rest.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/staff.h"
#include "dom/stem.h"
#include "dom/stemslash.h"
#include "dom/system.h"
#include "dom/tie.h"
#include "dom/slur.h"

#include "dom/tremolosinglechord.h"
#include "dom/tremolotwochord.h"

#include "dom/undo.h"
#include "dom/utils.h"

#include "arpeggiolayout.h"
#include "tlayout.h"
#include "slurtielayout.h"
#include "beamlayout.h"
#include "tremololayout.h"
#include "autoplace.h"
#include "stemlayout.h"

using namespace muse;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

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
}

void ChordLayout::layoutPitched(Chord* item, LayoutContext& ctx)
{
    for (Chord* c : item->graceNotes()) {
        layoutPitched(c, ctx);
    }

    double mag_             = item->staff() ? item->staff()->staffMag(item) : 1.0;      // palette elements do not have a staff
    double dotNoteDistance  = ctx.conf().styleMM(Sid::dotNoteDistance) * mag_;

    double chordX           = (item->noteType() == NoteType::NORMAL) ? item->ldata()->pos().x() : 0.0;

    double lll    = 0.0;           // space to leave at left of chord
    double rrr    = 0.0;           // space to leave at right of chord
    double lhead  = 0.0;           // amount of notehead to left of chord origin
    Note* upnote = item->upNote();
    Note* downnote = item->downNote();
    Note* leftNote = nullptr;

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
        if (!leftNote || note->x() < leftNote->x()) {
            leftNote = note;
        }

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

    // A chord can have its own arpeggio and also be part of another arpeggio's span.  We need to lay out both of these arpeggios properly
    Arpeggio* oldSpanArp = item->spanArpeggio();
    Arpeggio* newSpanArp = nullptr;

    // If item has an arpeggio: mark chords which are part of the arpeggio
    if (item->arpeggio()) {
        item->arpeggio()->findAndAttachToChords();
        item->arpeggio()->mutldata()->maxChordPad = 0.0;
        item->arpeggio()->mutldata()->minChordX = DBL_MAX;
        TLayout::layoutArpeggio(item->arpeggio(), item->arpeggio()->mutldata(), ctx.conf());
    }

    if (item->spanArpeggio() != oldSpanArp) {
        newSpanArp = item->spanArpeggio();
    }
    // If item is within arpeggio span, keep track of largest space needed between glissando and chord across staves
    double lllMax = lll;
    for (Arpeggio* spanArp : { oldSpanArp, newSpanArp }) {
        if (!spanArp || !spanArp->chord()) {
            continue;
        }
        Arpeggio::LayoutData* arpldata = spanArp->mutldata();
        const Segment* seg = spanArp->chord()->segment();
        const EngravingItem* endItem = seg->elementAt(spanArp->endTrack());
        const Chord* endChord = item;
        if (endItem && endItem->isChord()) {
            endChord = toChord(endItem);
        }

        // If a note is covered in the voice span but located outside the visual span of the arpeggio calculate accidental offset later
        bool aboveStart
            = std::make_pair(item->vStaffIdx(), item->downLine()) < std::make_pair(spanArp->vStaffIdx(), spanArp->chord()->upLine());
        bool belowEnd = std::make_pair(item->vStaffIdx(), item->upLine()) > std::make_pair(endChord->vStaffIdx(), endChord->downLine());

        if (!(aboveStart || belowEnd)) {
            const PaddingTable& paddingTable = item->score()->paddingTable();
            double arpeggioNoteDistance = paddingTable.at(ElementType::ARPEGGIO).at(ElementType::NOTE) * mag_;
            double arpeggioLedgerDistance = paddingTable.at(ElementType::ARPEGGIO).at(ElementType::LEDGER_LINE) * mag_;
            int firstLedgerBelow = item->staff()->lines(item->downNote()->tick()) * 2 - 1;
            int firstLedgerAbove = -1;

            double gapSize = arpeggioNoteDistance;

            if (leftNote && muse::RealIsNull(leftNote->x())) {
                if (downnote->line() > firstLedgerBelow || upnote->line() < firstLedgerAbove) {
                    gapSize = arpeggioLedgerDistance + ctx.conf().styleS(Sid::ledgerLineLength).val() * item->spatium();
                }
            } else if (leftNote && (leftNote->line() > firstLedgerBelow || leftNote->line() < firstLedgerAbove)) {
                gapSize = arpeggioLedgerDistance + ctx.conf().styleS(Sid::ledgerLineLength).val() * item->spatium();
            }

            double arpChordX = std::min(chordX, 0.0);

            if (!chordAccidentals.empty()) {
                double arpeggioAccidentalDistance = paddingTable.at(ElementType::ARPEGGIO).at(ElementType::ACCIDENTAL) * mag_;
                double accidentalDistance = ctx.conf().styleMM(Sid::accidentalDistance) * mag_;
                gapSize = arpeggioAccidentalDistance - accidentalDistance;
                gapSize -= ArpeggioLayout::insetDistance(spanArp, ctx, mag_, item, chordAccidentals);
            }

            double extraX = spanArp->width() + gapSize;

            // Track leftmost chord position, as we we always want the arpeggio to be to the left of this
            arpldata->minChordX = std::min(arpldata->minChordX, arpChordX);

            // Save this to arpeggio if largest
            arpldata->maxChordPad = std::max(arpldata->maxChordPad, lll + extraX);

            // If first chord in arpeggio set y
            if (item->arpeggio() && item->arpeggio() == spanArp) {
                double y1 = upnote->pos().y() - upnote->headHeight() * .5;
                item->arpeggio()->mutldata()->setPosY(y1);
            }

            Note* endDownNote = endChord->downNote();

            // If last chord in arpeggio, set x
            if (endDownNote->track() == item->track()) {
                // Amount to move arpeggio from it's parent chord to factor in chords further to the left
                double firstChordX = spanArp->chord()->ldata()->pos().x();
                double xDiff = firstChordX - arpldata->minChordX;

                double offset = -(xDiff + arpldata->maxChordPad);
                spanArp->mutldata()->setPosX(offset);
                if (spanArp->visible()) {
                    lllMax = std::max(lllMax, offset);
                }
            }
        }
    }

    lll = lllMax;

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
    double xNote = DBL_MAX;
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

    layoutLvArticulation(item, ctx);

    fillShape(item, item->mutldata(), ctx.conf());
}

void ChordLayout::layoutTablature(Chord* item, LayoutContext& ctx)
{
    double _spatium          = item->spatium();
    double mag_ = item->staff() ? item->staff()->staffMag(item) : 1.0;    // palette elements do not have a staff
    double dotNoteDistance = ctx.conf().styleMM(Sid::dotNoteDistance) * mag_;
    double minNoteDistance = ctx.conf().styleMM(Sid::minNoteDistance) * mag_;
    double minTieLength = ctx.conf().styleMM(Sid::minTieLength) * mag_;

    for (Chord* c : item->graceNotes()) {
        layoutTablature(c, ctx);
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
        note->setDotRelativeLine(0);
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
        if (tie && tie->addToSkyline()) {
            SlurTieLayout::calculateDirection(tie);
            double overlap = 0.0;                // how much tie can overlap start and end notes
            bool shortStart = false;            // whether tie should clear start note or not
            Note* startNote = tie->startNote();
            Chord* startChord = startNote ? startNote->chord() : nullptr;
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
                    if (!RealIsNull(note->ldata()->pos().x())) {                      // this probably does not work for TAB, as
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
        item->resizeLedgerLinesTo(ledgerLines);
        double extraLen    = 0;
        double llX         = stemX - (headWidth + extraLen) * 0.5;
        for (int i = 0; i < ledgerLines; i++) {
            LedgerLine* ldgLin = item->ledgerLines()[i];
            ldgLin->setParent(item);
            ldgLin->setTrack(item->track());
            ldgLin->setVisible(item->visible());
            ldgLin->setLen(headWidth + extraLen);
            ldgLin->setPos(llX, llY);
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
            ctx.mutDom().addElement(stem);
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
            ctx.mutDom().doUndoRemoveElement(item->stem());
            item->remove(item->stem());
        }
        if (item->hook()) {
            ctx.mutDom().doUndoRemoveElement(item->hook());
            item->remove(item->hook());
        }
        if (item->beam()) {
            ctx.mutDom().doUndoRemoveElement(item->beam());
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
        item->arpeggio()->findAndAttachToChords();
        double y = upnote->pos().y() - upnote->headHeight() * .5;
        TLayout::layoutArpeggio(item->arpeggio(), item->arpeggio()->mutldata(), ctx.conf());
        lll += item->arpeggio()->width() + _spatium * .5;
        item->arpeggio()->setPos(-lll, y);

        // handle the special case of _arpeggio->span() > 1
        // in layoutArpeggio2() after page layout has done so we
        // know the y position of the next staves
    }

    // allocate enough room for glissandi
    if (item->endsNoteAnchoredLine()) {
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

    layoutLvArticulation(item, ctx);

    fillShape(item, item->mutldata(), ctx.conf());
}

void ChordLayout::layoutLvArticulation(Chord* item, LayoutContext& ctx)
{
    // Laissez vib ties must be layed out with the rest of the note, as they need to be taken into account for horizontal spacing
    for (Articulation* a : item->articulations()) {
        if (a->isLaissezVib()) {
            // Must set direction before laying out
            if (item->measure()->hasVoices(a->staffIdx(), item->tick(), item->actualTicks()) && a->anchor() == ArticulationAnchor::AUTO) {
                a->setUp(item->up());         // if there are voices place articulation at stem
            } else if (a->anchor() == ArticulationAnchor::AUTO) {
                a->setUp(!item->up());         // place articulation at note head
            } else {
                a->setUp(a->anchor() == ArticulationAnchor::TOP);
            }

            TLayout::layoutItem(a, ctx);

            const Articulation::LayoutData* ldata = a->ldata();
            const double sp = a->spatium();
            const Note* note = ldata->up() ? item->upNote() : item->downNote();
            const int upDir = ldata->up() ? -1 : 1;
            const double noteHeight = note->height();

            PointF result = note->pos() + item->pos();
            double center = SlurTieLayout::noteOpticalCenterForTie(note, ldata->up);
            double visualInsetSp = 0.0;
            if (note->headGroup() == NoteHeadGroup::HEAD_SLASH || note->shouldHideFret()) {
                visualInsetSp = 0.2;
            } else if (item->up() && ldata->up) {
                visualInsetSp = 0.7;
            } else {
                visualInsetSp = 0.1;
            }
            double visualInset = visualInsetSp * sp;

            result.rx() += center + visualInset - ldata->bbox().left();
            result.ry() += (noteHeight / 2 + 0.2 * sp) * upDir;

            a->setPos(result);
        }
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
            TLayout::layoutSpanner(sp, ctx);
        }
        for (Spanner* sp : n->spannerFor()) {
            if ((sp->isGlissando() || sp->isGuitarBend()) && sp->tick2() >= ctx.state().page()->endTick()) {
                TLayout::layoutSpanner(sp, ctx);
            }
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
    const double minDist = ctx.conf().styleMM(Sid::articulationMinDistance) * mag;
    const ArticulationStemSideAlign articulationHAlign = ctx.conf().styleV(Sid::articulationStemHAlign).value<ArticulationStemSideAlign>();
    const bool keepArticsTogether = ctx.conf().styleB(Sid::articulationKeepTogether);
    const double stemSideDistance = ctx.conf().styleMM(Sid::propertyDistanceStem) * mag;
    const double headSideDistance = ctx.conf().styleMM(Sid::propertyDistanceHead) * mag;
    const double tenutoAdditionalTieDistance = 0.6 * _spatium;
    const double staccatoAdditionalTieDistance = 0.4 * _spatium;

    int numCloseArtics = 0;
    bool hasStaffArticsUp = false;
    bool hasStaffArticsDown = false;
    if (keepArticsTogether) {
        // find out how many close-to-note artics there are, and whether there is a staff-anchored artic to align to
        for (Articulation* a : item->articulations()) {
            if (!a->visible()) {
                continue;
            }
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

    Articulation* prevVisibleArticulation = nullptr;
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
        bool leaveTieSpace = leaveSpaceForTie(a);

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
                bool hasBeam = item->beam() || (item->tremoloTwoChord());
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
                    line = std::max(line, lines - (1 + (numCloseArtics * 2)));
                }
                if (line < lines - 1) {
                    if (leaveTieSpace && line % 2) {
                        line += 1;
                    }
                    y = ((line & ~1) + 3) * _lineDist;
                    y -= a->height() * .5;
                } else {
                    y = item->downPos() + 0.5 * item->downNote()->headHeight() + headSideDistance;
                    if (leaveTieSpace) {
                        y += a->isStaccato() ? staccatoAdditionalTieDistance : tenutoAdditionalTieDistance;
                    }
                }
            }
            if (prevVisibleArticulation && (prevVisibleArticulation->up() == a->up())) {
                int staffBottom = (staffType->lines() - 2) * 2;
                if ((headSide && item->downLine() < staffBottom)
                    || (!headSide && !muse::RealIsEqualOrMore(y, (staffBottom + 1) * _lineDist))) {
                    y += _spatium;
                } else {
                    y += prevVisibleArticulation->height() + minDist;
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
                bool hasBeam = item->beam() || item->tremoloTwoChord();
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
                    line = std::min(line, 1 + (numCloseArtics * 2));
                }
                if (line > 1) {
                    if (leaveTieSpace && line % 2) {
                        line -= 1;
                    }
                    y = (((line + 1) & ~1) - 3) * _lineDist;
                    y += a->height() * .5;
                } else {
                    y = item->upPos() - 0.5 * item->downNote()->headHeight() - headSideDistance;
                    if (leaveTieSpace) {
                        y -= a->isStaccato() ? staccatoAdditionalTieDistance : tenutoAdditionalTieDistance;
                    }
                }
            }
            if (prevVisibleArticulation && (prevVisibleArticulation->up() == a->up())) {
                if ((headSide && item->upLine() > 2) || (!headSide && !muse::RealIsEqualOrLess(y, 0.0))) {
                    y -= item->spatium();
                } else {
                    y -= prevVisibleArticulation->height() + minDist;
                }
            }
        }
        a->setPos(x, y);
        if (a->visible()) {
            prevVisibleArticulation = a;
        }
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
    double mag = item->mag();
    double minDist = ctx.conf().styleMM(Sid::articulationMinDistance) * mag;
    double staffDist = ctx.conf().styleMM(Sid::propertyDistance) * mag;
    double stemDist = ctx.conf().styleMM(Sid::propertyDistanceStem) * mag;
    double noteDist = ctx.conf().styleMM(Sid::propertyDistanceHead) * mag;
    double yOffset = item->staffOffsetY();

    double chordTopY = item->upPos() - 0.5 * item->upNote()->headHeight() + yOffset;       // note position of highest note
    double chordBotY = item->downPos() + 0.5 * item->upNote()->headHeight() + yOffset;     // note position of lowest note

    double staffTopY = -staffDist + yOffset;
    double staffBotY = item->staff()->staffHeight(item->tick()) + staffDist + yOffset;

    // avoid collisions of staff articulations with chord notes:
    // gap between note and staff articulation is distance0 + 0.5 spatium

    if (item->stem()) {
        // Check if there's a hook, because the tip of the hook always extends slightly past the end of the stem
        if (item->up()) {
            double tip = item->hook()
                         ? item->hook()->ldata()->bbox().translated(item->hook()->pos()).top()
                         : item->stem()->ldata()->bbox().translated(item->stem()->pos()).top();

            chordTopY = tip + yOffset;
        } else {
            double tip = item->hook()
                         ? item->hook()->ldata()->bbox().translated(item->hook()->pos()).bottom()
                         : item->stem()->ldata()->bbox().translated(item->stem()->pos()).bottom();

            chordBotY = tip + yOffset;
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
                chordTopY = a->y() - a->height() - minDist + yOffset;
            } else {
                chordBotY = a->y() + a->height() + minDist + yOffset;
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
                chordTopY = a->y() - a->height() - minDist + yOffset;
            }
        } else {
            if (a->visible()) {
                chordBotY = a->y() + a->height() + minDist + yOffset;
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
        if ((layoutOnCrossBeamSide && !a->isOnCrossBeamSide()) || a->isLaissezVib()) {
            continue;
        }
        if (a->isStaccato()) {
            stacc = a;
        } else if (stacc && a->isAccent() && stacc->up() == a->up()
                   && (muse::RealIsEqualOrLess(stacc->ldata()->pos().y(), 0.0)
                       || muse::RealIsEqualOrMore(stacc->ldata()->pos().y(), item->staff()->staffHeight(item->tick())))) {
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
                a->setPos(!item->up() || !a->isBasicArticulation() ? headSideX : stemSideX, staffTopY + kearnHeight - yOffset);
                if (a->visible()) {
                    staffTopY = a->y() - a->height() - minDist + yOffset;
                }
            } else {
                a->setPos(item->up() || !a->isBasicArticulation() ? headSideX : stemSideX, staffBotY - kearnHeight - yOffset);
                if (a->visible()) {
                    staffBotY = a->y() + a->height() + minDist + yOffset;
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
            Shape sh = a->shape().translate(a->pos() + item->pos() + item->staffOffset());
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
        Shape aShape = a->shape().translate(a->pos() + item->pos() + s->pos() + m->pos() + item->staffOffset());
        Shape sShape = ss->shape().translate(ss->pos());
        double minDist = ctx.conf().styleMM(Sid::articulationMinDistance);
        double vertClearance = a->up() ? aShape.verticalClearance(sShape) : sShape.verticalClearance(aShape);
        if (vertClearance < minDist) {
            minDist += slur->up()
                       ? std::max(aShape.minVerticalDistance(sShape), 0.0)
                       : std::max(sShape.minVerticalDistance(aShape), 0.0);
            minDist *= slur->up() ? -1 : 1;
            for (auto iter2 = iter; iter2 != item->articulations().end(); ++iter2) {
                Articulation* aa = *iter2;
                aa->mutldata()->moveY(minDist);
                if (sstaff && aa->addToSkyline()) {
                    sstaff->skyline().add(aa->shape().translate(aa->pos() + item->pos() + s->pos() + m->pos() + item->staffOffset()));
                    for (ShapeElement& sh : s->staffShape(item->staffIdx()).elements()) {
                        if (sh.item() == aa) {
                            sh.translate(0.0, minDist);
                        }
                    }
                }
            }
        }
    }
}

//! May be called again when the chord is added to or removed from a beam.
void ChordLayout::layoutStem(Chord* item, const LayoutContext& ctx)
{
    TRACEFUNC;

    LAYOUT_CALL() << "chord: " << item->eid();

    // Stem needs to know hook's bbox and SMuFL anchors.
    // This is done before calcDefaultStemLength because the presence or absence of a hook affects stem length
    if (item->hook()) {
        item->hook()->setHookType(item->up() ? item->durationType().hooks() : -item->durationType().hooks());
        TLayout::layoutHook(item->hook(), item->hook()->mutldata());
    }

    // we should calculate default stem length for this chord even if it doesn't have a stem
    // because this length is used for tremolos or other things that attach to where the stem WOULD be
    item->setDefaultStemLength(StemLayout::calcDefaultStemLength(item, ctx));

    if (!item->stem()) {
        return;
    }

    item->stem()->mutldata()->setPosX(item->stemPosX());

    item->stem()->setBaseLength(Millimetre(item->defaultStemLength()));
    TLayout::layoutStem(item->stem(), item->stem()->mutldata(), ctx.conf());

    // And now we need to set the position of the flag.
    if (item->hook()) {
        item->hook()->setPos(item->stem()->flagPosition());
    }

    // Add Stem slash
    if ((item->showStemSlash()) && !(item->beam() && item->beam()->elements().front() != item)) {
        if (!item->stemSlash()) {
            item->add(Factory::createStemSlash(item));
        }
        TLayout::layoutStemSlash(item->stemSlash(), item->stemSlash()->mutldata(), ctx.conf());
    } else if (item->stemSlash()) {
        item->remove(item->stemSlash());
    }
}

void ChordLayout::computeUpBeamCase(Chord* item, Beam* beam)
{
    if (beam->userModified()) {
        item->setUp(isChordPosBelowBeam(item, beam));
    } else if (beam->cross()) {
        item->setUp(item->isBelowCrossBeam(beam));
    } else {
        item->setUp(beam->up());
    }
}

bool ChordLayout::isChordPosBelowBeam(Chord* item, Beam* beam)
{
    assert(!beam->beamFragments().empty());

    Note* baseNote = item->up() ? item->downNote() : item->upNote();
    double noteY = baseNote->pagePos().y();

    PointF startAnchor = beam->startAnchor();
    PointF endAnchor = beam->endAnchor();

    if (startAnchor.isNull() || endAnchor.isNull()) {
        return beam->cross() ? item->isBelowCrossBeam(beam) : beam->up();
    }

    if (item == beam->elements().front()) {
        return noteY > startAnchor.y();
    }

    if (item == beam->elements().back()) {
        return noteY > endAnchor.y();
    }

    PointF noteAnchor = BeamLayout::chordBeamAnchor(beam, item, ChordBeamAnchorType::Middle);
    double noteX = noteAnchor.x();

    double proportionAlongX
        = muse::RealIsEqual(startAnchor.x(), endAnchor.x()) ? 0.0 : (noteX - startAnchor.x()) / (endAnchor.x() - startAnchor.x());
    double desiredY = proportionAlongX * (endAnchor.y() - startAnchor.y()) + startAnchor.y();
    return noteY > desiredY;
}

bool ChordLayout::isChordPosBelowTrem(const Chord* item, TremoloTwoChord* trem)
{
    if (!item || !trem || !trem->ldata()->isValid()) {
        return true;
    }
    Note* baseNote = item->up() ? item->downNote() : item->upNote();
    double noteY = baseNote->pagePos().y();
    double tremY = trem->chordBeamAnchor(item, ChordBeamAnchorType::Middle).y();

    return noteY > tremY;
}

static bool computeUp_TremoloTwoNotesCase(const Chord* item, TremoloTwoChord* tremolo, const LayoutContext& ctx)
{
    const Chord* c1 = tremolo->chord1();
    const Chord* c2 = tremolo->chord2();
    if (!c1 || !c2) {
        return true;
    }
    bool cross = c1->staffMove() != c2->staffMove();
    if (item == c1) {
        // we have to lay out the tremolo because it hasn't been laid out at all yet, and we need its direction
        TremoloLayout::layout(tremolo, ctx);
    }

    const Measure* measure = item->findMeasure();
    if (!cross && !tremolo->userModified()) {
        return tremolo->up();
    }

    if (!measure->explicitParent()) {
        // this method will be called later (from Measure::layoutCrossStaff) after the
        // system is completely laid out.
        // this is necessary because otherwise there's no way to deal with cross-staff beams
        // because we don't know how far apart the staves actually are
        return item->ldata()->up;
    }

    if (tremolo->userModified()) {
        return ChordLayout::isChordPosBelowTrem(item, tremolo);
    } else if (cross) {
        // unmodified cross-staff trem, should be one note per staff
        if (item->staffMove() != 0) {
            return item->staffMove() > 0;
        } else {
            int otherStaffMove = item->staffMove() == c1->staffMove() ? c2->staffMove() : c1->staffMove();
            return otherStaffMove < 0;
        }
    }

    if (!cross && !tremolo->userModified()) {
        return tremolo->up();
    }

    return item->ldata()->up;
}

void ChordLayout::computeUp(const Chord* item, Chord::LayoutData* ldata, const LayoutContext& ctx)
{
    LAYOUT_CALL() << LAYOUT_ITEM_INFO(item);

    assert(!item->notes().empty());

    // Beams with one chord will be removed later, ignore
    const bool hasBeam = item->beam() && item->beam()->elements().size() > 1;

    const StaffType* tab = item->staff() ? item->staff()->staffTypeForElement(item) : 0;
    bool isTabStaff = tab && tab->isTabStaff();
    if (isTabStaff) {
        if (tab->stemless()) {
            ldata->up = false;
            return;
        }
        if (!tab->stemThrough()) {
            bool staffHasMultipleVoices = item->measure()->hasVoices(item->staffIdx(), item->tick(), item->actualTicks());
            if (staffHasMultipleVoices) {
                bool isTrackEven = item->track() % 2 == 0;
                ldata->up = isTrackEven;
                return;
            }
            ldata->up = !tab->stemsDown();
            return;
        }
    }

    if (item->stemDirection() != DirectionV::AUTO
        && !hasBeam
        && !item->tremoloTwoChord()) {
        ldata->up = item->stemDirection() == DirectionV::UP;
        return;
    }

    if (item->isUiItem()) {
        ldata->up = true;
        return;
    }

    if (hasBeam) {
        computeUpBeamCase(const_cast<Chord*>(item), item->beam());
        return;
    } else if (item->tremoloTwoChord()) {
        ldata->up = computeUp_TremoloTwoNotesCase(item, item->tremoloTwoChord(), ctx);
        return;
    }

    bool staffHasMultipleVoices = item->measure()->hasVoices(item->staffIdx(), item->tick(), item->actualTicks());
    if (staffHasMultipleVoices) {
        bool isTrackEven = item->track() % 2 == 0;
        ldata->up = isTrackEven;
        return;
    }

    bool isGraceNote = item->noteType() != NoteType::NORMAL;
    if (isGraceNote) {
        ldata->up = true;
        return;
    }

    bool chordIsCrossStaff = item->staffMove() != 0;
    if (chordIsCrossStaff) {
        ldata->up = item->staffMove() > 0;
        return;
    }

    std::vector<int> distances = item->noteDistances();
    int direction = ChordLayout::computeAutoStemDirection(distances);
    ldata->up = direction > 0;
}

void ChordLayout::computeUp(ChordRest* item, const LayoutContext& ctx)
{
    if (item->isChord()) {
        Chord* ch = item_cast<Chord*>(item);
        computeUp(ch, ch->mutldata(), ctx);
    } else {
        // base ChordRest
        // Beams with one chord will be removed later, ignore
        const bool hasBeam = item->beam() && item->beam()->elements().size() > 1;
        if (hasBeam) {
            item->mutldata()->up = item->beam()->up();
        } else {
            item->mutldata()->up = true;
        }
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

ChordPosInfo ChordLayout::calculateChordPosInfo(Segment* segment, staff_idx_t staffIdx, track_idx_t partStartTrack,
                                                track_idx_t partEndTrack, LayoutContext& ctx)
{
    const Staff* staff = ctx.dom().staff(staffIdx);

    ChordPosInfo posInfo = ChordPosInfo();

    for (track_idx_t track = partStartTrack; track < partEndTrack; ++track) {
        EngravingItem* e = segment->element(track);
        const bool calcChordPos = e && e->isChord() && toChord(e)->vStaffIdx() == staffIdx;
        if (!calcChordPos) {
            continue;
        }

        Chord* chord = toChord(e);
        posInfo.chords.push_back(chord);
        bool hasGraceBefore = false;
        for (Chord* c : chord->graceNotes()) {
            if (c->isGraceBefore()) {
                hasGraceBefore = true;
            }
            layoutChords2(c->notes(), c->up(), ctx);     // layout grace note noteheads
            layoutChords3({ c }, c->notes(), staff, ctx);     // layout grace note chords
        }
        if (chord->up()) {
            ++posInfo.upVoices;
            posInfo.upStemNotes.insert(posInfo.upStemNotes.end(), chord->notes().begin(), chord->notes().end());
            posInfo.upDots   = std::max(posInfo.upDots, chord->dots());
            posInfo.maxUpMag = std::max(posInfo.maxUpMag, chord->mag());
            if (!posInfo.upHooks && !chord->beam()) {
                posInfo.upHooks = chord->hook();
            }
            if (hasGraceBefore) {
                posInfo.upGrace = true;
            }
        } else {
            ++posInfo.downVoices;
            posInfo.downStemNotes.insert(posInfo.downStemNotes.end(), chord->notes().begin(), chord->notes().end());
            posInfo.downDots = std::max(posInfo.downDots, chord->dots());
            posInfo.maxDownMag = std::max(posInfo.maxDownMag, chord->mag());
            if (!posInfo.downHooks && !chord->beam()) {
                posInfo.downHooks = chord->hook();
            }
            if (hasGraceBefore) {
                posInfo.downGrace = true;
            }
        }
    }

    return posInfo;
}

void ChordLayout::calculateMaxNoteWidths(ChordPosInfo& posInfo, const Fraction& tick, const Staff* staff, LayoutContext& ctx)
{
    double nominalWidth  = ctx.conf().noteHeadWidth() * staff->staffMag(tick);
    posInfo.maxUpWidth   = nominalWidth * posInfo.maxUpMag;
    posInfo.maxDownWidth = nominalWidth * posInfo.maxDownMag;

    // layout upstem noteheads
    if (posInfo.upVoices > 1) {
        std::sort(posInfo.upStemNotes.begin(), posInfo.upStemNotes.end(),
                  [](Note* n1, const Note* n2) ->bool { return n1->stringOrLine() > n2->stringOrLine(); });
    }
    if (posInfo.upVoices) {
        double hw = layoutChords2(posInfo.upStemNotes, true, ctx);
        posInfo.maxUpWidth = std::max(posInfo.maxUpWidth, hw);
    }

    // layout downstem noteheads
    if (posInfo.downVoices > 1) {
        std::sort(posInfo.downStemNotes.begin(), posInfo.downStemNotes.end(),
                  [](Note* n1, const Note* n2) ->bool { return n1->stringOrLine() > n2->stringOrLine(); });
    }
    if (posInfo.downVoices) {
        double hw = layoutChords2(posInfo.downStemNotes, false, ctx);
        posInfo.maxDownWidth = std::max(posInfo.maxDownWidth, hw);
    }
}

void ChordLayout::offsetAndLayoutChords(Segment* segment, staff_idx_t staffIdx, track_idx_t partStartTrack, track_idx_t partEndTrack,
                                        OffsetInfo& offsetInfo, const ChordPosInfo& posInfo, LayoutContext& ctx)
{
    const Staff* staff = ctx.dom().staff(staffIdx);
    const bool isTab = staff->isTabStaff(segment->tick());

    // apply chord offsets
    for (track_idx_t track = partStartTrack; track < partEndTrack; ++track) {
        EngravingItem* e = segment->element(track);
        const bool layoutChord = e && e->isChord() && toChord(e)->vStaffIdx() == staffIdx;
        if (!layoutChord) {
            continue;
        }
        Chord* chord = toChord(e);
        Chord::LayoutData* chordLdata = chord->mutldata();
        // only centre chords if we are separating voices and this voice has no collision
        const bool combineVoices = chord->shouldCombineVoice();
        if (!combineVoices && !muse::contains(offsetInfo.tracksToAdjust, track)) {
            if (chord->up()) {
                chordLdata->moveX(offsetInfo.centerUp);
            } else {
                chordLdata->moveX(offsetInfo.centerDown);
            }
            continue;
        }

        if (chord->up()) {
            if (!muse::RealIsNull(offsetInfo.upOffset)) {
                offsetInfo.oversizeUp = isTab ? offsetInfo.oversizeUp / 2 : offsetInfo.oversizeUp;
                chordLdata->moveX(offsetInfo.upOffset + offsetInfo.centerAdjustUp + offsetInfo.oversizeUp);
                if (posInfo.downDots && !posInfo.upDots) {
                    chordLdata->moveX(offsetInfo.dotAdjust);
                }
            } else {
                chordLdata->moveX(offsetInfo.centerUp);
            }
        } else {
            if (!muse::RealIsNull(offsetInfo.downOffset)) {
                chordLdata->moveX(offsetInfo.downOffset + offsetInfo.centerAdjustDown);
                if (posInfo.upDots && !posInfo.downDots) {
                    chordLdata->moveX(offsetInfo.dotAdjust);
                }
            } else {
                chordLdata->moveX(offsetInfo.centerDown);
            }
        }
    }

    // layout chords
    std::vector<Note*> notes;
    if (posInfo.upVoices) {
        notes.insert(notes.end(), posInfo.upStemNotes.begin(), posInfo.upStemNotes.end());
    }
    if (posInfo.downVoices) {
        notes.insert(notes.end(), posInfo.downStemNotes.begin(), posInfo.downStemNotes.end());
    }
    if (posInfo.upVoices + posInfo.downVoices > 1) {
        std::sort(notes.begin(), notes.end(),
                  [](Note* n1, const Note* n2) ->bool { return n1->stringOrLine() > n2->stringOrLine(); });
    }
    layoutChords3(posInfo.chords, notes, staff, ctx);
}

OffsetInfo ChordLayout::centreChords(const Segment* segment, ChordPosInfo& posInfo, staff_idx_t staffIdx,
                                     const Fraction& tick, LayoutContext& ctx)
{
    const Staff* staff = ctx.dom().staff(staffIdx);
    const bool isTab = staff->isTabStaff(segment->tick());

    OffsetInfo offsetInfo = OffsetInfo();
    // only center chords on standard staves.  This is a simpler process for TAB and done elsewhere
    if (isTab) {
        return offsetInfo;
    }

    double sp = staff->spatium(tick);
    double nominalWidth = ctx.conf().noteHeadWidth() * staff->staffMag(tick);

    // only center chords if they differ from nominal by at least this amount
    // this avoids unnecessary centering on differences due only to floating point roundoff
    // it also allows for the possibility of disabling centering
    // for notes only "slightly" larger than nominal, like half notes
    // but this will result in them not being aligned with each other between voices
    // unless you change to left alignment as described in the comments below
    double centerThreshold = 0.01 * sp;

    // amount by which actual width exceeds nominal, adjusted for staff mag() only
    double headDiff = posInfo.maxUpWidth - nominalWidth;
    // amount by which actual width exceeds nominal, adjusted for staff & chord/note mag()
    double headDiff2 = posInfo.maxUpWidth - nominalWidth * (posInfo.maxUpMag / staff->staffMag(tick));
    if (headDiff > centerThreshold && !isTab) {
        // larger than nominal
        offsetInfo.centerUp = headDiff * -0.5;
        // maxUpWidth is true width, but we no longer will care about that
        // instead, we care only about portion to right of origin
        posInfo.maxUpWidth += offsetInfo.centerUp;
        // to left align rather than center, delete both of the above
        if (headDiff2 > centerThreshold) {
            // if max notehead is wider than nominal with chord/note mag() applied
            // then noteheads extend to left of origin
            // because stemPosX() is based on nominal width
            // so we need to correct for that too
            offsetInfo.centerUp += headDiff2;
            offsetInfo.oversizeUp = headDiff2;
        }
    } else if (-headDiff > centerThreshold) {
        // smaller than nominal
        offsetInfo.centerUp = -headDiff * 0.5;
        if (headDiff2 > centerThreshold) {
            // max notehead is wider than nominal with chord/note mag() applied
            // perform same adjustment as above
            offsetInfo.centerUp += headDiff2;
            offsetInfo.oversizeUp = headDiff2;
        }
        offsetInfo.centerAdjustDown = offsetInfo.centerUp;
    }

    headDiff = posInfo.maxDownWidth - nominalWidth;
    if (headDiff > centerThreshold) {
        // larger than nominal
        offsetInfo.centerDown = headDiff * -0.5;
        // to left align rather than center, change the above to
        //centerAdjustUp = headDiff;
        posInfo.maxDownWidth = nominalWidth - offsetInfo.centerDown;
    } else if (-headDiff > centerThreshold && !isTab) {
        // smaller than nominal
        offsetInfo.centerDown = -headDiff * 0.5;
        offsetInfo.centerAdjustUp = offsetInfo.centerDown;
    }

    return offsetInfo;
}

void ChordLayout::calculateChordOffsets(Segment* segment, staff_idx_t staffIdx, const Fraction& tick,
                                        OffsetInfo& offsetInfo, ChordPosInfo& posInfo, LayoutContext& ctx)
{
    // handle conflict between upstem and downstem chords
    if (!posInfo.upVoices || !posInfo.downVoices) {
        return;
    }
    const Staff* staff = ctx.dom().staff(staffIdx);
    double sp = staff->spatium(tick);
    const bool isTab = staff->isTabStaff(segment->tick());

    Note* bottomUpNote = posInfo.upStemNotes.front();
    Note* topDownNote  = posInfo.downStemNotes.back();
    int separation = topDownNote->stringOrLine() - bottomUpNote->stringOrLine();

    std::vector<Note*> overlapNotes;
    overlapNotes.reserve(8);

    if (separation == 1) {
        // second
        if (posInfo.upDots && !posInfo.downDots) {
            offsetInfo.upOffset = posInfo.maxDownWidth + 0.1 * sp;
            offsetInfo.tracksToAdjust.insert(bottomUpNote->track());
        } else {
            offsetInfo.downOffset = posInfo.maxUpWidth;
            // align stems if present
            if (topDownNote->chord()->stem() && bottomUpNote->chord()->stem()) {
                const Stem* topDownStem = topDownNote->chord()->stem();
                offsetInfo.downOffset -= topDownStem->lineWidthMag();
            } else if (topDownNote->chord()->durationType().headType() != NoteHeadType::HEAD_BREVIS
                       && bottomUpNote->chord()->durationType().headType() != NoteHeadType::HEAD_BREVIS) {
                // stemless notes should be aligned as is they were stemmed
                // (except in case of brevis, cause the notehead has the side bars)
                offsetInfo.downOffset -= ctx.conf().styleMM(Sid::stemWidth) * topDownNote->chord()->mag();
            }
            offsetInfo.tracksToAdjust.insert(topDownNote->track());
        }
    } else if (separation < 1) {
        // overlap (possibly unison)

        // build list of overlapping notes
        for (size_t i = 0, n = posInfo.upStemNotes.size(); i < n; ++i) {
            if (posInfo.upStemNotes[i]->stringOrLine() >= topDownNote->stringOrLine() - 1) {
                overlapNotes.push_back(posInfo.upStemNotes[i]);
            } else {
                break;
            }
        }
        for (size_t i = posInfo.downStemNotes.size(); i > 0; --i) {             // loop most probably needs to be in this reverse order
            if (posInfo.downStemNotes[i - 1]->stringOrLine() <= bottomUpNote->stringOrLine() + 1) {
                overlapNotes.push_back(posInfo.downStemNotes[i - 1]);
            } else {
                break;
            }
        }
        std::sort(overlapNotes.begin(), overlapNotes.end(),
                  [](Note* n1, const Note* n2) ->bool { return n1->stringOrLine() > n2->stringOrLine(); });

        // determine nature of overlap
        bool shareHeads = true;                   // can all overlapping notes share heads?
        bool matchPending = false;                // looking for a unison match
        bool conflictUnison = false;              // unison found
        bool conflictSecondUpHigher = false;                  // second found
        bool conflictSecondDownHigher = false;                // second found
        int lastLine = 1000;
        track_idx_t lastTrack = muse::nidx;
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
            track_idx_t track = n->track();
            int line = n->stringOrLine();
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
                // and chords must be same size if notehead is anything other than HEAD_QUARTER
                if (n->headGroup() != p->headGroup() || n->tpc() != p->tpc() || n->ldata()->mirror()
                    || p->ldata()->mirror()
                    || (nchord->isSmall() != pchord->isSmall()
                        && (nHeadType != NoteHeadType::HEAD_QUARTER || pHeadType != NoteHeadType::HEAD_QUARTER))) {
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
            if (!shareHeads) {
                offsetInfo.tracksToAdjust.insert({ track, lastTrack });
            }
            p = n;
            lastLine = line;
            lastTrack = track;
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
            if (bottomUpNote->stringOrLine() > firstLedgerBelow - 1 && topDownNote->stringOrLine() < bottomUpNote->stringOrLine()
                && topDownNote->stringOrLine() + topDownStemLen >= firstLedgerBelow) {
                ledgerOverlapBelow = true;
            }
        }

        int firstLedgerAbove = -2;
        int bottomUpStemLen = 0;
        if (!conflictUnison && bottomUpNote->chord()->stem()) {
            bottomUpStemLen = std::round(bottomUpNote->chord()->stem()->ldata()->bbox().height() / sp * 2);
            if (topDownNote->stringOrLine() < -1 && topDownNote->stringOrLine() < bottomUpNote->stringOrLine()
                && bottomUpNote->stringOrLine() - bottomUpStemLen <= firstLedgerAbove) {
                ledgerOverlapAbove = true;
            }
        }

        // calculate offsets
        if (shareHeads) {
            for (int i = static_cast<int>(overlapNotes.size()) - 1; i >= 1; i -= 2) {
                Note* previousNote = overlapNotes[i - 1];
                Note* n = overlapNotes[i];
                const bool skipNote = previousNote->chord()->isNudged() || n->chord()->isNudged();
                if (skipNote) {
                    continue;
                }
                const bool prevChordSmall = previousNote->chord()->isSmall();
                const bool nChordSmall = n->chord()->isSmall();
                if (previousNote->chord()->dots() == n->chord()->dots()) {
                    // hide one set of dots
                    // Hide the small augmentation dot if present
                    bool onLine = !(previousNote->line() & 1);
                    if (prevChordSmall) {
                        previousNote->setDotsHidden(true);
                    } else if (nChordSmall) {
                        n->setDotsHidden(true);
                    } else if (onLine) {
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
                // If either chord is small, adjust offset
                Chord* smallChord = prevChordSmall ? previousNote->chord() : nullptr;
                smallChord = nChordSmall ? n->chord() : smallChord;
                if (smallChord && !(prevChordSmall && nChordSmall)) {
                    if (smallChord->up()) {
                        offsetInfo.centerUp *= 2;
                    } else {
                        offsetInfo.centerDown = 0;
                    }
                }
                // formerly we hid noteheads in an effort to fix playback
                // but this doesn't work for cases where noteheads cannot be shared
                // so better to solve the problem elsewhere
            }
        } else if (conflict && (posInfo.upDots && !posInfo.downDots)) {
            offsetInfo.upOffset = posInfo.maxDownWidth + 0.1 * sp;
        } else if (conflictUnison && separation == 0 && (!posInfo.downGrace || posInfo.upGrace)) {
            offsetInfo.downOffset = posInfo.maxUpWidth + 0.15 * sp;
        } else if (conflictUnison) {
            offsetInfo.upOffset = posInfo.maxDownWidth + 0.15 * sp;
        } else if (conflictSecondUpHigher) {
            offsetInfo.upOffset = posInfo.maxDownWidth + 0.15 * sp;
        } else if ((posInfo.downHooks && !posInfo.upHooks) && !(posInfo.upDots && !posInfo.downDots)) {
            // Shift by ledger line length if ledger line conflict or just 0.3sp if no ledger lines
            double adjSpace = (ledgerOverlapAbove || ledgerOverlapBelow) ? ledgerGap + ledgerLen : 0.3 * sp;
            offsetInfo.downOffset = posInfo.maxUpWidth + adjSpace;
        } else if (conflictSecondDownHigher) {
            if (posInfo.downDots && !posInfo.upDots) {
                double adjSpace = (ledgerOverlapAbove || ledgerOverlapBelow) ? ledgerGap + ledgerLen : 0.2 * sp;
                offsetInfo.downOffset = posInfo.maxUpWidth + adjSpace;
            } else {
                // Prevent ledger line & notehead collision
                double adjSpace
                    = (topDownNote->stringOrLine() <= firstLedgerAbove
                       || bottomUpNote->stringOrLine() >= firstLedgerBelow) ? ledgerLen - ledgerGap - 0.2 * sp : -0.2 * sp;
                offsetInfo.upOffset = posInfo.maxDownWidth + adjSpace;
                if (posInfo.downHooks) {
                    bool needsHookSpace = (ledgerOverlapBelow || ledgerOverlapAbove);
                    Hook* hook = topDownNote->chord()->hook();
                    double hookSpace = hook ? hook->width() : 0.0;
                    offsetInfo.upOffset = needsHookSpace ? hookSpace + ledgerLen + ledgerGap : offsetInfo.upOffset + 0.3 * sp;
                }
            }
        } else {
            // no direct conflict, so parts can overlap (downstem on left)
            // just be sure that stems clear opposing noteheads and ledger lines
            // Stems are in the middle of fret marks on TAB staves
            double clearLeft = isTab ? bottomUpNote->chord()->stemPosX() : 0.0;
            double clearRight = isTab ? bottomUpNote->chord()->stemPosX() : 0.0;
            double overlapPadding = (isTab ? 0 : 0.3) * sp;
            if (const Stem* topDownStem = topDownNote->chord()->stem()) {
                if (ledgerOverlapBelow) {
                    // Create space between stem and ledger line below staff
                    clearLeft += ledgerLen + ledgerGap + topDownStem->lineWidthMag();
                } else {
                    clearLeft += topDownStem->lineWidthMag() + overlapPadding;
                }
            }
            if (const Stem* bottomUpStem = bottomUpNote->chord()->stem()) {
                if (ledgerOverlapAbove) {
                    // Create space between stem and ledger line above staff
                    clearRight += posInfo.maxDownWidth + ledgerLen + ledgerGap - posInfo.maxUpWidth
                                  + bottomUpStem->lineWidthMag();
                } else {
                    clearRight += bottomUpStem->lineWidthMag()
                                  + std::max(posInfo.maxDownWidth - posInfo.maxUpWidth, 0.0) + overlapPadding;
                }
            } else {
                posInfo.downDots = 0;                 // no need to adjust for dots in this case
            }
            offsetInfo.upOffset = std::max(clearLeft, clearRight);
            // Check if there's enough space to tuck under a flag
            Note* topUpNote = posInfo.upStemNotes.back();
            // Move notes out of the way of straight flags
            int pad = ctx.conf().styleB(Sid::useStraightNoteFlags) ? 2 : 1;
            bool overlapsFlag = topDownNote->stringOrLine() + topDownStemLen + pad > topUpNote->stringOrLine();
            if (posInfo.downHooks && (ledgerOverlapBelow || overlapsFlag)) {
                // we will need more space to avoid collision with hook
                // but we won't need as much dot adjustment
                Hook* hook = topDownNote->chord()->hook();
                double hookWidth = hook ? hook->width() : 0.0;
                if (ledgerOverlapBelow) {
                    offsetInfo.upOffset = hookWidth + ledgerLen + ledgerGap;
                }
                if (isTab) {
                    offsetInfo.upOffset = hookWidth + posInfo.maxDownWidth;
                }
                offsetInfo.upOffset = std::max(offsetInfo.upOffset, posInfo.maxDownWidth + 0.1 * sp);
                offsetInfo.dotAdjustThreshold = posInfo.maxUpWidth - 0.3 * sp;
            }
            // if downstem chord is small, don't center
            // and we might not need as much dot adjustment either
            if (offsetInfo.centerDown > 0.0) {
                offsetInfo.centerDown = 0.0;
                offsetInfo.centerAdjustUp = 0.0;
                offsetInfo.dotAdjustThreshold = (offsetInfo.upOffset - posInfo.maxDownWidth) + posInfo.maxUpWidth - 0.3 * sp;
            }
        }
    }

    // adjust for dots
    if ((posInfo.upDots && !posInfo.downDots) || (posInfo.downDots && !posInfo.upDots)) {
        // only one sets of dots
        // place between chords
        int dots;
        double mag;
        if (posInfo.upDots) {
            dots = posInfo.upDots;
            mag = posInfo.maxUpMag;
        } else {
            dots = posInfo.downDots;
            mag = posInfo.maxDownMag;
        }
        double dotWidth = segment->symWidth(SymId::augmentationDot);
        // first dot
        offsetInfo.dotAdjust = ctx.conf().styleMM(Sid::dotNoteDistance) + dotWidth;
        // additional dots
        if (dots > 1) {
            offsetInfo.dotAdjust += ctx.conf().styleMM(Sid::dotDotDistance).val() * (dots - 1);
        }
        offsetInfo.dotAdjust *= mag;
        // only by amount over threshold
        offsetInfo.dotAdjust = std::max(offsetInfo.dotAdjust - offsetInfo.dotAdjustThreshold, 0.0);
    }
    if (separation == 1) {
        offsetInfo.dotAdjust += 0.1 * sp;
    }
}

//---------------------------------------------------------
//   layoutChords1
//    - layout upstem and downstem chords
//    - offset as necessary to avoid conflict
//---------------------------------------------------------

void ChordLayout::layoutChords1(LayoutContext& ctx, Segment* segment, staff_idx_t staffIdx)
{
    TRACEFUNC;
    LAYOUT_CALL() << LAYOUT_ITEM_INFO(segment);

    const Staff* staff = ctx.dom().staff(staffIdx);
    const bool isTab = staff->isTabStaff(segment->tick());
    const track_idx_t startTrack = staffIdx * VOICES;
    const track_idx_t endTrack   = startTrack + VOICES;
    const Fraction tick = segment->tick();
    const StaffType* staffType = staff->staffType(segment->tick());

    // we need to check all the notes in all the staves of the part so that we don't get weird collisions
    // between accidentals etc with moved notes
    const Part* part = staff->part();
    const track_idx_t partStartTrack = part ? part->startTrack() : startTrack;
    const track_idx_t partEndTrack = part ? part->endTrack() : endTrack;

    if (isTab) {
        skipAccidentals(segment, startTrack, endTrack);
    }

    const bool isSimpleTab = !staff->staffType() || (!staff->staffType()->stemThrough() && !staff->staffType()->isCommonTabStaff());
    if (staff && staff->isTabStaff(tick) && isSimpleTab) {
        // Positions of notes should be reset in case we are changing from common or full tab with offsets to simple which should have none
        for (track_idx_t track = partStartTrack; track < partEndTrack; ++track) {
            EngravingItem* e = segment->element(track);
            if (e && e->isChord() && toChord(e)->vStaffIdx() == staffIdx) {
                Chord* chord = toChord(e);
                Chord::LayoutData* chordLdata = chord->mutldata();
                chordLdata->setPosX(0.0);
            }
        }

        layoutSegmentElements(segment, startTrack, endTrack, staffIdx, ctx);
        return;
    }

    ChordPosInfo posInfo = calculateChordPosInfo(segment, staffIdx, partStartTrack, partEndTrack, ctx);

    if (posInfo.upVoices + posInfo.downVoices && (staffType->stemThrough() || staffType->isCommonTabStaff())) {
        // TODO: use track as secondary sort criteria?
        // otherwise there might be issues with unisons between voices
        // in some corner cases

        calculateMaxNoteWidths(posInfo, tick, staff, ctx);

        OffsetInfo offsetInfo = centreChords(segment, posInfo, staffIdx, tick, ctx);

        calculateChordOffsets(segment, staffIdx, tick, offsetInfo, posInfo, ctx);

        offsetAndLayoutChords(segment, staffIdx, partStartTrack, partEndTrack, offsetInfo, posInfo, ctx);
    }

    if (!isTab) {
        layoutLedgerLines(posInfo.chords);
        AccidentalsLayout::layoutAccidentals(posInfo.chords, ctx);
        for (Chord* chord : posInfo.chords) {
            for (Chord* grace : chord->graceNotes()) {
                AccidentalsLayout::layoutAccidentals({ grace }, ctx);
            }
        }
    }

    layoutSegmentElements(segment, partStartTrack, partEndTrack, staffIdx, ctx);

    for (Chord* chord : posInfo.chords) {
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

    int prevLine = 1000;            // line of previous notehead
                                    // hack: start high so first note won't show as conflict
    bool prevVisible = false;       // was last note visible?
    bool mirror = false;            // should current notehead be mirrored?
                                    // value is retained and may be used on next iteration
                                    // to track mirror status of previous note
    bool isLeft   = notes[startIdx]->chord()->up();               // is notehead on left?
    Chord* prevChord = notes[startIdx]->chord();
    staff_idx_t prevStaffIdx = prevChord->vStaffIdx();        // staff of last note (including staffMove)
    track_idx_t prevTrackIdx = prevChord->track();

    for (int idx = startIdx; idx != endIdx; idx += incIdx) {
        Note* note    = notes[idx];                         // current note
        const int line      = note->stringOrLine();                       // line of current note
        Chord* chord  = note->chord();

        const staff_idx_t staffIdx  = chord->vStaffIdx();                 // staff of current note
        const track_idx_t trackIdx = chord->track();
        const Staff* st = note->staff();
        const StaffType* tab = st->staffTypeForElement(note);
        const bool isTab = note->staff() && note->staff()->isTabStaff(note->chord()->tick());

        if (isTab) {
            // TAB notes need to be laid out to set the fret string so we can calulate their width
            // Standard staves can get this information from the notehead symbol, so no need to lay out
            TLayout::layoutNote(note, note->mutldata());
        }

        // there is a conflict
        // if this is same or adjacent line as previous note (and chords are on same staff!)
        // but no need to do anything about it if either note is invisible
        bool sameTrack = trackIdx == prevTrackIdx;

        bool conflict = (std::abs(prevLine - line) < 2) && (prevStaffIdx == staffIdx) && note->visible() && prevVisible
                        && (sameTrack || Chord::combineVoice(chord, prevChord));

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
        if (!chord->isGrace()) {
            chord->mutldata()->setPosX(0.0);
        }

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
            chord->stem()->mutldata()->setPosX(chord->stemPosX());
            TLayout::layoutStem(chord->stem(), chord->stem()->mutldata(), ctx.conf()); // needed because mirroring can cause stem position to change
        }

        // accumulate return value
        if (!mirror) {
            const double fretBackground = ctx.conf().styleS(Sid::tabFretPadding).val() * note->spatium();
            const double noteWidth = isTab ? note->tabHeadWidth(tab) + 2 * fretBackground : note->bboxRightPos();
            maxWidth = std::max(maxWidth, noteWidth);
        }

        // prepare for next iteration
        prevChord = chord;
        prevVisible = note->visible();
        prevStaffIdx = staffIdx;
        prevTrackIdx = trackIdx;
        prevLine = line;
    }

    return maxWidth;
}

bool ChordLayout::chordHasDotsAllInvisible(Chord* chord)
{
    if (!chord->dots()) {
        return false;
    }

    for (Note* note : chord->notes()) {
        for (NoteDot* dot : note->dots()) {
            if (dot->visible()) {
                return false;
            }
        }
    }

    return true;
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
    if (!chord || (chord->staff()->isTabStaff(chord->tick()) && !chord->staff()->staffType(chord->tick())->stemThrough())) {
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
                // Make sure invisible dots have no effect on visible dots, but are still layed out sensibly
                if (otherNote->visible() != note->visible()) {
                    continue;
                }
                int dotMove = otherNote->dotPosition() == DirectionV::UP ? -1 : 1;
                int otherDotLoc = otherNote->line() + dotMove;
                bool added = alreadyAdded.count(otherDotLoc);
                if (!added && muse::contains(anchoredDots, otherDotLoc)) {
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
                    if (otherNote->visible() != note->visible()) {
                        continue;
                    }
                    int dotMove = otherNote->dotPosition() == DirectionV::DOWN ? 1 : -1;
                    int otherDotLoc = otherNote->line() + dotMove;
                    bool added = alreadyAdded.count(otherDotLoc);
                    if (!added && muse::contains(anchoredDots, otherDotLoc)) {
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

void ChordLayout::setDotX(const std::vector<Chord*>& chords, const std::array<double, 3 * VOICES>& dotPos, const Staff* staff,
                          const double upDotPosX, const double downDotPosX)
{
    // Look for conflicts in up-stem and down-stemmed chords. If conflicts, all dots are aligned
    // to the same vertical line. If no conflicts, each chords aligns the dots individually.
    // Also check for conflicts between similarly stemmed voices where at least one voice is laid out independently
    std::set<track_idx_t> combineChordConflicts;
    std::set<track_idx_t> separateChordConflicts;
    for (Chord* chord1 : chords) {
        for (Chord* chord2 : chords) {
            if ((chord1 != chord2)
                && ((chord1->up() && !chord2->up() && chord2->upNote()->line() - chord1->downNote()->line() < 2)
                    || (!chord1->up() && chord2->up() && chord1->upNote()->line() - chord2->downNote()->line() < 2))
                && Chord::combineVoice(chord1, chord2)) {
                combineChordConflicts.insert({ chord1->track(), chord2->track() });
            } else if ((chord1 != chord2)
                       && ((chord1->up() && chord2->up() && chord1->upNote()->line() - chord2->downNote()->line() < 2)
                           || (!chord1->up() && !chord2->up() && chord1->upNote()->line() - chord2->downNote()->line() < 2))
                       && !Chord::combineVoice(chord1, chord2)) {
                separateChordConflicts.insert({ chord1->track(), chord2->track() });
            }
        }
    }

    const double maxPosX = std::max(upDotPosX, downDotPosX);
    for (Chord* chord : chords) {
        if (chordHasDotsAllInvisible(chord)) {
            continue;
        }
        const bool combineVoices = chord->shouldCombineVoice();
        const size_t idx = (VOICES * (chord->staffIdx() - staff->idx() + 1)) + chord->voice();
        if (!combineChordConflicts.empty()) {
            // There are conflicts
            if (muse::contains(combineChordConflicts, chord->track())) {
                // In this voice
                chord->setDotPosX(maxPosX);
            } else {
                // Elsewhere
                // If combining voices, set to max pos, if separating set to own dot pos
                chord->setDotPosX(combineVoices ? maxPosX : dotPos.at(idx));
            }
        } else {
            // There are no conflicts
            if (combineVoices) {
                if (muse::contains(separateChordConflicts, chord->track())) {
                    // Set to own dot pos if this is conflicting with a chord set to lay out independently
                    chord->setDotPosX(dotPos.at(idx));
                } else {
                    // Combine with other voices
                    if (chord->up()) {
                        chord->setDotPosX(upDotPosX);
                    } else {
                        chord->setDotPosX(downDotPosX);
                    }
                }
            } else {
                // Separate
                chord->setDotPosX(dotPos.at(idx));
            }
        }
    }
}

//---------------------------------------------------------
//   layoutChords3
//    - calculate positions of dots
//---------------------------------------------------------

void ChordLayout::layoutChords3(const std::vector<Chord*>& chords,
                                const std::vector<Note*>& notes, const Staff* staff, LayoutContext& ctx)
{
    Fraction tick      =  notes.front()->chord()->segment()->tick();
    const MStyle& style = ctx.conf().style();
    double sp           = staff->spatium(tick);
    double stepDistance = sp * staff->lineDistance(tick) * .5;
    int stepOffset     = staff->staffType(tick)->stepOffset();

    double upDotPosX    = 0.0;
    double downDotPosX  = 0.0;
    // Track dot position of voices on this stave and possible cross voices from above & below
    std::array<double, 3 * VOICES> dotPos{};

    int nNotes = int(notes.size());

    for (int i = nNotes - 1; i >= 0; --i) {
        Note* note     = notes[i];
        DirectionV dotPosition = note->userDotPosition();
        const Chord* chord = note->chord();
        if (chord->dots()) {
            if (dotPosition == DirectionV::AUTO && nNotes > 1 && note->visible() && !note->dotsHidden()) {
                // resolve dot conflicts
                int line = note->line();
                Note* above = (i < nNotes - 1) ? notes[i + 1] : nullptr;
                if (above
                    && (!above->visible() || above->dotsHidden() || above->chord()->dots() == 0
                        || !Chord::combineVoice(chord, above->chord()))) {
                    above = nullptr;
                }
                int intervalAbove = above ? line - above->line() : 1000;
                Note* below = (i > 0) ? notes[i - 1] : nullptr;
                if (below
                    && (!below->visible() || below->dotsHidden() || below->chord()->dots() == 0
                        || !Chord::combineVoice(chord, below->chord()))) {
                    below = nullptr;
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

    // Calculate the chords' dotPosX, and find the leftmost point for accidental layout
    for (Chord* chord : chords) {
        bool _up = chord->up();

        if (chord->stemSlash()) {
            TLayout::layoutStemSlash(chord->stemSlash(), chord->stemSlash()->mutldata(), ctx.conf());
        }

        double overlapMirror;
        Stem* stem = chord->stem();
        if (stem) {
            overlapMirror = stem->lineWidthMag();
        } else if (chord->durationType().headType() == NoteHeadType::HEAD_WHOLE) {
            overlapMirror = style.styleMM(Sid::stemWidth) * chord->mag();
        } else {
            overlapMirror = 0.0;
        }

        double minDotPosX = 0.0;

        std::vector<Note*> chordNotes = chord->notes();
        std::sort(chordNotes.begin(), chordNotes.end(),
                  [](Note* n1, const Note* n2) ->bool { return n1->line() < n2->line(); });
        for (Note* note : chordNotes) {
            double noteX = 0.0;
            if (note->ldata()->mirror()) {
                if (_up) {
                    noteX = chord->stemPosX() - overlapMirror;
                } else {
                    noteX = -note->headBodyWidth() + overlapMirror;
                }
            } else if (_up) {
                noteX = chord->stemPosX() - note->headBodyWidth();
            }

            double ny = (note->line() + stepOffset) * stepDistance;
            if (note->ldata()->pos().y() != ny) {
                note->mutldata()->setPosY(ny);
                if (stem) {
                    TLayout::layoutStem(chord->stem(), chord->stem()->mutldata(), ctx.conf());
                    if (chord->hook()) {
                        chord->hook()->mutldata()->setPosY(chord->stem()->flagPosition().y());
                    }
                }
            }

            const double dotX = noteX + note->headBodyWidth() + chord->pos().x();
            if (chord->dots()) {
                const size_t idx = (VOICES * (chord->staffIdx() - staff->idx() + 1)) + chord->voice();
                dotPos.at(idx) = std::max(dotPos.at(idx), dotX);
            }

            note->mutldata()->setPosX(noteX);
            minDotPosX = std::max(minDotPosX, dotX);
        }

        //---------------------------------------------------
        //    layout dots simply
        //     we will check for conflicts after all the notes have been processed
        //---------------------------------------------------

        if (chord->dots()) {
            if (chordHasDotsAllInvisible(chord)) {
                chord->setDotPosX(minDotPosX);
            } else if (chord->up()) {
                upDotPosX = std::max(upDotPosX, minDotPosX);
            } else {
                downDotPosX = std::max(downDotPosX, minDotPosX);
            }
        }
    }

    setDotX(chords, dotPos, staff, upDotPosX, downDotPosX);
}

void ChordLayout::layoutLedgerLines(const std::vector<Chord*>& chords)
{
    for (Chord* item : chords) {
        item->updateLedgerLines();
        for (Chord* grace : item->graceNotes()) {
            grace->updateLedgerLines();
        }
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
    bool hasVoices = measure->hasVoices(c->vStaffIdx(), c->tick(), c->ticks(), true);
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
        EngravingItem* item = segment->preAppendedItem(track);
        if (item && item->isGraceNotesGroup()) {
            GraceNotesGroup* gng = toGraceNotesGroup(item);
            gng->insert(gng->end(), gnb.begin(), gnb.end());
        } else {
            gnb.setAppendedSegment(segment);
            segment->preAppend(&gnb, track);
        }
    }

    //Attach graceNotesAfter of this chord to the *following* segment
    if (!gna.empty()) {
        Segment* followingSeg = measure->tick2segment(segment->tick() + chord->actualTicks(), SegmentType::All);
        while (followingSeg
               && (!followingSeg->hasElements(staff2track(staffIdx), staff2track(staffIdx) + 3) || followingSeg->isTimeTickType())) {
            // If there is nothing on this staff, go to next segment
            followingSeg = followingSeg->next();
        }
        if (followingSeg) {
            gna.setAppendedSegment(followingSeg);
            followingSeg->preAppend(&gna, track);
        }
    }
}

/* Grace-notes-after have the special property of belonging to
*  a segment but being pre-appended to another. This repositioning
*  is needed and must be called AFTER horizontal spacing is calculated. */
void ChordLayout::repositionGraceNotesAfter(Segment* segment, size_t tracks)
{
    for (track_idx_t track = 0; track < tracks; track++) {
        EngravingItem* item = segment->preAppendedItem(track);
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
    if (chord->endsNoteAnchoredLine()) {
        for (Note* note : chord->notes()) {
            for (Spanner* sp : note->spannerBack()) {
                if (sp->isGlissando()) {
                    TLayout::layoutGlissando(toGlissando(sp), ctx);
                } else if (sp->isGuitarBend()) {
                    TLayout::layoutGuitarBend(toGuitarBend(sp), ctx);
                } else if (sp->isNoteLine()) {
                    TLayout::layoutNoteLine(toNoteLine(sp), ctx);
                }
            }
        }
    }
    if (isFirstInMeasure) {
        for (Note* note : chord->notes()) {
            Tie* tieBack = note->tieBack();
            if (tieBack && (note->incomingPartialTie() || tieBack->startNote()->findMeasure() != note->findMeasure())) {
                SlurTieLayout::layoutTieBack(tieBack, note->findMeasure()->system(), ctx);
            }
        }
    }
    for (Note* note : chord->notes()) {
        Tie* tie = note->tieFor();
        if (tie) {
            if (tie->isPartialTie()) {
                SlurTieLayout::layoutTieFor(tie, note->findMeasure()->system());  // line attach points are updated here
            }

            Note* endNote = tie->endNote();
            if (!endNote) {
                continue;
            }
            const Measure* endNoteMeasure = endNote->findMeasure();
            const Measure* noteMeasure = note->findMeasure();
            if (endNoteMeasure == noteMeasure || endNoteMeasure->system() != noteMeasure->system()) {
                SlurTieLayout::layoutTieFor(tie, note->findMeasure()->system());  // line attach points are updated here
            }
        }
    }

    SlurTieLayout::layoutLaissezVibChord(chord, ctx);
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

            Shape chordShape = chord->shape().translate(chord->pos());
            chordShape.removeInvisibles();
            chordShape.removeTypes({ ElementType::ARPEGGIO });
            if (chordShape.empty()) {
                continue;
            }

            double clearance = 0.0;
            Shape restShape = rest->shape().translate(rest->pos() - offset);
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
                int steps = ceil(std::abs(margin) / (lineDistance / 2));
                yMove = steps * lineDistance / 2 * upSign;
                rest->mutldata()->moveY(yMove);
            } else {
                int steps = ceil(std::abs(margin) / lineDistance);
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
        Shape shape1 = rest1->shape().translate(rest1->pos() - rest1->offset());

        Rest* rest2 = rests[i + 1];
        if (!rest2->visible() || !rest2->autoplace()) {
            continue;
        }

        if (muse::contains(rest1->ldata()->mergedRests, rest2) || muse::contains(rest2->ldata()->mergedRests, rest1)) {
            continue;
        }

        Shape shape2 = rest2->shape().translate(rest2->pos() - rest2->offset());
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

        int steps = ceil(std::abs(margin) / lineDistance);
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
            shape1 = rest1->shape().translate(rest1->pos() - rest1->offset());
            shape2 = rest2->shape().translate(rest2->pos() - rest2->offset());

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
            system->staff(f->note()->chord()->vStaffIdx())->skyline().add(r, f);
        }
        shapesToRecreate.insert(f->staffIdx());
    }
    for (staff_idx_t staffIdx : shapesToRecreate) {
        segment->createShape(staffIdx);
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
    Chord* chord = item->chord();
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
    ShowTiedFret showTiedFret = item->style().value(Sid::tabShowTiedFret).value<ShowTiedFret>();
    bool useParens = isTabStaff && !item->fixed() && item->tieBack()
                     && (showTiedFret != ShowTiedFret::TIE_AND_FRET || item->isContinuationOfBend()) && !item->shouldHideFret();

    if (item->harmonic() && item->displayFret() != Note::DisplayFretOption::NaturalHarmonic) {
        useParens = false;
    }

    if (useParens) {
        double widthWithoutParens = item->tabHeadWidth(staffType);
        item->setHeadHasParentheses(true, /* addToLinked= */ false, /* generated= */ true);
        double w = item->tabHeadWidth(staffType);
        double xOff = 0.5 * (w - widthWithoutParens);
        ldata->moveX(-xOff);
        ldata->setBbox(0, staffType->fretBoxY(ctx.conf().style()) * item->magS(), w,
                       staffType->fretBoxH(ctx.conf().style()) * item->magS());
    } else if (isTabStaff && (!item->ghost() || item->shouldHideFret())) {
        item->setHeadHasParentheses(false, /*addToLinked=*/ false);
    }
    int dots = chord->dots();
    if (dots && !item->dots().empty()) {
        if (chord->slash() && !item->visible()) {
            item->setDotsHidden(true);
        }
        // if chords have notes with different mag, dots must still  align
        double correctMag = chord->notes().size() > 1 ? chord->mag() : item->mag();
        double d  = ctx.conf().point(ctx.conf().styleS(Sid::dotNoteDistance)) * correctMag;
        double dd = ctx.conf().point(ctx.conf().styleS(Sid::dotDotDistance)) * correctMag;
        double x  = isTabStaff ? chord->dotPosX() - item->pos().x() : chord->dotPosX() - item->pos().x() - chord->pos().x();
        // in case of dots with different size, center-align them
        if (item->mag() != chord->mag() && chord->notes().size() > 1) {
            double relativeMag = item->mag() / chord->mag();
            double centerAlignOffset = item->dot(0)->width() * (1 / relativeMag - 1) / 2;
            x += centerAlignOffset;
        }
        // adjust dot distance for hooks
        Hook* hook = chord->hook();
        if (chord->up() && hook && hook->visible()) {
            double hookRight = hook->width() + hook->x() + chord->pos().x();
            double hookBottom = hook->height() + hook->y() + chord->pos().y()
                                + (0.25 * item->spatium());
            // the top dot in the chord, not the dot for this particular note:
            double dotY = chord->notes().back()->y() + chord->notes().back()->dots().front()->pos().y();
            if (chord->dotPosX() < hookRight && dotY < hookBottom) {
                d = hook->width();
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
        // Invisible dots are laid out separately,
        // with a distance of 0.1sp to distinguish from visible ones
        double visibleX = x + d;
        double invisibleX = x;

        for (NoteDot* dot : item->dots()) {
            if (dot->visible()) {
                dot->mutldata()->setPosX(visibleX);
                visibleX += dd;
            } else {
                invisibleX += (0.1 * item->spatium());
                dot->mutldata()->setPosX(invisibleX);
                invisibleX += item->symWidth(SymId::augmentationDot) * dot->mag();
            }
        }
    }

    // layout elements attached to note
    for (EngravingItem* e : item->el()) {
        if (e->isSymbol()) {
            e->mutldata()->setMag(item->mag());
            Shape noteShape = item->shape();
            noteShape.remove_if([e](ShapeElement& s) { return s.item() == e || s.item()->isBend(); });
            LedgerLine* ledger = (item->line() < -1 || item->line() > item->staff()->lines(item->tick())) && !chord->ledgerLines().empty()
                                 ? chord->ledgerLines().front() : nullptr;
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

                e->mutldata()->setPosX(right + parenthesisPadding);
            } else if (sym->sym() == SymId::noteheadParenthesisLeft) {
                e->mutldata()->setPosX(-left - e->width() - parenthesisPadding);
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
        fillShape(static_cast<const Chord*>(item), static_cast<Chord::LayoutData*>(ldata));
        break;
    case ElementType::REST:
        fillShape(static_cast<const Rest*>(item), static_cast<Rest::LayoutData*>(ldata));
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

Shape ChordLayout::chordRestShape(const ChordRest* item)
{
    Shape shape;
    {
        for (Lyrics* l : item->lyrics()) {
            if (!l || !l->addToSkyline() || l->xmlText().empty()) {
                continue;
            }
            RectF bbox = l->ldata()->bbox().translated(l->pos());
            shape.addHorizontalSpacing(l, bbox.left(), bbox.right());
        }
    }

    return shape;
}

bool ChordLayout::leaveSpaceForTie(const Articulation* item)
{
    if (!item->explicitParent() || !item->explicitParent()->isChord()) {
        return false;
    }

    Chord* chord = toChord(item->chordRest());
    bool up = item->ldata()->up;
    Note* note = up ? chord->upNote() : chord->downNote();
    Tie* tieFor = note->tieFor();
    Tie* tieBack = note->tieBack();

    if (!tieFor && !tieBack) {
        return false;
    }

    bool leaveSpace = (tieFor && tieFor->up() == up && tieFor->isOuterTieOfChord(Grip::START))
                      || (tieBack && tieBack->up() == up && tieBack->isOuterTieOfChord(Grip::END));

    return leaveSpace;
}

void ChordLayout::fillShape(const Chord* item, ChordRest::LayoutData* ldata)
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

    TremoloSingleChord* tremolo = item->tremoloSingleChord();
    if (tremolo) {
        LD_CONDITION(tremolo->ldata()->isSetShape());
    }

    Arpeggio* arpeggio = item->arpeggio();
    if (arpeggio) {
        LD_CONDITION(arpeggio->ldata()->isSetShape());
    }

    Arpeggio* spanArpeggio = item->spanArpeggio();
    if (spanArpeggio) {
        LD_CONDITION(spanArpeggio->ldata()->isSetShape());
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

    if (tremolo && tremolo->addToSkyline()) {
        shape.add(tremolo->shape().translate(tremolo->pos()));
    }

    if (arpeggio && arpeggio->addToSkyline()) {
        shape.add(arpeggio->shape().translate(arpeggio->pos()));
    }

    if (spanArpeggio && !arpeggio && spanArpeggio->vStaffIdx() == item->vStaffIdx() && spanArpeggio->addToSkyline()) {
        PointF spanArpPos = spanArpeggio->pos() - (item->pagePos() - spanArpeggio->chord()->pagePos());
        shape.add(spanArpeggio->shape().translate(spanArpPos));
    }

    for (Note* note : item->notes()) {
        shape.add(note->shape().translate(note->pos()));
    }

    for (EngravingItem* e : item->el()) {
        if (e->addToSkyline()) {
            shape.add(e->shape().translate(e->pos()));
        }
    }

    shape.add(chordRestShape(item));      // add lyrics

    for (const LedgerLine* l : item->ledgerLines()) {
        shape.add(l->shape().translate(l->pos() - l->staffOffset()));
    }

    if (beamlet && stem) {
        double xPos = beamlet->line.p1().x() - stem->ldata()->pos().x();
        if (beamlet->isBefore && !item->up()) {
            xPos -= stem->width();
        } else if (!beamlet->isBefore && item->up()) {
            xPos += stem->width();
        }
        shape.add(beamlet->shape().translate(PointF(-xPos, 0.0)));
    }

    for (const Articulation* a : item->articulations()) {
        // Only add lv to shape
        if (a->isLaissezVib()) {
            shape.add(a->shape().translate(a->pos()));
        }
    }

    ldata->setShape(shape);
}

void ChordLayout::fillShape(const Rest* item, Rest::LayoutData* ldata)
{
    Shape shape(Shape::Type::Composite);

    if (!item->isGap() && !item->shouldNotBeDrawn()) {
        shape.add(chordRestShape(item));
        shape.add(item->symBbox(ldata->sym), item);
        for (const NoteDot* dot : item->dotList()) {
            if (dot->addToSkyline()) {
                shape.add(item->symBbox(SymId::augmentationDot).translated(dot->pos()), dot);
            }
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

    shape.add(item->numberRect(), item);
    shape.add(item->symBbox(ldata->symId), item);

    ldata->setShape(shape);
}

void ChordLayout::fillShape(const MMRest* item, MMRest::LayoutData* ldata, const LayoutConfiguration& conf)
{
    Shape shape(Shape::Type::Composite);

    double vStrokeHeight = conf.styleMM(Sid::mmRestHBarVStrokeHeight);
    shape.add(RectF(0.0, -(vStrokeHeight * .5), ldata->restWidth, vStrokeHeight), item);
    if (item->shouldShowNumber()) {
        shape.add(item->numberRect().translated(item->numberPos()), item);
    }

    ldata->setShape(shape);
}

void ChordLayout::addLineAttachPoints(Spanner* spanner)
{
    assert(spanner->anchor() == Spanner::Anchor::NOTE);

    const SpannerSegment* frontSeg = toSpannerSegment(spanner->frontSegment());
    const SpannerSegment* backSeg = toSpannerSegment(spanner->backSegment());
    Note* startNote = nullptr;
    Note* endNote = nullptr;

    EngravingItem* startElement = spanner->startElement();
    EngravingItem* endElement = spanner->endElement();
    if (startElement && startElement->isNote()) {
        startNote = toNote(startElement);
    }
    if (endElement && endElement->isNote()) {
        endNote = toNote(endElement);
    }
    if (!frontSeg || !backSeg || !startNote || !endNote) {
        return;
    }
    double startX = frontSeg->ldata()->pos().x();
    double endX = backSeg->pos2().x() + backSeg->ldata()->pos().x(); // because pos2 is relative to ipos
    // Here we don't pass y() because its value is unreliable during the first stages of layout.
    // The y() is irrelevant anyway for horizontal spacing.
    startNote->addStartLineAttachPoint(PointF(startX, 0.0), spanner);
    endNote->addEndLineAttachPoint(PointF(endX, 0.0), spanner);
}
