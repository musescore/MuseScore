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
#include "slurtielayout.h"

#include "iengravingfont.h"

#include "compat/dummyelement.h"

#include "dom/slur.h"
#include "dom/chord.h"
#include "dom/score.h"
#include "dom/system.h"
#include "dom/staff.h"
#include "dom/stafftype.h"
#include "dom/ledgerline.h"
#include "dom/note.h"
#include "dom/hook.h"
#include "dom/stem.h"
#include "dom/tremolotwochord.h"
#include "dom/tie.h"
#include "dom/engravingitem.h"
#include "dom/measure.h"
#include "dom/guitarbend.h"
#include "dom/laissezvib.h"
#include "dom/parenthesis.h"
#include "dom/partialtie.h"
#include "dom/hammeronpulloff.h"

#include "tlayout.h"
#include "chordlayout.h"
#include "stemlayout.h"
#include "tremololayout.h"
#include "../engraving/types/symnames.h"

#include "draw/types/transform.h"

using namespace muse::draw;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

SpannerSegment* SlurTieLayout::layoutSystem(Slur* item, System* system, LayoutContext& ctx)
{
    const double horizontalTieClearance = 0.35 * item->spatium();
    const double tieClearance = 0.65 * item->spatium();
    const double continuedSlurOffsetY = item->spatium() * .4;
    const double continuedSlurMaxDiff = 2.5 * item->spatium();
    Fraction stick = system->firstMeasure()->tick();
    Fraction etick = system->lastMeasure()->endTick();

    SlurSegment* slurSegment = toSlurSegment(TLayout::getNextLayoutSystemSegment(item, system, [item](System* parent) {
        return item->newSlurTieSegment(parent);
    }));

    SpannerSegmentType sst;
    if (item->tick() >= stick) {
        //
        // this is the first call to layoutSystem,
        // processing the first line segment
        //
        if (item->track2() == muse::nidx) {
            item->setTrack2(item->track());
        }
        if (item->startCR() == 0 || item->startCR()->measure() == 0) {
            LOGD("Slur::layout(): track %zu-%zu  %p - %p tick %d-%d null start anchor",
                 item->track(), item->track2(), item->startCR(), item->endCR(), item->tick().ticks(), item->tick2().ticks());
            return slurSegment;
        }
        if (item->endCR() == 0) {         // sanity check
            item->setEndElement(item->startCR());
            item->setTick2(item->tick());
        }
        computeUp(item, ctx);
        sst = item->tick2() < etick ? SpannerSegmentType::SINGLE : SpannerSegmentType::BEGIN;
    } else if (item->tick() < stick && item->tick2() >= etick) {
        sst = SpannerSegmentType::MIDDLE;
    } else {
        sst = SpannerSegmentType::END;
    }
    slurSegment->setSpannerSegmentType(sst);

    SlurTiePos sPos;
    slurPos(item, &sPos, ctx);
    PointF p1, p2;
    // adjust for ties
    p1 = sPos.p1;
    p2 = sPos.p2;
    bool constrainLeftAnchor = false;

    const bool incomingPartialSlur = item->isIncoming();
    const bool outgoingPartialSlur = item->isOutgoing();

    // start anchor, either on the start chordrest or at the beginning of the system
    if ((sst == SpannerSegmentType::SINGLE || sst == SpannerSegmentType::BEGIN) && !incomingPartialSlur) {
        Chord* sc = item->startCR()->isChord() ? toChord(item->startCR()) : nullptr;

        // on chord
        if (sc) {
            Tie* tie = (item->up() ? sc->upNote() : sc->downNote())->tieFor();
            PointF endPoint = PointF();
            if (tie && (tie->isInside() || tie->up() != item->up())) {
                // there is a tie that starts on this chordrest
                tie = nullptr;
            }
            if (tie && !tie->segmentsEmpty()) {
                endPoint = tie->segmentAt(0)->ups(Grip::START).pos();
            }
            bool adjustedVertically = false;
            if (tie) {
                if (item->up() && tie->up()) {
                    if (endPoint.y() - p1.y() < tieClearance) {
                        p1.ry() = endPoint.y() - tieClearance;
                        adjustedVertically = true;
                    }
                } else if (!item->up() && !tie->up()) {
                    if (p1.y() - endPoint.y() < tieClearance) {
                        p1.ry() = endPoint.y() + tieClearance;
                        adjustedVertically = true;
                    }
                }
            }
            Tie* tieBack = sc->notes()[0]->tieBack();
            if (!adjustedVertically && tieBack && (!tieBack->isInside() || tieBack->isPartialTie())
                && tieBack->up() == item->up()) {
                // there is a tie that ends on this chordrest
                tie = tieBack;
                if (!tie->segmentsEmpty()) {
                    endPoint = tie->segmentAt(static_cast<int>(tie->nsegments()) - 1)->ups(Grip::END).pos();
                    if (std::abs(endPoint.y() - p1.y()) < tieClearance) {
                        p1.rx() += horizontalTieClearance;
                    }
                }
            }
        }
    } else if (sst == SpannerSegmentType::END || sst == SpannerSegmentType::MIDDLE || incomingPartialSlur) {
        // beginning of system
        Measure* measure = item->startCR()->measure();
        ChordRest* firstCr = incomingPartialSlur ? measure->firstChordRest(item->track()) : system->firstChordRest(item->track2());
        double y = p1.y();
        if (firstCr && firstCr == item->endCR()) {
            constrainLeftAnchor = true;
        }
        if (firstCr && firstCr->isChord()) {
            Chord* chord = toChord(firstCr);
            Shape chordShape = chord->shape();
            chordShape.removeTypes({ ElementType::ACCIDENTAL });
            y = item->up() ? chordShape.top() : chordShape.bottom();
            bool isAboveStem = chord->stem() && chord->up() == item->up();
            if (!isAboveStem) {
                y += continuedSlurOffsetY * (item->up() ? -1 : 1);
            }
        }

        double segmentX = incomingPartialSlur ? measure->firstNoteRestSegmentX(true) : system->firstNoteRestSegmentX(true);
        p1 = PointF(segmentX, y);

        // adjust for ties at the start of the system
        ChordRest* cr = incomingPartialSlur ? measure->firstChordRest(item->track()) : system->firstChordRest(item->track());
        if (cr && cr->isChord() && cr->tick() >= stick && cr->tick() <= etick) {
            Chord* c = toChord(cr);
            Tie* tie = nullptr;
            PointF endPoint;
            Tie* tieBack = c->notes()[0]->tieBack();
            if (tieBack && (tieBack->isPartialTie() || !tieBack->isInside()) && tieBack->up() == item->up()) {
                // there is a tie that ends on this chordrest
                if (!tieBack->segmentsEmpty()) { //Checks for spanner segment esxists
                    tie = tieBack;
                    endPoint = tie->backSegment()->ups(Grip::START).pos();
                }
            }
            if (tie) {
                if (tie->isPartialTie()) {
                    p1.rx() = endPoint.rx();
                }
                if (item->up() && tie->up()) {
                    if (endPoint.y() - p1.y() < tieClearance) {
                        p1.ry() = endPoint.y() - tieClearance;
                    }
                } else if (!item->up() && !tie->up()) {
                    if (p1.y() - endPoint.y() < tieClearance) {
                        p1.ry() = endPoint.y() + tieClearance;
                    }
                }
            }
        }
    }

    // end anchor
    if ((sst == SpannerSegmentType::SINGLE || sst == SpannerSegmentType::END) && !outgoingPartialSlur) {
        Chord* ec = item->endCR()->isChord() ? toChord(item->endCR()) : nullptr;

        // on chord
        if (ec) {
            Tie* tie = (item->up() ? ec->upNote() : ec->downNote())->tieBack();
            PointF endPoint;
            if (tie && (tie->isInside() || tie->up() != item->up())) {
                tie = nullptr;
            }
            bool adjustedVertically = false;
            if (tie && !tie->segmentsEmpty()) {
                endPoint = tie->segmentAt(0)->ups(Grip::END).pos();
                if (item->up() && tie->up()) {
                    if (endPoint.y() - p2.y() < tieClearance) {
                        p2.ry() = endPoint.y() - tieClearance;
                        adjustedVertically = true;
                    }
                } else if (!item->up() && !tie->up()) {
                    if (p2.y() - endPoint.y() < tieClearance) {
                        p2.ry() = endPoint.y() + tieClearance;
                        adjustedVertically = true;
                    }
                }
            }
            Tie* tieFor = ec->notes()[0]->tieFor();
            if (!adjustedVertically && tieFor && (tieFor->isPartialTie() || !tieFor->isInside()) && tieFor->up() == item->up()) {
                // there is a tie that starts on this chordrest
                if (!tieFor->segmentsEmpty() && std::abs(tieFor->frontSegment()->ups(Grip::START).pos().y() - p2.y()) < tieClearance) {
                    p2.rx() -= horizontalTieClearance;
                }
            }
        }
    } else {
        // at end of system
        Measure* measure = item->endCR()->measure();
        ChordRest* lastCr = outgoingPartialSlur ? measure->lastChordRest(item->track()) : system->lastChordRest(item->track());
        double y = p1.y();
        if (lastCr && lastCr == item->startCR()) {
            y += 0.25 * item->spatium() * (item->up() ? -1 : 1);
        } else if (lastCr && lastCr->isChord()) {
            Chord* chord = toChord(lastCr);
            Shape chordShape = chord->shape();
            chordShape.removeTypes({ ElementType::ACCIDENTAL });
            y = item->up() ? chordShape.top() : chordShape.bottom();
            bool isAboveStem = chord->stem() && chord->up() == item->up();
            if (!isAboveStem) {
                y += continuedSlurOffsetY * (item->up() ? -1 : 1);
            }
            double diff = item->up() ? y - p1.y() : p1.y() - y;
            if (diff > continuedSlurMaxDiff) {
                y = p1.y() + (y > p1.y() ? continuedSlurMaxDiff : -continuedSlurMaxDiff);
            }
        }

        double endingX = outgoingPartialSlur ? measure->endingXForOpenEndedLines() : system->endingXForOpenEndedLines();
        p2 = PointF(endingX, y);

        // adjust for ties at the end of the system
        ChordRest* cr = outgoingPartialSlur ? measure->lastChordRest(item->track()) : system->lastChordRest(item->track());

        if (cr && cr->isChord() && cr->tick() >= stick && cr->tick() <= etick) {
            Chord* c = toChord(cr);
            Tie* tie = nullptr;
            PointF endPoint;
            Tie* tieFor = c->notes()[0]->tieFor();
            if (tieFor && (tieFor->isPartialTie() || !tieFor->isInside()) && tieFor->up() == item->up()) {
                // there is a tie that starts on this chordrest
                if (!tieFor->segmentsEmpty()) { //Checks is spanner segment exists
                    tie = tieFor;
                    endPoint = tie->segmentAt(0)->ups(Grip::END).pos();
                }

                if (outgoingPartialSlur && tie->type() == ElementType::TIE && tie->nsegments() == 1) {
                    // For partial slurs ending midway through a tie, get top of the tie shape at the slur's end X
                    const TieSegment* tieSeg = tie->frontSegment();
                    const Shape tieShape = tieSeg->shape().translate(tieSeg->pos());
                    if (item->up() && tie->up()) {
                        endPoint.ry() = p2.y() + tieShape.topDistance(p2);
                    } else if (!item->up() && !tie->up()) {
                        endPoint.ry() = p2.y() - tieShape.bottomDistance(p2);
                    }
                }
            }
            if (tie) {
                if (item->up() && tie->up()) {
                    if (endPoint.y() - p2.y() < tieClearance) {
                        p2.ry() = endPoint.y() - tieClearance;
                    }
                } else if (!item->up() && !tie->up()) {
                    if (p2.y() - endPoint.y() < tieClearance) {
                        p2.ry() = endPoint.y() + tieClearance;
                    }
                }
            }
        }
    }
    if (constrainLeftAnchor) {
        p1.ry() = p2.y() + (0.25 * item->spatium() * (item->up() ? -1 : 1));
    }

    if (item->isTappingHalfSlur()) {
        p1 = p2 - PointF(2.5 * item->spatium(), 0.0);
    }

    bool isHangingSlur = sst == SpannerSegmentType::BEGIN || sst == SpannerSegmentType::END || incomingPartialSlur || outgoingPartialSlur;
    if (isHangingSlur && ctx.conf().styleB(Sid::angleHangingSlursAwayFromStaff)) {
        adjustSlurFloatingEndPointAngles(slurSegment, p1, p2, incomingPartialSlur, outgoingPartialSlur);
    }

    layoutSegment(slurSegment,  p1, p2);

    return slurSegment;
}

void SlurTieLayout::adjustSlurFloatingEndPointAngles(SlurSegment* slurSeg, PointF& p1, PointF& p2, bool incomingPartial,
                                                     bool outgoingPartial)
{
    bool startIsHanging = slurSeg->spannerSegmentType() == SpannerSegmentType::END || incomingPartial;
    bool endIsHanging = slurSeg->spannerSegmentType() == SpannerSegmentType::BEGIN || outgoingPartial;

    IF_ASSERT_FAILED(startIsHanging != endIsHanging) {
        return;
    }

    bool up = slurSeg->slur()->up();
    const double heightDiff = 1.0 * slurSeg->spatium();

    if (startIsHanging) {
        double yCur = p1.y();
        p1.setY(up ? std::min(yCur, p2.y() - heightDiff) : std::max(yCur, p2.y() + heightDiff));
    } else if (endIsHanging) {
        double yCur = p2.y();
        p2.setY(up ? std::min(yCur, p1.y() - heightDiff) : std::max(yCur, p1.y() + heightDiff));
    }
}

//---------------------------------------------------------
//   slurPos
//    calculate position of start- and endpoint of slur
//    relative to System() position
//---------------------------------------------------------

void SlurTieLayout::slurPos(Slur* item, SlurTiePos* sp, LayoutContext& ctx)
{
    item->stemFloated().reset();
    double _spatium = (item->staffType() ? item->staffType()->lineDistance().val() : 1.0) * item->spatium();
    const double stemSideInset = 0.5;
    const double stemOffsetX = 0.35;
    const double beamClearance = 0.35;
    const double beamAnchorInset = 0.15;
    const double straightStemXOffset = 0.5; // how far down a straight stem a slur attaches (percent)
    const double minOffset = 0.2;
    // hack alert!! -- fakeCutout
    // The fakeCutout const describes the slope of a line from the top of the stem to the full width of the hook.
    // this is necessary because hooks don't have SMuFL cutouts
    // Gonville and MuseJazz have really weirdly-shaped hooks compared to Leland and Bravura and Emmentaler,
    // so we need to adjust the slope of our hook-avoidance line. this will be unnecessary when hooks have
    // SMuFL anchors
    bool bulkyHook = ctx.engravingFont()->family() == "Gonville" || ctx.engravingFont()->family() == "MuseJazz";
    const double fakeCutoutSlope = bulkyHook ? 1.5 : 1.0;

    if (item->endCR() == 0) {
        sp->p1 = item->startCR()->pagePos();
        sp->p1.rx() += item->startCR()->width();
        sp->p2 = sp->p1;
        sp->p2.rx() += 5 * _spatium;
        sp->system1 = item->startCR()->measure()->system();
        sp->system2 = sp->system1;
        return;
    }

    bool useTablature = item->staff() && item->staff()->isTabStaff(item->endCR()->tick());
    bool staffHasStems = true;       // assume staff uses stems
    const StaffType* stt = 0;
    if (useTablature) {
        stt = item->staff()->staffType(item->tick());
        staffHasStems = stt->stemThrough();       // if tab with stems beside, stems do not count for slur pos
    }

    // start and end cr, chord, and note
    ChordRest* scr = item->startCR();
    ChordRest* ecr = item->endCR();
    Chord* sc = 0;
    Note* note1 = 0;
    if (scr->isChord()) {
        sc = toChord(scr);
        note1 = item->up() ? sc->upNote() : sc->downNote();
    }
    Chord* ec = 0;
    Note* note2 = 0;
    if (ecr->isChord()) {
        ec = toChord(ecr);
        note2 = item->up() ? ec->upNote() : ec->downNote();
    }

    sp->system1 = scr->measure()->system();
    sp->system2 = ecr->measure()->system();

    if (sp->system1 == 0) {
        LOGD("no system1");
        return;
    }

    sp->p1 = scr->pos() + scr->segment()->pos() + scr->measure()->pos();
    sp->p2 = ecr->pos() + ecr->segment()->pos() + ecr->measure()->pos();

    // adjust for cross-staff
    if (scr->vStaffIdx() != item->vStaffIdx() && sp->system1) {
        double diff = sp->system1->staff(scr->vStaffIdx())->y() - sp->system1->staff(item->vStaffIdx())->y();
        sp->p1.ry() += diff;
    }
    if (ecr->vStaffIdx() != item->vStaffIdx() && sp->system2) {
        double diff = sp->system2->staff(ecr->vStaffIdx())->y() - sp->system2->staff(item->vStaffIdx())->y();
        sp->p2.ry() += diff;
    }

    // account for centering or other adjustments (other than mirroring)
    if (note1 && !note1->ldata()->mirror.value()) {
        sp->p1.rx() += note1->x();
    }
    if (note2 && !note2->ldata()->mirror.value()) {
        sp->p2.rx() += note2->x();
    }

    PointF po = PointF();

    Stem* stem1 = sc && staffHasStems ? sc->stem() : 0;
    Stem* stem2 = ec && staffHasStems ? ec->stem() : 0;

    enum class SlurAnchor : char {
        NONE, STEM
    };
    SlurAnchor sa1 = SlurAnchor::NONE;
    SlurAnchor sa2 = SlurAnchor::NONE;
    if (staffHasStems) {
        if (sc && sc->hook() && sc->up() == item->up()) {
            sa1 = SlurAnchor::STEM;
        }
        if (scr->up() == ecr->up() && scr->up() == item->up()) {
            if (stem1 && !stemSideStartForBeam(item)) {
                sa1 = SlurAnchor::STEM;
            }
            if (stem2 && !stemSideEndForBeam(item)) {
                sa2 = SlurAnchor::STEM;
            }
        } else if (ecr->segment()->system() != scr->segment()->system()) {
            // in the case of continued slurs, we anchor to stem when necessary
            if (scr->up() == item->up() && stem1 && !scr->beam()) {
                sa1 = SlurAnchor::STEM;
            }
            if (ecr->up() == item->up() && stem2 && !ecr->beam()) {
                sa2 = SlurAnchor::STEM;
            }
        }
    }

    double __up = item->up() ? -1.0 : 1.0;
    double hw1 = note1 ? note1->tabHeadWidth(stt) : scr->width() * scr->mag();        // if stt == 0, tabHeadWidth()
    double hw2 = note2 ? note2->tabHeadWidth(stt) : ecr->width() * ecr->mag();        // defaults to headWidth()
    PointF pt;
    switch (sa1) {
    case SlurAnchor::STEM:                //sc can't be null
    {
        // place slur starting point at stem end point
        pt = StemLayout::stemPos(sc) - sc->pagePos() + sc->stem()->ldata()->line.p2();
        if (useTablature) {                           // in tabs, stems are centred on note:
            pt.rx() = hw1 * 0.5 + (note1 ? note1->bboxXShift() : 0.0);                      // skip half notehead to touch stem, anatoly-os: incorrect. half notehead width is not always the stem position
        }
        // clear the stem (x)
        // allow slight overlap (y)
        // don't allow overlap with hook if not disabling the autoplace checks against start/end segments in SlurSegment::layoutSegment()
        double yadj = -stemSideInset* sc->intrinsicMag();
        yadj *= _spatium * __up;
        double offset = std::max(stemOffsetX * sc->intrinsicMag(), minOffset);
        pt += PointF(offset * _spatium, yadj);
        // account for articulations
        fixArticulations(item, pt, sc, __up, true);
        // adjust for hook
        double fakeCutout = 0.0;
        if (!ctx.conf().styleB(Sid::useStraightNoteFlags)) {
            Hook* hook = sc->hook();
            // regular flags

            if (hook && hook->ldata()->bbox().translated(hook->pos()).contains(pt)) {
                // TODO: in the utopian far future where all hooks have SMuFL cutouts, this fakeCutout business will no
                // longer be used. for the time being fakeCutout describes a point on the line y=mx+b, out from the top of the stem
                // where y = yadj, m = fakeCutoutSlope, and x = y/m + fakeCutout
                fakeCutout = std::min(0.0, std::abs(yadj) - (hook->width() / fakeCutoutSlope));
                pt.rx() = StemLayout::stemPosX(sc) - fakeCutout;
            }
        } else {
            Hook* hook = sc->hook();
            // straight flags
            if (hook && hook->ldata()->bbox().translated(hook->pos()).contains(pt)) {
                double hookWidth = hook->width() * hook->mag();
                pt.rx() = (hookWidth * straightStemXOffset) + (hook->pos().x() + sc->x());
                if (item->up()) {
                    pt.ry() = sc->downNote()->pos().y() - stem1->height() - (beamClearance * _spatium * .7);
                } else {
                    pt.ry() = sc->upNote()->pos().y() + stem1->height() + (beamClearance * _spatium * .7);
                }
            }
        }
        sp->p1 += pt;
    }
    break;
    case SlurAnchor::NONE:
        break;
    }
    switch (sa2) {
    case SlurAnchor::STEM:                //ec can't be null
    {
        pt = StemLayout::stemPos(ec) - ec->pagePos() + ec->stem()->ldata()->line.p2();
        if (useTablature) {
            pt.rx() = hw2 * 0.5;
        }
        // don't allow overlap with beam
        double yadj;
        if (ec->beam() && ec->beam()->elements().front() != ec) {
            yadj = 0.75;
        } else if (ec->tremoloTwoChord() && ec->tremoloTwoChord()->chord2() == ec) {
            yadj = 0.75;
        } else {
            yadj = -stemSideInset;
        }
        yadj *= _spatium * __up;
        double offset = std::max(stemOffsetX * ec->intrinsicMag(), minOffset);
        pt += PointF(-offset * _spatium, yadj);
        // account for articulations
        fixArticulations(item, pt, ec, __up, true);
        sp->p2 += pt;
    }
    break;
    case SlurAnchor::NONE:
        break;
    }

    //
    // default position:
    //    horizontal: middle of notehead
    //    vertical:   _spatium * .4 above/below notehead
    //
    //------p1
    // Compute x0, y0 and stemPos
    if (sa1 == SlurAnchor::NONE || sa2 == SlurAnchor::NONE) {   // need stemPos if sa2 == SlurAnchor::NONE
        bool stemPos = false;       // p1 starts at chord stem side

        // default positions
        po.rx() = hw1 * .5 + (note1 ? note1->bboxXShift() : 0.0);
        if (note1) {
            po.ry() = note1->pos().y();
        } else if (item->up()) {
            po.ry() = scr->ldata()->bbox().top();
        } else {
            po.ry() = scr->ldata()->bbox().top() + scr->height();
        }
        double offset = useTablature ? 0.75 : 0.9;
        po.ry() += scr->intrinsicMag() * _spatium * offset * __up;

        // adjustments for stem and/or beam
        TremoloTwoChord* trem = sc ? sc->tremoloTwoChord() : nullptr;
        if (stem1 || trem) {     //sc not null
            Beam* beam1 = sc->beam();
            if (beam1 && (beam1->elements().back() != sc) && (sc->up() == item->up())) {
                TLayout::layoutBeam(beam1, ctx);
                // start chord is beamed but not the last chord of beam group
                // and slur direction is same as start chord (stem side)

                // in these cases, layout start of slur to stem
                double beamWidthSp = ctx.conf().styleS(Sid::beamWidth).val() * beam1->magS();
                double offset2 = std::max(beamClearance * sc->intrinsicMag(), minOffset) * _spatium;
                double sh = stem1->length() + (beamWidthSp / 2) + offset2;
                if (item->up()) {
                    po.ry() = StemLayout::stemPos(sc).y() - sc->pagePos().y() - sh;
                } else {
                    po.ry() = StemLayout::stemPos(sc).y() - sc->pagePos().y() + sh;
                }
                po.rx() = StemLayout::stemPosX(sc) + (beamAnchorInset * _spatium * sc->intrinsicMag()) + (stem1->lineWidthMag() / 2 * __up);

                // account for articulations
                fixArticulations(item, po, sc, __up, true);

                // force end of slur to layout to stem as well,
                // if start and end chords have same stem direction
                stemPos = true;
            } else if (trem && trem->chord2() != sc && sc->up() == item->up()) {
                TremoloLayout::layout(trem, ctx);
                Note* note = item->up() ? sc->upNote() : sc->downNote();
                double stemHeight = stem1 ? stem1->length() : defaultStemLengthStart(trem);
                double offset2 = std::max(beamClearance * sc->intrinsicMag(), minOffset) * _spatium;
                double sh = stemHeight + offset2;

                if (item->up()) {
                    po.ry() = StemLayout::stemPos(sc).y() - sc->pagePos().y() - sh;
                } else {
                    po.ry() = StemLayout::stemPos(sc).y() - sc->pagePos().y() + sh;
                }
                if (!stem1) {
                    po.rx() = note->noteheadCenterX();
                } else {
                    po.rx() = StemLayout::stemPosX(sc) + (beamAnchorInset * _spatium * sc->intrinsicMag())
                              + (stem1->lineWidthMag() / 2. * __up);
                }
                fixArticulations(item, po, sc, __up, true);

                stemPos = true;
            } else {
                // start chord is not beamed or is last chord of beam group
                // or slur direction is opposite that of start chord

                // at this point slur is in default position relative to note on slur side
                // but we may need to make further adjustments

                // if stem and slur are both up
                // we need to clear stem horizontally
                double stemOffsetMag = stemOffsetX * sc->intrinsicMag();
                if (sc->up() && item->up()) {
                    // stems in tab staves come from the middle of the head, which means it's much easier
                    // to just subtract an offset from the notehead center (which po already is)
                    if (useTablature) {
                        po.rx() += stemOffsetMag * _spatium;
                    } else {
                        po.rx() = hw1 + _spatium * stemOffsetMag;
                    }
                }

                //
                // handle case: stem up   - stem down
                //              stem down - stem up
                //
                if ((sc->up() != ecr->up()) && (sc->up() == item->up())) {
                    item->stemFloated().left = true;
                    // start and end chord have opposite direction
                    // and slur direction is same as start chord
                    // (so slur starts on stem side)

                    // float the start point along the stem to follow direction of movement
                    // see for example Gould p. 111

                    // get position of note on slur side for start & end chords
                    Note* n1  = sc->up() ? sc->upNote() : sc->downNote();
                    Note* n2  = 0;
                    if (ec) {
                        n2 = ec->up() ? ec->upNote() : ec->downNote();
                    }

                    // differential in note positions
                    double yd  = (n2 ? n2->pos().y() : ecr->pos().y()) - n1->pos().y();
                    yd *= .5;

                    // float along stem according to differential
                    double sh = stem1->height();
                    if (item->up() && yd < 0.0) {
                        po.ry() = std::max(po.y() + yd, sc->downNote()->pos().y() - sh - _spatium);
                    } else if (!item->up() && yd > 0.0) {
                        po.ry() = std::min(po.y() + yd, sc->upNote()->pos().y() + sh + _spatium);
                    }

                    // account for articulations
                    fixArticulations(item, po, sc, __up, true);

                    // we may wish to force end to align to stem as well,
                    // if it is in same direction
                    // (but it won't be, so this assignment should have no effect)
                    stemPos = true;
                } else {
                    // avoid articulations
                    fixArticulations(item, po, sc, __up, sc->up() == item->up());
                }
            }
        } else if (sc) {
            // avoid articulations
            fixArticulations(item, po, sc, __up, sc->up() == item->up());
        }

        // TODO: offset start position if there is another slur ending on this cr

        if (sa1 == SlurAnchor::NONE) {
            sp->p1 += po;
        }

        //------p2
        if (sa2 == SlurAnchor::NONE) {
            // default positions
            po.rx() = hw2 * .5 + (note2 ? note2->bboxXShift() : 0.0);
            if (note2) {
                po.ry() = note2->pos().y();
            } else if (item->up()) {
                po.ry() = item->endCR()->ldata()->bbox().top();
            } else {
                po.ry() = item->endCR()->ldata()->bbox().top() + item->endCR()->height();
            }
            double offset2 = useTablature ? 0.75 : 0.9;
            po.ry() += ecr->intrinsicMag() * _spatium * offset2 * __up;

            // adjustments for stem and/or beam
            TremoloTwoChord* trem2 = ec ? ec->tremoloTwoChord() : nullptr;
            if (stem2 || trem2) {       //ec can't be null
                Beam* beam2 = ec->beam();
                if ((stemPos && (scr->up() == ec->up()))
                    || (beam2
                        && (!beam2->elements().empty())
                        && (beam2->elements().front() != ec)
                        && (ec->up() == item->up())
                        && sc && (sc->noteType() == NoteType::NORMAL)
                        )
                    || (trem2 && ec->up() == item->up())
                    ) {
                    if (beam2) {
                        TLayout::layoutBeam(beam2, ctx);
                    }
                    if (trem2) {
                        TremoloLayout::layout(trem2, ctx);
                    }
                    // slur start was laid out to stem and start and end have same direction
                    // OR
                    // end chord is beamed but not the first chord of beam group
                    // and slur direction is same as end chord (stem side)
                    // and start chordrest is not a grace chord

                    // in these cases, layout end of slur to stem
                    double beamWidthSp = beam2 ? ctx.conf().styleS(Sid::beamWidth).val() : 0;
                    Note* note = item->up() ? ec->upNote() : ec->downNote();
                    double stemHeight = stem2 ? stem2->length() + (beamWidthSp / 2) : defaultStemLengthEnd(trem2);
                    double offset3 = std::max(beamClearance * ec->intrinsicMag(), minOffset) * _spatium;
                    double sh = stemHeight + offset3;

                    if (item->up()) {
                        po.ry() = StemLayout::stemPos(ec).y() - ec->pagePos().y() - sh;
                    } else {
                        po.ry() = StemLayout::stemPos(ec).y() - ec->pagePos().y() + sh;
                    }
                    if (!stem2) {
                        // tremolo whole notes
                        po.setX(note->noteheadCenterX());
                    } else {
                        po.setX(StemLayout::stemPosX(ec) + (stem2->lineWidthMag() / 2 * __up)
                                - (beamAnchorInset * _spatium * ec->intrinsicMag()));
                    }

                    // account for articulations
                    fixArticulations(item, po, ec, __up, true);
                } else {
                    // slur was not aligned to stem or start and end have different direction
                    // AND
                    // end chord is not beamed or is first chord of beam group
                    // or slur direction is opposite that of end chord

                    // if stem and slur are both down,
                    // we need to clear stem horizontally
                    double stemOffsetMag = stemOffsetX * ec->intrinsicMag();
                    if (!ec->up() && !item->up()) {
                        // stems in tab staves come from the middle of the head, which means it's much easier
                        // to just subtract an offset from the notehead center (which po already is)
                        if (useTablature) {
                            po.rx() -= stemOffsetMag * _spatium;
                        } else {
                            po.rx() = -_spatium * stemOffsetMag + note2->x();
                        }
                    } else if (useTablature && item->up() && ec->up()) {
                        // same as above
                        po.rx() -= _spatium * stemOffsetMag;
                    }

                    //
                    // handle case: stem up   - stem down
                    //              stem down - stem up
                    //
                    if ((scr->up() != ec->up()) && (ec->up() == item->up())) {
                        item->stemFloated().right = true;
                        // start and end chord have opposite direction
                        // and slur direction is same as end chord
                        // (so slur end on stem side)

                        // float the end point along the stem to follow direction of movement
                        // see for example Gould p. 111

                        Note* n1 = 0;
                        if (sc) {
                            n1 = sc->up() ? sc->upNote() : sc->downNote();
                        }
                        Note* n2 = ec->up() ? ec->upNote() : ec->downNote();

                        double yd = n2->pos().y() - (n1 ? n1->pos().y() : item->startCR()->pos().y());
                        yd *= .5;

                        double mh = stem2->height();
                        if (item->up() && yd > 0.0) {
                            po.ry() = std::max(po.y() - yd, ec->downNote()->pos().y() - mh - _spatium);
                        } else if (!item->up() && yd < 0.0) {
                            po.ry() = std::min(po.y() - yd, ec->upNote()->pos().y() + mh + _spatium);
                        }

                        // account for articulations
                        fixArticulations(item, po, ec, __up, true);
                    } else {
                        // avoid articulations
                        fixArticulations(item, po, ec, __up, ec->up() == item->up());
                    }
                }
            } else if (ec) {
                // avoid articulations
                fixArticulations(item, po, ec, __up, ec->up() == item->up());
            }
            // TODO: offset start position if there is another slur ending on this cr
            sp->p2 += po;
        }
    }

    if (item->staffType()->isTabStaff()) {
        SlurTieLayout::avoidPreBendsOnTab(sc, ec, sp);
    }

    if (item->isTappingHalfSlur()) {
        adjustForTappingHalfSlurs(toTappingHalfSlur(item), sp, note2);
    }
}

void SlurTieLayout::adjustForTappingHalfSlurs(TappingHalfSlur* item, SlurTiePos* sp, Note* endNote)
{
    if (item->staffType()->isTabStaff()) {
        return;
    }

    int staffLines = endNote->staff()->lines(endNote->tick());
    bool noteIsInsideStaff = endNote->line() > 1 && endNote->line() < 2 * (staffLines - 1) - 1;
    bool noteIsOnSpace = endNote->line() % 2 != 0;
    if (noteIsInsideStaff && noteIsOnSpace) {
        sp->p2.ry() += (item->up() ? 0.2 : -0.2) * item->spatium();
    }
}

void SlurTieLayout::fixArticulations(Slur* item, PointF& pt, Chord* c, double up, bool stemSide)
{
    //
    // handle special case of tenuto and staccato
    // yo = current offset of slur from chord position
    // return unchanged position, or position of outmost "close" articulation
    //
    double slurTipToArticVertDist = c->spatium() * 0.5 * up;
    double slurTipInwardAdjust = 0.1 * item->spatium();
    bool start = item->startCR() && item->startCR() == c;
    bool end = item->endCR() && item->endCR() == c;
    for (Articulation* a : c->articulations()) {
        if (!a->layoutCloseToNote() || !a->addToSkyline()) {
            continue;
        }
        // skip if articulation on stem side but slur is not or vice versa
        if ((a->up() == c->up()) != stemSide) {
            continue;
        }
        // Correct x-position inwards
        Note* note = c->up() ? c->downNote() : c->upNote();
        pt.rx() = a->x() + 0.5 * a->width() - note->x();
        if (start) {
            pt.rx() += slurTipInwardAdjust;
        } else if (end) {
            pt.rx() -= slurTipInwardAdjust;
        }
        // Adjust y-position
        if (a->up()) {
            pt.ry() = std::min(pt.y(), a->y() + a->height() / 2 * up + slurTipToArticVertDist);
        } else {
            pt.ry() = std::max(pt.y(), a->y() + a->height() / 2 * up + slurTipToArticVertDist);
        }
    }
}

void SlurTieLayout::adjustEndPoints(SlurSegment* slurSeg)
{
    double spatium = slurSeg->spatium();
    double lw = (slurSeg->staffType() ? slurSeg->staffType()->lineDistance().val() : 1.0) * spatium;
    const double staffLineMargin = 0.175 + (0.5 * slurSeg->style().styleS(Sid::staffLineWidth).val() * (spatium / lw));
    PointF p1 = slurSeg->ups(Grip::START).p;
    PointF p2 = slurSeg->ups(Grip::END).p;

    double y1sp = p1.y() / lw;
    double y2sp = p2.y() / lw;

    // point 1
    const Staff* staff = slurSeg->staff();
    int lines = staff ? staff->lines(slurSeg->tick()) : 0;
    auto adjustPoint = [staffLineMargin](bool up, double ysp) {
        double y1offset = ysp - floor(ysp);
        double adjust = 0;
        if (up) {
            if (y1offset < staffLineMargin) {
                // endpoint too close to the line above
                adjust = -(y1offset + staffLineMargin);
            } else if (y1offset > 1 - staffLineMargin) {
                // endpoint too close to the line below
                adjust = -(y1offset - (1 - staffLineMargin));
            }
        } else {
            if (y1offset < staffLineMargin) {
                // endpoint too close to the line above
                adjust = staffLineMargin - y1offset;
            }
            if (y1offset > 1 - staffLineMargin) {
                // endpoint too close to the line below
                adjust = (1 - y1offset) + staffLineMargin;
            }
        }
        return adjust;
    };
    if (y1sp > -staffLineMargin && y1sp < (lines - 1) + staffLineMargin) {
        slurSeg->ups(Grip::START).p.ry() += adjustPoint(slurSeg->slur()->up(), y1sp) * lw;
    }
    if (y2sp > -staffLineMargin && y2sp < (lines - 1) + staffLineMargin) {
        slurSeg->ups(Grip::END).p.ry() += adjustPoint(slurSeg->slur()->up(), y2sp) * lw;
    }
}

void SlurTieLayout::avoidCollisions(SlurSegment* slurSeg, PointF& pp1, PointF& p2, PointF& p3, PointF& p4,
                                    Transform& toSystemCoordinates, double& slurAngle)
{
    TRACEFUNC;
    Slur* slur = slurSeg->slur();
    double spatium = slurSeg->spatium();
    double slurLength = std::abs(p2.x() / spatium);
    bool slurUp = slur->up();
    int upSign = slurUp ? -1 : 1;

    ChordRest* startCR = slur->startCR();
    ChordRest* endCR = slur->endCR();

    if (!startCR || !endCR) {
        return;
    }

    Shape segShapes = getSegmentShapes(slurSeg, startCR, endCR); // Shape of the music under this slur
    addMinClearanceToShapes(segShapes, spatium, slurUp, startCR, endCR);

    if (segShapes.empty()) {
        return;
    }

    const double arcClearance = -upSign* computeArcClearance(spatium, slurLength, slurAngle);  // Collision clearance at the center of the slur

    // balance: determines how much endpoint adjustment VS shape adjustment we will do.
    // 0 = end point is fixed, only the shape can be adjusted,
    // 1 = shape is fixed, only end the point can be adjusted.
    double leftBalance = 0.0;
    double rightBalance = 0.0;
    computeAdjustmentBalance(slurSeg, startCR, endCR, leftBalance, rightBalance);

    static constexpr unsigned MAX_ITER = 30;     // Max iterations allowed

    double step = computeAdjustmentStep(upSign, spatium, slurLength);

    // Divide slur in several rectangles to localize collisions
    const unsigned npoints = 20;
    std::vector<RectF> slurRects;
    slurRects.reserve(npoints);

    // Define separate collision areas (left-mid-center)
    struct SlurCollision
    {
        bool left = false;
        bool mid = false;
        bool right = false;

        void reset()
        {
            left = false;
            mid = false;
            right = false;
        }
    };
    SlurCollision collision;

    // CHECK FOR COLLISIONS
    unsigned iter = 0;
    do {
        collision.reset();
        // Update tranform because pp1 may change
        toSystemCoordinates.reset();
        toSystemCoordinates.translate(pp1.x(), pp1.y());
        toSystemCoordinates.rotateRadians(slurAngle);
        // Create rectangles
        slurRects.clear();
        CubicBezier clearanceBezier(PointF(0, 0), p3 + PointF(0.0, arcClearance), p4 + PointF(0.0, arcClearance), p2);
        for (unsigned i = 0; i < npoints - 1; i++) {
            PointF clearancePoint1 = clearanceBezier.pointAtPercent(double(i) / double(npoints));
            PointF clearancePoint2 = clearanceBezier.pointAtPercent(double(i + 1) / double(npoints));
            clearancePoint1 = toSystemCoordinates.map(clearancePoint1);
            clearancePoint2 = toSystemCoordinates.map(clearancePoint2);
            slurRects.push_back(RectF(clearancePoint1, clearancePoint2));
        }
        // Check collisions
        for (unsigned i=0; i < slurRects.size(); i++) {
            bool leftSection = i < slurRects.size() / 3;
            bool midSection = i >= slurRects.size() / 3 && i < 2 * slurRects.size() / 3;
            bool rightSection = i >= 2 * slurRects.size() / 3;
            if ((leftSection && collision.left)
                || (midSection && collision.mid)
                || (rightSection && collision.right)) {         // If a collision is already found in this section, no need to check again
                continue;
            }
            bool intersection = slur->up() ? !Shape(slurRects[i]).clearsVertically(segShapes)
                                : !segShapes.clearsVertically(slurRects[i]);
            if (intersection) {
                if (leftSection) {
                    collision.left = true;
                }
                if (midSection) {
                    collision.mid = true;
                }
                if (rightSection) {
                    collision.right = true;
                }
            }
        }

        // In the even iterations, adjust the shape
        if (iter % 2 == 0) {
            static constexpr double SHAPE_PREFER_CENTER_FACTOR = 2.0;
            if (collision.left) {
                double shapeLeftStep = (1 - leftBalance) * step / SHAPE_PREFER_CENTER_FACTOR;
                // Move left Bezier point up(/down) and outwards
                p3 += PointF(-abs(shapeLeftStep), shapeLeftStep);
                // and a bit also the right point to compensate asymmetry
                p4 += PointF(std::abs(shapeLeftStep), shapeLeftStep) / 2.0;
            }
            if (collision.mid) {     // Move both Bezier points up(/down)
                double shapeLeftStep = (1 - leftBalance) * step;
                double shapeRightStep = (1 - rightBalance) * step;
                p3 += PointF(0.0, (shapeLeftStep + shapeRightStep) / 2);
                p4 += PointF(0.0, (shapeLeftStep + shapeRightStep) / 2);
            }
            if (collision.right) {
                double shapeRightStep = (1 - rightBalance) * step / SHAPE_PREFER_CENTER_FACTOR;
                // Move right Bezier point up(/down) and outwards
                p4 += PointF(std::abs(shapeRightStep), shapeRightStep);
                // and a bit also the left point to compensate asymmetry
                p3 += PointF(-abs(shapeRightStep), shapeRightStep) / 2.0;
            }
        } else if (!slurSeg->isEndPointsEdited()) {
            // In the odd iterations, adjust the end points
            // Slurs steeper than 45° are gently compensated
            static constexpr double STEEP_LIMIT = M_PI / 4;
            // If the collision is in mid region, move more the endpoint which reduces the slur slant
            static constexpr double SLANT_REDUCTION_ANGLE = 5 * M_PI / 180;
            static constexpr double SLANT_REDUCTION_LENGTH = 8;
            PointF p2SysCoords = toSystemCoordinates.map(p2);
            double curSlant = (p2SysCoords.y() - pp1.y()) / (p2SysCoords.x() - pp1.x());
            if (collision.left || collision.mid || slurAngle < -STEEP_LIMIT) {
                double endPointLeftStep = leftBalance * step;
                if (!collision.left && collision.mid && std::abs(curSlant) > SLANT_REDUCTION_ANGLE
                    && slurLength > SLANT_REDUCTION_LENGTH) {
                    double slantReductionRatio = std::abs(curSlant) / SLANT_REDUCTION_ANGLE;
                    bool leftIsOuter = (slurUp && curSlant > 0) || (!slurUp && curSlant < 0);
                    endPointLeftStep *= leftIsOuter ? (1 / slantReductionRatio) : slantReductionRatio;
                }
                // Lift the left end point, i.e. tilt the slur around p2
                double stepX = sin(slurAngle) * endPointLeftStep;
                double stepY = cos(slurAngle) * endPointLeftStep;
                PointF pp1delta = PointF(stepX, stepY);
                pp1 += PointF(0.0, endPointLeftStep);
                p3 += pp1delta * (p2.x() - p3.x()) / p2.x();
                p4 += pp1delta * (p2.x() - p4.x()) / p2.x();
                // All points are expressed with respect to pp1, so we need
                // to subtract pp1delta to avoid the whole slur moving up
                p2 -= pp1delta;
                p3 -= pp1delta;
                p4 -= pp1delta;
            }
            if (collision.right || collision.mid || slurAngle > STEEP_LIMIT) {
                double endPointRightStep = rightBalance * step;
                if (!collision.right && collision.mid && std::abs(curSlant) > SLANT_REDUCTION_ANGLE
                    && slurLength > SLANT_REDUCTION_LENGTH) {
                    double slantReductionRatio = std::abs(curSlant) / SLANT_REDUCTION_ANGLE;
                    bool rightIsOuter = (slurUp && curSlant < 0) || (!slurUp && curSlant > 0);
                    endPointRightStep *= rightIsOuter ? (1 / slantReductionRatio) : slantReductionRatio;
                }
                // Lift the right end point, i.e. tilt the slur around p1
                double stepX = sin(slurAngle) * endPointRightStep;
                double stepY = cos(slurAngle) * endPointRightStep;
                PointF p2delta = PointF(stepX, stepY);
                p2 += p2delta;
                p3 += p2delta * p3.x() / p2.x();
                p4 += p2delta * p4.x() / p2.x();
            }
        }
        // Enforce non-ugliness rules
        // 1) Slur cannot be taller than it is wide
        const double maxRelativeHeight = std::abs(p2.x());
        p3 = slur->up() ? PointF(p3.x(), std::max(p3.y(), -maxRelativeHeight)) : PointF(p3.x(), std::min(p3.y(), maxRelativeHeight));
        p4 = slur->up() ? PointF(p4.x(), std::max(p4.y(), -maxRelativeHeight)) : PointF(p4.x(), std::min(p4.y(), maxRelativeHeight));
        // 2) Tangent rule: p3 and p4 cannot be further left than p1 nor further right than p2
        PointF p3SysCoord = toSystemCoordinates.map(p3);
        PointF p4SysCoord = toSystemCoordinates.map(p4);
        PointF p2SysCoord = toSystemCoordinates.map(p2);
        p3SysCoord = PointF(std::max(pp1.x(), p3SysCoord.x()), p3SysCoord.y());
        p3SysCoord = PointF(std::min(p2SysCoord.x(), p3SysCoord.x()), p3SysCoord.y());
        p4SysCoord = PointF(std::max(pp1.x(), p4SysCoord.x()), p4SysCoord.y());
        p4SysCoord = PointF(std::min(p2SysCoord.x(), p4SysCoord.x()), p4SysCoord.y());
        p3 = toSystemCoordinates.inverted().map(p3SysCoord);
        p4 = toSystemCoordinates.inverted().map(p4SysCoord);

        ++iter;
    } while ((collision.left || collision.mid || collision.right) && iter < MAX_ITER);
}

Shape SlurTieLayout::getSegmentShapes(SlurSegment* slurSeg, ChordRest* startCR, ChordRest* endCR)
{
    Shape segShapes;

    // Determine start and end segments for collision checks
    Segment* startSeg = nullptr;
    if (slurSeg->isSingleBeginType()) {
        if (startCR->isChord() && toChord(startCR)->isGraceAfter()) {
            // if this is a grace-note-after, the shape is stored the *appended* segment
            Chord* parent = toChord(startCR->parentItem());
            if (parent) {
                startSeg = parent->graceNotesAfter().appendedSegment();
            }
        } else {
            startSeg = startCR->segment(); // first of the slur
        }
    } else {
        startSeg = slurSeg->system()->firstMeasure()->findFirstR(SegmentType::ChordRest, Fraction(0, 0)); // first of the system
    }

    Segment* endSeg = nullptr;
    if (slurSeg->isSingleEndType()) {
        if (endCR->isChord() && toChord(endCR)->isGraceAfter()) {
            // if this is a grace-note-after, the shape is stored the *appended* segment
            Chord* parent = toChord(endCR->parentItem());
            if (parent) {
                endSeg = parent->graceNotesAfter().appendedSegment();
            }
        } else {
            endSeg = endCR->segment(); // last of the slur
        }
    } else {
        endSeg = slurSeg->system()->lastMeasure()->last(); // last of the system
    }

    if (!startSeg || !endSeg) {
        return segShapes;
    }

    for (Segment* seg = startSeg; seg && (seg->isBefore(endSeg) || seg == endSeg); seg = seg->next1enabled()) {
        if (seg->isType(SegmentType::BarLineType) || seg->isBreathType() || seg->hasTimeSigAboveStaves()) {
            continue;
        }
        segShapes.add(getSegmentShape(slurSeg, seg, startCR, endCR));
    }

    return segShapes;
}

Shape SlurTieLayout::getSegmentShape(SlurSegment* slurSeg, Segment* seg, ChordRest* startCR, ChordRest* endCR)
{
    Slur* slur = slurSeg->slur();
    staff_idx_t startStaffIdx = startCR->staffIdx();
    staff_idx_t endStaffIdx = endCR->staffIdx();
    Shape segShape = seg->staffShape(startStaffIdx).translated(seg->pos() + seg->measure()->pos());

    // If cross-staff, also add the shape of second staff
    if (slur->isCrossStaff() && seg != startCR->segment()) {
        endStaffIdx = (endCR->staffIdx() != startStaffIdx) ? endCR->staffIdx() : endCR->vStaffIdx();
        SysStaff* startStaff = slurSeg->system()->staves().at(startStaffIdx);
        SysStaff* endStaff = slurSeg->system()->staves().at(endStaffIdx);
        double dist = endStaff->y() - startStaff->y();
        Shape secondStaffShape = seg->staffShape(endStaffIdx).translated(seg->pos() + seg->measure()->pos()); // translate horizontally
        secondStaffShape.translate(PointF(0.0, dist)); // translate vertically
        segShape.add(secondStaffShape);
    }

    for (track_idx_t track = staff2track(startStaffIdx); track < staff2track(endStaffIdx, VOICES); ++track) {
        EngravingItem* e = seg->elementAt(track);
        if (!e || !e->isChordRest()) {
            continue;
        }
        // Gets tie and 2 note tremolo shapes
        if (e->isChord()) {
            Chord* chord = toChord(e);
            if (chord->tremoloTwoChord()) {
                segShape.add(chord->tremoloTwoChord()->shape());
            }
            for (Note* note : toChord(e)->notes()) {
                Tie* tieFor = note->tieFor();
                Tie* tieBack = note->tieBack();
                if (tieFor && tieFor->up() == slur->up() && !tieFor->segmentsEmpty()) {
                    TieSegment* tieSegment = tieFor->frontSegment();
                    if (tieSegment->isSingleBeginType()) {
                        segShape.add(tieSegment->shape());
                    }
                }
                if (tieBack && tieBack->up() == slur->up() && !tieBack->segmentsEmpty()) {
                    TieSegment* tieSegment = tieBack->backSegment();
                    if (tieSegment->isEndType()) {
                        segShape.add(tieSegment->shape());
                    }
                }
            }
        }
    }

    // Remove items that the slur shouldn't try to avoid
    segShape.remove_if([&](ShapeElement& shapeEl) {
        if (!shapeEl.item() || !shapeEl.item()->parentItem() || !shapeEl.item()->visible()) {
            return true;
        }
        const EngravingItem* item = shapeEl.item();
        const EngravingItem* parent = item->parentItem();
        // Don't remove arpeggio starting on a different voice and ending on the same voice as endCR when slur is on the outside
        if (item->isArpeggio() && (endCR->track() == toArpeggio(item)->endTrack()) && endCR->tick() == item->tick()
            && (!slur->up() && toArpeggio(item)->span() > 1)) {
            return false;
        }

        // Its own startCR or items belonging to it, lyrics, fingering, ledger lines, articulation on endCR
        if (item == startCR || parent == startCR || item->isTextBase() || item->isLedgerLine()
            || (item->isArticulationFamily() && parent == endCR) || item->isBend() || item->isTappingHalfSlurSegment()) {
            return true;
        }
        // Ornament accidentals on start or end chord
        if (item->isAccidental() && parent->isOrnament()) {
            EngravingItem* parentParent = parent->parentItem();
            if (parentParent && (parentParent == startCR || parentParent == endCR)) {
                return true;
            }
        }
        // Items that are on the start segment but in a different voice
        if ((item->tick() == startCR->tick() && item->track() != startCR->track())
            || (item->tick() == endCR->tick() && item->track() != endCR->track())) {
            return true;
        }
        // Edge-case: multiple voices and slur is on the inside
        if (item->vStaffIdx() == startCR->staffIdx()
            && ((!slur->up() && item->track() > startCR->track()) // slur-down: ignore lower voices
                || (slur->up() && item->track() < startCR->track()))) { // slur-up: ignore higher voices
            return true;
        }
        // Remove arpeggios spanning more than 1 voice starting on endCR's voice when the slur is on the inside
        if (item->isArpeggio() && (endCR->track() != item->track() || (!slur->up() && toArpeggio(item)->span() > 1))) {
            return true;
        }
        // Ignore big time signatures
        if (item->isTimeSig() && toTimeSig(item)->timeSigPlacement() != TimeSigPlacement::NORMAL) {
            return true;
        }
        // Ignore fermatas
        if (item->isFermata()) {
            return true;
        }
        return false;
    });

    return segShape;
}

void SlurTieLayout::addMinClearanceToShapes(Shape& segShapes, double spatium, bool slurUp, const ChordRest* startCR, const ChordRest* endCR)
{
    const double noteClearance = 0.4 * spatium;
    const double articulationClearance = 0.20 * spatium;
    const double itemClearance = 0.1 * spatium;

    for (ShapeElement& shapeEl : segShapes.elements()) {
        const EngravingItem* item = shapeEl.item();
        if (!item || item->parent() == startCR || item->parent() == endCR) {
            continue;
        }

        double clearance = 0.0;
        switch (item->type()) {
        case ElementType::NOTE:
            clearance = noteClearance;
            break;
        case ElementType::ARTICULATION:
        case ElementType::ORNAMENT:
        case ElementType::TAPPING:
            clearance = articulationClearance;
            break;
        default:
            clearance = itemClearance;
            break;
        }

        slurUp ? shapeEl.adjust(0.0, -clearance, 0.0, 0.0) : shapeEl.adjust(0.0, 0.0, 0.0, clearance);
    }
}

double SlurTieLayout::computeArcClearance(double spatium, double slurLength, double slurAngle)
{
    static constexpr double CLEARANCE_TO_LENGTH_RATIO = 0.04;
    static constexpr double MAX_CLEARANCE = 0.5;
    static constexpr double MIN_CLEARANCE = 0.1;

    double clearance = CLEARANCE_TO_LENGTH_RATIO * slurLength * std::pow(cos(slurAngle), 2);
    clearance = std::clamp(clearance, MIN_CLEARANCE, MAX_CLEARANCE);

    return clearance * spatium;
}

void SlurTieLayout::computeAdjustmentBalance(SlurSegment* slurSeg, const ChordRest* startCR, const ChordRest* endCR,  double& leftBalance,
                                             double& rightBalance)
{
    Slur* slur = slurSeg->slur();
    if (slurSeg->isSingleBeginType() && !slur->stemFloated().left) {
        if (slurSeg->isBeginType() || hasArticulationAbove(slurSeg, startCR) || slur->isCrossStaff()) {
            leftBalance = 0.1;
        } else {
            leftBalance = 0.4;
        }
    } else {
        leftBalance = 0.9;
    }

    if (slurSeg->isSingleEndType() && !slur->stemFloated().right) {
        if (slurSeg->isEndType() || hasArticulationAbove(slurSeg, endCR) || slur->isCrossStaff()) {
            rightBalance = 0.1;
        } else {
            rightBalance = 0.4;
        }
    } else {
        rightBalance = 0.9;
    }
}

bool SlurTieLayout::hasArticulationAbove(SlurSegment* slurSeg, const ChordRest* chordRest)
{
    IF_ASSERT_FAILED(chordRest) {
        return false;
    }

    if (!chordRest->isChord()) {
        return false;
    }

    const Chord* chord = toChord(chordRest);

    bool slurUp = slurSeg->slur()->up();
    for (const Articulation* artic : chord->articulations()) {
        if (!artic->layoutCloseToNote() && artic->up() == slurUp) {
            return true;
        }
    }

    if (Ornament* ornament = chord->findOrnament()) {
        if (ornament->up() == slurUp) {
            return true;
        }
    }

    return false;
}

double SlurTieLayout::computeAdjustmentStep(int upSign, double spatium, double slurLength)
{
    double step = upSign * 0.30 * spatium;

    static constexpr double LONG_SLUR_LIMIT = 16.0; // in spaces
    if (slurLength > LONG_SLUR_LIMIT) {
        step *= slurLength / LONG_SLUR_LIMIT;
        step = std::min(step, 1.5 * spatium);
    }

    return step;
}

bool SlurTieLayout::stemSideForBeam(Slur* slur, bool start)
{
    // determines if the anchor point is exempted from the stem inset due to beams or tremolos.
    if (!slur) {
        return false;
    }

    ChordRest* cr = start ? slur->startCR() : slur->endCR();
    Chord* c = toChord(cr);
    bool adjustForBeam = cr && cr->beam() && cr->up() == slur->up();
    if (start) {
        adjustForBeam = adjustForBeam && cr->beam()->elements().back() != cr;
    } else {
        adjustForBeam = adjustForBeam && cr->beam()->elements().front() != cr;
    }
    if (adjustForBeam) {
        return true;
    }

    bool adjustForTrem = false;
    TremoloTwoChord* trem = c ? c->tremoloTwoChord() : nullptr;
    adjustForTrem = trem && trem->up() == slur->up();
    if (start) {
        adjustForTrem = adjustForTrem && trem->chord2() != c;
    } else {
        adjustForTrem = adjustForTrem && trem->chord1() != c;
    }
    return adjustForTrem;
}

bool SlurTieLayout::isOverBeams(Slur* slur)
{
    // returns true if all the chords spanned by the slur are beamed, and all beams are on the same side of the slur
    const ChordRest* startCR = slur->startCR();
    const ChordRest* endCR = slur->endCR();
    if (!startCR || !endCR) {
        return false;
    }
    if (startCR->track() != endCR->track()
        || startCR->tick() >= endCR->tick()) {
        return false;
    }
    size_t track = startCR->track();
    Segment* seg = startCR->segment();
    while (seg && seg->tick() <= endCR->tick()) {
        if (!seg->isChordRestType()
            || !seg->elist().at(track)
            || !seg->elist().at(track)->isChordRest()) {
            return false;
        }
        ChordRest* cr = toChordRest(seg->elist().at(track));
        bool hasBeam = cr->beam() && cr->up() == slur->up();
        bool hasTrem = false;
        if (cr->isChord()) {
            Chord* c = toChord(cr);
            hasTrem = c->tremoloTwoChord() && c->up() == slur->up();
        }
        if (!(hasBeam || hasTrem)) {
            return false;
        }
        if ((!seg->next() || seg->next()->isEndBarLineType()) && seg->measure()->nextMeasure()) {
            seg = seg->measure()->nextMeasure()->first();
        } else {
            seg = seg->next();
        }
    }
    return true;
}

void SlurTieLayout::avoidPreBendsOnTab(const Chord* sc, const Chord* ec, SlurTiePos* sp)
{
    GuitarBend* bendOnStart = nullptr;
    GuitarBend* bendOnEnd = nullptr;
    if (sc) {
        for (Note* note : sc->notes()) {
            GuitarBend* bf = note->bendFor();
            GuitarBend* bb = note->bendBack();
            if (bf && !bf->segmentsEmpty() && bf->type() == GuitarBendType::PRE_BEND && !bf->angledPreBend()) {
                bendOnStart = bf;
            } else if (bb && !bb->segmentsEmpty() && bb->type() == GuitarBendType::PRE_BEND && !bb->angledPreBend()) {
                bendOnStart = bb;
            }
            if (bendOnStart) {
                break;
            }
        }
    }
    if (ec) {
        for (Note* note : ec->notes()) {
            GuitarBend* bf = note->bendFor();
            GuitarBend* bb = note->bendBack();
            if (bf && !bf->segmentsEmpty() && bf->type() == GuitarBendType::PRE_BEND && !bf->angledPreBend()) {
                bendOnEnd = bf;
            } else if (bb && !bb->segmentsEmpty() && bb->type() == GuitarBendType::PRE_BEND && !bb->angledPreBend()) {
                bendOnEnd = bb;
            }
            if (bendOnEnd) {
                break;
            }
        }
    }

    if (bendOnStart) {
        sp->p1.rx() = std::max(sp->p1.x(), bendOnStart->frontSegment()->pos().x() + 0.33 * sc->spatium());
    }
    if (bendOnEnd) {
        sp->p2.rx() = std::min(sp->p2.x(), bendOnEnd->frontSegment()->pos().x() - 0.33 * ec->spatium());
    }
}

static bool tieSegmentShouldBeSkipped(Tie* item)
{
    Note* startNote = item->startNote();
    StaffType* st = item->staff()->staffType(startNote ? startNote->tick() : Fraction(0, 1));
    if (!st || !st->isTabStaff()) {
        return false;
    }

    if (startNote && startNote->isContinuationOfBend()) {
        return true;
    }

    ShowTiedFret showTiedFret = item->style().value(Sid::tabShowTiedFret).value<ShowTiedFret>();

    return showTiedFret == ShowTiedFret::NONE;
}

TieSegment* SlurTieLayout::layoutTieFor(Tie* item, System* system)
{
    item->setPos(0, 0);

    if (!item->startNote()) {
        LOGD("no start note");
        return nullptr;
    }

    // do not layout ties in tablature if not showing back-tied fret marks
    if (tieSegmentShouldBeSkipped(item)) {
        if (!item->segmentsEmpty()) {
            item->eraseSpannerSegments();
        }

        return nullptr;
    }

    if (item->isPartialTie()) {
        return layoutPartialTie(toPartialTie(item));
    }

    calculateDirection(item);
    calculateIsInside(item);

    SlurTiePos sPos;
    sPos.p1 = computeDefaultStartOrEndPoint(item, Grip::START);

    computeStartAndEndSystem(item, sPos);

    int segmentCount = sPos.system1 == sPos.system2 ? 1 : 2;
    if (segmentCount == 2) {
        sPos.p2 = PointF(system->endingXForOpenEndedLines(), sPos.p1.y());
    } else {
        sPos.p2 = computeDefaultStartOrEndPoint(item, Grip::END);
    }

    correctForCrossStaff(item, sPos, sPos.system1 != sPos.system2 ? SpannerSegmentType::BEGIN : SpannerSegmentType::SINGLE);
    forceHorizontal(item, sPos);

    item->fixupSegments(segmentCount);
    TieSegment* segment = item->segmentAt(0);
    segment->setTrack(item->track());
    segment->setSpannerSegmentType(sPos.system1 != sPos.system2 ? SpannerSegmentType::BEGIN : SpannerSegmentType::SINGLE);
    segment->setSystem(system);   // Needed to populate System.spannerSegments
    segment->resetAdjustmentOffset();
    segment->mutldata()->allJumpPointsInactive = item->allJumpPointsInactive();

    const Chord* startChord = item->startNote()->chord();
    item->setTick(startChord->tick()); // Why is this here?? (M.S.)

    if (segment->autoplace() && !segment->isEdited()) {
        adjustX(segment, sPos, Grip::START);
        if (segment->isSingleType()) {
            adjustX(segment, sPos, Grip::END);
        }
    }

    adjustYforLedgerLines(segment, sPos);

    segment->ups(Grip::START).p = sPos.p1;
    segment->ups(Grip::END).p = sPos.p2;

    if (segment->autoplace() && !segment->isEdited()) {
        adjustY(segment);
    } else {
        computeBezier(segment);
    }

    addLineAttachPoints(segment); // add attach points to start and end note
    return segment;
}

TieSegment* SlurTieLayout::layoutTieBack(Tie* item, System* system, LayoutContext& ctx)
{
    Chord* chord = item->endNote() ? item->endNote()->chord() : nullptr;

    if (item->staffType() && item->staffType()->isTabStaff()) {
        // On TAB, the presence of this tie may require to add a parenthesis
        ChordLayout::layout(chord, ctx);
    }
    // do not layout ties in tablature if not showing back-tied fret marks
    if (tieSegmentShouldBeSkipped(item)) {
        if (!item->segmentsEmpty()) {
            item->eraseSpannerSegments();
        }

        return nullptr;
    }

    if (item->isPartialTie()) {
        return layoutPartialTie(toPartialTie(item));
    }

    SlurTiePos sPos;
    computeStartAndEndSystem(item, sPos);
    sPos.p2 = computeDefaultStartOrEndPoint(item, Grip::END);

    double x = system ? system->firstNoteRestSegmentX(true) : 0;
    double y = sPos.p2.y();
    sPos.p1 = PointF(x, y);

    item->fixupSegments(2);
    TieSegment* segment = item->segmentAt(1);
    segment->setTrack(item->track());
    segment->setSystem(system);
    segment->resetAdjustmentOffset();

    if (chord) {
        segment->setStaffMove(static_cast<int>(chord->vStaffIdx() - segment->staffIdx()));
    }

    adjustY(segment);
    segment->setSpannerSegmentType(SpannerSegmentType::END);

    if (segment->autoplace() && !segment->isEdited()) {
        adjustX(segment, sPos, Grip::END);
    }

    adjustYforLedgerLines(segment, sPos);

    segment->ups(Grip::START).p = sPos.p1;
    segment->ups(Grip::END).p = sPos.p2;

    if (segment->autoplace() && !segment->isEdited()) {
        adjustY(segment);
    } else {
        computeBezier(segment);
    }

    addLineAttachPoints(segment);
    return segment;
}

void SlurTieLayout::computeStartAndEndSystem(Tie* item, SlurTiePos& slurTiePos)
{
    Chord* startChord = item->startNote() ? item->startNote()->chord() : nullptr;
    Chord* endChord = item->endNote() ? item->endNote()->chord() : nullptr;

    System* startSystem = startChord ? startChord->measure()->system() : nullptr;

    System* endSystem = endChord ? endChord->measure()->system() : startSystem;

    if (!startSystem && !endSystem) {
        if (startChord) {
            Measure* m = startChord->measure();
            LOGD("No system: measure is %d has %d count %d", m->isMMRest(), m->hasMMRest(), m->mmRestCount());
        } else {
            LOGD("No start or end system for tie at ") << item->tick().toString();
        }
    }

    slurTiePos.system1 = startSystem;
    slurTiePos.system2 = endSystem;
}

PointF SlurTieLayout::computeDefaultStartOrEndPoint(const Tie* tie, Grip startOrEnd)
{
    if (startOrEnd != Grip::START && startOrEnd != Grip::END) {
        return PointF();
    }

    bool start = startOrEnd == Grip::START;

    Note* note = start ? tie->startNote() : tie->endNote();
    Chord* chord = note ? note->chord() : nullptr;

    if (!chord) {
        return PointF();
    }

    PointF result = note->pos() + chord->pos() + chord->segment()->pos() + chord->measure()->pos();

    const bool up = tie->up();
    const bool inside = tie->isInside();
    const bool noteIsHiddenFret = note->shouldHideFret();
    const int upSign = up ? -1 : 1;
    const int leftRightSign = start ? +1 : -1;
    const double noteWidth = note->width();
    const double noteHeight = note->height();
    const double spatium = tie->spatium();

    double baseX = (inside && !noteIsHiddenFret) ? (start ? noteWidth : 0.0) : noteOpticalCenterForTie(note, up);
    double baseY = inside ? 0.0 : upSign * noteHeight / 2;

    result += PointF(baseX, baseY);

    double visualInsetSp = 0.0;
    if (inside || note->headGroup() == NoteHeadGroup::HEAD_SLASH || noteIsHiddenFret) {
        visualInsetSp = 0.2;
    } else if (note->hasAnotherStraightAboveOrBelow(up)) {
        visualInsetSp = 0.45;
    } else {
        visualInsetSp = 0.1;
    }

    double visualInset = visualInsetSp * spatium * leftRightSign;
    const double yOffset = 0.20 * spatium * upSign; // TODO: style

    result += PointF(visualInset, yOffset);

    return result;
}

double SlurTieLayout::noteOpticalCenterForTie(const Note* note, bool up)
{
    if (note->headGroup() == NoteHeadGroup::HEAD_SLASH) {
        double singleSlashWidth = note->symBbox(SymId::noteheadSlashHorizontalEnds).width();
        double noteWidth = note->headWidth();
        double center = 0.20 * note->spatium() + 0.5 * (noteWidth - singleSlashWidth);
        return up ? note->ldata()->bbox().right() - center : note->ldata()->bbox().left() + center;
    }
    SymId symId = note->ldata()->cachedNoteheadSym.value();
    PointF cutOutLeft = note->symSmuflAnchor(symId, up ? SmuflAnchorId::cutOutNW : SmuflAnchorId::cutOutSW);
    PointF cutOutRight = note->symSmuflAnchor(symId, up ? SmuflAnchorId::cutOutNE : SmuflAnchorId::cutOutSE);

    if (cutOutLeft.isNull() || cutOutRight.isNull()) {
        return 0.5 * note->width();
    }

    return 0.5 * (cutOutLeft.x() + cutOutRight.x());
}

void SlurTieLayout::createSlurSegments(Slur* item, LayoutContext& ctx)
{
    const ChordRest* startCR = item->startCR();
    const ChordRest* endCR = item->endCR();
    assert(startCR && endCR);

    const System* startSys = startCR->measure()->system();
    const System* endSys = endCR->measure()->system();

    const std::vector<System*>& systems = ctx.dom().systems();
    system_idx_t startSysIdx = muse::indexOf(systems, startSys);
    system_idx_t endSysIdx = muse::indexOf(systems, endSys);
    if (startSysIdx == muse::nidx || endSysIdx == muse::nidx) {
        return;
    }

    int segmentsNeeded = 0;
    for (system_idx_t i = startSysIdx; i <= endSysIdx; ++i) {
        if (systems.at(i)->vbox()) {
            continue;
        }
        ++segmentsNeeded;
    }
    int currentSegments = int(item->spannerSegments().size());

    if (currentSegments != segmentsNeeded) {
        item->fixupSegments(segmentsNeeded);
    }

    int segIdx = 0;
    for (system_idx_t i = startSysIdx; i <= endSysIdx; ++i) {
        System* system = systems.at(i);
        if (system->vbox()) {
            continue;
        }
        SlurSegment* lineSegm = item->segmentAt(segIdx++);
        lineSegm->setSystem(system);
        if (startSysIdx == endSysIdx) {
            lineSegm->setSpannerSegmentType(SpannerSegmentType::SINGLE);
        } else if (i == startSysIdx) {
            lineSegm->setSpannerSegmentType(SpannerSegmentType::BEGIN);
        } else if (i > 0 && i != endSysIdx) {
            lineSegm->setSpannerSegmentType(SpannerSegmentType::MIDDLE);
        } else if (i == endSysIdx) {
            lineSegm->setSpannerSegmentType(SpannerSegmentType::END);
        }
    }
}

void SlurTieLayout::adjustOverlappingSlurs(const std::list<SpannerSegment*>& spannerSegments)
{
    std::vector<SlurSegment*> segments;
    for (SpannerSegment* seg : spannerSegments) {
        if (seg->isSlurSegment()) {
            segments.push_back(toSlurSegment(seg));
        }
    }
    if (segments.size() <= 1) {
        return;
    }

    //how far vertically an endpoint should adjust to avoid other slur endpoints:
    const double spatium = segments.front()->spatium();
    const double slurCollisionVertOffset = 0.65 * spatium;
    const double slurCollisionHorizOffset = 0.2 * spatium;
    const double fuzzyHorizCompare = 0.25 * spatium;
    auto compare = [fuzzyHorizCompare](double x1, double x2) { return std::abs(x1 - x2) < fuzzyHorizCompare; };
    for (SlurSegment* slur1 : segments) {
        for (SlurSegment* slur2 : segments) {
            if (slur2 == slur1) {
                continue;
            }
            if (slur1->slur()->endChord() == slur2->slur()->startChord()
                && compare(slur1->ups(Grip::END).p.y(), slur2->ups(Grip::START).p.y())) {
                slur1->ups(Grip::END).p.rx() -= slurCollisionHorizOffset;
                slur2->ups(Grip::START).p.rx() += slurCollisionHorizOffset;
                SlurTieLayout::computeBezier(slur1);
                SlurTieLayout::computeBezier(slur2);
                continue;
            }

            SlurTieSegment* slurTie2 = toSlurTieSegment(slur2);

            // slurs don't collide with themselves or slurs on other staves
            if (slur1->vStaffIdx() != slurTie2->vStaffIdx()) {
                continue;
            }
            // slurs which don't overlap don't need to be checked
            if (slur1->ups(Grip::END).p.x() < slurTie2->ups(Grip::START).p.x()
                || slurTie2->ups(Grip::END).p.x() < slur1->ups(Grip::START).p.x()
                || slur1->slur()->up() != slurTie2->slurTie()->up()) {
                continue;
            }
            // START POINT
            if (compare(slur1->ups(Grip::START).p.x(), slurTie2->ups(Grip::START).p.x())) {
                if (slur1->ups(Grip::END).p.x() > slurTie2->ups(Grip::END).p.x() || slurTie2->isTieSegment()) {
                    // slur1 is the "outside" slur
                    slur1->ups(Grip::START).p.ry() += slurCollisionVertOffset * (slur1->slur()->up() ? -1 : 1);
                    SlurTieLayout::computeBezier(slur1);
                }
            }
            // END POINT
            if (compare(slur1->ups(Grip::END).p.x(), slurTie2->ups(Grip::END).p.x())) {
                // slurs have the same endpoint
                if (slur1->ups(Grip::START).p.x() < slurTie2->ups(Grip::START).p.x() || slurTie2->isTieSegment()) {
                    // slur1 is the "outside" slur
                    slur1->ups(Grip::END).p.ry() += slurCollisionVertOffset * (slur1->slur()->up() ? -1 : 1);
                    SlurTieLayout::computeBezier(slur1);
                }
            }
        }
    }
}

LaissezVibSegment* SlurTieLayout::createLaissezVibSegment(LaissezVib* item)
{
    Chord* startChord = item->startNote()->chord();
    item->setTick(startChord->tick());

    calculateDirection(item);
    calculateIsInside(item);

    item->fixupSegments(1);
    LaissezVibSegment* segment = item->segmentAt(0);
    segment->setSpannerSegmentType(SpannerSegmentType::SINGLE);
    segment->setTrack(item->track());
    segment->setSystem(item->startNote()->chord()->segment()->measure()->system());
    segment->resetAdjustmentOffset();

    return segment;
}

void SlurTieLayout::calculateLaissezVibX(LaissezVibSegment* segment, SlurTiePos& sPos, bool smufl)
{
    LaissezVib* lv = segment->laissezVib();

    computeStartAndEndSystem(lv, sPos);
    sPos.p1 = computeDefaultStartOrEndPoint(lv, Grip::START);

    if (segment->autoplace() && !segment->isEdited()) {
        adjustX(segment, sPos, Grip::START);
    }

    if (smufl) {
        LaissezVibSegment::LayoutData* ldata = segment->mutldata();
        ldata->symbol = lv->symId();
        ldata->setBbox(segment->symBbox(ldata->symbol));
        ldata->setShape(Shape(ldata->bbox(), segment));
        ldata->setMag(segment->laissezVib()->startNote()->mag());
    }

    const double width = smufl ? segment->width() : lv->absoluteFromSpatium(lv->minLength());

    sPos.p2 = PointF(sPos.p1.x() + width, sPos.p1.y());
}

void SlurTieLayout::calculateLaissezVibY(LaissezVibSegment* segment, SlurTiePos& sPos)
{
    LaissezVib* lv = segment->laissezVib();
    correctForCrossStaff(lv, sPos, SpannerSegmentType::SINGLE);

    adjustYforLedgerLines(segment, sPos);

    segment->ups(Grip::START).p = sPos.p1;
    segment->ups(Grip::END).p = sPos.p2;

    if (segment->autoplace() && !segment->isEdited()) {
        adjustY(segment);
    } else {
        computeBezier(segment);
    }
}

PartialTieSegment* SlurTieLayout::createPartialTieSegment(PartialTie* item)
{
    Chord* chord = item->isOutgoing() ? item->startNote()->chord() : item->endNote()->chord();
    item->setTick(chord->tick());

    calculateDirection(item);
    calculateIsInside(item);

    item->fixupSegments(1);
    PartialTieSegment* segment = item->segmentAt(0);
    segment->setSpannerSegmentType(SpannerSegmentType::SINGLE);
    segment->setSystem(chord->segment()->measure()->system());
    segment->setTrack(item->track());
    segment->resetAdjustmentOffset();
    segment->mutldata()->allJumpPointsInactive = item->allJumpPointsInactive();

    return segment;
}

PartialTieSegment* SlurTieLayout::layoutPartialTie(PartialTie* item)
{
    const bool outgoing = item->isOutgoing();
    PartialTieSegment* segment = createPartialTieSegment(item);
    SlurTiePos sPos;

    computeStartAndEndSystem(item, sPos);
    if (outgoing) {
        sPos.p1 = computeDefaultStartOrEndPoint(item, Grip::START);
    } else {
        sPos.p2 = computeDefaultStartOrEndPoint(item, Grip::END);
    }

    if (segment->autoplace() && !segment->isEdited()) {
        adjustX(segment, sPos, outgoing ? Grip::START : Grip::END);
    }

    setPartialTieEndPos(item, sPos);

    correctForCrossStaff(item, sPos, SpannerSegmentType::SINGLE);

    adjustYforLedgerLines(segment, sPos);

    segment->ups(Grip::START).p = sPos.p1;
    segment->ups(Grip::END).p = sPos.p2;

    if (segment->autoplace() && !segment->isEdited()) {
        adjustY(segment);
    } else {
        computeBezier(segment);
    }

    addLineAttachPoints(segment);

    return segment;
}

void SlurTieLayout::setPartialTieEndPos(PartialTie* item, SlurTiePos& sPos)
{
    const bool outgoing = item->isOutgoing();

    const Chord* chord = item->parentNote()->chord();
    const Segment* seg = chord->segment();
    const Measure* measure = seg->measure();
    const System* system = measure->system();

    if (seg->measure()->isFirstInSystem() && !outgoing) {
        sPos.p1 = PointF((system ? system->firstNoteRestSegmentX(true) : 0), sPos.p2.y());
        return;
    }

    auto shouldSkipSegment = [](const Segment* adjSeg, staff_idx_t staff) {
        bool inactiveOrInvisible = !adjSeg->isActive() || !adjSeg->enabled() || adjSeg->allElementsInvisible()
                                   || !adjSeg->hasElements(staff);
        bool isAboveStaff = adjSeg->isBreathType() || adjSeg->hasTimeSigAboveStaves();
        return inactiveOrInvisible || isAboveStaff;
    };

    const Segment* adjSeg = outgoing ? seg->next1() : seg->prev1();
    while (adjSeg && shouldSkipSegment(adjSeg, item->vStaffIdx())) {
        adjSeg = outgoing ? adjSeg->next1() : adjSeg->prev1();
    }

    double widthToSegment = 0.0;
    if (adjSeg) {
        EngravingItem* element = adjSeg->element(staff2track(item->vStaffIdx()));
        track_idx_t strack = track2staff(item->track());
        track_idx_t etrack = strack + VOICES - 1;
        for (EngravingItem* paren : adjSeg->findAnnotations(ElementType::PARENTHESIS, strack, etrack)) {
            if ((outgoing && toParenthesis(paren)->direction() == DirectionH::LEFT)
                || (!outgoing && toParenthesis(paren)->direction() == DirectionH::RIGHT)) {
                element = paren;
                break;
            }
        }

        const double elementWidth = element ? element->width() : 0.0;
        const double elPos = adjSeg->xPosInSystemCoords() + (element ? element->pos().x() + element->shape().bbox().x() : 0.0);
        widthToSegment = outgoing ? elPos - sPos.p1.x() : sPos.p2.x() - (elPos + elementWidth);
        bool incomingFromBarline = !outgoing && element->isBarLine() && toBarLine(element)->barLineType() != BarLineType::START_REPEAT;
        widthToSegment -= item->style().styleMM(incomingFromBarline ? Sid::barlineToLineStartDistance : Sid::lineEndToBarlineDistance);
    }

    if (outgoing) {
        sPos.p2 = PointF(sPos.p1.x() + widthToSegment, sPos.p1.y());
    } else {
        sPos.p1 = PointF(sPos.p2.x() - widthToSegment, sPos.p2.y());
    }
}

void SlurTieLayout::layoutLaissezVibChord(Chord* chord, LayoutContext& ctx)
{
    // Laissez vib ties should all end at the same rightmost point while honouring each tie's minimum length
    // Ties drawn by MuseScore can start at differing points
    // SMuFL symbols will also start at the same point because they are of fixed length
    double chordLvEndPoint = -DBL_MAX;
    std::map<LaissezVibSegment*, SlurTiePos> lvSegmentsWithPositions;
    const bool smuflLayout = ctx.conf().styleB(Sid::laissezVibUseSmuflSym);
    const PointF chordPos = chord->pos() + chord->segment()->pos() + chord->measure()->pos();

    for (const Note* note : chord->notes()) {
        LaissezVib* lv = note->laissezVib();
        if (!lv) {
            continue;
        }
        SlurTiePos sPos;
        LaissezVibSegment* lvSeg = createLaissezVibSegment(lv);

        calculateLaissezVibX(lvSeg, sPos, smuflLayout);

        chordLvEndPoint = std::max(chordLvEndPoint, sPos.p2.x());

        lvSegmentsWithPositions.insert({ lvSeg, sPos });
    }

    for (auto& segWithPos : lvSegmentsWithPositions) {
        LaissezVibSegment* lvSeg = segWithPos.first;
        const Note* note = lvSeg->laissezVib()->startNote();
        SlurTiePos sPos = segWithPos.second;
        const double xDiff = chordLvEndPoint - sPos.p2.x();
        sPos.p2.setX(chordLvEndPoint);
        if (smuflLayout) {
            sPos.p1.setX(sPos.p1.x() + xDiff);
        }

        calculateLaissezVibY(lvSeg, sPos);

        LaissezVibSegment::LayoutData* ldata = lvSeg->mutldata();
        if (smuflLayout) {
            ldata->setBbox(lvSeg->symBbox(ldata->symbol));
            ldata->setShape(Shape(ldata->bbox(), lvSeg));
            ldata->setPos(sPos.p1);
        }

        const PointF notePos = chordPos + note->pos();
        ldata->posRelativeToNote = sPos.p1 - notePos;
    }
}

void SlurTieLayout::correctForCrossStaff(Tie* tie, SlurTiePos& sPos, SpannerSegmentType type)
{
    Chord* startChord = tie->startNote() ? tie->startNote()->chord() : nullptr;
    Chord* endChord = tie->endNote() ? tie->endNote()->chord() : nullptr;

    if (!startChord) {
        return;
    }

    bool startStaffDiff = startChord->vStaffIdx() != tie->staffIdx();
    bool endStaffDiff = endChord ? endChord->vStaffIdx() != tie->staffIdx() : false;
    double curY1 = sPos.p1.y();
    double curY2 = sPos.p2.y();

    if (type == SpannerSegmentType::BEGIN) {
        // Cross-system start tie
        if (startStaffDiff && sPos.system1) {
            double yOrigin = sPos.system1->staff(tie->staffIdx())->y();
            double yMoved = sPos.system1->staff(startChord->vStaffIdx())->y();
            double yDiff = yMoved - yOrigin;
            sPos.p1.setY(curY1 + yDiff);
            sPos.p2.setY(curY2 + yDiff);
        }
    } else if (type == SpannerSegmentType::SINGLE) {
        // Same system single tie
        if (startStaffDiff && sPos.system1) {
            double yOrigin = sPos.system1->staff(tie->staffIdx())->y();
            double yMoved = sPos.system1->staff(startChord->vStaffIdx())->y();
            double yDiff = yMoved - yOrigin;
            sPos.p1.setY(curY1 + yDiff);
            if (tie->isLaissezVib()) {
                sPos.p2.setY(curY2 + yDiff);
                return;
            }
        }
        if (!endChord) {
            return;
        }
        if (endStaffDiff && sPos.system2) {
            double yOrigin = sPos.system2->staff(tie->staffIdx())->y();
            double yMoved = sPos.system2->staff(endChord->vStaffIdx())->y();
            double yDiff = yMoved - yOrigin;
            sPos.p2.setY(curY2 + yDiff);
        }
    }
}

void SlurTieLayout::forceHorizontal(Tie* tie, SlurTiePos& sPos)
{
    Note* startNote = tie->startNote();
    Note* endNote = tie->endNote();

    if (startNote && endNote
        && startNote->line() == endNote->line()
        && startNote->chord()->vStaffIdx() == endNote->chord()->vStaffIdx()) {
        double y1 = sPos.p1.y();
        double y2 = sPos.p2.y();
        double outerY = tie->up() ? std::min(y1, y2) : std::max(y1, y2);
        sPos.p1.setY(outerY);
        sPos.p2.setY(outerY);
    }
}

void SlurTieLayout::adjustX(TieSegment* tieSegment, SlurTiePos& sPos, Grip startOrEnd)
{
    bool start = startOrEnd == Grip::START;

    PointF& tiePoint = start ? sPos.p1 : sPos.p2;
    double resultingX = tiePoint.x();

    Tie* tie = tieSegment->tie();
    Note* note = start ? tie->startNote() : tie->endNote();
    const Note* otherNote = start ? tie->endNote() : tie->startNote();
    if (!note || note->shouldHideFret()) {
        return;
    }

    Chord* chord = note->chord();
    const Chord* otherChord = otherNote ? otherNote->chord() : nullptr;
    const Segment* chordSeg = chord->segment();
    const double spatium = tieSegment->spatium();
    const double padding = 0.20 * spatium * (start ? 1 : -1); // TODO: style
    const bool isGrace = (chord && chord->isGrace())
                         || ((otherChord && otherChord->isGrace()) && chord->segment() == otherChord->segment());

    PointF systemPos = chordSeg->pos() + chord->measure()->pos() + (isGrace ? chord->pos() : PointF());
    if (chord->vStaffIdx() != tieSegment->vStaffIdx()) {
        System* system = tieSegment->system();
        double yDiff = system->staff(chord->vStaffIdx())->y() - system->staff(tie->staffIdx())->y();
        systemPos += PointF(0.0, yDiff);
    }

    const bool isOuterTieOfChord = tie->isOuterTieOfChord(startOrEnd);

    TieDotsPlacement dotsPlacement = tie->style().styleV(Sid::tieDotsPlacement).value<TieDotsPlacement>();
    bool clearAllDots = start && dotsPlacement == TieDotsPlacement::AFTER_DOTS && !isOuterTieOfChord
                        && chord->dots() > 0 && !ChordLayout::chordHasDotsAllInvisible(chord);
    if (clearAllDots) {
        double dotsOuterEdge = -DBL_MAX;
        for (const Note* n : chord->notes()) {
            for (const NoteDot* dot : n->dots()) {
                dotsOuterEdge = std::max(dotsOuterEdge, dot->x() + dot->width() + n->x());
            }
        }
        dotsOuterEdge += systemPos.x();
        resultingX = dotsOuterEdge + padding;
        tieSegment->addAdjustmentOffset(PointF(resultingX - tiePoint.x(), 0.0), startOrEnd);
        tiePoint.setX(resultingX);
        return;
    }

    if (isOuterTieOfChord) {
        Tie* otherTie = start ? note->tieBack() : note->tieFor();
        bool avoidOtherTie = otherTie && otherTie->up() == tie->up() && !otherTie->isInside();
        if (avoidOtherTie) {
            resultingX += 0.1 * spatium * (start ? 1 : -1);
        }
    }

    const bool avoidStem = chord->stem() && chord->stem()->visible() && chord->up() == tie->up();

    if (isOuterTieOfChord && !avoidStem) {
        tieSegment->addAdjustmentOffset(PointF(resultingX - tiePoint.x(), 0.0), startOrEnd);
        tiePoint.setX(resultingX);
        return;
    }

    Shape shape = isGrace ? chord->shape().translate(systemPos) : chordSeg->staffShape(chord->vStaffIdx()).translated(systemPos);
    bool ignoreDot = start && (isOuterTieOfChord || dotsPlacement == TieDotsPlacement::BEFORE_DOTS);
    const bool ignoreAccidental = !start && isOuterTieOfChord;
    bool ignoreLvSeg = tieSegment->isLaissezVibSegment();
    static const std::set<ElementType> IGNORED_TYPES = {
        ElementType::HOOK,
        ElementType::STEM_SLASH,
        ElementType::LEDGER_LINE,
        ElementType::LYRICS,
        ElementType::HARMONY,
        ElementType::FRET_DIAGRAM
    };
    shape.remove_if([&](ShapeElement& s) {
        bool remove =  !s.item() || s.item() == note || muse::contains(IGNORED_TYPES, s.item()->type())
                      || (s.item()->isNoteDot() && ignoreDot)
                      || (s.item()->isAccidental() && ignoreAccidental && s.item()->track() == chord->track())
                      || (s.item()->isLaissezVibSegment() && ignoreLvSeg) || !s.item()->addToSkyline();
        return remove;
    });

    const double arcSideMargin = 0.3 * spatium;
    const double pointsSideMargin = 0.15 * spatium;
    const double yBelow = tiePoint.y() - (tie->up() ? arcSideMargin : pointsSideMargin);
    const double yAbove = tiePoint.y() + (tie->up() ? pointsSideMargin : arcSideMargin);
    double pointToClear = start ? shape.rightMostEdgeAtHeight(yBelow, yAbove)
                          : shape.leftMostEdgeAtHeight(yBelow, yAbove);

    pointToClear += padding;

    resultingX = start ? std::max(resultingX, pointToClear) : std::min(resultingX, pointToClear);

    adjustXforLedgerLines(tieSegment, start, chord, note, systemPos, padding, resultingX);

    tieSegment->addAdjustmentOffset(PointF(resultingX - tiePoint.x(), 0.0), startOrEnd);
    tiePoint.setX(resultingX);
}

void SlurTieLayout::adjustXforLedgerLines(TieSegment* tieSegment, bool start, Chord* chord, Note* note,
                                          const PointF& chordSystemPos, double padding, double& resultingX)
{
    if (tieSegment->tie()->isInside() || chord->ledgerLines().empty()) {
        return;
    }

    bool isOuterNote = note == chord->upNote() || note == chord->downNote();
    if (isOuterNote) {
        return;
    }

    bool ledgersAbove = false;
    bool ledgersBelow = false;
    for (LedgerLine* ledger : chord->ledgerLines()) {
        if (ledger->y() < 0.0) {
            ledgersAbove = true;
        } else {
            ledgersBelow = true;
        }
        if (ledgersAbove && ledgersBelow) {
            break;
        }
    }

    int noteLine = note->line();
    bool isOddLine = noteLine % 2 != 0;
    bool isAboveStaff = noteLine <= 0;
    bool isBelowStaff = noteLine >= 2 * (note->staff()->lines(note->tick()) - 1);
    bool isInsideStaff = !isAboveStaff && !isBelowStaff;
    if (isOddLine || isInsideStaff || (isAboveStaff && !ledgersAbove) || (isBelowStaff && !ledgersBelow)) {
        return;
    }

    Shape noteShape = note->shape();
    noteShape.remove_if([&](ShapeElement& s) {
        return !s.item()->addToSkyline() || s.item()->isNoteDot() || (tieSegment->isLaissezVibSegment() && s.item()->isLaissezVibSegment());
    });
    noteShape.translate(note->pos() + chordSystemPos);
    double xNoteEdge = (start ? noteShape.right() : -noteShape.left()) + padding;

    resultingX = start ? std::max(resultingX, xNoteEdge) : std::min(resultingX, xNoteEdge);
}

void SlurTieLayout::adjustYforLedgerLines(TieSegment* tieSegment, SlurTiePos& sPos)
{
    Tie* tie = tieSegment->tie();
    Note* note = tieSegment->isSingleBeginType() ? tie->startNote() : tie->endNote();
    if (!note) {
        return;
    }

    Chord* chord = note->chord();
    if (chord->ledgerLines().empty()) {
        return;
    }
    PointF chordSystemPos = chord->pos() + chord->segment()->pos() + chord->segment()->measure()->pos();
    PointF& tiePoint = tieSegment->isSingleBeginType() ? sPos.p1 : sPos.p2;
    double spatium = tie->spatium();
    int upSign = tie->up() ? -1 : 1;
    double margin = 0.4 * spatium;

    for (LedgerLine* ledger : chord->ledgerLines()) {
        PointF ledgerPos = ledger->pos() + chordSystemPos;
        double yDiff = upSign * (ledgerPos.y() - tiePoint.y());
        bool collision = yDiff > 0 && yDiff < margin;
        if (collision) {
            sPos.p1 += PointF(0.0, -upSign * (margin - yDiff));
            sPos.p2 += PointF(0.0, -upSign * (margin - yDiff));
            computeBezier(tieSegment);
            break;
        }
    }
}

void SlurTieLayout::adjustY(TieSegment* tieSegment)
{
    Staff* staff = tieSegment->score() ? tieSegment->score()->staff(tieSegment->vStaffIdx()) : nullptr;
    if (!staff) {
        return;
    }

    const double spatium = tieSegment->spatium();
    TieSegment::LayoutData* ldata = tieSegment->mutldata();

    ldata->setPos(PointF());
    ldata->moveY(tieSegment->staffOffsetY());

    Fraction tick = tieSegment->tick();

    computeBezier(tieSegment);

    bool up = tieSegment->tie()->up();
    int upSign = up ? -1 : 1;

    const double staffLineDist = staff->lineDistance(tick) * spatium;
    const double staffLineThickness = tieSegment->style().styleMM(Sid::staffLineWidth) * staff->staffMag(tick);

    // 1. Check for bad end point protrusion

    const double endPointY = tieSegment->ups(Grip::START).p.y();
    const int closestLineToEndpoints = up ? floor(endPointY / staffLineDist) : ceil(endPointY / staffLineDist);
    const bool isEndInsideStaff = closestLineToEndpoints >= 0 && closestLineToEndpoints < staff->lines(tick);
    const bool isEndInsideLedgerLines = !isEndInsideStaff && !tieSegment->tie()->isOuterTieOfChord(Grip::START);

    const double halfLineThicknessCorrection = 0.5 * staffLineThickness * upSign;
    const double protrusion = std::abs(endPointY - (closestLineToEndpoints * spatium - halfLineThicknessCorrection));
    const double badIntersectionLimit = 0.15 * spatium; // TODO: style

    bool badIntersection = protrusion < badIntersectionLimit && (isEndInsideStaff || isEndInsideLedgerLines);
    if (badIntersection) {
        double correctedY = closestLineToEndpoints * spatium + halfLineThicknessCorrection + badIntersectionLimit * upSign;
        tieSegment->addAdjustmentOffset(PointF(0.0, correctedY - endPointY), Grip::START);
        tieSegment->addAdjustmentOffset(PointF(0.0, correctedY - endPointY), Grip::END);
        tieSegment->ups(Grip::START).p.setY(correctedY);
        tieSegment->ups(Grip::END).p.setY(correctedY);
        computeBezier(tieSegment);
    }

    // 2. Check for bad arc protrusion

    RectF tieSegmentBBox = tieSegment->ldata()->bbox();
    double tieLength = tieSegmentBBox.width();
    double tieHeight = tieSegmentBBox.height();
    double midThickness = tieSegment->ldata()->midThickness() * 2;
    double yOuterApogee = up ? tieSegmentBBox.top() : tieSegmentBBox.bottom();
    double yInnerApogee = yOuterApogee - midThickness * upSign;
    double yMidApogee = 0.5 * (yOuterApogee + yInnerApogee);

    int closestLineToArc = round(yMidApogee / staffLineDist);
    bool isArcInsideStaff =  closestLineToArc >= 0 && closestLineToArc < staff->lines(tick);
    if (!isArcInsideStaff) {
        return;
    }

    double outwardMargin = -upSign * (yOuterApogee - (closestLineToArc * staffLineDist - halfLineThicknessCorrection));
    double inwardMargin = upSign * (yInnerApogee - (closestLineToArc * staffLineDist + halfLineThicknessCorrection));
    const double badArcIntersectionLimit = tieLength < 3 * spatium ? 0.1 * spatium : 0.15 * spatium;

    bool increaseArc = outwardMargin - 0.5 * badArcIntersectionLimit < inwardMargin;
    bool correctOutwards = inwardMargin < badArcIntersectionLimit && increaseArc;
    bool correctInwards = outwardMargin < badArcIntersectionLimit && !increaseArc;

    if (!correctInwards && !correctOutwards) {
        return;
    }

    bool isSmallTie = tieLength < 2.0 * spatium && tieHeight < 0.7 * spatium;
    // For ties this small, try to fit them within a staff space before changing their arc
    if (isSmallTie) {
        Tie* tie2 = tieSegment->tie();
        bool isInside = tie2->isInside();
        bool isOuterOfChord = tie2->isOuterTieOfChord(Grip::START) || tie2->isOuterTieOfChord(Grip::END);
        bool hasTiedSecondInside = tie2->hasTiedSecondInside();
        if (!isInside && !isOuterOfChord && !hasTiedSecondInside && !hasEndPointAboveNote(tieSegment)) {
            double currentY = tieSegment->ups(Grip::START).p.y();
            double yCorrection = -upSign * (badArcIntersectionLimit - outwardMargin);
            tieSegment->addAdjustmentOffset(PointF(0.0, yCorrection), Grip::START);
            tieSegment->addAdjustmentOffset(PointF(0.0, yCorrection), Grip::END);
            tieSegment->ups(Grip::START).p.setY(currentY + yCorrection);
            tieSegment->ups(Grip::END).p.setY(currentY + yCorrection);
            computeBezier(tieSegment);
            return;
        } else {
            correctOutwards = true;
        }
    }

    double arcCorrection = correctOutwards ? (badArcIntersectionLimit - inwardMargin) : (badArcIntersectionLimit - outwardMargin);
    double maxArcCorrection = 0.75 * tieHeight;
    double rest = arcCorrection > maxArcCorrection ? arcCorrection - maxArcCorrection : 0.0;
    arcCorrection = std::min(arcCorrection, maxArcCorrection);
    if (correctOutwards) {
        if (rest > 0) {
            rest *= upSign;
            double currentY = tieSegment->ups(Grip::START).p.y();
            tieSegment->addAdjustmentOffset(PointF(0.0, rest), Grip::START);
            tieSegment->addAdjustmentOffset(PointF(0.0, rest), Grip::END);
            tieSegment->ups(Grip::START).p.setY(currentY + rest);
            tieSegment->ups(Grip::END).p.setY(currentY + rest);
        }
        computeBezier(tieSegment, PointF(0.0, -arcCorrection));
    } else if (correctInwards) {
        computeBezier(tieSegment, PointF(0.0, arcCorrection));
    }
}

bool SlurTieLayout::hasEndPointAboveNote(TieSegment* tieSegment)
{
    Note* startNote = tieSegment->tie()->startNote();
    Note* endNote = tieSegment->tie()->endNote();

    if ((tieSegment->isSingleBeginType() && !startNote) || (tieSegment->isSingleEndType() && !endNote)) {
        return false;
    }

    Chord* startChord = startNote->chord();
    PointF startNotePos = startNote->pos() + startChord->pos() + startChord->segment()->pos() + startChord->measure()->pos();

    Chord* endChord = endNote->chord();
    PointF endNotePos = endNote->pos() + endChord->pos() + endChord->segment()->pos() + endChord->measure()->pos();

    PointF tieStartPos = tieSegment->ups(Grip::START).pos();
    PointF tieEndPos = tieSegment->ups(Grip::END).pos();

    return (tieStartPos.x() < startNotePos.x() + startNote->width() && !startNote->shouldHideFret())
           || (tieEndPos.x() > endNotePos.x() && !endNote->shouldHideFret());
}

void SlurTieLayout::resolveVerticalTieCollisions(const std::vector<TieSegment*>& stackedTies)
{
    if (stackedTies.size() < 2) {
        return;
    }

    std::list<TieSegment*> downwardTies;
    std::list<TieSegment*> upwardTies;
    for (TieSegment* tieSegment : stackedTies) {
        if (!tieSegment->tie()->up()) {
            downwardTies.push_front(tieSegment);
        } else {
            upwardTies.push_back(tieSegment);
        }
    }

    auto fixTieCollision = [](TieSegment* thisTie, TieSegment* nextTie) {
        double spatium = thisTie->spatium();
        bool up = thisTie->tie()->up();
        int upSign = up ? -1 : 1;

        double thisTieOuterY = up ? thisTie->ldata()->bbox().top() : thisTie->ldata()->bbox().bottom();
        double nextTieInnerY = (up ? nextTie->ldata()->bbox().top() : nextTie->ldata()->bbox().bottom())
                               - upSign * 2 * nextTie->ldata()->midThickness();
        double clearanceMargin = 0.15 * spatium;
        bool collision = upSign * (nextTieInnerY - thisTieOuterY) < clearanceMargin;
        if (!collision) {
            return;
        }

        if (!thisTie->score()) {
            return;
        }
        Staff* staff = thisTie->score() ? thisTie->score()->staff(thisTie->vStaffIdx()) : nullptr;
        if (!staff) {
            return;
        }

        Fraction tick = thisTie->tick();
        double yMidPoint = 0.5 * (thisTieOuterY + nextTieInnerY);
        double halfLineDist = 0.5 * staff->lineDistance(tick) * spatium;
        int midLine = round(yMidPoint / halfLineDist);
        bool insideStaff = midLine >= -1 && midLine <= 2 * (staff->lines(tick) - 1) + 1;
        if (insideStaff) {
            yMidPoint = midLine * halfLineDist;
        }

        double thisTieYCorrection = yMidPoint - upSign * 0.5 * clearanceMargin - thisTieOuterY;
        double nextTieYCorrection = yMidPoint + upSign * 0.5 * clearanceMargin - nextTieInnerY;

        double thisShoulderOff = thisTie->adjustmentOffset(Grip::BEZIER1).y();
        double nextShoulderOff = nextTie->adjustmentOffset(Grip::BEZIER1).y();
        // Subtract it otherwise it gets summed twice
        thisTie->addAdjustmentOffset(PointF(0.0, -thisShoulderOff), Grip::BEZIER1);
        thisTie->addAdjustmentOffset(PointF(0.0, -thisShoulderOff), Grip::BEZIER2);
        nextTie->addAdjustmentOffset(PointF(0.0, -nextShoulderOff), Grip::BEZIER1);
        nextTie->addAdjustmentOffset(PointF(0.0, -nextShoulderOff), Grip::BEZIER2);

        computeBezier(thisTie, PointF(0.0, -upSign * (thisShoulderOff + thisTieYCorrection)));
        computeBezier(nextTie, PointF(0.0, -upSign * (nextShoulderOff + nextTieYCorrection)));
    };

    if (upwardTies.size() >= 2) {
        for (auto it = upwardTies.begin(); std::next(it, 1) != upwardTies.end(); ++it) {
            TieSegment* thisTie = *it;
            TieSegment* nextTie = *(std::next(it, 1));
            fixTieCollision(thisTie, nextTie);
        }
    }

    if (downwardTies.size() >= 2) {
        for (auto it = downwardTies.begin(); std::next(it, 1) != downwardTies.end(); ++it) {
            TieSegment* thisTie = *it;
            TieSegment* nextTie = *(std::next(it, 1));
            fixTieCollision(thisTie, nextTie);
        }
    }
}

void SlurTieLayout::computeUp(Slur* slur, LayoutContext& ctx)
{
    switch (slur->slurDirection()) {
    case DirectionV::UP:
        slur->setUp(true);
        break;
    case DirectionV::DOWN:
        slur->setUp(false);
        break;
    case DirectionV::AUTO:
    {
        //
        // assumption:
        // slurs have only chords or rests as start/end elements
        //
        ChordRest* chordRest1 = slur->startCR();
        ChordRest* chordRest2 = slur->endCR();
        if (chordRest1 == 0 || chordRest2 == 0) {
            slur->setUp(true);
            break;
        }
        Chord* chord1 = slur->startCR()->isChord() ? toChord(slur->startCR()) : 0;
        Chord* chord2 = slur->endCR()->isChord() ? toChord(slur->endCR()) : 0;
        if (chord2 && !chord2->staff()->isDrumStaff(chord2->tick())
            && slur->startCR()->measure()->system() != slur->endCR()->measure()->system()) {
            // HACK: if the end chord is in a different system, it may have never been laid out yet.
            // But we need to know its direction to decide slur direction, so need to compute it here.
            for (Note* note : chord2->notes()) {
                note->updateLine(); // because chord direction is based on note lines
            }

            ChordLayout::computeUp(chord2, ctx);
        }

        if (chord1 && chord1->beam() && chord1->beam()->cross()) {
            // TODO: stem direction is not finalized, so we cannot use it here
            slur->setUp(true);
            break;
        }

        slur->setUp(!(chordRest1->up()));

        // Check if multiple voices
        bool multipleVoices = false;
        Measure* m1 = chordRest1->measure();
        while (m1 && m1->tick() <= chordRest2->tick()) {
            if ((m1->hasVoices(chordRest1->staffIdx(), slur->tick(), slur->ticks() + chordRest2->ticks()))
                && chord1) {
                multipleVoices = true;
                break;
            }
            m1 = m1->nextMeasure();
        }
        if (multipleVoices) {
            // slurs go on the stem side
            if (chordRest1->voice() > 0 || chordRest2->voice() > 0) {
                slur->setUp(false);
            } else {
                slur->setUp(true);
            }
        } else if (chord1 && chord2 && !chord1->isGrace() && isDirectionMixture(chord1, chord2, ctx)) {
            // slurs go above if there are mixed direction stems between c1 and c2
            // but grace notes are exceptions
            slur->setUp(true);
        } else if (chord1 && chord2 && chord1->isGrace() && chord2 != chord1->parent() && isDirectionMixture(chord1, chord2, ctx)) {
            slur->setUp(true);
        }
    }
    break;
    }
}

void SlurTieLayout::computeBezier(TieSegment* tieSeg, PointF shoulderOffset)
{
    const PointF tieStart = tieSeg->ups(Grip::START).p + tieSeg->ups(Grip::START).off;
    const PointF tieEnd = tieSeg->ups(Grip::END).p + tieSeg->ups(Grip::END).off;

    PointF tieEndNormalized = tieEnd - tieStart;  // normalize to zero
    if (muse::RealIsNull(tieEndNormalized.x())) {
        return;
    }

    const double tieAngle = atan(tieEndNormalized.y() / tieEndNormalized.x()); // angle required from tie start to tie end--zero if horizontal
    Transform t;
    t.rotateRadians(-tieAngle);  // rotate so that we are working with horizontal ties regardless of endpoint height difference
    tieEndNormalized = t.map(tieEndNormalized);  // apply that rotation
    shoulderOffset = t.map(shoulderOffset);  // also apply to shoulderOffset

    const double _spatium = tieSeg->spatium();
    double tieLengthInSp = tieEndNormalized.x() / _spatium;

    const double minShoulderHeight = tieSeg->minShoulderHeight();
    const double maxShoulderHeight = tieSeg->maxShoulderHeight();
    double shoulderH = minShoulderHeight + _spatium * 0.3 * sqrt(std::abs(tieLengthInSp - 1));
    shoulderH = std::clamp(shoulderH, minShoulderHeight, maxShoulderHeight);

    shoulderH -= shoulderOffset.y();

    PointF shoulderAdjustOffset = tieSeg->tie()->up() ? PointF(0.0, shoulderOffset.y()) : PointF(0.0, -shoulderOffset.y());
    tieSeg->addAdjustmentOffset(shoulderAdjustOffset, Grip::BEZIER1);
    tieSeg->addAdjustmentOffset(shoulderAdjustOffset, Grip::BEZIER2);

    if (!tieSeg->tie()->up()) {
        shoulderH = -shoulderH;
    }

    double shoulderW = 0.6; // TODO: style

    const double tieWidth = tieEndNormalized.x();
    const double bezier1X = (tieWidth - tieWidth * shoulderW) * .5 + shoulderOffset.x();
    const double bezier2X = bezier1X + tieWidth * shoulderW + shoulderOffset.x();

    const PointF tieDrag = PointF(tieWidth * .5, 0.0);

    const PointF bezier1(bezier1X, -shoulderH);
    const PointF bezier2(bezier2X, -shoulderH);

    computeMidThickness(tieSeg, tieLengthInSp);

    PointF tieThickness(0.0, tieSeg->ldata()->midThickness());

    const PointF bezier1Offset = t.map(tieSeg->ups(Grip::BEZIER1).off);
    const PointF bezier2Offset = t.map(tieSeg->ups(Grip::BEZIER2).off);

    //-----------------------------------calculate p6
    const PointF bezier1Final = bezier1 + bezier1Offset;
    const PointF bezier2Final = bezier2 + bezier2Offset;

    const PointF tieShoulder = 0.5 * (bezier1Final + bezier2Final);
    //-----------------------------------

    PainterPath path = PainterPath();
    path.moveTo(PointF());
    path.cubicTo(bezier1 + bezier1Offset - tieThickness, bezier2 + bezier2Offset - tieThickness, tieEndNormalized);
    if (tieSeg->tie()->styleType() == SlurStyleType::Solid) {
        path.cubicTo(bezier2 + bezier2Offset + tieThickness, bezier1 + bezier1Offset + tieThickness, PointF());
    }

    // translate back
    t.reset();
    t.translate(tieStart.x(), tieStart.y());
    t.rotateRadians(tieAngle);
    path = t.map(path);
    tieSeg->mutldata()->path.set_value(path);

    tieSeg->ups(Grip::BEZIER1).p = t.map(bezier1);
    tieSeg->ups(Grip::BEZIER2).p = t.map(bezier2);
    tieSeg->ups(Grip::END).p = t.map(tieEndNormalized) - tieSeg->ups(Grip::END).off;
    tieSeg->ups(Grip::DRAG).p = t.map(tieDrag);
    tieSeg->ups(Grip::SHOULDER).p = t.map(tieShoulder);

    fillShape(tieSeg, tieLengthInSp);
}

void SlurTieLayout::computeBezier(SlurSegment* slurSeg, PointF shoulderOffset)
{
    /* ************************************************
     * LEGEND: pp1 = start point
     *         pp2 = end point
     *         p2 = end point (in slur coordinates)
     *         p3 = first bezier point (in slur coord.)
     *         p4 = second bezier point (in slur coord.)
     *         p5 = whole slur drag point (in slur coord.)
     *         p6 = shoulder drag point (in slur coord.)
     * REMEMBER! ups().pos() = ups().p + ups().off
     * ***********************************************/
    // Avoid bad staff line intersections
    if (slurSeg->autoplace()) {
        adjustEndPoints(slurSeg);
    }
    // If end point adjustment is locked, restore the endpoints to
    // where they were before
    if (slurSeg->isEndPointsEdited()) {
        slurSeg->ups(Grip::START).off += slurSeg->endPointOff1();
        slurSeg->ups(Grip::END).off += slurSeg->endPointOff2();
    }
    // Get start and end points (have been calculated before)
    PointF pp1 = slurSeg->ups(Grip::START).p + slurSeg->ups(Grip::START).off;
    PointF pp2 = slurSeg->ups(Grip::END).p + slurSeg->ups(Grip::END).off;
    // Keep track of the original value before it gets changed
    PointF oldp1 = pp1;
    PointF oldp2 = pp2;

    // Set up coordinate transforms
    // CAUTION: transform operations are applies in reverse order to how
    // they are added to the transformation.
    double slurAngle = atan((pp2.y() - pp1.y()) / (pp2.x() - pp1.x()));
    Transform rotate;
    rotate.rotateRadians(-slurAngle);
    Transform toSlurCoordinates;
    toSlurCoordinates.rotateRadians(-slurAngle);
    toSlurCoordinates.translate(-pp1.x(), -pp1.y());
    Transform toSystemCoordinates = toSlurCoordinates.inverted();
    // Transform p2 and shoulder offset
    PointF p2 = toSlurCoordinates.map(pp2);
    shoulderOffset = rotate.map(shoulderOffset);

    // COMPUTE DEFAULT SLUR SHAPE
    // Compute default shoulder height and width
    double _spatium  = slurSeg->spatium();
    double shoulderW; // expressed as fraction of slur-length
    double d = p2.x() / _spatium;

    if (d < 0) {
        //! NOTE A negative d means that end point is before the start point.
        //! This only exists as a temporary state when horizontal spacing hasn't yet been computed,
        //! and it makes no sense for any of the following calculations
        return;
    }

    if (d < 2) {
        shoulderW = 0.60;
    } else if (d < 10) {
        shoulderW = 0.5;
    } else if (d < 18) {
        shoulderW = 0.6;
    } else {
        shoulderW = 0.7;
    }

    double shoulderH = computeShoulderHeight(slurSeg, d, shoulderOffset);

    double c    = p2.x();
    double c1   = (c - c * shoulderW) * .5 + shoulderOffset.x();
    double c2   = c1 + c * shoulderW + shoulderOffset.x();
    PointF p3(c1, -shoulderH);
    PointF p4(c2, -shoulderH);
    // Set Bezier points default position
    slurSeg->ups(Grip::BEZIER1).p  = toSystemCoordinates.map(p3);
    slurSeg->ups(Grip::BEZIER2).p  = toSystemCoordinates.map(p4);
    // Add offsets
    p3 += shoulderOffset + rotate.map(slurSeg->ups(Grip::BEZIER1).off);
    p4 += shoulderOffset + rotate.map(slurSeg->ups(Grip::BEZIER2).off);
    slurSeg->ups(Grip::BEZIER1).off += rotate.inverted().map(shoulderOffset);
    slurSeg->ups(Grip::BEZIER2).off += rotate.inverted().map(shoulderOffset);

    // ADAPT SLUR SHAPE AND ENDPOINT POSITION
    // to clear collisions with underlying items
    if (slurSeg->autoplace() && !slurSeg->isTappingHalfSlurSegment()) {
        avoidCollisions(slurSeg, pp1, p2, p3, p4, toSystemCoordinates, slurAngle);
    }

    // Re-check end points for bad staff line collisions
    slurSeg->ups(Grip::START).p = pp1 - slurSeg->ups(Grip::START).off;
    slurSeg->ups(Grip::END).p = toSystemCoordinates.map(p2) - slurSeg->ups(Grip::END).off;
    adjustEndPoints(slurSeg);
    PointF newpp1 = slurSeg->ups(Grip::START).p + slurSeg->ups(Grip::START).off;
    PointF difference = rotate.map(newpp1 - pp1);
    pp1 = newpp1;
    pp2 = slurSeg->ups(Grip::END).p + slurSeg->ups(Grip::END).off;
    p3 -= difference;
    p4 -= difference;
    // Keep track of how much the end points position has changed
    if (!slurSeg->isEndPointsEdited()) {
        slurSeg->setEndPointOff1(pp1 - oldp1);
        slurSeg->setEndPointOff2(pp2 - oldp2);
    } else {
        slurSeg->setEndPointOff1(PointF());
        slurSeg->setEndPointOff2(PointF());
    }
    // Recompute the transformation because pp1 and pp1 may have changed
    toSlurCoordinates.reset();
    toSlurCoordinates.rotateRadians(-slurAngle);
    toSlurCoordinates.translate(-pp1.x(), -pp1.y());
    toSystemCoordinates = toSlurCoordinates.inverted();
    p2 = toSlurCoordinates.map(pp2);

    // Calculate p5 and p6
    PointF p5 = 0.5 * p2; // mid-point between pp1 and p2
    PointF p6 = 0.5 * (p3 + p4); // mid-point between p3 and p4

    // Update all slur points after collision avoidance
    slurSeg->ups(Grip::BEZIER1).p  = toSystemCoordinates.map(p3) - slurSeg->ups(Grip::BEZIER1).off;
    slurSeg->ups(Grip::BEZIER2).p  = toSystemCoordinates.map(p4) - slurSeg->ups(Grip::BEZIER2).off;
    slurSeg->ups(Grip::DRAG).p     = toSystemCoordinates.map(p5);
    slurSeg->ups(Grip::SHOULDER).p = toSystemCoordinates.map(p6);

    // Set slur thickness
    computeMidThickness(slurSeg, p2.x() / slurSeg->spatium());
    PointF thick(0.0, slurSeg->ldata()->midThickness());

    // Set path
    PainterPath path = PainterPath();
    path.moveTo(PointF());
    path.cubicTo(p3 - thick, p4 - thick, p2);
    if (slurSeg->slur()->styleType() == SlurStyleType::Solid) {
        path.cubicTo(p4 + thick, p3 + thick, PointF());
    }

    path = toSystemCoordinates.map(path);
    slurSeg->mutldata()->path.set_value(path);

    fillShape(slurSeg, p2.x() / slurSeg->spatium());
}

double SlurTieLayout::computeShoulderHeight(SlurSegment* slurSeg, double slurLengthInSp, PointF shoulderOffset)
{
    double spatium = slurSeg->spatium();

    if (slurSeg->isTappingHalfSlurSegment()) {
        double shoulderH = (slurSeg->staffType()->isTabStaff() ? 1.0 : 0.6) * spatium;
        return slurSeg->slur()->up() ? shoulderH : -shoulderH;
    }

    double shoulderH = sqrt(slurLengthInSp / 4) * spatium;

    static constexpr double shoulderReduction = 0.75;
    if (isOverBeams(slurSeg->slur())) {
        shoulderH *= shoulderReduction;
    }

    shoulderH -= shoulderOffset.y();
    if (!slurSeg->slur()->up()) {
        shoulderH = -shoulderH;
    }

    return shoulderH;
}

double SlurTieLayout::defaultStemLengthStart(TremoloTwoChord* tremolo)
{
    return TremoloLayout::extendedStemLenWithTwoNoteTremolo(tremolo,
                                                            tremolo->chord1()->defaultStemLength(),
                                                            tremolo->chord2()->defaultStemLength()).first;
}

double SlurTieLayout::defaultStemLengthEnd(TremoloTwoChord* tremolo)
{
    return TremoloLayout::extendedStemLenWithTwoNoteTremolo(tremolo,
                                                            tremolo->chord1()->defaultStemLength(),
                                                            tremolo->chord2()->defaultStemLength()).second;
}

bool SlurTieLayout::isDirectionMixture(const Chord* c1, const Chord* c2, LayoutContext& ctx)
{
    if (c1->track() != c2->track()) {
        return false;
    }
    const bool up = c1->up();
    if (c2->isGrace() && c2->up() != up) {
        return true;
    }
    if (c1->isGraceBefore() && c2->isGraceAfter() && c1->parentItem() == c2->parentItem()) {
        if (toChord(c1->parentItem())->stem() && toChord(c1->parentItem())->up() != up) {
            return true;
        }
    }
    const track_idx_t track = c1->track();
    for (const Segment* seg = c1->segment(); seg && seg->tick() <= c2->tick(); seg = seg->next1(SegmentType::ChordRest)) {
        if ((c1->isGrace() || c2->isGraceBefore()) && seg->tick() == c2->tick()) {
            // if slur ends at a grace-note-before, we don't need to look at the main note
            return false;
        }
        EngravingItem* e = seg->element(track);
        if (!e || !e->isChord()) {
            continue;
        }
        Chord* c = toChord(e);
        const Measure* m = c->measure();
        const System* c1Sys = c1->measure()->system();
        const System* cSys = m->system();
        if (!c->staff()->isDrumStaff(c->tick()) && c1Sys && cSys && c1Sys != cSys) {
            // This chord is on a different system and may not have been laid out yet
            for (Note* note : c->notes()) {
                note->updateLine();     // because chord direction is based on note lines
            }
            ChordLayout::computeUp(c, ctx);
        }

        if (c->up() != up) {
            return true;
        }
    }
    return false;
}

bool SlurTieLayout::shouldHideSlurSegment(SlurSegment* item)
{
    if (item->configuration()->specificSlursLayoutWorkaround()) {
        Slur* slur = item->slur();
        if (slur->connectedElement() == Slur::ConnectedElement::GLISSANDO) {
            return false;
        }
    }

    const StaffType* stType = item->staffType();
    if (stType && stType->isHiddenElementOnTab(Sid::slurShowTabCommon, Sid::slurShowTabSimple)) {
        return true;
    }

    return false;
}

void SlurTieLayout::addLineAttachPoints(TieSegment* segment)
{
    // Add tie attach point to start and end note of segment
    Tie* tie = segment->tie();
    Note* startNote = tie->startNote();
    Note* endNote = tie->endNote();

    const bool singleOrBegin = segment->spannerSegmentType() == SpannerSegmentType::SINGLE
                               || segment->spannerSegmentType() == SpannerSegmentType::BEGIN;

    const bool singleOrEnd = segment->spannerSegmentType() == SpannerSegmentType::SINGLE
                             || segment->spannerSegmentType() == SpannerSegmentType::END;

    if (startNote && singleOrBegin) {
        startNote->addStartLineAttachPoint(segment->ups(Grip::START).pos(), tie);
    }
    if (endNote && singleOrEnd) {
        endNote->addEndLineAttachPoint(segment->ups(Grip::END).pos(), tie);
    }
}

void SlurTieLayout::addLineAttachPoints(PartialTieSegment* segment)
{
    // Add tie attach point to parent note
    PartialTie* tie = segment->partialTie();
    Note* note = tie ? tie->note() : nullptr;
    if (!note) {
        return;
    }

    const bool isOutgoing = tie->isOutgoing();

    if (isOutgoing) {
        note->addStartLineAttachPoint(segment->ups(Grip::START).pos(), tie);
    } else {
        note->addEndLineAttachPoint(segment->ups(Grip::END).pos(), tie);
    }
}

void SlurTieLayout::calculateDirection(Tie* item)
{
    if (!item->startNote() && !item->endNote()) {
        return;
    }
    const bool tieHasBothNotes = item->startNote() && item->endNote();

    const Note* primaryNote = item->startNote() ? item->startNote() : item->endNote();
    const Chord* primaryChord = primaryNote->chord();
    const Measure* primaryMeasure = primaryChord->measure();

    const Note* secondaryNote = tieHasBothNotes ? item->endNote() : nullptr;
    const Chord* secondaryChord = secondaryNote ? secondaryNote->chord() : nullptr;
    const Measure* secondaryMeasure = secondaryChord ? secondaryChord->measure() : nullptr;

    if (item->slurDirection() == DirectionV::AUTO) {
        std::vector<Note*> notes = primaryChord->notes();
        size_t n = notes.size();
        StaffType* st = item->staff()->staffType(primaryNote ? primaryNote->tick() : Fraction(0, 1));
        bool simpleException = st && st->isSimpleTabStaff();
        // if there are multiple voices, the tie direction goes on stem side
        if (primaryMeasure->hasVoices(primaryChord->staffIdx(), primaryChord->tick(), primaryChord->actualTicks())) {
            item->setUp(simpleException ? isUpVoice(primaryChord->voice()) : primaryChord->up());
        } else if (tieHasBothNotes && secondaryMeasure->hasVoices(secondaryChord->staffIdx(), secondaryChord->tick(),
                                                                  secondaryChord->actualTicks())) {
            item->setUp(simpleException ? isUpVoice(secondaryChord->voice()) : secondaryChord->up());
        } else if (n == 1) {
            //
            // single note
            //
            if (tieHasBothNotes && primaryChord->up() != secondaryChord->up()) {
                // if stem direction is mixed, always up
                item->setUp(true);
            } else {
                item->setUp(!primaryChord->up());
            }
        } else {
            //
            // chords
            //
            // first, find pivot point in chord (below which all ties curve down and above which all ties curve up)
            Note* pivotPoint = nullptr;
            bool multiplePivots = false;
            for (size_t i = 0; i < n - 1; ++i) {
                if (!notes[i]->tieFor()) {
                    continue; // don't include notes that don't have ties
                }
                for (size_t j = i + 1; j < n; ++j) {
                    if (!notes[j]->tieFor()) {
                        continue;
                    }
                    int noteDiff = compareNotesPos(notes[i], notes[j]);
                    if (!multiplePivots && std::abs(noteDiff) <= 1) {
                        // TODO: Fix unison ties somehow--if noteDiff == 0 then we need to determine which of the unison is 'lower'
                        if (pivotPoint) {
                            multiplePivots = true;
                            pivotPoint = nullptr;
                        } else {
                            pivotPoint = noteDiff < 0 ? notes[i] : notes[j];
                        }
                    }
                }
            }
            if (!pivotPoint) {
                // if the pivot point was not found (either there are no unisons/seconds or there are more than one),
                // determine if this note is in the lower or upper half of this chord
                int notesAbove = 0, tiesAbove = 0;
                int notesBelow = 0, tiesBelow = 0;
                int unisonTies = 0;
                for (size_t i = 0; i < n; ++i) {
                    if (notes[i] == primaryNote) {
                        // skip counting if this note is the current note or if this note doesn't have a tie
                        continue;
                    }
                    int noteDiff = compareNotesPos(primaryNote, notes[i]);
                    if (noteDiff == 0) {  // unison
                        if (notes[i]->tieFor()) {
                            unisonTies++;
                        }
                    }
                    if (noteDiff < 0) { // the note is above startNote
                        notesAbove++;
                        if (notes[i]->tieFor()) {
                            tiesAbove++;
                        }
                    }
                    if (noteDiff > 0) { // the note is below startNote
                        notesBelow++;
                        if (notes[i]->tieFor()) {
                            tiesBelow++;
                        }
                    }
                }

                if (tiesAbove == 0 && tiesBelow == 0 && unisonTies == 0) {
                    // this is the only tie in the chord.
                    if (notesAbove == notesBelow) {
                        item->setUp(!primaryChord->up());
                    } else {
                        item->setUp(notesAbove < notesBelow);
                    }
                } else if (tiesAbove == tiesBelow) {
                    // this note is dead center, so its tie should go counter to the stem direction
                    item->setUp(!primaryChord->up());
                } else {
                    item->setUp(tiesAbove < tiesBelow);
                }
            } else if (pivotPoint == primaryNote) {
                // the current note is the lower of the only second or unison in the chord; tie goes down.
                item->setUp(false);
            } else {
                // if lower than the pivot, tie goes down, otherwise up
                int noteDiff = compareNotesPos(primaryNote, pivotPoint);
                item->setUp(noteDiff >= 0);
            }
        }
    } else {
        item->setUp(item->slurDirection() == DirectionV::UP ? true : false);
    }
}

void SlurTieLayout::calculateIsInside(Tie* item)
{
    if (item->tiePlacement() != TiePlacement::AUTO) {
        item->setIsInside(item->tiePlacement() == TiePlacement::INSIDE);
        return;
    }

    const Note* startN = item->startNote();
    const Chord* startChord = startN ? startN->chord() : nullptr;
    const Note* endN = item->endNote();
    const Chord* endChord = endN ? endN->chord() : nullptr;

    if (!startChord && !endChord) {
        item->setIsInside(false);
        return;
    }

    const bool startIsSingleNote = startChord ? startChord->notes().size() <= 1 : false;
    const bool endIsSingleNote = endChord ? endChord->notes().size() <= 1 : false;

    const bool shouldPlaceInside = startChord && endChord ? startIsSingleNote && endIsSingleNote : startIsSingleNote || endIsSingleNote;

    if (shouldPlaceInside) {
        item->setIsInside(item->style().styleV(Sid::tiePlacementSingleNote).value<TiePlacement>() == TiePlacement::INSIDE);
    } else {
        item->setIsInside(item->style().styleV(Sid::tiePlacementChord).value<TiePlacement>() == TiePlacement::INSIDE);
    }
}

void SlurTieLayout::layoutSegment(SlurSegment* item, const PointF& p1, const PointF& p2)
{
    SlurSegment::LayoutData* ldata = item->mutldata();
    if (shouldHideSlurSegment(item)) {
        ldata->setIsSkipDraw(true);
        return;
    }

    ldata->setIsSkipDraw(false);

    ldata->setPos(PointF());
    item->ups(Grip::START).p = p1;
    item->ups(Grip::END).p   = p2;
    item->setExtraHeight(0.0);

    //Adjust Y pos to staff type yOffset before other calculations
    ldata->moveY(item->staffOffsetY());

    computeBezier(item);
}

void SlurTieLayout::computeMidThickness(SlurTieSegment* slurTieSeg, double slurTieLengthInSp)
{
    const double endWidth = slurTieSeg->endWidth();
    const double midWidth = slurTieSeg->midWidth();
    Staff* staff = slurTieSeg->score() ? slurTieSeg->score()->staff(slurTieSeg->vStaffIdx()) : nullptr;
    const double mag = staff ? staff->staffMag(slurTieSeg->slurTie()->tick()) : 1.0;
    const double minTieLength = mag * slurTieSeg->style().styleS(Sid::minTieLength).val();
    const double shortTieLimit = mag * 4.0;
    const double minTieThickness = mag * (0.15 * slurTieSeg->spatium() - endWidth);
    const double normalThickness = mag * (midWidth - endWidth);

    bool invalid = muse::RealIsEqualOrMore(minTieLength, shortTieLimit);

    double finalThickness;
    if (slurTieLengthInSp > shortTieLimit || invalid) {
        finalThickness = normalThickness;
    } else {
        const double A = 1 / (shortTieLimit - minTieLength);
        const double B = normalThickness - minTieThickness;
        const double C = shortTieLimit * minTieThickness - minTieLength * normalThickness;
        finalThickness = A * (B * slurTieLengthInSp + C);
    }

    double scalingFactor = slurTieSeg->slurTie()->scalingFactor();

    finalThickness = std::min(finalThickness, normalThickness * scalingFactor);

    slurTieSeg->mutldata()->midThickness.set_value(finalThickness);
}

void SlurTieLayout::fillShape(SlurTieSegment* slurTieSeg, double slurTieLengthInSp)
{
    Shape shape(Shape::Type::Composite);
    PointF startPoint = slurTieSeg->ups(Grip::START).pos();

    double midThickness = 2 * slurTieSeg->ldata()->midThickness();
    int nbShapes = round(5.0 * slurTieLengthInSp);
    nbShapes = std::max(nbShapes, 20);
    nbShapes = std::min(nbShapes, 50);
    const CubicBezier b(startPoint, slurTieSeg->ups(Grip::BEZIER1).pos(), slurTieSeg->ups(Grip::BEZIER2).pos(),
                        slurTieSeg->ups(Grip::END).pos());
    for (int i = 1; i <= nbShapes; i++) {
        double percent = pow(sin(0.5 * M_PI * (double(i) / double(nbShapes))), 2);
        const PointF point = b.pointAtPercent(percent);
        RectF re = RectF(startPoint, point).normalized();
        double approxThicknessAtPercent = (1 - 2 * std::abs(0.5 - percent)) * midThickness;
        if (re.height() < approxThicknessAtPercent) {
            double adjust = (approxThicknessAtPercent - re.height()) * .5;
            re.adjust(0.0, -adjust, 0.0, adjust);
        }
        shape.add(re, slurTieSeg);
        startPoint = point;
    }

    slurTieSeg->mutldata()->setShape(shape);
}
