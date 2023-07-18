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

void Score::transposeChord(Chord* c, const Fraction& tick)
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
    Staff* staff = c->staff();
    Interval dstTranspose = staff->transpose(tick);

    if (dstTranspose.isZero()) {
        for (Note* n : c->notes()) {
            n->setTpc2(n->tpc1());
        }
    } else {
        dstTranspose.flip();
        for (Note* n : c->notes()) {
            int npitch;
            int ntpc;
            transposeInterval(n->pitch(), n->tpc1(), &npitch, &ntpc, dstTranspose, true);
            n->setTpc2(ntpc);
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
    return rw::RWRegister::reader()->pasteStaff(e, dst, dstStaff, scale);
}

//---------------------------------------------------------
//   pasteChordRest
//---------------------------------------------------------

void Score::pasteChordRest(ChordRest* cr, const Fraction& t)
{
    Fraction tick(t);
// LOGD("pasteChordRest %s at %d, len %d/%d", cr->typeName(), tick, cr->ticks().numerator(), cr->ticks().denominator() );

    Measure* measure = tick2measure(tick);
    if (!measure) {
        return;
    }

    int twoNoteTremoloFactor = 1;
    if (cr->isChord()) {
        transposeChord(toChord(cr), tick);
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
    //! NOTE Needs refactoring - reading should be separated from insertion
    //! (we read the elements into some structure, then inserted them)
    rw::RWRegister::reader()->pasteSymbols(e, dst);
}

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
    if ((m_selection.isSingle() || m_selection.isList()) && ms->hasFormat(mimeSymbolFormat)) {
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
        if (m_selection.isSingle()) {
            els.push_back(m_selection.element());
        } else {
            els.insert(els.begin(), m_selection.elements().begin(), m_selection.elements().end());
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
    } else if ((m_selection.isRange() || m_selection.isList()) && ms->hasFormat(mimeStaffListFormat)) {
        ChordRest* cr = 0;
        if (m_selection.isRange()) {
            cr = m_selection.firstChordRest();
        } else if (m_selection.isSingle()) {
            EngravingItem* e = m_selection.element();
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
        if (m_selection.isRange()) {
            cr = m_selection.firstChordRest();
        } else if (m_selection.isSingle()) {
            EngravingItem* e = m_selection.element();
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
        if (m_selection.isSingle()) {
            els.push_back(m_selection.element());
        } else {
            els.insert(els.begin(), m_selection.elements().begin(), m_selection.elements().end());
        }

        for (EngravingItem* target : els) {
            EngravingItem* nel = image->clone();
            addRefresh(target->abbox());         // layout() ?!
            EditData ddata(view);
            ddata.dropElement    = nel;
            if (target->acceptDrop(ddata)) {
                target->drop(ddata);
                if (m_selection.element()) {
                    addRefresh(m_selection.element()->abbox());
                }
            }
        }
        delete image;
    } else {
        LOGD("cannot paste selState %d staffList %s", int(m_selection.state()), (ms->hasFormat(mimeStaffListFormat)) ? "true" : "false");
        for (const std::string& s : ms->formats()) {
            LOGD() << " format: " << s;
        }
    }
}
}
