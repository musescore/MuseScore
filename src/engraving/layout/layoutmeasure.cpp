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
#include "layoutmeasure.h"

#include "libmscore/ambitus.h"
#include "libmscore/barline.h"
#include "libmscore/beam.h"
#include "libmscore/factory.h"
#include "libmscore/keysig.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/lyrics.h"
#include "libmscore/marker.h"
#include "libmscore/measure.h"
#include "libmscore/mmrest.h"
#include "libmscore/part.h"
#include "libmscore/score.h"
#include "libmscore/stem.h"
#include "libmscore/timesig.h"
#include "libmscore/undo.h"

#include "layoutcontext.h"
#include "layoutbeams.h"
#include "layoutchords.h"
#include "layouttremolo.h"

#include "log.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   createMMRest
//    create a multimeasure rest
//    from firstMeasure to lastMeasure (inclusive)
//---------------------------------------------------------

void LayoutMeasure::createMMRest(const LayoutOptions& options, Score* score, Measure* firstMeasure, Measure* lastMeasure,
                                 const Fraction& len)
{
    int numMeasuresInMMRest = 1;
    if (firstMeasure != lastMeasure) {
        for (Measure* m = firstMeasure->nextMeasure(); m; m = m->nextMeasure()) {
            ++numMeasuresInMMRest;
            m->setMMRestCount(-1);
            if (m->mmRest()) {
                score->undo(new ChangeMMRest(m, 0));
            }
            if (m == lastMeasure) {
                break;
            }
        }
    }

    // mmrMeasure coexists with n undisplayed measures of rests
    Measure* mmrMeasure = firstMeasure->mmRest();
    if (mmrMeasure) {
        // reuse existing mmrest
        if (mmrMeasure->ticks() != len) {
            Segment* s = mmrMeasure->findSegmentR(SegmentType::EndBarLine, mmrMeasure->ticks());
            // adjust length
            mmrMeasure->setTicks(len);
            // move existing end barline
            if (s) {
                s->setRtick(len);
            }
        }
        mmrMeasure->removeSystemTrailer();
    } else {
        mmrMeasure = Factory::createMeasure(score->dummy()->system());
        mmrMeasure->setTicks(len);
        mmrMeasure->setTick(firstMeasure->tick());
        score->undo(new ChangeMMRest(firstMeasure, mmrMeasure));
    }
    mmrMeasure->setTimesig(firstMeasure->timesig());
    mmrMeasure->setPageBreak(lastMeasure->pageBreak());
    mmrMeasure->setLineBreak(lastMeasure->lineBreak());
    mmrMeasure->setMMRestCount(numMeasuresInMMRest);
    mmrMeasure->setNo(firstMeasure->no());

    //
    // set mmrMeasure with same barline as last underlying measure
    //
    Segment* lastMeasureEndBarlineSeg = lastMeasure->findSegmentR(SegmentType::EndBarLine, lastMeasure->ticks());
    if (lastMeasureEndBarlineSeg) {
        Segment* mmrEndBarlineSeg = mmrMeasure->undoGetSegmentR(SegmentType::EndBarLine, mmrMeasure->ticks());
        for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            EngravingItem* e = lastMeasureEndBarlineSeg->element(staffIdx * VOICES);
            if (e) {
                bool generated = e->generated();
                if (!mmrEndBarlineSeg->element(staffIdx * VOICES)) {
                    EngravingItem* eClone = generated ? e->clone() : e->linkedClone();
                    eClone->setGenerated(generated);
                    eClone->setParent(mmrEndBarlineSeg);
                    score->undoAddElement(eClone);
                } else {
                    BarLine* mmrEndBarline = toBarLine(mmrEndBarlineSeg->element(staffIdx * VOICES));
                    BarLine* lastMeasureEndBarline = toBarLine(e);
                    if (!generated && !mmrEndBarline->links()) {
                        score->undo(new Link(mmrEndBarline, lastMeasureEndBarline));
                    }
                    if (mmrEndBarline->barLineType() != lastMeasureEndBarline->barLineType()) {
                        // change directly when generating mmrests, do not change underlying measures or follow links
                        score->undo(new ChangeProperty(mmrEndBarline, Pid::BARLINE_TYPE,
                                                       PropertyValue::fromValue(lastMeasureEndBarline->barLineType()),
                                                       PropertyFlags::NOSTYLE));
                        score->undo(new ChangeProperty(mmrEndBarline, Pid::GENERATED, generated, PropertyFlags::NOSTYLE));
                    }
                }
            }
        }
    }

    //
    // if last underlying measure ends with clef change, show same at end of mmrest
    //
    Segment* lastMeasureClefSeg = lastMeasure->findSegmentR(SegmentType::Clef | SegmentType::HeaderClef,
                                                            lastMeasure->ticks());
    if (lastMeasureClefSeg) {
        Segment* mmrClefSeg = mmrMeasure->undoGetSegment(lastMeasureClefSeg->segmentType(), lastMeasure->endTick());
        for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            const track_idx_t track = staff2track(staffIdx);
            EngravingItem* e = lastMeasureClefSeg->element(track);
            if (e && e->isClef()) {
                Clef* lastMeasureClef = toClef(e);
                if (!mmrClefSeg->element(track)) {
                    Clef* mmrClef = lastMeasureClef->generated() ? lastMeasureClef->clone() : toClef(
                        lastMeasureClef->linkedClone());
                    mmrClef->setParent(mmrClefSeg);
                    score->undoAddElement(mmrClef);
                } else {
                    Clef* mmrClef = toClef(mmrClefSeg->element(track));
                    mmrClef->setClefType(lastMeasureClef->clefType());
                    mmrClef->setShowCourtesy(lastMeasureClef->showCourtesy());
                }
            }
        }
    }

    mmrMeasure->setRepeatStart(firstMeasure->repeatStart() || lastMeasure->repeatStart());
    mmrMeasure->setRepeatEnd(firstMeasure->repeatEnd() || lastMeasure->repeatEnd());
    mmrMeasure->setSectionBreak(lastMeasure->sectionBreak());

    //
    // copy markers to mmrMeasure
    //
    ElementList oldList = mmrMeasure->takeElements();
    ElementList newList = lastMeasure->el();
    for (EngravingItem* e : firstMeasure->el()) {
        if (e->isMarker()) {
            newList.push_back(e);
        }
    }
    for (EngravingItem* e : newList) {
        bool found = false;
        for (EngravingItem* ee : oldList) {
            if (ee->type() == e->type() && ee->subtype() == e->subtype()) {
                mmrMeasure->add(ee);
                auto i = std::find(oldList.begin(), oldList.end(), ee);
                if (i != oldList.end()) {
                    oldList.erase(i);
                }
                found = true;
                break;
            }
        }
        if (!found) {
            mmrMeasure->add(e->clone());
        }
    }
    for (EngravingItem* e : oldList) {
        delete e;
    }
    Segment* s = mmrMeasure->undoGetSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        track_idx_t track = staffIdx * VOICES;
        if (s->element(track) == 0) {
            MMRest* mmr = Factory::createMMRest(s);
            mmr->setDurationType(DurationType::V_MEASURE);
            mmr->setTicks(mmrMeasure->ticks());
            mmr->setTrack(track);
            mmr->setParent(s);
            score->undo(new AddElement(mmr));
        }
    }

    //
    // further check for clefs
    //
    Segment* underlyingSeg = lastMeasure->findSegmentR(SegmentType::Clef, lastMeasure->ticks());
    Segment* mmrSeg = mmrMeasure->findSegment(SegmentType::Clef, lastMeasure->endTick());
    if (underlyingSeg) {
        if (mmrSeg == 0) {
            mmrSeg = mmrMeasure->undoGetSegmentR(SegmentType::Clef, lastMeasure->ticks());
        }
        mmrSeg->setEnabled(underlyingSeg->enabled());
        mmrSeg->setTrailer(underlyingSeg->trailer());
        for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            Clef* clef = toClef(underlyingSeg->element(track));
            if (clef) {
                if (mmrSeg->element(track) == 0) {
                    mmrSeg->add(clef->clone());
                } else {
                    //TODO: check if same clef
                }
            }
        }
    } else if (mmrSeg) {
        // TODO: remove elements from mmrSeg?
        score->undo(new RemoveElement(mmrSeg));
    }

    //
    // check for time signature
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::TimeSig, Fraction(0, 1));
    mmrSeg = mmrMeasure->findSegment(SegmentType::TimeSig, firstMeasure->tick());
    if (underlyingSeg) {
        if (mmrSeg == 0) {
            mmrSeg = mmrMeasure->undoGetSegmentR(SegmentType::TimeSig, Fraction(0, 1));
        }
        mmrSeg->setEnabled(underlyingSeg->enabled());
        mmrSeg->setHeader(underlyingSeg->header());
        for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            TimeSig* underlyingTimeSig = toTimeSig(underlyingSeg->element(track));
            if (underlyingTimeSig) {
                TimeSig* mmrTimeSig = toTimeSig(mmrSeg->element(track));
                if (!mmrTimeSig) {
                    mmrTimeSig = underlyingTimeSig->generated() ? underlyingTimeSig->clone() : toTimeSig(
                        underlyingTimeSig->linkedClone());
                    mmrTimeSig->setParent(mmrSeg);
                    score->undo(new AddElement(mmrTimeSig));
                } else {
                    mmrTimeSig->setSig(underlyingTimeSig->sig(), underlyingTimeSig->timeSigType());
                    mmrTimeSig->layout();
                }
            }
        }
    } else if (mmrSeg) {
        // TODO: remove elements from mmrSeg?
        score->undo(new RemoveElement(mmrSeg));
    }

    //
    // check for ambitus
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::Ambitus, Fraction(0, 1));
    mmrSeg = mmrMeasure->findSegment(SegmentType::Ambitus, firstMeasure->tick());
    if (underlyingSeg) {
        if (mmrSeg == 0) {
            mmrSeg = mmrMeasure->undoGetSegmentR(SegmentType::Ambitus, Fraction(0, 1));
        }
        for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            Ambitus* underlyingAmbitus = toAmbitus(underlyingSeg->element(track));
            if (underlyingAmbitus) {
                Ambitus* mmrAmbitus = toAmbitus(mmrSeg->element(track));
                if (!mmrAmbitus) {
                    mmrAmbitus = underlyingAmbitus->clone();
                    mmrAmbitus->setParent(mmrSeg);
                    score->undo(new AddElement(mmrAmbitus));
                } else {
                    mmrAmbitus->initFrom(underlyingAmbitus);
                    mmrAmbitus->layout();
                }
            }
        }
    } else if (mmrSeg) {
        // TODO: remove elements from mmrSeg?
        score->undo(new RemoveElement(mmrSeg));
    }

    //
    // check for key signature
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::KeySig, Fraction(0, 1));
    mmrSeg = mmrMeasure->findSegmentR(SegmentType::KeySig, Fraction(0, 1));
    if (underlyingSeg) {
        if (mmrSeg == 0) {
            mmrSeg = mmrMeasure->undoGetSegmentR(SegmentType::KeySig, Fraction(0, 1));
        }
        mmrSeg->setEnabled(underlyingSeg->enabled());
        mmrSeg->setHeader(underlyingSeg->header());
        for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            KeySig* underlyingKeySig  = toKeySig(underlyingSeg->element(track));
            if (underlyingKeySig) {
                KeySig* mmrKeySig = toKeySig(mmrSeg->element(track));
                if (!mmrKeySig) {
                    mmrKeySig = underlyingKeySig->generated() ? underlyingKeySig->clone() : toKeySig(
                        underlyingKeySig->linkedClone());
                    mmrKeySig->setParent(mmrSeg);
                    mmrKeySig->setGenerated(true);
                    score->undo(new AddElement(mmrKeySig));
                } else {
                    if (!(mmrKeySig->keySigEvent() == underlyingKeySig->keySigEvent())) {
                        bool addKey = underlyingKeySig->isChange();
                        score->undo(new ChangeKeySig(mmrKeySig, underlyingKeySig->keySigEvent(), mmrKeySig->showCourtesy(),
                                                     addKey));
                    }
                }
            }
        }
    } else if (mmrSeg) {
        mmrSeg->setEnabled(false);
        // TODO: remove elements from mmrSeg, then delete mmrSeg
        // previously we removed the segment if not empty,
        // but this resulted in "stale" keysig in mmrest after removed from underlying measure
        //undo(new RemoveElement(mmrSeg));
    }

    mmrMeasure->checkHeader();
    mmrMeasure->checkTrailer();

    //
    // check for rehearsal mark etc.
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    if (underlyingSeg) {
        // clone elements from underlying measure to mmr
        for (EngravingItem* e : underlyingSeg->annotations()) {
            // look at elements in underlying measure
            if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText() || e->isTripletFeel()
                  || e->isPlayTechAnnotation() || e->isInstrumentChange())) {
                continue;
            }
            // try to find a match in mmr
            bool found = false;
            for (EngravingItem* ee : s->annotations()) {
                if (mu::contains(e->linkList(), static_cast<EngravingObject*>(ee))) {
                    found = true;
                    break;
                }
            }
            // add to mmr if no match found
            if (!found) {
                EngravingItem* eClone = e->linkedClone();
                eClone->setParent(s);
                score->undo(new AddElement(eClone));
            }
        }

        // remove stray elements (possibly leftover from a previous layout of this mmr)
        // this should not happen since the elements are linked?
        const auto annotations = s->annotations(); // make a copy since we alter the list
        for (EngravingItem* e : annotations) { // look at elements in mmr
            if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText() || e->isTripletFeel()
                  || e->isPlayTechAnnotation() || e->isInstrumentChange())) {
                continue;
            }
            // try to find a match in underlying measure
            bool found = false;
            for (EngravingItem* ee : underlyingSeg->annotations()) {
                if (mu::contains(e->linkList(), static_cast<EngravingObject*>(ee))) {
                    found = true;
                    break;
                }
            }
            // remove from mmr if no match found
            if (!found) {
                score->undo(new RemoveElement(e));
            }
        }
    }

    MeasureBase* nm = options.showVBox ? lastMeasure->next() : lastMeasure->nextMeasure();
    mmrMeasure->setNext(nm);
    mmrMeasure->setPrev(firstMeasure->prev());
}

//---------------------------------------------------------
// validMMRestMeasure
//    return true if this might be a measure in a
//    multi measure rest
//---------------------------------------------------------

static bool validMMRestMeasure(const LayoutContext& ctx, Measure* m)
{
    if (m->irregular()) {
        return false;
    }

    int n = 0;
    for (Segment* s = m->first(); s; s = s->next()) {
        for (EngravingItem* e : s->annotations()) {
            if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText() || e->isTripletFeel()
                  || e->isPlayTechAnnotation() || e->isInstrumentChange())) {
                return false;
            }
        }
        if (s->isChordRestType()) {
            bool restFound = false;
            size_t tracks = ctx.score()->ntracks();
            for (track_idx_t track = 0; track < tracks; ++track) {
                if ((track % VOICES) == 0 && !ctx.score()->staff(track / VOICES)->show()) {
                    track += VOICES - 1;
                    continue;
                }
                if (s->element(track)) {
                    if (!s->element(track)->isRest()) {
                        return false;
                    }
                    restFound = true;
                }
            }
            for (EngravingItem* e : s->annotations()) {
                if (e->isFermata()) {
                    return false;
                }
            }
            if (restFound) {
                ++n;
            }
            // measure is not empty if there is more than one rest
            if (n > 1) {
                return false;
            }
        }
    }
    return true;
}

//---------------------------------------------------------
//  breakMultiMeasureRest
//    return true if this measure should start a new
//    multi measure rest
//---------------------------------------------------------

static bool breakMultiMeasureRest(const LayoutContext& ctx, Measure* m)
{
    if (m->breakMultiMeasureRest()) {
        return true;
    }

    if (m->repeatStart()
        || (m->prevMeasure() && m->prevMeasure()->repeatEnd())
        || (m->isIrregular())
        || (m->prevMeasure() && m->prevMeasure()->isIrregular())
        || (m->prevMeasure() && (m->prevMeasure()->sectionBreak()))) {
        return true;
    }

    auto sl = ctx.score()->spannerMap().findOverlapping(m->tick().ticks(), m->endTick().ticks());
    for (auto i : sl) {
        Spanner* s = i.value;
        // break for first measure of volta or textline and first measure *after* volta
        if ((s->isVolta() || s->isGradualTempoChange() || s->isTextLine()) && (s->tick() == m->tick() || s->tick2() == m->tick())) {
            return true;
        }
    }

    // break for marker in this measure
    for (EngravingItem* e : m->el()) {
        if (e->isMarker()) {
            Marker* mark = toMarker(e);
            if (!(mark->align() == AlignH::RIGHT)) {
                return true;
            }
        }
    }

    // break for marker & jump in previous measure
    Measure* pm = m->prevMeasure();
    if (pm) {
        for (EngravingItem* e : pm->el()) {
            if (e->isJump()) {
                return true;
            } else if (e->isMarker()) {
                Marker* mark = toMarker(e);
                if (mark->align() == AlignH::RIGHT) {
                    return true;
                }
            }
        }
    }

    // break for MeasureRepeat group
    for (size_t staffIdx = 0; staffIdx < ctx.score()->nstaves(); ++staffIdx) {
        if (m->isMeasureRepeatGroup(staffIdx)
            || (m->prevMeasure() && m->prevMeasure()->isMeasureRepeatGroup(staffIdx))) {
            return true;
        }
    }

    for (Segment* s = m->first(); s; s = s->next()) {
        for (EngravingItem* e : s->annotations()) {
            if (!e->visible()) {
                continue;
            }
            if (e->isRehearsalMark()
                || e->isTempoText()
                || ((e->isHarmony() || e->isStaffText() || e->isSystemText() || e->isTripletFeel() || e->isPlayTechAnnotation()
                     || e->isInstrumentChange())
                    && (e->systemFlag() || ctx.score()->staff(e->staffIdx())->show()))) {
                return true;
            }
        }
        for (size_t staffIdx = 0; staffIdx < ctx.score()->nstaves(); ++staffIdx) {
            if (!ctx.score()->staff(staffIdx)->show()) {
                continue;
            }
            EngravingItem* e = s->element(staffIdx * VOICES);
            if (!e || e->generated()) {
                continue;
            }
            if (s->isStartRepeatBarLineType()) {
                return true;
            }
            if (s->isType(SegmentType::KeySig | SegmentType::TimeSig) && m->tick().isNotZero()) {
                return true;
            }
            if (s->isClefType()) {
                if (s->tick() != m->endTick() && m->tick().isNotZero()) {
                    return true;
                }
            }
        }
    }
    if (pm) {
        Segment* s = pm->findSegmentR(SegmentType::EndBarLine, pm->ticks());
        if (s) {
            for (size_t staffIdx = 0; staffIdx < ctx.score()->nstaves(); ++staffIdx) {
                BarLine* bl = toBarLine(s->element(staffIdx * VOICES));
                if (bl) {
                    BarLineType t = bl->barLineType();
                    if (t != BarLineType::NORMAL && t != BarLineType::BROKEN && t != BarLineType::DOTTED && !bl->generated()) {
                        return true;
                    } else {
                        break;
                    }
                }
            }
        }
        if (pm->findSegment(SegmentType::Clef, m->tick())) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   layoutDrumsetChord
//---------------------------------------------------------

static void layoutDrumsetChord(Chord* c, const Drumset* drumset, const StaffType* st, double spatium)
{
    for (Note* note : c->notes()) {
        int pitch = note->pitch();
        if (!drumset->isValid(pitch)) {
            // LOGD("unmapped drum note %d", pitch);
        } else if (!note->fixed()) {
            note->undoChangeProperty(Pid::HEAD_GROUP, int(drumset->noteHead(pitch)));
            int line = drumset->line(pitch);
            note->setLine(line);

            int off  = st->stepOffset();
            double ld = st->lineDistance().val();
            note->setPosY((line + off * 2.0) * spatium * .5 * ld);
        }
    }
}

void LayoutMeasure::getNextMeasure(const LayoutOptions& options, LayoutContext& ctx)
{
    Score* score = ctx.score();
    ctx.prevMeasure = ctx.curMeasure;
    ctx.curMeasure  = ctx.nextMeasure;
    if (!ctx.curMeasure) {
        ctx.nextMeasure = options.showVBox ? score->first() : score->firstMeasure();
    } else {
        ctx.nextMeasure = options.showVBox ? ctx.curMeasure->next() : ctx.curMeasure->nextMeasure();
    }
    if (!ctx.curMeasure) {
        return;
    }

    int mno = adjustMeasureNo(ctx, ctx.curMeasure);

    if (ctx.curMeasure->isMeasure()) {
        if (ctx.score()->styleB(Sid::createMultiMeasureRests)) {
            Measure* m = toMeasure(ctx.curMeasure);
            Measure* nm = m;
            Measure* lm = nm;
            int n       = 0;
            Fraction len;

            while (validMMRestMeasure(ctx, nm)) {
                MeasureBase* mb = options.showVBox ? nm->next() : nm->nextMeasure();
                if (breakMultiMeasureRest(ctx, nm) && n) {
                    break;
                }
                if (nm != m) {
                    adjustMeasureNo(ctx, nm);
                }
                ++n;
                len += nm->ticks();
                lm = nm;
                if (!(mb && mb->isMeasure())) {
                    break;
                }
                nm = toMeasure(mb);
            }
            if (n >= score->styleI(Sid::minEmptyMeasures)) {
                createMMRest(options, score, m, lm, len);
                ctx.curMeasure  = m->mmRest();
                ctx.nextMeasure = options.showVBox ? lm->next() : lm->nextMeasure();
            } else {
                if (m->mmRest()) {
                    score->undo(new ChangeMMRest(m, 0));
                }
                m->setMMRestCount(0);
                ctx.measureNo = mno;
            }
        } else if (toMeasure(ctx.curMeasure)->isMMRest()) {
            LOGD("mmrest: no %d += %d", ctx.measureNo, toMeasure(ctx.curMeasure)->mmRestCount());
            ctx.measureNo += toMeasure(ctx.curMeasure)->mmRestCount() - 1;
        }
    }
    if (!ctx.curMeasure->isMeasure()) {
        ctx.curMeasure->setTick(ctx.tick);
        return;
    }

    //-----------------------------------------
    //    process one measure
    //-----------------------------------------

    Measure* measure = toMeasure(ctx.curMeasure);
    measure->moveTicks(ctx.tick - measure->tick());

    if (score->linearMode() && (measure->tick() < ctx.startTick || measure->tick() > ctx.endTick)) {
        // needed to reset segment widths if they can change after measure width is computed
        //for (Segment& s : measure->segments())
        //      s.createShapes();
        ctx.tick += measure->ticks();
        return;
    }

    measure->connectTremolo();

    //
    // calculate accidentals and note lines,
    // create stem and set stem direction
    //
    for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        const Staff* staff     = score->Score::staff(staffIdx);
        const Drumset* drumset
            = staff->part()->instrument(measure->tick())->useDrumset() ? staff->part()->instrument(measure->tick())->drumset() : 0;
        AccidentalState as;          // list of already set accidentals for this measure
        as.init(staff->keySigEvent(measure->tick()));

        for (Segment& segment : measure->segments()) {
            // TODO? maybe we do need to process it here to make it possible to enable later
            //if (!segment.enabled())
            //      continue;
            if (segment.isKeySigType()) {
                KeySig* ks = toKeySig(segment.element(staffIdx * VOICES));
                if (!ks) {
                    continue;
                }
                Fraction tick = segment.tick();
                as.init(staff->keySigEvent(tick));
                ks->layout();
            } else if (segment.isChordRestType()) {
                const StaffType* st = staff->staffTypeForElement(&segment);
                track_idx_t track     = staffIdx * VOICES;
                track_idx_t endTrack  = track + VOICES;

                for (track_idx_t t = track; t < endTrack; ++t) {
                    ChordRest* cr = segment.cr(t);
                    if (!cr) {
                        continue;
                    }
                    // Check if requested cross-staff is possible
                    if (cr->staffMove() || cr->storedStaffMove()) {
                        cr->checkStaffMoveValidity();
                    }

                    double m = staff->staffMag(&segment);
                    if (cr->isSmall()) {
                        m *= score->styleD(Sid::smallNoteMag);
                    }

                    if (cr->isChord()) {
                        Chord* chord = toChord(cr);
                        chord->cmdUpdateNotes(&as);
                        for (Chord* c : chord->graceNotes()) {
                            c->setMag(m * score->styleD(Sid::graceNoteMag));
                            c->setTrack(t);
                            c->computeUp();
                            if (drumset) {
                                layoutDrumsetChord(c, drumset, st, score->spatium());
                            }
                            c->layoutStem();
                        }
                        if (drumset) {
                            layoutDrumsetChord(chord, drumset, st, score->spatium());
                        }
                        chord->computeUp();
                        chord->layoutStem();               // create stems needed to calculate spacing
                                                           // stem direction can change later during beam processing

                        // if there is a two-note tremolo attached, and it is too steep,
                        // extend stem of one of the chords (if not cross-staff)
                        // or extend both stems (if cross-staff)
                        // this should be done after the stem lengths of two notes are both calculated
                        if (chord->tremolo() && chord == chord->tremolo()->chord2()) {
                            Stem* stem1 = chord->tremolo()->chord1()->stem();
                            Stem* stem2 = chord->tremolo()->chord2()->stem();
                            if (stem1 && stem2) {
                                std::pair<double, double> extendedLen = LayoutTremolo::extendedStemLenWithTwoNoteTremolo(
                                    chord->tremolo(),
                                    stem1->p2().y(),
                                    stem2->p2().y());
                                stem1->setBaseLength(Millimetre(extendedLen.first));
                                stem2->setBaseLength(Millimetre(extendedLen.second));
                            }
                        }
                    }
                    cr->setMag(m);
                }
            } else if (segment.isClefType()) {
                EngravingItem* e = segment.element(staffIdx * VOICES);
                if (e) {
                    toClef(e)->setSmall(true);
                    e->layout();
                }
            } else if (segment.isType(SegmentType::TimeSig | SegmentType::Ambitus | SegmentType::HeaderClef)) {
                EngravingItem* e = segment.element(staffIdx * VOICES);
                if (e) {
                    e->layout();
                }
            }
        }
    }

    LayoutBeams::createBeams(score, ctx, measure);
    /* HACK: The real beam layout is computed at much later stage (you can't do the beams until you know
     * horizontal spacing). However, horizontal spacing needs to know stems extensions to avoid collision
     * with stems, and stems extensions depend on beams. Solution: we compute dummy beams here, *before*
     * horizontal spacing. It is pointless for the beams themselves, but it *does* correctly extend the
     * stems, thus allowing to compute horizontal spacing correctly. (M.S.) */
    for (Segment& s : measure->segments()) {
        if (!s.isChordRestType()) {
            continue;
        }
        LayoutBeams::layoutNonCrossBeams(&s);
    }

    for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        for (Segment& segment : measure->segments()) {
            if (segment.isChordRestType()) {
                LayoutChords::layoutChords1(score, &segment, staffIdx);
                for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
                    ChordRest* cr = segment.cr(staffIdx * VOICES + voice);
                    if (cr) {
                        for (Lyrics* l : cr->lyrics()) {
                            if (l) {
                                l->layout();
                            }
                        }
                    }
                }
            }
        }
    }

    for (Segment& segment : measure->segments()) {
        if (segment.isBreathType()) {
            for (EngravingItem* e : segment.elist()) {
                if (e && e->isBreath()) {
                    e->layout();
                }
            }
        } else if (segment.isChordRestType()) {
            for (EngravingItem* e : segment.annotations()) {
                if (e->isSymbol()) {
                    e->layout();
                }
            }
        }
    }

    Segment* seg = measure->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
    if (measure->repeatStart()) {
        if (!seg) {
            seg = measure->getSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
        }
        measure->barLinesSetSpan(seg);          // this also creates necessary barlines
        for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            BarLine* b = toBarLine(seg->element(staffIdx * VOICES));
            if (b) {
                b->setBarLineType(BarLineType::START_REPEAT);
                b->layout();
            }
        }
    } else if (seg) {
        score->undoRemoveElement(seg);
    }

    for (Segment& s : measure->segments()) {
        if (s.isEndBarLineType()) {
            continue;
        }
        s.createShapes();
    }

    LayoutChords::updateGraceNotes(measure);

    measure->computeTicks(); // Must be called *after* Segment::createShapes() because it relies on the
    // Segment::visible() property, which is determined by Segment::createShapes().

    ctx.tick += measure->ticks();
}

//---------------------------------------------------------
//   adjustMeasureNo
//---------------------------------------------------------

int LayoutMeasure::adjustMeasureNo(LayoutContext& lc, MeasureBase* m)
{
    lc.measureNo += m->noOffset();
    m->setNo(lc.measureNo);
    if (!m->irregular()) {          // don’t count measure
        ++lc.measureNo;
    }

    const LayoutBreak* layoutBreak = m->sectionBreakElement();
    if (layoutBreak && layoutBreak->startWithMeasureOne()) {
        lc.measureNo = 0;
    }

    return lc.measureNo;
}

/****************************************************************
 * computePreSpacingItems
 * Computes information that is needed before horizontal spacing.
 * Caution: assumes that the system is known! (which is why we
 * cannot compute this stuff in LayoutMeasure::getNextMeasure().)
 * **************************************************************/
void LayoutMeasure::computePreSpacingItems(Measure* m)
{
    // Compute chord properties
    bool isFirstChordInMeasure = true;
    LayoutChords::clearLineAttachPoints(m);
    for (Segment& seg : m->segments()) {
        if (!seg.isChordRestType()) {
            continue;
        }
        for (EngravingItem* e : seg.elist()) {
            if (!e || !e->isChord()) {
                continue;
            }
            Chord* chord = toChord(e);

            LayoutChords::updateLineAttachPoints(chord, isFirstChordInMeasure);
            for (Chord* gn : chord->graceNotes()) {
                LayoutChords::updateLineAttachPoints(gn, false);
            }
            isFirstChordInMeasure = false;

            chord->layoutArticulations();
            chord->layoutArticulations2();
            chord->checkStartEndSlurs();
            chord->computeKerningExceptions();
        }
        seg.createShapes();
    }
}
