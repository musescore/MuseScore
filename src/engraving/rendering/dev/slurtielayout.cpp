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

#include "compat/dummyelement.h"

#include "dom/slur.h"
#include "dom/chord.h"
#include "dom/system.h"
#include "dom/staff.h"
#include "dom/stafftype.h"
#include "dom/ledgerline.h"
#include "dom/note.h"
#include "dom/hook.h"
#include "dom/stem.h"
#include "dom/tremolo.h"
#include "dom/tremolotwochord.h"
#include "dom/fretcircle.h"
#include "dom/tie.h"
#include "dom/engravingitem.h"
#include "dom/measure.h"
#include "dom/guitarbend.h"

#include "tlayout.h"
#include "chordlayout.h"
#include "tremololayout.h"
#include "../engraving/types/symnames.h"

#include "draw/types/transform.h"

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
        item->setbbox(item->frontSegment()->ldata()->bbox());
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

        if (c1 && c2 && isDirectionMixture(c1, c2, ctx) && (c1->noteType() == NoteType::NORMAL)) {
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

    SlurTiePos sPos;
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
            double x = system->ldata()->bbox().width();
            layoutSegment(segment, ctx, sPos.p1, PointF(x, sPos.p1.y()));
        }
        // case 3: middle segment
        else if (i != 0 && system != sPos.system2) {
            segment->setSpannerSegmentType(SpannerSegmentType::MIDDLE);
            double x1 = system->firstNoteRestSegmentX(true);
            double x2 = system->ldata()->bbox().width();
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
    item->setbbox(item->spannerSegments().empty() ? RectF() : item->frontSegment()->ldata()->bbox());
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

    SlurTiePos sPos;
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
        pt = sc->stemPos() - sc->pagePos() + sc->stem()->ldata()->line.p2();
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
                pt.rx() = sc->stemPosX() - fakeCutout;
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
        pt = ec->stemPos() - ec->pagePos() + ec->stem()->ldata()->line.p2();
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
            } else if (trem && trem->chord2() != sc && sc->up() == item->up()) {
                TremoloLayout::layout(trem, ctx);
                Note* note = item->up() ? sc->upNote() : sc->downNote();
                double stemHeight = stem1 ? stem1->length() : defaultStemLengthStart(trem);
                double offset2 = std::max(beamClearance * sc->intrinsicMag(), minOffset) * _spatium;
                double sh = stemHeight + offset2;

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
                    Note* note = item->up() ? sc->upNote() : sc->downNote();
                    double stemHeight = stem2 ? stem2->length() + (beamWidthSp / 2) : defaultStemLengthEnd(trem2);
                    double offset3 = std::max(beamClearance * ec->intrinsicMag(), minOffset) * _spatium;
                    double sh = stemHeight + offset3;

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

    if (item->staffType()->isTabStaff()) {
        SlurTieLayout::avoidPreBendsOnTab(sc, ec, sp);
    }

    /// adding extra space above slurs for notes in circles
    if (Slur::engravingConfiguration()->enableExperimentalFretCircle() && item->staff()->staffType()->isCommonTabStaff()) {
        auto adjustSlur = [](Chord* ch, PointF& coord, bool up) {
            const Fraction halfFraction = Fraction(1, 2);
            if (ch && ch->ticks() >= halfFraction) {
                for (EngravingItem* item : ch->el()) {
                    if (item && item->isFretCircle()) {
                        coord += PointF(0, toFretCircle(item)->ldata()->offsetFromUpNote * (up ? -1 : 1));
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
    int lines = slurSeg->staff()->lines(slurSeg->tick());
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
                                    mu::draw::Transform& toSystemCoordinates, double& slurAngle)
{
    TRACEFUNC;
    Slur* slur = slurSeg->slur();
    ChordRest* startCR = slur->startCR();
    ChordRest* endCR = slur->endCR();

    if (!startCR || !endCR) {
        return;
    }

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
        return;
    }

    // Collect all the segments shapes spanned by this slur segment in a single vector
    std::vector<Shape> segShapes;
    for (Segment* seg = startSeg; seg && seg->tick() <= endSeg->tick(); seg = seg->next1enabled()) {
        if (seg->isType(SegmentType::BarLineType) || seg->isBreathType()) {
            continue;
        }
        segShapes.push_back(getSegmentShape(slurSeg, seg, startCR, endCR));
    }
    if (segShapes.empty()) {
        return;
    }

    // Collision clearance at the center of the slur
    double spatium = slurSeg->spatium();
    double slurLength = abs(p2.x() / spatium);
    double clearance;
    if (slurLength < 4) {
        clearance = 0.15 * spatium;
    } else if (slurLength < 8) {
        clearance = 0.4 * spatium;
    } else if (slurLength < 12) {
        clearance = 0.6 * spatium;
    } else {
        clearance = 0.75 * spatium;
    }
    // balance: determines how much endpoint adjustment VS shape adjustment we will do.
    // 0 = end point is fixed, only the shape can be adjusted,
    // 1 = shape is fixed, only end the point can be adjusted.
    // left and right side of the slur may have different balance depending on context:
    double leftBalance, rightBalance;
    if (slurSeg->isSingleBeginType() && !slur->stemFloated().left) {
        if (startCR->isChord() && toChord(startCR)->stem() && startCR->up() == slur->up()) {
            leftBalance = 0.1;
        } else {
            leftBalance = 0.4;
        }
    } else {
        leftBalance = 0.9;
    }
    if (slurSeg->isSingleEndType() && !slur->stemFloated().right) {
        if (endCR->isChord() && toChord(endCR)->stem() && endCR->up() == slur->up()) {
            rightBalance = 0.1;
        } else {
            rightBalance = 0.4;
        }
    } else {
        rightBalance = 0.9;
    }

    static constexpr unsigned maxIter = 30;     // Max iterations allowed
    const double vertClearance = slur->up() ? clearance : -clearance;
    // Optimize the slur shape and position in quarter-space steps
    double step = slur->up() ? -0.25 * spatium : 0.25 * spatium;
    // ...but allow long slurs to user coarser steps
    static constexpr double longSlurLimit = 16.0; // in spaces
    if (slurLength > longSlurLimit) {
        step *= slurLength / longSlurLimit;
        step = std::min(step, 1.5 * spatium);
    }
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
        CubicBezier clearanceBezier(PointF(0, 0), p3 + PointF(0.0, vertClearance), p4 + PointF(0.0, vertClearance), p2);
        for (unsigned i = 0; i < npoints - 1; i++) {
            PointF clearancePoint1 = clearanceBezier.pointAtPercent(double(i) / double(npoints));
            PointF clearancePoint2 = clearanceBezier.pointAtPercent(double(i + 1) / double(npoints));
            clearancePoint1 = toSystemCoordinates.map(clearancePoint1);
            clearancePoint2 = toSystemCoordinates.map(clearancePoint2);
            slurRects.push_back(RectF(clearancePoint1, clearancePoint2));
        }
        // Check collisions
        for (Shape& segShape : segShapes) {
            for (unsigned i=0; i < slurRects.size(); i++) {
                bool leftSection = i < slurRects.size() / 3;
                bool midSection = i >= slurRects.size() / 3 && i < 2 * slurRects.size() / 3;
                bool rightSection = i >= 2 * slurRects.size() / 3;
                if ((leftSection && collision.left)
                    || (midSection && collision.mid)
                    || (rightSection && collision.right)) {     // If a collision is already found in this section, no need to check again
                    continue;
                }
                bool intersection = slur->up() ? !Shape(slurRects[i]).clearsVertically(segShape)
                                    : !segShape.clearsVertically(slurRects[i]);
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
        }
        // In the even iterations, adjust the shape
        if (iter % 2 == 0) {
            double shapeLeftStep = (1 - leftBalance) * step;
            double shapeRightStep = (1 - rightBalance) * step;
            if (collision.left) {
                // Move left Bezier point up(/down) and outwards
                p3 += PointF(-abs(shapeLeftStep), shapeLeftStep);
                // and a bit also the right point to compensate asymmetry
                p4 += PointF(abs(shapeLeftStep), shapeLeftStep) / 2.0;
            }
            if (collision.mid) {     // Move both Bezier points up(/down)
                p3 += PointF(0.0, (shapeLeftStep + shapeRightStep) / 2);
                p4 += PointF(0.0, (shapeLeftStep + shapeRightStep) / 2);
            }
            if (collision.right) {
                // Move right Bezier point up(/down) and outwards
                p4 += PointF(abs(shapeRightStep), shapeRightStep);
                // and a bit also the left point to compensate asymmetry
                p3 += PointF(-abs(shapeRightStep), shapeRightStep) / 2.0;
            }
        } else if (!slurSeg->isEndPointsEdited()) {
            // In the odd iterations, adjust the end points
            // Slurs steeper than 45° are gently compensated
            static constexpr double steepLimit = M_PI / 4;
            if (collision.left || collision.mid || slurAngle < -steepLimit) {
                double endPointLeftStep = leftBalance * step;
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
            if (collision.right || collision.mid || slurAngle > steepLimit) {
                double endPointRightStep = rightBalance * step;
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
        const double maxRelativeHeight = abs(p2.x());
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
    } while ((collision.left || collision.mid || collision.right) && iter < maxIter);
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
        if (!shapeEl.item() || !shapeEl.item()->parentItem()) {
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
            || (item->isArticulationFamily() && parent == endCR) || item->isBend() || item->isStretchedBend()) {
            return true;
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
        return false;
    });

    return segShape;
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

TieSegment* SlurTieLayout::layoutTieWithNoEndNote(Tie* item)
{
    StaffType* st = item->staff()->staffType(item->startNote()->tick());
    Chord* c1 = item->startNote()->chord();
    item->setTick(c1->tick());

    if (item->slurDirection() == DirectionV::AUTO) {
        bool simpleException = st && st->isSimpleTabStaff();
        if (simpleException) {
            item->setUp(isUpVoice(c1->voice()));
        } else {
            if (c1->measure()->hasVoices(c1->staffIdx(), c1->tick(), c1->actualTicks())) {
                // in polyphonic passage, ties go on the stem side
                item->setUp(c1->up());
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
    segment->resetAdjustmentOffset();

    SlurTiePos sPos;
    computeStartAndEndSystem(item, sPos);
    sPos.p1 = computeDefaultStartOrEndPoint(item, Grip::START);
    sPos.p2 = computeDefaultStartOrEndPoint(item, Grip::END);

    segment->ups(Grip::START).p = sPos.p1;
    segment->ups(Grip::END).p = sPos.p2;

    computeBezier(segment);
    return segment;
}

static bool tieSegmentShouldBeSkipped(Tie* item)
{
    Note* startNote = item->startNote();
    StaffType* st = item->staff()->staffType(startNote ? startNote->tick() : Fraction(0, 1));
    if (!st || !st->isTabStaff()) {
        return false;
    }

    return !st->showBackTied() || (startNote && startNote->harmonic());
}

TieSegment* SlurTieLayout::tieLayoutFor(Tie* item, System* system)
{
    item->setPos(0, 0);

    if (!item->startNote()) {
        LOGD("no start note");
        return nullptr;
    }

    if (!item->endNote()) {
        return layoutTieWithNoEndNote(item);
    }

    // do not layout ties in tablature if not showing back-tied fret marks
    if (tieSegmentShouldBeSkipped(item)) {
        if (!item->segmentsEmpty()) {
            item->eraseSpannerSegments();
        }

        return nullptr;
    }

    item->calculateDirection();
    item->calculateIsInside();

    SlurTiePos sPos;
    sPos.p1 = computeDefaultStartOrEndPoint(item, Grip::START);

    computeStartAndEndSystem(item, sPos);

    int segmentCount = sPos.system1 == sPos.system2 ? 1 : 2;
    if (segmentCount == 2) {
        sPos.p2 = PointF(system->endingXForOpenEndedLines(), sPos.p1.y());
    } else {
        sPos.p2 = computeDefaultStartOrEndPoint(item, Grip::END);
    }

    correctForCrossStaff(item, sPos);
    forceHorizontal(item, sPos);

    item->fixupSegments(segmentCount);
    TieSegment* segment = item->segmentAt(0);
    segment->setTrack(item->track());
    segment->setSpannerSegmentType(sPos.system1 != sPos.system2 ? SpannerSegmentType::BEGIN : SpannerSegmentType::SINGLE);
    segment->setSystem(system);   // Needed to populate System.spannerSegments
    segment->resetAdjustmentOffset();

    Chord* startChord = item->startNote()->chord();
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

    segment->addLineAttachPoints(); // add attach points to start and end note
    return segment;
}

TieSegment* SlurTieLayout::tieLayoutBack(Tie* item, System* system)
{
    // do not layout ties in tablature if not showing back-tied fret marks
    if (tieSegmentShouldBeSkipped(item)) {
        if (!item->segmentsEmpty()) {
            item->eraseSpannerSegments();
        }

        return nullptr;
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

    segment->addLineAttachPoints();
    return segment;
}

void SlurTieLayout::computeStartAndEndSystem(Tie* item, SlurTiePos& slurTiePos)
{
    Chord* startChord = item->startNote()->chord();
    Chord* endChord = item->endNote() ? item->endNote()->chord() : nullptr;

    System* startSystem = startChord->measure()->system();

    if (!startSystem) {
        Measure* m = startChord->measure();
        LOGD("No system: measure is %d has %d count %d", m->isMMRest(), m->hasMMRest(), m->mmRestCount());
    }

    System* endSystem = endChord ? endChord->measure()->system() : startSystem;

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
    const int upSign = up ? -1 : 1;
    const int leftRightSign = start ? +1 : -1;
    const double noteWidth = note->width();
    const double noteHeight = note->height();
    const double spatium = tie->spatium();

    double baseX, baseY = 0.0;
    if (inside) {
        baseX = start ? noteWidth : 0.0;
    } else {
        baseX = noteOpticalCenterForTie(note, up);
        baseY = upSign * noteHeight / 2;
    }

    result += PointF(baseX, baseY);

    double visualInsetSp = 0.0;
    if (inside || note->headGroup() == NoteHeadGroup::HEAD_SLASH) {
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
        double noteWidth = note->width();
        double center = 0.20 * note->spatium() + 0.5 * (noteWidth - singleSlashWidth);
        return up ? note->shape().right() - center : note->shape().left() + center;
    }
    SymId symId = note->ldata()->cachedNoteheadSym.value();
    PointF cutOutLeft = note->symSmuflAnchor(symId, up ? SmuflAnchorId::cutOutNW : SmuflAnchorId::cutOutSW);
    PointF cutOutRight = note->symSmuflAnchor(symId, up ? SmuflAnchorId::cutOutNE : SmuflAnchorId::cutOutSE);

    if (cutOutLeft.isNull() || cutOutRight.isNull()) {
        return 0.5 * note->width();
    }

    return 0.5 * (cutOutLeft.x() + cutOutRight.x());
}

void SlurTieLayout::correctForCrossStaff(Tie* tie, SlurTiePos& sPos)
{
    Chord* startChord = tie->startNote() ? tie->startNote()->chord() : nullptr;
    Chord* endChord = tie->endNote() ? tie->endNote()->chord() : nullptr;

    if (!startChord) {
        return;
    }

    if (startChord->vStaffIdx() != tie->staffIdx() && sPos.system1) {
        double yOrigin = sPos.system1->staff(tie->staffIdx())->y();
        double yMoved = sPos.system1->staff(startChord->vStaffIdx())->y();
        double yDiff = yMoved - yOrigin;
        double curY = sPos.p1.y();
        sPos.p1.setY(curY + yDiff);
    }

    if (!endChord) {
        return;
    }

    if (endChord->vStaffIdx() != tie->staffIdx() && sPos.system2) {
        double yOrigin = sPos.system2->staff(tie->staffIdx())->y();
        double yMoved = sPos.system2->staff(endChord->vStaffIdx())->y();
        double yDiff = yMoved - yOrigin;
        double curY = sPos.p2.y();
        sPos.p2.setY(curY + yDiff);
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

    Tie* tie = tieSegment->tie();
    Note* note = start ? tie->startNote() : tie->endNote();
    if (!note) {
        return;
    }

    Chord* chord = note->chord();
    const double spatium = tieSegment->spatium();

    PointF& tiePoint = start ? sPos.p1 : sPos.p2;
    double resultingX = tiePoint.x();

    bool isOuterTieOfChord = tie->isOuterTieOfChord(startOrEnd);

    if (isOuterTieOfChord) {
        Tie* otherTie = start ? note->tieBack() : note->tieFor();
        bool avoidOtherTie = otherTie && otherTie->up() == tie->up() && !otherTie->isInside();
        if (avoidOtherTie) {
            resultingX += 0.1 * spatium * (start ? 1 : -1);
        }
    }

    bool avoidStem = chord->stem() && chord->stem()->visible() && chord->up() == tie->up();

    if (isOuterTieOfChord && !avoidStem) {
        tieSegment->addAdjustmentOffset(PointF(resultingX - tiePoint.x(), 0.0), startOrEnd);
        tiePoint.setX(resultingX);
        return;
    }

    PointF chordSystemPos = chord->pos() + chord->segment()->pos() + chord->measure()->pos();
    if (chord->vStaffIdx() != tieSegment->staffIdx()) {
        System* system = tieSegment->system();
        double yDiff = system->staff(chord->vStaffIdx())->y() - system->staff(tie->staffIdx())->y();
        chordSystemPos += PointF(0.0, yDiff);
    }
    Shape chordShape = chord->shape().translate(chordSystemPos);
    bool ignoreDot = start && isOuterTieOfChord;
    chordShape.remove_if([&](ShapeElement& s) {
        return !s.item() || (s.item() == note || s.item()->isHook() || s.item()->isLedgerLine() || (s.item()->isNoteDot() && ignoreDot));
    });

    const double arcSideMargin = 0.3 * spatium;
    const double pointsSideMargin = 0.15 * spatium;
    const double yBelow = tiePoint.y() - (tie->up() ? arcSideMargin : pointsSideMargin);
    const double yAbove = tiePoint.y() + (tie->up() ? pointsSideMargin : arcSideMargin);
    double pointToClear = start ? chordShape.rightMostEdgeAtHeight(yBelow, yAbove)
                          : chordShape.leftMostEdgeAtHeight(yBelow, yAbove);

    const double padding = 0.20 * spatium * (start ? 1 : -1); // TODO: style
    pointToClear += padding;

    resultingX = start ? std::max(resultingX, pointToClear) : std::min(resultingX, pointToClear);

    adjustXforLedgerLines(tieSegment, start, chord, note, chordSystemPos, padding, resultingX);

    tieSegment->addAdjustmentOffset(PointF(resultingX - tiePoint.x(), 0.0), startOrEnd);
    tiePoint.setX(resultingX);
}

void SlurTieLayout::adjustXforLedgerLines(TieSegment* tieSegment, bool start, Chord* chord, Note* note,
                                          const PointF& chordSystemPos, double padding, double& resultingX)
{
    if (tieSegment->tie()->isInside() || !chord->ledgerLines()) {
        return;
    }

    bool isOuterNote = note == chord->upNote() || note == chord->downNote();
    if (isOuterNote) {
        return;
    }

    bool ledgersAbove = false;
    bool ledgersBelow = false;
    for (LedgerLine* ledger = chord->ledgerLines(); ledger; ledger = ledger->next()) {
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

    Shape noteShape = note->shape().translated(note->pos() + chordSystemPos);
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
    if (!chord->ledgerLines()) {
        return;
    }
    PointF chordSystemPos = chord->pos() + chord->segment()->pos() + chord->segment()->measure()->pos();
    PointF& tiePoint = tieSegment->isSingleBeginType() ? sPos.p1 : sPos.p2;
    double spatium = tie->spatium();
    int upSign = tie->up() ? -1 : 1;
    double margin = 0.4 * spatium;

    for (LedgerLine* ledger = chord->ledgerLines(); ledger; ledger = ledger->next()) {
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
    Staff* staff = tieSegment->staff();
    if (!staff) {
        return;
    }

    Fraction tick = tieSegment->tick();

    computeBezier(tieSegment);

    bool up = tieSegment->tie()->up();
    int upSign = up ? -1 : 1;

    const double spatium = tieSegment->spatium();
    const double staffLineDist = staff->lineDistance(tick) * spatium;
    const double staffLineThickness = tieSegment->style().styleMM(Sid::staffLineWidth) * staff->staffMag(tick);

    // 1. Check for bad end point protrusion

    const double endPointY = tieSegment->ups(Grip::START).p.y();
    const int closestLineToEndpoints = up ? floor(endPointY / staffLineDist) : ceil(endPointY / staffLineDist);
    const bool isEndInsideStaff = closestLineToEndpoints >= 0 && closestLineToEndpoints < staff->lines(tick);
    const bool isEndInsideLedgerLines = !isEndInsideStaff && !tieSegment->tie()->isOuterTieOfChord(Grip::START);

    const double halfLineThicknessCorrection = 0.5 * staffLineThickness * upSign;
    const double protrusion = abs(endPointY - (closestLineToEndpoints * spatium - halfLineThicknessCorrection));
    const double badIntersectionLimit = 0.20 * spatium; // TODO: style

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

    double outwardMargin = -upSign * (yOuterApogee - (closestLineToArc * spatium - halfLineThicknessCorrection));
    double inwardMargin = upSign * (yInnerApogee - (closestLineToArc * spatium + halfLineThicknessCorrection));
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

    return tieStartPos.x() < startNotePos.x() + startNote->width() || tieEndPos.x() > endNotePos.x();
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

        Staff* staff = thisTie->staff();
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
    if (RealIsNull(tieEndNormalized.x())) {
        return;
    }

    const double tieAngle = atan(tieEndNormalized.y() / tieEndNormalized.x()); // angle required from tie start to tie end--zero if horizontal
    mu::draw::Transform t;
    t.rotateRadians(-tieAngle);  // rotate so that we are working with horizontal ties regardless of endpoint height difference
    tieEndNormalized = t.map(tieEndNormalized);  // apply that rotation
    shoulderOffset = t.map(shoulderOffset);  // also apply to shoulderOffset

    const double _spatium = tieSeg->spatium();
    double tieLengthInSp = tieEndNormalized.x() / _spatium;

    const double minShoulderHeight = tieSeg->style().styleMM(Sid::tieMinShoulderHeight);
    const double maxShoulderHeight = tieSeg->style().styleMM(Sid::tieMaxShoulderHeight);
    double shoulderH = minShoulderHeight + _spatium * 0.3 * sqrt(abs(tieLengthInSp - 1));
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

    mu::draw::PainterPath path = PainterPath();
    path.moveTo(PointF());
    path.cubicTo(bezier1 + bezier1Offset - tieThickness, bezier2 + bezier2Offset - tieThickness, tieEndNormalized);
    if (tieSeg->tie()->styleType() == SlurStyleType::Solid) {
        path.cubicTo(bezier2 + bezier2Offset + tieThickness, bezier1 + bezier1Offset + tieThickness, PointF());
    }

    tieThickness = PointF(0.0, 3.0 * tieSeg->ldata()->midThickness());

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

    Shape shape(Shape::Type::Composite);
    PointF start;
    start = t.map(start);

    double minH = std::abs(2 * tieSeg->ldata()->midThickness());
    int nbShapes = 15;
    const CubicBezier b(tieStart, tieSeg->ups(Grip::BEZIER1).pos(), tieSeg->ups(Grip::BEZIER2).pos(), tieSeg->ups(Grip::END).pos());
    for (int i = 1; i <= nbShapes; i++) {
        const PointF point = b.pointAtPercent(i / float(nbShapes));
        RectF re = RectF(start, point).normalized();
        if (re.height() < minH) {
            tieLengthInSp = (minH - re.height()) * .5;
            re.adjust(0.0, -tieLengthInSp, 0.0, tieLengthInSp);
        }
        shape.add(re, tieSeg);
        start = point;
    }

    tieSeg->mutldata()->setShape(shape);
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
    // Check slur integrity
    if (pp2 == pp1) {
        Measure* m1 = slurSeg->slur()->startCR()->segment()->measure();
        Measure* m2 = slurSeg->slur()->endCR()->segment()->measure();
        LOGD("zero slur at tick %d(%d) track %zu in measure %d-%d  tick %d ticks %d",
             m1->tick().ticks(), slurSeg->tick().ticks(), slurSeg->track(), m1->no(), m2->no(),
             slurSeg->slur()->tick().ticks(), slurSeg->slur()->ticks().ticks());
        slurSeg->slur()->setBroken(true);
        return;
    }

    // Set up coordinate transforms
    // CAUTION: transform operations are applies in reverse order to how
    // they are added to the transformation.
    double slurAngle = atan((pp2.y() - pp1.y()) / (pp2.x() - pp1.x()));
    mu::draw::Transform rotate;
    rotate.rotateRadians(-slurAngle);
    mu::draw::Transform toSlurCoordinates;
    toSlurCoordinates.rotateRadians(-slurAngle);
    toSlurCoordinates.translate(-pp1.x(), -pp1.y());
    mu::draw::Transform toSystemCoordinates = toSlurCoordinates.inverted();
    // Transform p2 and shoulder offset
    PointF p2 = toSlurCoordinates.map(pp2);
    shoulderOffset = rotate.map(shoulderOffset);

    // COMPUTE DEFAULT SLUR SHAPE
    // Compute default shoulder height and width
    double _spatium  = slurSeg->spatium();
    double shoulderW; // expressed as fraction of slur-length
    double shoulderH;
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
    shoulderH = sqrt(d / 4) * _spatium;

    static constexpr double shoulderReduction = 0.75;
    if (slurSeg->slur()->isOverBeams()) {
        shoulderH *= shoulderReduction;
    }
    shoulderH -= shoulderOffset.y();
    if (!slurSeg->slur()->up()) {
        shoulderH = -shoulderH;
    }

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
    if (slurSeg->autoplace()) {
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
    double w = slurSeg->style().styleMM(Sid::SlurMidWidth) - slurSeg->style().styleMM(Sid::SlurEndWidth);
    if (slurSeg->staff()) {
        w *= slurSeg->staff()->staffMag(slurSeg->slur()->tick());
    }
    if ((c2 - c1) <= _spatium) {
        w *= .5;
    }
    PointF thick(0.0, w);

    // Set path
    PainterPath path = PainterPath();
    path.moveTo(PointF());
    path.cubicTo(p3 - thick, p4 - thick, p2);
    if (slurSeg->slur()->styleType() == SlurStyleType::Solid) {
        path.cubicTo(p4 + thick, p3 + thick, PointF());
    }
    thick = PointF(0.0, 3.0 * w);

    path = toSystemCoordinates.map(path);
    slurSeg->mutldata()->path.set_value(path);

    // Create shape for the skyline
    Shape shape;
    PointF start = pp1;
    int nbShapes  = 32;
    double minH    = abs(2 * w);
    const CubicBezier b(slurSeg->ups(Grip::START).pos(), slurSeg->ups(Grip::BEZIER1).pos(), slurSeg->ups(Grip::BEZIER2).pos(),
                        slurSeg->ups(Grip::END).pos());
    for (int i = 1; i <= nbShapes; i++) {
        const PointF point = b.pointAtPercent(i / float(nbShapes));
        RectF re     = RectF(start, point).normalized();
        if (re.height() < minH) {
            double d1 = (minH - re.height()) * .5;
            re.adjust(0.0, -d1, 0.0, d1);
        }
        shape.add(re, slurSeg);
        start = point;
    }

    slurSeg->mutldata()->setShape(shape);
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
        if (!c->staff()->isDrumStaff(c->tick()) && c1->measure()->system() != m->system()) {
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

void SlurTieLayout::layoutSegment(SlurSegment* item, LayoutContext& ctx, const PointF& p1, const PointF& p2)
{
    SlurSegment::LayoutData* ldata = item->mutldata();
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

    computeBezier(item);
}

void SlurTieLayout::computeMidThickness(SlurTieSegment* slurTieSeg, double slurTieLengthInSp)
{
    const double mag = slurTieSeg->staff() ? slurTieSeg->staff()->staffMag(slurTieSeg->slurTie()->tick()) : 1.0;
    const double minTieLength = mag * slurTieSeg->style().styleS(Sid::MinTieLength).val();
    const double shortTieLimit = mag * 4.0;
    const double minTieThickness = mag * (0.15 * slurTieSeg->spatium() - slurTieSeg->style().styleMM(Sid::SlurEndWidth));
    const double normalThickness = mag * (slurTieSeg->style().styleMM(Sid::SlurMidWidth) - slurTieSeg->style().styleMM(Sid::SlurEndWidth));

    bool invalid = RealIsEqualOrMore(minTieLength, shortTieLimit);

    if (slurTieLengthInSp > shortTieLimit || invalid) {
        slurTieSeg->mutldata()->midThickness.set_value(normalThickness);
    } else {
        const double A = 1 / (shortTieLimit - minTieLength);
        const double B = normalThickness - minTieThickness;
        const double C = shortTieLimit * minTieThickness - minTieLength * normalThickness;
        slurTieSeg->mutldata()->midThickness.set_value(A * (B * slurTieLengthInSp + C));
    }
}
