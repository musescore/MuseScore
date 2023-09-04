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
#include "slurtielayout.h"

#include "iengravingfont.h"
#include "dom/slur.h"
#include "dom/score.h"
#include "dom/chord.h"
#include "dom/system.h"
#include "dom/staff.h"
#include "dom/stafftype.h"
#include "dom/note.h"
#include "dom/hook.h"
#include "dom/stem.h"
#include "dom/tremolo.h"
#include "dom/fretcircle.h"
#include "dom/tie.h"

#include "tlayout.h"
#include "chordlayout.h"
#include "tremololayout.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

void SlurTieLayout::layout(Slur* item, LayoutContext& ctx)
{
    if (item->track2() == mu::nidx) {
        item->setTrack2(item->track());
    }

    double _spatium = item->spatium();

    if (ctx.conf().isPaletteMode() || item->tick() == Fraction(-1, 1)) {
        //
        // when used in a palette, slur has no parent and
        // tick and tick2 has no meaning so no layout is
        // possible and needed
        //
        SlurSegment* s;
        if (item->spannerSegments().empty()) {
            s = new SlurSegment(ctx.mutDom().dummyParent()->system());
            s->setTrack(item->track());
            item->add(s);
        } else {
            s = item->frontSegment();
        }
        s->setSpannerSegmentType(SpannerSegmentType::SINGLE);
        layoutSegment(s, ctx, PointF(0, 0), PointF(_spatium * 6, 0));
        item->setbbox(item->frontSegment()->layoutData()->bbox());
        return;
    }

    if (item->startCR() == 0 || item->startCR()->measure() == 0) {
        LOGD("track %zu-%zu  %p - %p tick %d-%d null start anchor",
             item->track(), item->track2(), item->startCR(), item->endCR(), item->tick().ticks(), item->tick2().ticks());
        return;
    }
    if (item->endCR() == 0) {       // sanity check
        LOGD("no end CR for %d", (item->tick() + item->ticks()).ticks());
        item->setEndElement(item->startCR());
        item->setTick2(item->tick());
    }
    switch (item->slurDirection()) {
    case DirectionV::UP:
        item->setUp(true);
        break;
    case DirectionV::DOWN:
        item->setUp(false);
        break;
    case DirectionV::AUTO:
    {
        //
        // assumption:
        // slurs have only chords or rests as start/end elements
        //
        if (item->startCR() == 0 || item->endCR() == 0) {
            item->setUp(true);
            break;
        }
        Measure* m1 = item->startCR()->measure();

        Chord* c1 = item->startCR()->isChord() ? toChord(item->startCR()) : 0;
        Chord* c2 = item->endCR()->isChord() ? toChord(item->endCR()) : 0;

        item->setUp(!(item->startCR()->up()));

        if ((item->endCR()->tick() - item->startCR()->tick()) > m1->ticks()) {
            // long slurs are always above
            item->setUp(true);
        } else {
            item->setUp(!(item->startCR()->up()));
        }

        if (c1 && c2 && Slur::isDirectionMixture(c1, c2) && (c1->noteType() == NoteType::NORMAL)) {
            // slurs go above if start and end note have different stem directions,
            // but grace notes are exceptions
            item->setUp(true);
        } else if (m1->hasVoices(item->startCR()->staffIdx(), item->tick(), item->ticks()) && c1 && c1->noteType() == NoteType::NORMAL) {
            // in polyphonic passage, slurs go on the stem side
            item->setUp(item->startCR()->up());
        }
    }
    break;
    }

    SlurPos sPos;
    slurPos(item, &sPos, ctx);

    const std::vector<System*>& sl = ctx.dom().systems();
    ciSystem is = sl.begin();
    while (is != sl.end()) {
        if (*is == sPos.system1) {
            break;
        }
        ++is;
    }
    if (is == sl.end()) {
        LOGD("Slur::layout  first system not found");
    }
    item->setPos(0, 0);

    //---------------------------------------------------------
    //   count number of segments, if no change, all
    //    user offsets (drags) are retained
    //---------------------------------------------------------

    unsigned nsegs = 1;
    for (ciSystem iis = is; iis != sl.end(); ++iis) {
        if ((*iis)->vbox()) {
            continue;
        }
        if (*iis == sPos.system2) {
            break;
        }
        ++nsegs;
    }

    item->fixupSegments(nsegs);

    for (int i = 0; is != sl.end(); ++i, ++is) {
        System* system  = *is;
        if (system->vbox()) {
            --i;
            continue;
        }
        SlurSegment* segment = item->segmentAt(i);
        segment->setSystem(system);

        // case 1: one segment
        if (sPos.system1 == sPos.system2) {
            segment->setSpannerSegmentType(SpannerSegmentType::SINGLE);
            layoutSegment(segment, ctx, sPos.p1, sPos.p2);
        }
        // case 2: start segment
        else if (i == 0) {
            segment->setSpannerSegmentType(SpannerSegmentType::BEGIN);
            double x = system->layoutData()->bbox().width();
            layoutSegment(segment, ctx, sPos.p1, PointF(x, sPos.p1.y()));
        }
        // case 3: middle segment
        else if (i != 0 && system != sPos.system2) {
            segment->setSpannerSegmentType(SpannerSegmentType::MIDDLE);
            double x1 = system->firstNoteRestSegmentX(true);
            double x2 = system->layoutData()->bbox().width();
            double y  = item->staffIdx() > system->staves().size() ? system->y() : system->staff(item->staffIdx())->y();
            layoutSegment(segment, ctx, PointF(x1, y), PointF(x2, y));
        }
        // case 4: end segment
        else {
            segment->setSpannerSegmentType(SpannerSegmentType::END);
            double x = system->firstNoteRestSegmentX(true);
            layoutSegment(segment, ctx, PointF(x, sPos.p2.y()), sPos.p2);
        }
        if (system == sPos.system2) {
            break;
        }
    }
    item->setbbox(item->spannerSegments().empty() ? RectF() : item->frontSegment()->layoutData()->bbox());
}

SpannerSegment* SlurTieLayout::layoutSystem(Slur* item, System* system, LayoutContext& ctx)
{
    const double horizontalTieClearance = 0.35 * item->spatium();
    const double tieClearance = 0.65 * item->spatium();
    const double continuedSlurOffsetY = item->spatium() * .4;
    const double continuedSlurMaxDiff = 2.5 * item->spatium();
    Fraction stick = system->firstMeasure()->tick();
    Fraction etick = system->lastMeasure()->endTick();

    SlurSegment* slurSegment = toSlurSegment(TLayout::getNextLayoutSystemSegment(item, system, [](System* parent) {
        return new SlurSegment(parent);
    }));

    SpannerSegmentType sst;
    if (item->tick() >= stick) {
        //
        // this is the first call to layoutSystem,
        // processing the first line segment
        //
        if (item->track2() == mu::nidx) {
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
        if (item->sourceStemArrangement() != -1) {
            if (item->sourceStemArrangement() != item->calcStemArrangement(item->startCR(), item->endCR())) {
                // copy & paste from incompatible stem arrangement, so reset bezier points
                for (int g = 0; g < (int)Grip::GRIPS; ++g) {
                    slurSegment->ups((Grip)g) = UP();
                }
            }
        }
        sst = item->tick2() < etick ? SpannerSegmentType::SINGLE : SpannerSegmentType::BEGIN;
    } else if (item->tick() < stick && item->tick2() >= etick) {
        sst = SpannerSegmentType::MIDDLE;
    } else {
        sst = SpannerSegmentType::END;
    }
    slurSegment->setSpannerSegmentType(sst);

    SlurPos sPos;
    slurPos(item, &sPos, ctx);
    PointF p1, p2;
    // adjust for ties
    p1 = sPos.p1;
    p2 = sPos.p2;
    bool constrainLeftAnchor = false;

    // start anchor, either on the start chordrest or at the beginning of the system
    if (sst == SpannerSegmentType::SINGLE || sst == SpannerSegmentType::BEGIN) {
        Chord* sc = item->startCR()->isChord() ? toChord(item->startCR()) : nullptr;

        // on chord
        if (sc && sc->notes().size() == 1) {
            Tie* tie = sc->notes()[0]->tieFor();
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
            if (!adjustedVertically && sc->notes()[0]->tieBack() && !sc->notes()[0]->tieBack()->isInside()
                && sc->notes()[0]->tieBack()->up() == item->up()) {
                // there is a tie that ends on this chordrest
                tie = sc->notes()[0]->tieBack();
                if (!tie->segmentsEmpty()) {
                    endPoint = tie->segmentAt(static_cast<int>(tie->nsegments()) - 1)->ups(Grip::END).pos();
                    if (abs(endPoint.y() - p1.y()) < tieClearance) {
                        p1.rx() += horizontalTieClearance;
                    }
                }
            }
        }
    } else if (sst == SpannerSegmentType::END || sst == SpannerSegmentType::MIDDLE) {
        // beginning of system
        ChordRest* firstCr = system->firstChordRest(item->track());
        double y = p1.y();
        if (firstCr && firstCr == item->endCR()) {
            constrainLeftAnchor = true;
        }
        if (firstCr && firstCr->isChord()) {
            Chord* chord = toChord(firstCr);
            if (chord) {
                // if both up or both down, deal with avoiding stems and beams
                Note* upNote = chord->upNote();
                Note* downNote = chord->downNote();
                // account for only the stem length that is above the top note (or below the bottom note)
                double stemLength = chord->stem() ? chord->stem()->length() - (downNote->pos().y() - upNote->pos().y()) : 0.0;
                if (item->up()) {
                    y = chord->upNote()->pos().y() - (chord->upNote()->height() / 2);
                    if (chord->up() && chord->stem() && firstCr != item->endCR()) {
                        y -= stemLength;
                    }
                } else {
                    y = chord->downNote()->pos().y() + (chord->downNote()->height() / 2);
                    if (!chord->up() && chord->stem() && firstCr != item->endCR()) {
                        y += stemLength;
                    }
                }
                y += continuedSlurOffsetY * (item->up() ? -1 : 1);
            }
        }
        p1 = PointF(system->firstNoteRestSegmentX(true), y);

        // adjust for ties at the end of the system
        ChordRest* cr = system->firstChordRest(item->track());
        if (cr && cr->isChord() && cr->tick() >= stick && cr->tick() <= etick) {
            // TODO: can ties go to or from rests?
            Chord* c = toChord(cr);
            Tie* tie = nullptr;
            PointF endPoint;
            Tie* tieBack = c->notes()[0]->tieBack();
            if (tieBack && !tieBack->isInside() && tieBack->up() == item->up()) {
                // there is a tie that ends on this chordrest
                if (!tieBack->segmentsEmpty()) { //Checks for spanner segment esxists
                    tie = tieBack;
                    endPoint = tie->backSegment()->ups(Grip::START).pos();
                }
            }
            if (tie) {
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
    if (sst == SpannerSegmentType::SINGLE || sst == SpannerSegmentType::END) {
        Chord* ec = item->endCR()->isChord() ? toChord(item->endCR()) : nullptr;

        // on chord
        if (ec && ec->notes().size() == 1) {
            Tie* tie = ec->notes()[0]->tieBack();
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
            if (!adjustedVertically && tieFor && !tieFor->isInside() && tieFor->up() == item->up()) {
                // there is a tie that starts on this chordrest
                if (!tieFor->segmentsEmpty() && std::abs(tieFor->frontSegment()->ups(Grip::START).pos().y() - p2.y()) < tieClearance) {
                    p2.rx() -= horizontalTieClearance;
                }
            }
        }
    } else {
        // at end of system
        ChordRest* lastCr = system->lastChordRest(item->track());
        double y = p1.y();
        if (lastCr && lastCr == item->startCR()) {
            y += 0.25 * item->spatium() * (item->up() ? -1 : 1);
        } else if (lastCr && lastCr->isChord()) {
            Chord* chord = toChord(lastCr);
            if (chord) {
                Note* upNote = chord->upNote();
                Note* downNote = chord->downNote();
                // account for only the stem length that is above the top note (or below the bottom note)
                double stemLength = chord->stem() ? chord->stem()->length() - (downNote->pos().y() - upNote->pos().y()) : 0.0;
                if (item->up()) {
                    y = chord->upNote()->pos().y() - (chord->upNote()->height() / 2);
                    if (chord->up() && chord->stem()) {
                        y -= stemLength;
                    }
                } else {
                    y = chord->downNote()->pos().y() + (chord->downNote()->height() / 2);
                    if (!chord->up() && chord->stem()) {
                        y += stemLength;
                    }
                }
                y += continuedSlurOffsetY * (item->up() ? -1 : 1);
            }
            double diff = item->up() ? y - p1.y() : p1.y() - y;
            if (diff > continuedSlurMaxDiff) {
                y = p1.y() + (y > p1.y() ? continuedSlurMaxDiff : -continuedSlurMaxDiff);
            }
        }

        p2 = PointF(system->endingXForOpenEndedLines(), y);

        // adjust for ties at the end of the system
        ChordRest* cr = system->lastChordRest(item->track());

        if (cr && cr->isChord() && cr->tick() >= stick && cr->tick() <= etick) {
            // TODO: can ties go to or from rests?
            Chord* c = toChord(cr);
            Tie* tie = nullptr;
            PointF endPoint;
            Tie* tieFor = c->notes()[0]->tieFor();
            if (tieFor && !tieFor->isInside() && tieFor->up() == item->up()) {
                // there is a tie that starts on this chordrest
                if (!tieFor->segmentsEmpty()) { //Checks is spanner segment exists
                    tie = tieFor;
                    endPoint = tie->segmentAt(0)->ups(Grip::END).pos();
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

    layoutSegment(slurSegment, ctx, p1, p2);

    return slurSegment;
}

//---------------------------------------------------------
//   slurPos
//    calculate position of start- and endpoint of slur
//    relative to System() position
//---------------------------------------------------------

void SlurTieLayout::slurPos(Slur* item, SlurPos* sp, LayoutContext& ctx)
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
    if (note1 && !note1->mirror()) {
        sp->p1.rx() += note1->x();
    }
    if (note2 && !note2->mirror()) {
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
            if (stem1 && !item->stemSideStartForBeam()) {
                sa1 = SlurAnchor::STEM;
            }
            if (stem2 && !item->stemSideEndForBeam()) {
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
        pt = sc->stemPos() - sc->pagePos() + sc->stem()->layoutData()->line.p2();
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

            if (hook && hook->layoutData()->bbox().translated(hook->pos()).contains(pt)) {
                // TODO: in the utopian far future where all hooks have SMuFL cutouts, this fakeCutout business will no
                // longer be used. for the time being fakeCutout describes a point on the line y=mx+b, out from the top of the stem
                // where y = yadj, m = fakeCutoutSlope, and x = y/m + fakeCutout
                fakeCutout = std::min(0.0, std::abs(yadj) - (hook->width() / fakeCutoutSlope));
                pt.rx() = sc->stemPosX() - fakeCutout;
            }
        } else {
            Hook* hook = sc->hook();
            // straight flags
            if (hook && hook->layoutData()->bbox().translated(hook->pos()).contains(pt)) {
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
        pt = ec->stemPos() - ec->pagePos() + ec->stem()->layoutData()->line.p2();
        if (useTablature) {
            pt.rx() = hw2 * 0.5;
        }
        // don't allow overlap with beam
        double yadj;
        if (ec->beam() && ec->beam()->elements().front() != ec) {
            yadj = 0.75;
        } else if (ec->tremolo() && ec->tremolo()->twoNotes() && ec->tremolo()->chord2() == ec) {
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
            po.ry() = scr->layoutData()->bbox().top();
        } else {
            po.ry() = scr->layoutData()->bbox().top() + scr->height();
        }
        double offset = useTablature ? 0.75 : 0.9;
        po.ry() += scr->intrinsicMag() * _spatium * offset * __up;

        // adjustments for stem and/or beam
        Tremolo* trem = sc ? sc->tremolo() : nullptr;
        if (stem1 || (trem && trem->twoNotes())) {     //sc not null
            Beam* beam1 = sc->beam();
            if (beam1 && (beam1->elements().back() != sc) && (sc->up() == item->up())) {
                TLayout::layout(beam1, ctx);
                // start chord is beamed but not the last chord of beam group
                // and slur direction is same as start chord (stem side)

                // in these cases, layout start of slur to stem
                double beamWidthSp = ctx.conf().styleS(Sid::beamWidth).val() * beam1->magS();
                double offset = std::max(beamClearance * sc->intrinsicMag(), minOffset) * _spatium;
                double sh = stem1->length() + (beamWidthSp / 2) + offset;
                if (item->up()) {
                    po.ry() = sc->stemPos().y() - sc->pagePos().y() - sh;
                } else {
                    po.ry() = sc->stemPos().y() - sc->pagePos().y() + sh;
                }
                po.rx() = sc->stemPosX() + (beamAnchorInset * _spatium * sc->intrinsicMag()) + (stem1->lineWidthMag() / 2 * __up);

                // account for articulations
                fixArticulations(item, po, sc, __up, true);

                // force end of slur to layout to stem as well,
                // if start and end chords have same stem direction
                stemPos = true;
            } else if (trem && trem->twoNotes() && trem->chord2() != sc && sc->up() == item->up()) {
                TLayout::layout(trem, ctx);
                Note* note = item->up() ? sc->upNote() : sc->downNote();
                double stemHeight = stem1 ? stem1->length() : defaultStemLengthStart(trem);
                double offset = std::max(beamClearance * sc->intrinsicMag(), minOffset) * _spatium;
                double sh = stemHeight + offset;

                if (item->up()) {
                    po.ry() = sc->stemPos().y() - sc->pagePos().y() - sh;
                } else {
                    po.ry() = sc->stemPos().y() - sc->pagePos().y() + sh;
                }
                if (!stem1) {
                    po.rx() = note->noteheadCenterX();
                } else {
                    po.rx() = sc->stemPosX() + (beamAnchorInset * _spatium * sc->intrinsicMag()) + (stem1->lineWidthMag() / 2. * __up);
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
                po.ry() = item->endCR()->layoutData()->bbox().top();
            } else {
                po.ry() = item->endCR()->layoutData()->bbox().top() + item->endCR()->height();
            }
            double offset = useTablature ? 0.75 : 0.9;
            po.ry() += ecr->intrinsicMag() * _spatium * offset * __up;

            // adjustments for stem and/or beam
            Tremolo* trem = ec ? ec->tremolo() : nullptr;
            if (stem2 || (trem && trem->twoNotes())) {       //ec can't be null
                Beam* beam2 = ec->beam();
                if ((stemPos && (scr->up() == ec->up()))
                    || (beam2
                        && (!beam2->elements().empty())
                        && (beam2->elements().front() != ec)
                        && (ec->up() == item->up())
                        && sc && (sc->noteType() == NoteType::NORMAL)
                        )
                    || (trem && trem->twoNotes() && ec->up() == item->up())
                    ) {
                    if (beam2) {
                        TLayout::layout(beam2, ctx);
                    }
                    if (trem) {
                        TLayout::layout(trem, ctx);
                    }
                    // slur start was laid out to stem and start and end have same direction
                    // OR
                    // end chord is beamed but not the first chord of beam group
                    // and slur direction is same as end chord (stem side)
                    // and start chordrest is not a grace chord

                    // in these cases, layout end of slur to stem
                    double beamWidthSp = beam2 ? ctx.conf().styleS(Sid::beamWidth).val() : 0;
                    Note* note = item->up() ? sc->upNote() : sc->downNote();
                    double stemHeight = stem2 ? stem2->length() + (beamWidthSp / 2) : defaultStemLengthEnd(trem);
                    double offset = std::max(beamClearance * ec->intrinsicMag(), minOffset) * _spatium;
                    double sh = stemHeight + offset;

                    if (item->up()) {
                        po.ry() = ec->stemPos().y() - ec->pagePos().y() - sh;
                    } else {
                        po.ry() = ec->stemPos().y() - ec->pagePos().y() + sh;
                    }
                    if (!stem2) {
                        // tremolo whole notes
                        po.setX(note->noteheadCenterX());
                    } else {
                        po.setX(ec->stemPosX() + (stem2->lineWidthMag() / 2 * __up) - (beamAnchorInset * _spatium * ec->intrinsicMag()));
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

    /// adding extra space above slurs for notes in circles
    if (Slur::engravingConfiguration()->enableExperimentalFretCircle() && item->staff()->staffType()->isCommonTabStaff()) {
        auto adjustSlur = [](Chord* ch, PointF& coord, bool up) {
            const Fraction halfFraction = Fraction(1, 2);
            if (ch && ch->ticks() >= halfFraction) {
                for (EngravingItem* item : ch->el()) {
                    if (item && item->isFretCircle()) {
                        coord += PointF(0, toFretCircle(item)->layoutData()->offsetFromUpNote * (up ? -1 : 1));
                        break;
                    }
                }
            }
        };

        adjustSlur(sc, sp->p1, item->up());
        adjustSlur(ec, sp->p2, item->up());
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
        pt.rx() = a->x() - note->x();
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

//---------------------------------------------------------
//   layoutFor
//    layout the first SpannerSegment of a slur
//---------------------------------------------------------

TieSegment* SlurTieLayout::tieLayoutFor(Tie* item, System* system)
{
    // do not layout ties in tablature if not showing back-tied fret marks
    StaffType* st = item->staff()->staffType(item->startNote() ? item->startNote()->tick() : Fraction(0, 1));
    if (st && st->isTabStaff() && !st->showBackTied()) {
        if (!item->segmentsEmpty()) {
            item->eraseSpannerSegments();
        }
        return nullptr;
    }
    //
    //    show short bow
    //
    if (item->startNote() == 0 || item->endNote() == 0) {
        if (item->startNote() == 0) {
            LOGD("no start note");
            return 0;
        }
        Chord* c1 = item->startNote()->chord();
        item->setTick(c1->tick());
        if (item->slurDirection() == DirectionV::AUTO) {
            bool simpleException = st && st->isSimpleTabStaff();
            if (st && st->isSimpleTabStaff()) {
                item->setUp(isUpVoice(c1->voice()));
            } else {
                if (c1->measure()->hasVoices(c1->staffIdx(), c1->tick(), c1->actualTicks())) {
                    // in polyphonic passage, ties go on the stem side
                    item->setUp(simpleException ? isUpVoice(c1->voice()) : c1->up());
                } else {
                    item->setUp(!c1->up());
                }
            }
        } else {
            item->setUp(item->slurDirection() == DirectionV::UP ? true : false);
        }
        item->fixupSegments(1);
        TieSegment* segment = item->segmentAt(0);
        segment->setSpannerSegmentType(SpannerSegmentType::SINGLE);
        segment->setSystem(item->startNote()->chord()->segment()->measure()->system());
        SlurPos sPos;
        tiePos(item, &sPos);
        segment->adjustY(sPos.p1, sPos.p2);
        segment->finalizeSegment();
        return segment;
    }
    item->calculateDirection();

    SlurPos sPos;
    tiePos(item, &sPos);  // get unadjusted x values and determine inside or outside

    item->setPos(0, 0);

    int n;
    if (sPos.system1 != sPos.system2) {
        n = 2;
        sPos.p2 = PointF(system->endingXForOpenEndedLines(), sPos.p1.y());
    } else {
        n = 1;
    }

    item->fixupSegments(n);
    TieSegment* segment = item->segmentAt(0);
    segment->setSystem(system);   // Needed to populate System.spannerSegments
    Chord* c1 = item->startNote()->chord();
    item->setTick(c1->tick());
    segment->adjustY(sPos.p1, sPos.p2); // adjust vertically
    segment->setSpannerSegmentType(sPos.system1 != sPos.system2 ? SpannerSegmentType::BEGIN : SpannerSegmentType::SINGLE);
    segment->adjustX(); // adjust horizontally for inside-style ties
    segment->finalizeSegment(); // compute bezier and set bbox
    segment->addLineAttachPoints(); // add attach points to start and end note
    return segment;
}

//---------------------------------------------------------
//   layoutBack
//    layout the second SpannerSegment of a split slur
//---------------------------------------------------------

TieSegment* SlurTieLayout::tieLayoutBack(Tie* item, System* system)
{
    // do not layout ties in tablature if not showing back-tied fret marks
    StaffType* st = item->staff()->staffType(item->startNote() ? item->startNote()->tick() : Fraction(0, 1));
    if (st->isTabStaff() && !st->showBackTied()) {
        if (!item->segmentsEmpty()) {
            item->eraseSpannerSegments();
        }
        return nullptr;
    }

    SlurPos sPos;
    tiePos(item, &sPos);

    item->fixupSegments(2);
    TieSegment* segment = item->segmentAt(1);
    segment->setSystem(system);

    double x = system ? system->firstNoteRestSegmentX(true) : 0;

    segment->adjustY(PointF(x, sPos.p2.y()), sPos.p2);
    segment->setSpannerSegmentType(SpannerSegmentType::END);
    segment->adjustX();
    segment->finalizeSegment();
    segment->addLineAttachPoints();
    return segment;
}

//---------------------------------------------------------
//   slurPos
//    Calculate position of start- and endpoint of slur
//    relative to System() position.
//---------------------------------------------------------

void SlurTieLayout::tiePos(Tie* item, SlurPos* sp)
{
    const StaffType* staffType = item->staffType();
    bool useTablature = staffType->isTabStaff();
    double _spatium = item->spatium();
    double hw = item->startNote()->tabHeadWidth(staffType); // if staffType == 0, defaults to headWidth()
    /* Inside-style and Outside-style ties
     Outside ties connect above the notehead, in the middle. Ideally, we'd use opticalcenter for this, but
     that Smufl anchor is not available for noteheads yet. For this reason, we rely on Note::outsideTieAttachX()
     which makes its best guess as to where ties should connect.

     As for y connection point, inside-style ties are decided by the space or line the note occupies in TieSegment::adjustY()
     so we don't need to worry about that. Outside-style ties will be 0.125 spatium from the top or bottom of the notehead.
     We can parameterize that later, but this describes only a minimum distance from the notehead, it can be changed, again,
     in TieSegment::adjustY().
    */

    Chord* sc = item->startNote()->chord();
    Chord* ec = item->endNote() ? item->endNote()->chord() : nullptr;
    sp->system1 = sc->measure()->system();
    if (!sp->system1) {
        Measure* m = sc->measure();
        LOGD("No system: measure is %d has %d count %d", m->isMMRest(), m->hasMMRest(), m->mmRestCount());
    }

    double x1, y1;
    double x2, y2;

    if (sc->notes().size() > 1 || (ec && ec->notes().size() > 1)) {
        item->setIsInside(true);
    } else {
        item->setIsInside(false);
    }
    const double smallInset = item->isInside() ? 0.0 : 0.125; // 1/8 spatium, slight visual adjust so that ties don't hit the center

    sp->p1 = sc->pos() + sc->segment()->pos() + sc->measure()->pos();

    //------p1
    y1 = item->startNote()->pos().y();
    y2 = item->endNote() ? item->endNote()->pos().y() : y1;

    // force tie to be horizontal except for cross-staff or if there is a difference of line (tpc, clef)
    int line1 = useTablature ? item->startNote()->string() : item->startNote()->line();
    int line2 = line1;
    if (item->endNote()) {
        line2 = useTablature ? item->endNote()->string() : item->endNote()->line();
    }
    bool isHorizontal = ec ? line1 == line2 && sc->vStaffIdx() == ec->vStaffIdx() : true;
    y1 += item->startNote()->layoutData()->bbox().y();
    if (item->endNote()) {
        y2 += item->endNote()->layoutData()->bbox().y();
    }
    if (!item->up()) {
        y1 += item->startNote()->layoutData()->bbox().height();
        if (item->endNote()) {
            y2 += item->endNote()->layoutData()->bbox().height();
        }
    }
    if (!item->endNote()) {
        y2 = y1;
    }

    // ensure that horizontal ties remain horizontal
    if (isHorizontal) {
        y1 = item->up() ? std::min(y1, y2) : std::max(y1, y2);
        y2 = item->up() ? std::min(y1, y2) : std::max(y1, y2);
    }

    if (item->isInside()) {
        x1 = item->startNote()->pos().x() + hw;  // the offset for these will be decided in TieSegment::adjustX()
    } else {
        if (sc->stem() && sc->stem()->visible() && sc->up() && item->up()) {
            // usually, outside ties start in the middle of the notehead, but
            // for up-ties on up-stems, we'll start at the end of the notehead
            // to avoid the stem
            x1 = item->startNote()->pos().x() + hw;
        } else {
            x1 = item->startNote()->outsideTieAttachX(item->up());
        }
    }
    if (!(item->up() && sc->up())) {
        x1 += smallInset * _spatium;
    }
    sp->p1 += PointF(x1, y1);

    //------p2
    if (!ec) {
        sp->p2 = sp->p1 + PointF(_spatium * 3, 0.0);
        sp->system2 = sp->system1;
        return;
    }
    sp->p2 = ec->pos() + ec->segment()->pos() + ec->measure()->pos();
    sp->system2 = ec->measure()->system();

    if (item->isInside()) {
        x2 = item->endNote()->x();
    } else {
        if (ec->stem() && ec->stem()->visible() && !ec->up() && !item->up()) {
            // as before, xo should account for stems that could get in the way
            x2 = item->endNote()->x();
        } else {
            x2 = item->endNote()->outsideTieAttachX(item->up());
        }
    }
    if (!(!item->up() && !ec->up())) {
        x2 -= smallInset * _spatium;
    }
    sp->p2 += PointF(x2, y2);
    // adjust for cross-staff
    if (sc->vStaffIdx() != item->staffIdx() && sp->system1) {
        double diff = sp->system1->staff(sc->vStaffIdx())->y() - sp->system1->staff(item->staffIdx())->y();
        sp->p1.ry() += diff;
    }
    if (ec->vStaffIdx() != item->staffIdx() && sp->system2) {
        double diff = sp->system2->staff(ec->vStaffIdx())->y() - sp->system2->staff(item->staffIdx())->y();
        sp->p2.ry() += diff;
    }

    /// adjusting ties for notes in circles
    if (Tie::engravingConfiguration()->enableExperimentalFretCircle() && item->staff()->staffType()->isCommonTabStaff()) {
        auto adjustTie = [item](Chord* ch, PointF& coord, bool tieStart) {
            const Fraction halfFraction = Fraction(1, 2);
            if (ch && ch->ticks() >= halfFraction) {
                for (EngravingItem* e : ch->el()) {
                    if (e && e->isFretCircle()) {
                        FretCircle* fretCircle = toFretCircle(e);
                        coord += PointF(0, fretCircle->layoutData()->offsetFromUpNote * (item->up() ? -1 : 1));
                        if (item->isInside()) {
                            coord += PointF(fretCircle->layoutData()->sideOffset * (tieStart ? 1 : -1), 0);
                        }

                        break;
                    }
                }
            }
        };

        adjustTie(sc, sp->p1, true);
        adjustTie(ec, sp->p2, false);
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
        } else if (chord1 && chord2 && !chord1->isGrace() && slur->isDirectionMixture(chord1, chord2)) {
            // slurs go above if there are mixed direction stems between c1 and c2
            // but grace notes are exceptions
            slur->setUp(true);
        } else if (chord1 && chord2 && chord1->isGrace() && chord2 != chord1->parent() && slur->isDirectionMixture(chord1, chord2)) {
            slur->setUp(true);
        }
    }
    break;
    }
}

double SlurTieLayout::defaultStemLengthStart(Tremolo* tremolo)
{
    return TremoloLayout::extendedStemLenWithTwoNoteTremolo(tremolo,
                                                            tremolo->chord1()->defaultStemLength(),
                                                            tremolo->chord2()->defaultStemLength()).first;
}

double SlurTieLayout::defaultStemLengthEnd(Tremolo* tremolo)
{
    return TremoloLayout::extendedStemLenWithTwoNoteTremolo(tremolo,
                                                            tremolo->chord1()->defaultStemLength(),
                                                            tremolo->chord2()->defaultStemLength()).second;
}

void SlurTieLayout::layoutSegment(SlurSegment* item, LayoutContext& ctx, const PointF& p1, const PointF& p2)
{
    SlurSegment::LayoutData* ldata = item->mutLayoutData();
    const StaffType* stType = item->staffType();

    if (stType && stType->isHiddenElementOnTab(ctx.conf().style(), Sid::slurShowTabCommon, Sid::slurShowTabSimple)) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    ldata->setPos(PointF());
    item->ups(Grip::START).p = p1;
    item->ups(Grip::END).p   = p2;
    item->setExtraHeight(0.0);

    //Adjust Y pos to staff type yOffset before other calculations
    if (item->staffType()) {
        ldata->moveY(item->staffType()->yoffset().val() * item->spatium());
    }

    item->computeBezier();
    ldata->setBbox(item->path().boundingRect());
}
