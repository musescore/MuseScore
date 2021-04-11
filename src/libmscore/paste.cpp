//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QMimeData>
#include <QBuffer>

#include "score.h"

#include "rest.h"
#include "staff.h"
#include "measure.h"
#include "harmony.h"
#include "fret.h"
#include "breath.h"
#include "beam.h"
#include "figuredbass.h"
#include "ottava.h"
#include "part.h"
#include "lyrics.h"
#include "hairpin.h"
#include "tie.h"
#include "tuplet.h"
#include "utils.h"
#include "xml.h"
#include "image.h"
#include "measurerepeat.h"
#include "chord.h"
#include "tremolo.h"
#include "slur.h"
#include "articulation.h"
#include "sig.h"
#include "undo.h"

namespace Ms {
//---------------------------------------------------------
//   transposeChord
//---------------------------------------------------------

static void transposeChord(Chord* c, Interval srcTranspose, const Fraction& tick)
{
    // set note track
    // check if staffMove moves a note to a
    // nonexistent staff
    //
    int track  = c->track();
    int nn     = (track / VOICES) + c->staffMove();
    if (nn < 0 || nn >= c->score()->nstaves()) {
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

bool Score::pasteStaff(XmlReader& e, Segment* dst, int dstStaff, Fraction scale)
{
    Q_ASSERT(dst->isChordRestType());

    std::vector<Harmony*> pastedHarmony;
    QList<Chord*> graceNotes;
    Beam* startingBeam = nullptr;
    Tuplet* tuplet = nullptr;
    Fraction dstTick = dst->tick();
    bool pasted = false;
    Fraction tickLen = Fraction(0,1);
    int staves  = 0;
    bool done   = false;
    bool doScale = (scale != Fraction(1, 1));

    while (e.readNextStartElement()) {
        if (done) {
            break;
        }
        if (e.name() != "StaffList") {
            e.unknown();
            break;
        }
        QString version = e.attribute("version", "NONE");
        if (!MScore::testMode) {
            if (version != MSC_VERSION) {
                qDebug("pasteStaff: bad version");
                break;
            }
        }
        Fraction tickStart = Fraction::fromTicks(e.intAttribute("tick", 0));
        tickLen       =  Fraction::fromTicks(e.intAttribute("len", 0));
        Fraction oTickLen =  tickLen;
        tickLen       *= scale;
        int staffStart    = e.intAttribute("staff", 0);
        staves        = e.intAttribute("staves", 0);

        Fraction oEndTick = dstTick + oTickLen;
        auto oSpanner = spannerMap().findContained(dstTick.ticks(), oEndTick.ticks());
        bool spannerFound = false;

        e.setTickOffset(dstTick - tickStart);
        e.setTick(Fraction(0,1));

        while (e.readNextStartElement()) {
            if (done) {
                break;
            }
            if (e.name() != "Staff") {
                e.unknown();
                break;
            }
            e.setTransposeChromatic(0);
            e.setTransposeDiatonic(0);

            int srcStaffIdx = e.attribute("id", "0").toInt();
            e.setTrack(srcStaffIdx * VOICES);
            e.setTrackOffset((dstStaff - staffStart) * VOICES);
            int dstStaffIdx = e.track() / VOICES;
            if (dstStaffIdx >= dst->score()->nstaves()) {
                qDebug("paste beyond staves");
                done = true;
                break;
            }

            while (e.readNextStartElement()) {
                pasted = true;
                const QStringRef& tag(e.name());

                if (tag == "transposeChromatic") {
                    e.setTransposeChromatic(e.readInt());
                } else if (tag == "transposeDiatonic") {
                    e.setTransposeDiatonic(e.readInt());
                } else if (tag == "voiceOffset") {
                    int voiceOffset[VOICES];
                    std::fill(voiceOffset, voiceOffset + VOICES, -1);
                    while (e.readNextStartElement()) {
                        if (e.name() != "voice") {
                            e.unknown();
                        }
                        int voiceId = e.attribute("id", "-1").toInt();
                        Q_ASSERT(voiceId >= 0 && voiceId < VOICES);
                        voiceOffset[voiceId] = e.readInt();
                    }
                    e.readNext();
                    if (!makeGap1(dstTick, dstStaffIdx, tickLen, voiceOffset)) {
                        qDebug("cannot make gap in staff %d at tick %d", dstStaffIdx, dstTick.ticks());
                        done = true;             // break main loop, cannot make gap
                        break;
                    }
                } else if (tag == "location") {
                    Location loc = Location::relative();
                    loc.read(e);
                    e.setLocation(loc);
                } else if (tag == "Tuplet") {
                    Tuplet* oldTuplet = tuplet;
                    Fraction tick = doScale ? (e.tick() - dstTick) * scale + dstTick : e.tick();
                    // no paste into local time signature
                    if (staff(dstStaffIdx)->isLocalTimeSignature(tick)) {
                        MScore::setError(MsError::DEST_LOCAL_TIME_SIGNATURE);
                        if (oldTuplet && oldTuplet->elements().empty()) {
                            delete oldTuplet;
                        }
                        return false;
                    }
                    tuplet = new Tuplet(this);
                    tuplet->setTrack(e.track());
                    tuplet->read(e);
                    if (doScale) {
                        tuplet->setTicks(tuplet->ticks() * scale);
                        tuplet->setBaseLen(tuplet->baseLen().fraction() * scale);
                    }
                    Measure* measure = tick2measure(tick);
                    tuplet->setParent(measure);
                    tuplet->setTick(tick);
                    tuplet->setTuplet(oldTuplet);
                    if (tuplet->rtick() + tuplet->actualTicks() > measure->ticks()) {
                        delete tuplet;
                        if (oldTuplet && oldTuplet->elements().empty()) {
                            delete oldTuplet;
                        }
                        MScore::setError(MsError::TUPLET_CROSSES_BAR);
                        return false;
                    }
                    if (oldTuplet) {
                        tuplet->readAddTuplet(oldTuplet);
                    }
                } else if (tag == "endTuplet") {
                    if (!tuplet) {
                        qDebug("Score::pasteStaff: encountered <endTuplet/> when no tuplet was started");
                        e.skipCurrentElement();
                        continue;
                    }
                    Tuplet* oldTuplet = tuplet;
                    tuplet = tuplet->tuplet();
                    if (oldTuplet->elements().empty()) {
                        qDebug("Score::pasteStaff: ended tuplet is empty");
                        if (tuplet) {
                            tuplet->remove(oldTuplet);
                        }
                        delete oldTuplet;
                    } else {
                        oldTuplet->sortElements();
                    }
                    e.readNext();
                } else if (tag == "Chord" || tag == "Rest" || tag == "MeasureRepeat") {
                    ChordRest* cr = toChordRest(Element::name2Element(tag, this));
                    cr->setTrack(e.track());
                    cr->read(e);
                    cr->setSelected(false);
                    Fraction tick = doScale ? (e.tick() - dstTick) * scale + dstTick : e.tick();
                    // no paste into local time signature
                    if (staff(dstStaffIdx)->isLocalTimeSignature(tick)) {
                        MScore::setError(MsError::DEST_LOCAL_TIME_SIGNATURE);
                        return false;
                    }
                    if (tick2measure(tick)->isMeasureRepeatGroup(dstStaffIdx)) {
                        MeasureRepeat* mr = tick2measure(tick)->measureRepeatElement(dstStaffIdx);
                        score()->deleteItem(mr);    // resets any measures related to mr
                    }
                    if (startingBeam) {
                        startingBeam->add(cr);             // also calls cr->setBeam(startingBeam)
                        startingBeam = nullptr;
                    }
                    if (cr->isGrace()) {
                        graceNotes.push_back(toChord(cr));
                    } else {
                        if (tuplet) {
                            cr->readAddTuplet(tuplet);
                        }
                        e.incTick(cr->actualTicks());
                        if (doScale) {
                            Fraction d = cr->durationTypeTicks();
                            cr->setTicks(cr->ticks() * scale);
                            cr->setDurationType(d * scale);
                            for (Lyrics* l : cr->lyrics()) {
                                l->setTicks(l->ticks() * scale);
                            }
                        }
                        if (cr->isChord()) {
                            Chord* chord = toChord(cr);
                            // disallow tie across barline within two-note tremolo
                            // tremolos can potentially still straddle the barline if no tie is required
                            // but these will be removed later
                            Tremolo* t = chord->tremolo();
                            if (t && t->twoNotes()) {
                                if (doScale) {
                                    Fraction d = t->durationType().ticks();
                                    t->setDurationType(d * scale);
                                }
                                Measure* m = tick2measure(tick);
                                Fraction ticks = cr->actualTicks();
                                Fraction rticks = m->endTick() - tick;
                                if (rticks < ticks || (rticks != ticks && rticks < ticks * 2)) {
                                    MScore::setError(MsError::DEST_TREMOLO);
                                    return false;
                                }
                            }
                            for (int i = 0; i < graceNotes.size(); ++i) {
                                Chord* gc = graceNotes[i];
                                gc->setGraceIndex(i);
                                transposeChord(gc, e.transpose(), tick);
                                chord->add(gc);
                            }
                            graceNotes.clear();
                        }
                        // delete pending ties, they are not selected when copy
                        if ((tick - dstTick) + cr->actualTicks() >= tickLen) {
                            if (cr->isChord()) {
                                Chord* c = toChord(cr);
                                for (Note* note: c->notes()) {
                                    Tie* tie = note->tieFor();
                                    if (tie) {
                                        note->setTieFor(0);
                                        delete tie;
                                    }
                                }
                            }
                        }
                        // shorten last cr to fit in the space made by makeGap
                        if ((tick - dstTick) + cr->actualTicks() > tickLen) {
                            Fraction newLength = tickLen - (tick - dstTick);
                            // check previous CR on same track, if it has tremolo, delete the tremolo
                            // we don't want a tremolo and two different chord durations
                            if (cr->isChord()) {
                                Segment* s = tick2leftSegment(tick - Fraction::fromTicks(1));
                                if (s) {
                                    ChordRest* crt = toChordRest(s->element(cr->track()));
                                    if (!crt) {
                                        crt = s->nextChordRest(cr->track(), true);
                                    }
                                    if (crt && crt->isChord()) {
                                        Chord* chrt = toChord(crt);
                                        Tremolo* tr = chrt->tremolo();
                                        if (tr) {
                                            tr->setChords(chrt, toChord(cr));
                                            chrt->remove(tr);
                                            delete tr;
                                        }
                                    }
                                }
                            }
                            if (!cr->tuplet()) {
                                // shorten duration
                                // exempt notes in tuplets, since we don't allow copy of partial tuplet anyhow
                                // TODO: figure out a reasonable fudge factor to make sure shorten tuplets appropriately if we do ever copy a partial tuplet
                                cr->setTicks(newLength);
                                cr->setDurationType(newLength);
                            }
                        }
                        pasteChordRest(cr, tick, e.transpose());
                    }
                } else if (tag == "Spanner") {
                    Spanner::readSpanner(e, this, e.track());
                    spannerFound = true;
                } else if (tag == "Harmony") {
                    Harmony* harmony = new Harmony(this);
                    harmony->setTrack(e.track());
                    harmony->read(e);
                    harmony->setTrack(e.track());
                    // transpose
                    Fraction tick = doScale ? (e.tick() - dstTick) * scale + dstTick : e.tick();
                    Part* partDest = staff(e.track() / VOICES)->part();
                    Interval interval = partDest->instrument(tick)->transpose();
                    if (!styleB(Sid::concertPitch) && !interval.isZero()) {
                        interval.flip();
                        int rootTpc = transposeTpc(harmony->rootTpc(), interval, true);
                        int baseTpc = transposeTpc(harmony->baseTpc(), interval, true);
                        undoTransposeHarmony(harmony, rootTpc, baseTpc);
                    }

                    Measure* m = tick2measure(tick);
                    Segment* seg = m->undoGetSegment(SegmentType::ChordRest, tick);
                    // remove pre-existing chords on this track
                    // but be sure not to remove any we just added
                    for (Element* el : seg->findAnnotations(ElementType::HARMONY, e.track(), e.track())) {
                        if (std::find(pastedHarmony.begin(), pastedHarmony.end(), el) == pastedHarmony.end()) {
                            undoRemoveElement(el);
                        }
                    }
                    harmony->setParent(seg);
                    undoAddElement(harmony);
                    pastedHarmony.push_back(harmony);
                } else if (tag == "Dynamic"
                           || tag == "Symbol"
                           || tag == "FretDiagram"
                           || tag == "TremoloBar"
                           || tag == "Marker"
                           || tag == "Jump"
                           || tag == "Image"
                           || tag == "Text"
                           || tag == "StaffText"
                           || tag == "TempoText"
                           || tag == "FiguredBass"
                           || tag == "Sticking"
                           || tag == "Fermata"
                           ) {
                    Element* el = Element::name2Element(tag, this);
                    el->setTrack(e.track());                // a valid track might be necessary for el->read() to work
                    if (el->isFermata()) {
                        el->setPlacement(el->track() & 1 ? Placement::BELOW : Placement::ABOVE);
                    }
                    el->read(e);

                    Fraction tick = doScale ? (e.tick() - dstTick) * scale + dstTick : e.tick();
                    Measure* m = tick2measure(tick);
                    Segment* seg = m->undoGetSegment(SegmentType::ChordRest, tick);
                    el->setParent(seg);

                    // be sure to paste the element in the destination track;
                    // setting track needs to be repeated, as it might have been overwritten by el->read()
                    // preserve *voice* from source, though
                    el->setStaffIdx(e.track() / VOICES);
                    undoAddElement(el);
                } else if (tag == "Clef") {
                    Clef* clef = new Clef(this);
                    clef->read(e);
                    clef->setTrack(e.track());
                    Fraction tick = doScale ? (e.tick() - dstTick) * scale + dstTick : e.tick();
                    Measure* m = tick2measure(tick);
                    if (m->tick().isNotZero() && m->tick() == tick) {
                        m = m->prevMeasure();
                    }
                    Segment* segment = m->undoGetSegment(SegmentType::Clef, tick);
                    clef->setParent(segment);
                    undoChangeElement(segment->element(e.track()), clef);
                } else if (tag == "Breath") {
                    Breath* breath = new Breath(this);
                    breath->setTrack(e.track());
                    breath->setPlacement(breath->track() & 1 ? Placement::BELOW : Placement::ABOVE);
                    breath->read(e);
                    Fraction tick = doScale ? (e.tick() - dstTick) * scale + dstTick : e.tick();
                    Measure* m = tick2measure(tick);
                    if (m->tick() == tick) {
                        m = m->prevMeasure();
                    }
                    Segment* segment = m->undoGetSegment(SegmentType::Breath, tick);
                    breath->setParent(segment);
                    undoChangeElement(segment->element(e.track()), breath);
                } else if (tag == "Beam") {
                    Beam* beam = new Beam(this);
                    beam->setTrack(e.track());
                    beam->read(e);
                    beam->setParent(0);
                    if (startingBeam) {
                        qDebug("The read beam was not used");
                        delete startingBeam;
                    }
                    startingBeam = beam;
                } else if (tag == "BarLine") {
                    e.skipCurrentElement();              // ignore bar line
                } else {
                    qDebug("PasteStaff: element %s not handled", tag.toUtf8().data());
                    e.skipCurrentElement();              // ignore
                }
            }

            e.checkConnectors();
            if (startingBeam) {
                qDebug("The read beam was not used");
                delete startingBeam;
            }
            if (tuplet) {
                qDebug("<endTuplet/> not found");
                if (tuplet->elements().empty()) {
                    if (tuplet->tuplet()) {
                        tuplet->tuplet()->remove(tuplet);
                    }
                    delete tuplet;
                }
            }
        }
        // fix up spanners
        if (doScale && spannerFound) {
            // build list of original spanners
            std::vector<Spanner*> oSpannerList;
            for (auto interval : oSpanner) {
                Spanner* sp = interval.value;
                oSpannerList.push_back(sp);
            }
            auto nSpanner = spannerMap().findContained(dstTick.ticks(), oEndTick.ticks());
            for (auto interval : nSpanner) {
                Spanner* sp = interval.value;
                // skip if not in this staff list
                if (sp->staffIdx() < dstStaff || sp->staffIdx() >= dstStaff + staves) {
                    continue;
                }
                // CHORD and NOTE spanners are normally handled already
                if (sp->anchor() == Spanner::Anchor::CHORD || sp->anchor() == Spanner::Anchor::NOTE) {
                    continue;
                }
                // skip if present oiginally
                auto i = std::find(oSpannerList.begin(), oSpannerList.end(), sp);
                if (i != oSpannerList.end()) {
                    continue;
                }
                Fraction tick = (sp->tick() - dstTick) * scale + dstTick;
                sp->undoChangeProperty(Pid::SPANNER_TICK, tick);
                sp->undoChangeProperty(Pid::SPANNER_TICKS, sp->ticks() * scale);
            }
        }
    }

    for (Score* s : scoreList()) {     // for all parts
        s->connectTies();
    }

    if (pasted) {                         //select only if we pasted something
        int endStaff = dstStaff + staves;
        if (endStaff > nstaves()) {
            endStaff = nstaves();
        }
        //check and add truly invisible rests instead of gaps
        //TODO: look if this could be done different
        Measure* dstM = tick2measure(dstTick);
        Measure* endM = tick2measure(dstTick + tickLen);
        for (int i = dstStaff; i < endStaff; i++) {
            for (Measure* m = dstM; m && m != endM->nextMeasure(); m = m->nextMeasure()) {
                m->checkMeasure(i, false);
            }
        }
        _selection.setRangeTicks(dstTick, dstTick + tickLen, dstStaff, endStaff);

        //finding the first element that has a track
        //the canvas position will be set to this element
        Element* el = nullptr;
        Segment* s = tick2segmentMM(dstTick);
        Segment* s2 = tick2segmentMM(dstTick + tickLen);
        bool found = false;
        if (s2) {
            s2 = s2->next1MM();
        }
        while (!found && s != s2) {
            for (int i = dstStaff * VOICES; i < (endStaff + 1) * VOICES; i++) {
                el = s->element(i);
                if (el) {
                    found = true;
                    break;
                }
            }
            s = s->next1MM();
        }

        for (MuseScoreView* v : qAsConst(viewer)) {
            v->adjustCanvasPosition(el, false);
        }
        if (!selection().isRange()) {
            _selection.setState(SelState::RANGE);
        }
    }
    return true;
}

//---------------------------------------------------------
//   Score::readAddConnector
//---------------------------------------------------------

void Score::readAddConnector(ConnectorInfoReader* info, bool pasteMode)
{
    if (!pasteMode) {
        // How did we get there?
        qDebug("Score::readAddConnector is called not in paste mode.");
        return;
    }
    const ElementType type = info->type();
    switch (type) {
    case ElementType::HAIRPIN:
    case ElementType::PEDAL:
    case ElementType::OTTAVA:
    case ElementType::TRILL:
    case ElementType::TEXTLINE:
    case ElementType::VOLTA:
    case ElementType::PALM_MUTE:
    case ElementType::LET_RING:
    case ElementType::VIBRATO:
    {
        Spanner* sp = toSpanner(info->connector());
        const Location& l = info->location();
        if (info->isStart()) {
            sp->setAnchor(Spanner::Anchor::SEGMENT);
            sp->setTrack(l.track());
            sp->setTrack2(l.track());
            sp->setTick(l.frac());
        } else if (info->isEnd()) {
            sp->setTick2(l.frac());
            undoAddElement(sp);
            if (sp->isOttava()) {
                sp->staff()->updateOttava();
            }
        }
    }
    break;
    default:
        break;
    }
}

//---------------------------------------------------------
//   pasteChordRest
//---------------------------------------------------------

void Score::pasteChordRest(ChordRest* cr, const Fraction& t, const Interval& srcTranspose)
{
    Fraction tick(t);
// qDebug("pasteChordRest %s at %d, len %d/%d", cr->name(), tick, cr->ticks().numerator(), cr->ticks().denominator() );

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
    bool convertMeasureRest = cr->isRest() && cr->durationType().type() == TDuration::DurationType::V_MEASURE
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
                        Tie* tie = new Tie(this);
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
                Rest* r = new Rest(this, dur);
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
    e.setPasteMode(true);   // ensure the reader is in paste mode
    Segment* currSegm = dst->segment();
    Fraction destTick = Fraction(0,1);                // the tick and track to place the pasted element at
    int destTrack   = 0;
    bool done        = false;
    int segDelta    = 0;
    Segment* startSegm= currSegm;
    Fraction startTick   = dst->tick();        // the initial tick and track where to start pasting
    int startTrack  = dst->track();
    int maxTrack    = ntracks();
    Fraction lastTick = lastSegment()->tick();

    while (e.readNextStartElement()) {
        if (done) {
            break;
        }
        if (e.name() != "SymbolList") {
            e.unknown();
            break;
        }
        QString version = e.attribute("version", "NONE");
        if (version != MSC_VERSION) {
            break;
        }

        while (e.readNextStartElement()) {
            if (done) {
                break;
            }
            const QStringRef& tag(e.name());

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
                        qDebug("PasteSymbols: no track or segment for %s", tag.toUtf8().data());
                        e.skipCurrentElement();                   // ignore
                        continue;
                    }
                    if (tag == "Harmony") {
                        Harmony* el = new Harmony(this);
                        el->setTrack(trackZeroVoice(destTrack));
                        el->read(e);
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
                        FretDiagram* el = new FretDiagram(this);
                        el->setTrack(trackZeroVoice(destTrack));
                        el->read(e);
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
                    Dynamic* d = new Dynamic(this);
                    d->setTrack(destTrack);
                    d->read(e);
                    d->setTrack(destTrack);
                    d->setParent(destCR->segment());
                    undoAddElement(d);
                } else if (tag == "HairPin") {
                    Hairpin* h = new Hairpin(this);
                    h->setTrack(destTrack);
                    h->read(e);
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
                        qDebug("PasteSymbols: no track or segment for %s", tag.toUtf8().data());
                        e.skipCurrentElement();                   // ignore
                        continue;
                    }
                    // check there is a segment element in the required track
                    if (currSegm->element(destTrack) == nullptr) {
                        qDebug("PasteSymbols: no track element for %s", tag.toUtf8().data());
                        e.skipCurrentElement();
                        continue;
                    }
                    ChordRest* cr = toChordRest(currSegm->element(destTrack));

                    if (tag == "Articulation") {
                        Articulation* el = new Articulation(this);
                        el->read(e);
                        el->setTrack(destTrack);
                        el->setParent(cr);
                        if (!el->isFermata() && cr->isRest()) {
                            delete el;
                        } else {
                            undoAddElement(el);
                        }
                    } else if (tag == "StaffText" || tag == "Sticking") {
                        Element* el = Element::name2Element(tag, this);
                        el->read(e);
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
                        FiguredBass* el = new FiguredBass(this);
                        el->setTrack(destTrack);
                        el->read(e);
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
                                foreach (Element* a, prevSegm->annotations()) {
                                    if (a->isFiguredBass() && a->track() == destTrack) {
                                        onNoteFB = toFiguredBass(a);
                                        done1 = true;
                                    }
                                }
                            }
                            if (!prevSegm) {
                                qDebug("PasteSymbols: can't place off-note FiguredBass");
                                delete el;
                                continue;
                            }
                            // by default, split on-note duration in half: half on-note and half off-note
                            Fraction totTicks  = currSegm->tick() - prevSegm->tick();
                            Fraction destTick1 = prevSegm->tick() + (totTicks * Fraction(1,2));
                            ticks         = totTicks * Fraction(1, 2);
                            if (onNoteFB) {
                                onNoteFB->setTicks(totTicks * Fraction(1,2));
                            }
                            // look for a segment at this tick; if none, create one
                            Segment* nextSegm = prevSegm;
                            while (nextSegm && nextSegm->tick() < destTick1) {
                                nextSegm = nextSegm->next1(SegmentType::ChordRest);
                            }
                            if (!nextSegm || nextSegm->tick() > destTick1) {                    // no ChordRest segm at this tick
                                nextSegm = new Segment(prevSegm->measure(), SegmentType::ChordRest, destTick1);
                                if (!nextSegm) {
                                    qDebug("PasteSymbols: can't find or create destination segment for FiguredBass");
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
                        foreach (Element* a, currSegm->annotations()) {
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
                            qDebug("PasteSymbols: no segment for Lyrics");
                            e.skipCurrentElement();
                            continue;
                        }
                        if (!cr->isChord()) {
                            qDebug("PasteSymbols: can't paste Lyrics to rest");
                            e.skipCurrentElement();
                            continue;
                        }
                        Lyrics* el = new Lyrics(this);
                        el->setTrack(destTrack);
                        el->read(e);
                        el->setTrack(destTrack);
                        el->setParent(cr);
                        undoAddElement(el);
                    } else {
                        qDebug("PasteSymbols: element %s not handled", tag.toUtf8().data());
                        e.skipCurrentElement();                // ignore
                    }
                }                         // if !Harmony
            }                             // if element
        }                                 // outer while readNextstartElement()
    }                                     // inner while readNextstartElement()
}                                         // pasteSymbolList()

//---------------------------------------------------------
//   cmdPaste
//---------------------------------------------------------

void Score::cmdPaste(const QMimeData* ms, MuseScoreView* view, Fraction scale)
{
    if (ms == 0) {
        qDebug("no application mime data");
        MScore::setError(MsError::NO_MIME);
        return;
    }
    if ((_selection.isSingle() || _selection.isList()) && ms->hasFormat(mimeSymbolFormat)) {
        QByteArray data(ms->data(mimeSymbolFormat));

        QPointF dragOffset;
        Fraction duration(1, 4);
        std::unique_ptr<Element> el(Element::readMimeData(this, data, &dragOffset, &duration));

        if (!el) {
            return;
        }

        QList<Element*> els;
        if (_selection.isSingle()) {
            els.append(_selection.element());
        } else {
            els.append(_selection.elements());
        }

        for (Element* target : els) {
            el->setTrack(target->track());
            Element* nel = el->clone();
            addRefresh(target->abbox());         // layout() ?!
            EditData ddata(view);
            ddata.view        = view;
            ddata.dropElement = nel;
            if (target->acceptDrop(ddata)) {
                if (el->isNote()) {
                    // dropping a note replaces and invalidates the target,
                    // so we need to deselect it
                    ElementType targetType = target->type();
                    deselect(target);

                    // perform the drop
                    target->drop(ddata);

                    TDuration durationType(duration);
                    if (target->isChordRest()) {
                        ChordRest* targetCR = dynamic_cast<ChordRest*>(target);
                        score()->changeCRlen(targetCR, durationType);
                    } else if (target->isNote()) {
                        ChordRest* targetCR = dynamic_cast<Note*>(target)->chord();
                        score()->changeCRlen(targetCR, durationType);
                    }

                    // if the target is a rest rather than a note,
                    // a new note is generated, and nel becomes invalid as well
                    // (ChordRest::drop() will select it for us)
                    if (targetType == ElementType::NOTE) {
                        select(nel);
                    }
                } else {
                    target->drop(ddata);
                }
                if (_selection.element()) {
                    addRefresh(_selection.element()->abbox());
                }
            } else {
                delete nel;
            }
        }
    } else if ((_selection.isRange() || _selection.isList()) && ms->hasFormat(mimeStaffListFormat)) {
        ChordRest* cr = 0;
        if (_selection.isRange()) {
            cr = _selection.firstChordRest();
        } else if (_selection.isSingle()) {
            Element* e = _selection.element();
            if (!e->isNote() && !e->isChordRest()) {
                qDebug("cannot paste to %s", e->name());
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
            QByteArray data(ms->data(mimeStaffListFormat));
            if (MScore::debugMode) {
                qDebug("paste <%s>", data.data());
            }
            XmlReader e(data);
            e.setPasteMode(true);
            if (!pasteStaff(e, cr->segment(), cr->staffIdx(), scale)) {
                return;
            }
        }
    } else if (ms->hasFormat(mimeSymbolListFormat)) {
        ChordRest* cr = 0;
        if (_selection.isRange()) {
            cr = _selection.firstChordRest();
        } else if (_selection.isSingle()) {
            Element* e = _selection.element();
            if (!e->isNote() && !e->isRest() && !e->isChord()) {
                qDebug("cannot paste to %s", e->name());
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
            QByteArray data(ms->data(mimeSymbolListFormat));
            if (MScore::debugMode) {
                qDebug("paste <%s>", data.data());
            }
            XmlReader e(data);
            pasteSymbols(e, cr);
        }
    } else if (ms->hasImage()) {
        QImage im = qvariant_cast<QImage>(ms->imageData());
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        im.save(&buffer, "PNG");

        Image* image = new Image(this);
        image->setImageType(ImageType::RASTER);
        image->loadFromData("dragdrop", ba);

        QList<Element*> els;
        if (_selection.isSingle()) {
            els.append(_selection.element());
        } else {
            els.append(_selection.elements());
        }

        for (Element* target : els) {
            Element* nel = image->clone();
            addRefresh(target->abbox());         // layout() ?!
            EditData ddata(view);
            ddata.view       = view;
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
        qDebug("cannot paste selState %d staffList %s",
               int(_selection.state()), (ms->hasFormat(mimeStaffListFormat)) ? "true" : "false");
        for (const QString& s : ms->formats()) {
            qDebug("  format %s", qPrintable(s));
        }
    }
}
}
