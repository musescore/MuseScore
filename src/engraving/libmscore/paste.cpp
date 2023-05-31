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

#include "io/buffer.h"

#include "imimedata.h"

#include "rw/read400/tread.h"
#include "rw/rwregister.h"
#include "types/typesconv.h"

#include "articulation.h"
#include "beam.h"
#include "breath.h"
#include "chord.h"
#include "dynamic.h"
#include "factory.h"
#include "figuredbass.h"
#include "fret.h"
#include "hairpin.h"
#include "harmony.h"
#include "image.h"
#include "lyrics.h"
#include "measure.h"
#include "measurerepeat.h"
#include "mscoreview.h"
#include "part.h"
#include "rest.h"
#include "score.h"
#include "sig.h"
#include "staff.h"
#include "tie.h"
#include "tremolo.h"
#include "tuplet.h"
#include "undo.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   transposeChord
//---------------------------------------------------------

void Score::transposeChord(Chord* c, Interval srcTranspose, const Fraction& tick)
{
    // set note track
    // check if staffMove moves a note to a
    // nonexistent staff
    //
    track_idx_t track = c->track();
    size_t nn = (track / VOICES) + c->staffMove();
    if (nn >= c->score()->nstaves()) {
        c->setStaffMove(0);
    }
    Part* part = c->part();
    Interval dstTranspose = part->instrument(tick)->transpose();

    if (srcTranspose != dstTranspose) {
        if (!dstTranspose.isZero()) {
            dstTranspose.flip();
            for (Note* n : c->notes()) {
                int npitch;
                int ntpc;
                transposeInterval(n->pitch(), n->tpc1(), &npitch, &ntpc, dstTranspose, true);
                n->setTpc2(ntpc);
            }
        } else {
            for (Note* n : c->notes()) {
                n->setTpc2(n->tpc1());
            }
        }
    }
}

//---------------------------------------------------------
//   pasteStaff
//    return false if paste fails
//---------------------------------------------------------
bool Score::pasteStaff(XmlReader& e, Segment* dst, staff_idx_t dstStaff, Fraction scale)
{
    //! NOTE Needs refactoring - reading should be separated from insertion
    //! (we read the elements into some structure, then inserted them)
    return rw::RWRegister::latestReader()->pasteStaff(e, dst, dstStaff, scale);
}

//---------------------------------------------------------
//   pasteChordRest
//---------------------------------------------------------

void Score::pasteChordRest(ChordRest* cr, const Fraction& t, const Interval& srcTranspose)
{
    Fraction tick(t);
// LOGD("pasteChordRest %s at %d, len %d/%d", cr->typeName(), tick, cr->ticks().numerator(), cr->ticks().denominator() );

    Measure* measure = tick2measure(tick);
    if (!measure) {
        return;
    }

    int twoNoteTremoloFactor = 1;
    if (cr->isChord()) {
        transposeChord(toChord(cr), srcTranspose, tick);
        if (toChord(cr)->tremolo() && toChord(cr)->tremolo()->twoNotes()) {
            twoNoteTremoloFactor = 2;
        } else if (cr->durationTypeTicks() == (cr->actualTicks() * 2)) {
            // this could be the 2nd note of a two-note tremolo
            // check previous CR on same track, if it has a two-note tremolo, then set twoNoteTremoloFactor to 2
            Segment* seg = measure->undoGetSegment(SegmentType::ChordRest, tick);
            ChordRest* crt = seg->nextChordRest(cr->track(), true);
            if (crt && crt->isChord()) {
                Chord* chrt = toChord(crt);
                Tremolo* tr = chrt->tremolo();
                if (tr && tr->twoNotes()) {
                    twoNoteTremoloFactor = 2;
                }
            }
        }
    }

    // we can paste a measure rest as such only at start of measure
    // and only if the lengths of the rest and measure match
    // otherwise, we need to convert to duration rest(s)
    // and potentially split the rest up (eg, 5/4 => whole + quarter)
    bool convertMeasureRest = cr->isRest() && cr->durationType().type() == DurationType::V_MEASURE
                              && (tick != measure->tick() || cr->ticks() != measure->ticks());

    Fraction measureEnd = measure->endTick();
    bool isGrace = cr->isChord() && toChord(cr)->noteType() != NoteType::NORMAL;

    // adjust measures for measure repeat
    if (cr->isMeasureRepeat()) {
        MeasureRepeat* mr = toMeasureRepeat(cr);
        Measure* m = (mr->numMeasures() == 4 ? measure->prevMeasure() : measure);
        for (int i = 1; i <= mr->numMeasures(); ++i) {
            undo(new ChangeMeasureRepeatCount(m, i, mr->staffIdx()));
            if (i < mr->numMeasures()) {
                m->undoSetNoBreak(true);
            }
            m = m->nextMeasure();
        }
    }

    // find out if the chordrest was only partially contained in the copied range
    bool partialCopy = false;
    if (cr->isMeasureRepeat()) {
        partialCopy = toMeasureRepeat(cr)->actualTicks() != measure->ticks();
    } else if (!isGrace && !cr->tuplet()) {
        partialCopy = cr->durationTypeTicks() != (cr->actualTicks() * twoNoteTremoloFactor);
    }

    // if note is too long to fit in measure, split it up with a tie across the barline
    // exclude tuplets from consideration
    // we have already disallowed a tuplet from crossing the barline, so there is no problem here
    // but due to rounding, it might appear from actualTicks() that the last note is too long by a couple of ticks

    if (!isGrace && !cr->tuplet() && (tick + cr->actualTicks() > measureEnd || partialCopy || convertMeasureRest)) {
        if (cr->isChord()) {
            // split Chord
            Chord* c = toChord(cr);
            Fraction rest = c->actualTicks();
            bool firstpart = true;
            while (rest.isNotZero()) {
                measure = tick2measure(tick);
                Chord* c2 = firstpart ? c : toChord(c->clone());
                if (!firstpart) {
                    c2->removeMarkings(true);
                }
                Fraction mlen = measure->tick() + measure->ticks() - tick;
                Fraction len = mlen > rest ? rest : mlen;
                std::vector<TDuration> dl = toRhythmicDurationList(len, false, tick - measure->tick(), sigmap()->timesig(
                                                                       tick).nominal(), measure, MAX_DOTS);
                TDuration d = dl[0];
                c2->setDurationType(d);
                c2->setTicks(d.fraction());
                rest -= c2->actualTicks();
                undoAddCR(c2, measure, tick);

                std::vector<Note*> nl1 = c->notes();
                std::vector<Note*> nl2 = c2->notes();

                if (!firstpart) {
                    for (unsigned i = 0; i < nl1.size(); ++i) {
                        Tie* tie = Factory::createTie(nl1[i]);
                        tie->setStartNote(nl1[i]);
                        tie->setEndNote(nl2[i]);
                        tie->setTick(tie->startNote()->tick());
                        tie->setTick2(tie->endNote()->tick());
                        tie->setTrack(c->track());
                        Tie* tie2 = nl1[i]->tieFor();
                        if (tie2) {
                            nl2[i]->setTieFor(nl1[i]->tieFor());
                            tie2->setStartNote(nl2[i]);
                        }
                        nl1[i]->setTieFor(tie);
                        nl2[i]->setTieBack(tie);
                    }
                }
                c = c2;
                firstpart = false;
                tick += c->actualTicks();
            }
        } else if (cr->isRest()) {
            // split Rest
            Rest* r       = toRest(cr);
            Fraction rest = r->ticks();

            bool firstpart = true;
            while (!rest.isZero()) {
                Rest* r2      = firstpart ? r : toRest(r->clone());
                measure       = tick2measure(tick);
                Fraction mlen = measure->tick() + measure->ticks() - tick;
                Fraction len  = rest > mlen ? mlen : rest;
                std::vector<TDuration> dl = toRhythmicDurationList(len, true, tick - measure->tick(), sigmap()->timesig(
                                                                       tick).nominal(), measure, MAX_DOTS);
                TDuration d = dl[0];
                r2->setDurationType(d);
                r2->setTicks(d.isMeasure() ? measure->ticks() : d.fraction());
                undoAddCR(r2, measure, tick);
                rest -= r2->ticks();
                tick += r2->actualTicks();
                firstpart = false;
            }
        } else if (cr->isMeasureRepeat()) {
            MeasureRepeat* mr = toMeasureRepeat(cr);
            std::vector<TDuration> list = toDurationList(mr->actualTicks(), true);
            for (auto dur : list) {
                Rest* r = Factory::createRest(this->dummy()->segment(), dur);
                r->setTrack(cr->track());
                Fraction rest = r->ticks();
                while (!rest.isZero()) {
                    Rest* r2      = toRest(r->clone());
                    measure       = tick2measure(tick);
                    Fraction mlen = measure->tick() + measure->ticks() - tick;
                    Fraction len  = rest > mlen ? mlen : rest;
                    std::vector<TDuration> dl = toDurationList(len, false);
                    TDuration d = dl[0];
                    r2->setTicks(d.fraction());
                    r2->setDurationType(d);
                    undoAddCR(r2, measure, tick);
                    rest -= d.fraction();
                    tick += r2->actualTicks();
                }
                delete r;
            }
            delete cr;
        }
    } else {
        undoAddCR(cr, measure, tick);
    }
}

//---------------------------------------------------------
//   pasteSymbols
//
//    pastes a list of symbols into cr and following ChordRest's
//
//    (Note: info about delta ticks is currently ignored)
//---------------------------------------------------------

void Score::pasteSymbols(XmlReader& e, ChordRest* dst)
{
    read400::ReadContext ctx(score());
    ctx.setPasteMode(true);

    Segment* currSegm = dst->segment();
    Fraction destTick = Fraction(0, 1);                // the tick and track to place the pasted element at
    track_idx_t destTrack = 0;
    bool done        = false;
    int segDelta    = 0;
    Segment* startSegm= currSegm;
    Fraction startTick   = dst->tick();        // the initial tick and track where to start pasting
    track_idx_t startTrack  = dst->track();
    track_idx_t maxTrack    = ntracks();
    Fraction lastTick = lastSegment()->tick();

    while (e.readNextStartElement()) {
        if (done) {
            break;
        }
        if (e.name() != "SymbolList") {
            e.unknown();
            break;
        }
        String version = e.attribute("version", u"NONE");
        if (version != MSC_VERSION) {
            break;
        }

        while (e.readNextStartElement()) {
            if (done) {
                break;
            }
            const AsciiStringView tag(e.name());

            if (tag == "trackOffset") {
                destTrack = startTrack + e.readInt();
                currSegm  = startSegm;
            } else if (tag == "tickOffset") {
                destTick = startTick + Fraction::fromTicks(e.readInt());
            } else if (tag == "segDelta") {
                segDelta = e.readInt();
            } else {
                if (tag == "Harmony" || tag == "FretDiagram") {
                    //
                    // Harmony elements (= chord symbols) are positioned respecting
                    // the original tickOffset: advance to destTick (or near)
                    // same for FretDiagram elements
                    //
                    Segment* harmSegm;
                    for (harmSegm = startSegm; harmSegm && (harmSegm->tick() < destTick);
                         harmSegm = harmSegm->nextCR()) {
                    }
                    // if destTick overshot, no dest. segment: create one
                    if (destTick >= lastTick) {
                        harmSegm = nullptr;
                    } else if (!harmSegm || harmSegm->tick() > destTick) {
                        Measure* meas     = tick2measure(destTick);
                        harmSegm          = meas ? meas->undoGetSegment(SegmentType::ChordRest, destTick) : nullptr;
                    }
                    if (destTrack >= maxTrack || harmSegm == nullptr) {
                        LOGD("PasteSymbols: no track or segment for %s", tag.ascii());
                        e.skipCurrentElement();                   // ignore
                        continue;
                    }
                    if (tag == "Harmony") {
                        Harmony* el = Factory::createHarmony(harmSegm);
                        el->setTrack(trackZeroVoice(destTrack));
                        read400::TRead::read(el, e, ctx);
                        el->setTrack(trackZeroVoice(destTrack));
                        // transpose
                        Part* partDest = staff(track2staff(destTrack))->part();
                        Interval interval = partDest->instrument(destTick)->transpose();
                        if (!styleB(Sid::concertPitch) && !interval.isZero()) {
                            interval.flip();
                            int rootTpc = transposeTpc(el->rootTpc(), interval, true);
                            int baseTpc = transposeTpc(el->baseTpc(), interval, true);
                            undoTransposeHarmony(el, rootTpc, baseTpc);
                        }
                        el->setParent(harmSegm);
                        undoAddElement(el);
                    } else {
                        FretDiagram* el = Factory::createFretDiagram(harmSegm);
                        el->setTrack(trackZeroVoice(destTrack));
                        read400::TRead::read(el, e, ctx);
                        el->setTrack(trackZeroVoice(destTrack));
                        el->setParent(harmSegm);
                        undoAddElement(el);
                    }
                } else if (tag == "Dynamic") {
                    ChordRest* destCR = findCR(destTick, destTrack);
                    if (!destCR) {
                        e.skipCurrentElement();
                        continue;
                    }
                    Dynamic* d = Factory::createDynamic(destCR->segment());
                    d->setTrack(destTrack);
                    read400::TRead::read(d, e, ctx);
                    d->setTrack(destTrack);
                    d->setParent(destCR->segment());
                    undoAddElement(d);
                } else if (tag == "HairPin") {
                    Hairpin* h = Factory::createHairpin(this->dummy()->segment());
                    h->setTrack(destTrack);
                    read400::TRead::read(h, e, ctx);
                    h->setTrack(destTrack);
                    h->setTrack2(destTrack);
                    h->setTick(destTick);
                    undoAddElement(h);
                } else {
                    //
                    // All other elements are positioned respecting the distance in chords
                    //
                    for (; currSegm && segDelta > 0; segDelta--) {
                        currSegm = currSegm->nextCR(destTrack);
                    }
                    // check the intended dest. track and segment exist
                    if (destTrack >= maxTrack || currSegm == nullptr) {
                        LOGD("PasteSymbols: no track or segment for %s", tag.ascii());
                        e.skipCurrentElement();                   // ignore
                        continue;
                    }
                    // check there is a segment element in the required track
                    if (currSegm->element(destTrack) == nullptr) {
                        LOGD("PasteSymbols: no track element for %s", tag.ascii());
                        e.skipCurrentElement();
                        continue;
                    }
                    ChordRest* cr = toChordRest(currSegm->element(destTrack));

                    if (tag == "Articulation") {
                        Articulation* el = Factory::createArticulation(cr);
                        read400::TRead::read(el, e, ctx);
                        el->setTrack(destTrack);
                        el->setParent(cr);
                        if (!el->isFermata() && cr->isRest()) {
                            delete el;
                        } else {
                            undoAddElement(el);
                        }
                    } else if (tag == "StaffText" || tag == "PlayTechAnnotation" || tag == "Sticking" || tag == "HarpPedalDiagram") {
                        EngravingItem* el = Factory::createItemByName(tag, this->dummy());
                        read400::TRead::readItem(el, e, ctx);
                        el->setTrack(destTrack);
                        el->setParent(currSegm);
                        if (el->isSticking() && cr->isRest()) {
                            delete el;
                        } else {
                            undoAddElement(el);
                        }
                    } else if (tag == "FiguredBass") {
                        // FiguredBass always belongs to first staff voice
                        destTrack = trackZeroVoice(destTrack);
                        Fraction ticks;
                        FiguredBass* el = Factory::createFiguredBass(currSegm);
                        el->setTrack(destTrack);
                        read400::TRead::read(el, e, ctx);
                        el->setTrack(destTrack);
                        // if f.b. is off-note, we have to locate a place before currSegm
                        // where an on-note f.b. element could (potentially) be
                        // (while having an off-note f.b. without an on-note one before it
                        // is un-idiomatic, possible mismatch in rhythmic patterns between
                        // copy source and paste destination does not allow to be too picky)
                        if (!el->onNote()) {
                            FiguredBass* onNoteFB = nullptr;
                            Segment* prevSegm = currSegm;
                            bool done1    = false;
                            while (prevSegm) {
                                if (done1) {
                                    break;
                                }
                                prevSegm = prevSegm->prev1(SegmentType::ChordRest);
                                // if there is a ChordRest in the dest. track
                                // this segment is a (potential) f.b. location
                                if (prevSegm->element(destTrack) != nullptr) {
                                    done1 = true;
                                }
                                // in any case, look for a f.b. in annotations:
                                // if there is a f.b. element in the right track,
                                // this is an (actual) f.b. location
                                for (EngravingItem* a : prevSegm->annotations()) {
                                    if (a->isFiguredBass() && a->track() == destTrack) {
                                        onNoteFB = toFiguredBass(a);
                                        done1 = true;
                                    }
                                }
                            }
                            if (!prevSegm) {
                                LOGD("PasteSymbols: can't place off-note FiguredBass");
                                delete el;
                                continue;
                            }
                            // by default, split on-note duration in half: half on-note and half off-note
                            Fraction totTicks  = currSegm->tick() - prevSegm->tick();
                            Fraction destTick1 = prevSegm->tick() + (totTicks * Fraction(1, 2));
                            ticks         = totTicks * Fraction(1, 2);
                            if (onNoteFB) {
                                onNoteFB->setTicks(totTicks * Fraction(1, 2));
                            }
                            // look for a segment at this tick; if none, create one
                            Segment* nextSegm = prevSegm;
                            while (nextSegm && nextSegm->tick() < destTick1) {
                                nextSegm = nextSegm->next1(SegmentType::ChordRest);
                            }
                            if (!nextSegm || nextSegm->tick() > destTick1) {                    // no ChordRest segm at this tick
                                nextSegm = Factory::createSegment(prevSegm->measure(), SegmentType::ChordRest, destTick1);
                                if (!nextSegm) {
                                    LOGD("PasteSymbols: can't find or create destination segment for FiguredBass");
                                    delete el;
                                    continue;
                                }
                                undoAddElement(nextSegm);
                            }
                            currSegm = nextSegm;
                        } else {
                            // by default, assign to FiguredBass element the duration of the chord it refers to
                            ticks = toChordRest(currSegm->element(destTrack))->ticks();
                        }
                        // in both cases, look for an existing f.b. element in segment and remove it, if found
                        FiguredBass* oldFB = nullptr;
                        for (EngravingItem* a : currSegm->annotations()) {
                            if (a->isFiguredBass() && a->track() == destTrack) {
                                oldFB = toFiguredBass(a);
                                break;
                            }
                        }
                        if (oldFB) {
                            undoRemoveElement(oldFB);
                        }
                        el->setParent(currSegm);
                        el->setTicks(ticks);
                        undoAddElement(el);
                    } else if (tag == "Lyrics") {
                        // with lyrics, skip rests
                        while (!cr->isChord() && currSegm) {
                            currSegm = currSegm->nextCR(destTrack);
                            if (currSegm) {
                                cr = toChordRest(currSegm->element(destTrack));
                            } else {
                                break;
                            }
                        }
                        if (currSegm == nullptr) {
                            LOGD("PasteSymbols: no segment for Lyrics");
                            e.skipCurrentElement();
                            continue;
                        }
                        if (!cr->isChord()) {
                            LOGD("PasteSymbols: can't paste Lyrics to rest");
                            e.skipCurrentElement();
                            continue;
                        }
                        Lyrics* el = Factory::createLyrics(cr);
                        el->setTrack(destTrack);
                        read400::TRead::read(el, e, ctx);
                        el->setTrack(destTrack);
                        el->setParent(cr);
                        undoAddElement(el);
                    } else {
                        LOGD("PasteSymbols: element %s not handled", tag.ascii());
                        e.skipCurrentElement();                // ignore
                    }
                }                         // if !Harmony
            }                             // if element
        }                                 // outer while readNextstartElement()
    }                                     // inner while readNextstartElement()
}                                         // pasteSymbolList()

static ChordRest* replaceWithRest(ChordRest* target)
{
    target->score()->undoRemoveElement(target);
    return target->score()->addRest(target->segment(), target->track(), target->ticks(), target->tuplet());
}

static Note* prepareTarget(ChordRest* target, Note* with, const Fraction& duration)
{
    if (!target->segment()->element(target->track())) {
        return nullptr; // target was removed by previous operation, ignore this
    }
    if (target->isChord() && target->ticks() > duration) {
        target = replaceWithRest(target); // prevent unexpected note splitting
    }
    Segment* segment = target->segment();
    if (segment->measure()->isMMRest()) {
        Measure* m = segment->measure()->mmRestFirst();
        segment = m->findSegment(SegmentType::ChordRest, m->tick());
    }
    segment = target->score()->setNoteRest(segment, target->track(),
                                           with->noteVal(), duration, DirectionV::AUTO, false, {}, false, &target->score()->inputState());
    return toChord(segment->nextChordRest(target->track()))->upNote();
}

static EngravingItem* prepareTarget(EngravingItem* target, Note* with, const Fraction& duration)
{
    if (target->isNote() && toNote(target)->chord()->ticks() != duration) {
        return prepareTarget(toNote(target)->chord(), with, duration);
    }
    if (target->isChordRest()
        && (toChordRest(target)->ticks() != duration || toChordRest(target)->durationType().type() == DurationType::V_MEASURE)) {
        return prepareTarget(toChordRest(target), with, duration);
    }
    return target;
}

static bool canPasteStaff(XmlReader& reader, const Fraction& scale)
{
    if (scale != Fraction(1, 1)) {
        while (reader.readNext() && reader.tokenType() != XmlReader::TokenType::EndDocument) {
            AsciiStringView tag(reader.name());
            Fraction len = Fraction::fromString(reader.attribute("len"));
            if (!len.isZero() && !TDuration(len * scale).isValid()) {
                return false;
            }
            if (tag == "durationType") {
                if (!TDuration(TDuration(TConv::fromXml(reader.readAsciiText(),
                                                        DurationType::V_INVALID)).fraction() * scale).isValid()) {
                    return false;
                }
            }
        }
    }
    return true;
}

inline static bool canPasteStaff(const ByteArray& mimeData, const Fraction& scale)
{
    XmlReader reader(mimeData);
    return canPasteStaff(reader, scale);
}

//---------------------------------------------------------
//   cmdPaste
//---------------------------------------------------------

void Score::cmdPaste(const IMimeData* ms, MuseScoreView* view, Fraction scale)
{
    if (ms == 0) {
        LOGD("no application mime data");
        MScore::setError(MsError::NO_MIME);
        return;
    }
    if ((_selection.isSingle() || _selection.isList()) && ms->hasFormat(mimeSymbolFormat)) {
        ByteArray data = ms->data(mimeSymbolFormat);

        PointF dragOffset;
        Fraction duration(1, 4);
        std::unique_ptr<EngravingItem> el(EngravingItem::readMimeData(this, data, &dragOffset, &duration));

        if (!el) {
            return;
        }
        duration *= scale;
        if (!TDuration(duration).isValid()) {
            return;
        }

        std::vector<EngravingItem*> els;
        if (_selection.isSingle()) {
            els.push_back(_selection.element());
        } else {
            els.insert(els.begin(), _selection.elements().begin(), _selection.elements().end());
        }
        EngravingItem* newEl = 0;
        for (EngravingItem* target : els) {
            el->setTrack(target->track());
            addRefresh(target->abbox());         // layout() ?!
            EditData ddata(view);
            ddata.dropElement = el.get();
            if (target->acceptDrop(ddata)) {
                if (!el->isNote() || (target = prepareTarget(target, toNote(el.get()), duration))) {
                    ddata.dropElement = el->clone();
                    EngravingItem* dropped = target->drop(ddata);
                    if (dropped) {
                        newEl = dropped;
                    }
                }
            }
        }
        if (newEl) {
            select(newEl);
        }
    } else if ((_selection.isRange() || _selection.isList()) && ms->hasFormat(mimeStaffListFormat)) {
        ChordRest* cr = 0;
        if (_selection.isRange()) {
            cr = _selection.firstChordRest();
        } else if (_selection.isSingle()) {
            EngravingItem* e = _selection.element();
            if (!e->isNote() && !e->isChordRest()) {
                LOGD("cannot paste to %s", e->typeName());
                MScore::setError(MsError::DEST_NO_CR);
                return;
            }
            if (e->isNote()) {
                e = toNote(e)->chord();
            }
            cr  = toChordRest(e);
        }
        if (cr == 0) {
            MScore::setError(MsError::NO_DEST);
            return;
        } else if (cr->tuplet() && cr->tick() != cr->topTuplet()->tick()) {
            MScore::setError(MsError::DEST_TUPLET);
            return;
        } else {
            ByteArray data = ms->data(mimeStaffListFormat);
            if (MScore::debugMode) {
                LOGD("paste <%s>", data.data());
            }
            if (canPasteStaff(data, scale)) {
                XmlReader e(data);
                if (!pasteStaff(e, cr->segment(), cr->staffIdx(), scale)) {
                    return;
                }
            }
        }
    } else if (ms->hasFormat(mimeSymbolListFormat)) {
        ChordRest* cr = 0;
        if (_selection.isRange()) {
            cr = _selection.firstChordRest();
        } else if (_selection.isSingle()) {
            EngravingItem* e = _selection.element();
            if (!e->isNote() && !e->isRest() && !e->isChord()) {
                LOGD("cannot paste to %s", e->typeName());
                MScore::setError(MsError::DEST_NO_CR);
                return;
            }
            if (e->isNote()) {
                e = toNote(e)->chord();
            }
            cr  = toChordRest(e);
        }
        if (cr == 0) {
            MScore::setError(MsError::NO_DEST);
            return;
        } else {
            ByteArray data = ms->data(mimeSymbolListFormat);
            if (MScore::debugMode) {
                LOGD("paste <%s>", data.data());
            }
            XmlReader e(data);
            pasteSymbols(e, cr);
        }
    } else if (ms->hasImage()) {
        ByteArray ba;
        Buffer buffer(&ba);
        buffer.open(IODevice::WriteOnly);

        auto px = ms->imageData();
        imageProvider()->saveAsPng(px, &buffer);

        Image* image = new Image(this->dummy());
        image->setImageType(ImageType::RASTER);
        image->loadFromData("dragdrop", ba);

        std::vector<EngravingItem*> els;
        if (_selection.isSingle()) {
            els.push_back(_selection.element());
        } else {
            els.insert(els.begin(), _selection.elements().begin(), _selection.elements().end());
        }

        for (EngravingItem* target : els) {
            EngravingItem* nel = image->clone();
            addRefresh(target->abbox());         // layout() ?!
            EditData ddata(view);
            ddata.dropElement    = nel;
            if (target->acceptDrop(ddata)) {
                target->drop(ddata);
                if (_selection.element()) {
                    addRefresh(_selection.element()->abbox());
                }
            }
        }
        delete image;
    } else {
        LOGD("cannot paste selState %d staffList %s", int(_selection.state()), (ms->hasFormat(mimeStaffListFormat)) ? "true" : "false");
        for (const std::string& s : ms->formats()) {
            LOGD() << " format: " << s;
        }
    }
}
}
