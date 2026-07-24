/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "paste.h"

#include "draw/iimageprovider.h"
#include "io/buffer.h"
#include "modularity/ioc.h"

#include "imimedata.h"

#include "editmeasures.h"
#include "editparentheses.h"
#include "editstaff.h"
#include "mscoreview.h"
#include "noteinput.h"
#include "transaction/transaction.h"
#include "transpose.h"

#include "rw/read400/tread.h"
#include "rw/rwregister.h"

#include "../dom/articulation.h"
#include "../dom/beam.h"
#include "../dom/breath.h"
#include "../dom/chord.h"
#include "../dom/drumset.h"
#include "../dom/dynamic.h"
#include "../dom/factory.h"
#include "../dom/figuredbass.h"
#include "../dom/fret.h"
#include "../dom/harmony.h"
#include "../dom/image.h"
#include "../dom/measure.h"
#include "../dom/measurerepeat.h"
#include "../dom/note.h"
#include "../dom/part.h"
#include "../dom/rest.h"
#include "../dom/score.h"
#include "../dom/sig.h"
#include "../dom/staff.h"
#include "../dom/tie.h"
#include "../dom/timesig.h"
#include "../dom/tuplet.h"
#include "../dom/tremolosinglechord.h"
#include "../dom/utils.h"

#include "log.h"

using namespace mu;
using namespace muse::io;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   pasteStaff
//    return false if paste fails
//---------------------------------------------------------
bool Paste::pasteStaff(Transaction&, Score*, XmlReader& e, Segment* dst, staff_idx_t dstStaff, Fraction scale)
{
    //! NOTE Needs refactoring - reading should be separated from insertion
    //! (we read the elements into some structure, then inserted them)
    return rw::RWRegister::reader()->pasteStaff(e, dst, dstStaff, scale);
}

//---------------------------------------------------------
//   pasteChordRest
//---------------------------------------------------------

void Paste::pasteChordRest(Transaction& tx, Score* score, ChordRest* cr, const Fraction& t)
{
    Fraction tick(t);
// LOGD("pasteChordRest %s at %d, len %d/%d", cr->typeName(), tick, cr->ticks().numerator(), cr->ticks().denominator() );

    Measure* measure = score->tick2measure(tick);
    if (!measure) {
        return;
    }

    int twoNoteTremoloFactor = 1;
    if (cr->isChord()) {
        Chord* chord = toChord(cr);
        if (chord->vStaffIdx() >= chord->score()->nstaves()) {
            // check if staffMove moves a note to a
            // nonexistent staff
            chord->setStaffMove(0);
        }
        Transpose::transposeChord(chord, tick);
        if (chord->tremoloTwoChord()) {
            twoNoteTremoloFactor = 2;
        } else if (cr->durationTypeTicks() == (cr->ticks() * 2)) {
            // this could be the 2nd note of a two-note tremolo
            // check previous CR on same track, if it has a two-note tremolo, then set twoNoteTremoloFactor to 2
            Segment* seg = measure->undoGetSegment(SegmentType::ChordRest, tick);
            ChordRest* crt = seg->nextChordRest(cr->track(), true);
            if (crt && crt->isChord()) {
                Chord* chrt = toChord(crt);
                TremoloTwoChord* tr = chrt->tremoloTwoChord();
                if (tr) {
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
                              && (tick != measure->tick() || cr->actualTicksAt(tick) != measure->ticks());

    Fraction measureEnd = measure->endTick();
    bool isGrace = cr->isChord() && toChord(cr)->noteType() != NoteType::NORMAL;

    // adjust measures for measure repeat
    if (cr->isMeasureRepeat()) {
        MeasureRepeat* mr = toMeasureRepeat(cr);
        Measure* m = (mr->numMeasures() == 4 ? measure->prevMeasure() : measure);
        for (int i = 1; i <= mr->numMeasures(); ++i) {
            tx.push(new ChangeMeasureRepeatCount(m, i, mr->staffIdx()));
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
        partialCopy = cr->durationTypeTicks() != (cr->ticks() * twoNoteTremoloFactor);
    }

    // if note is too long to fit in measure, split it up with a tie across the barline
    // exclude tuplets from consideration
    // we have already disallowed a tuplet from crossing the barline, so there is no problem here
    // but due to rounding, it might appear from actualTicks() that the last note is too long by a couple of ticks

    Staff* stf = cr->staff();
    const bool shouldSplit = tick + cr->actualTicksAt(tick) > measureEnd || partialCopy || convertMeasureRest;
    if (isGrace || cr->tuplet() || !shouldSplit) {
        score->undoAddCR(cr, measure, tick);
        return;
    }
    if (cr->isChord()) {
        // split Chord
        Chord* c = toChord(cr);
        Fraction rest = c->ticks();
        bool firstpart = true;
        while (rest.isNotZero()) {
            measure = score->tick2measure(tick);
            Chord* c2 = firstpart ? c : toChord(c->clone());
            if (!firstpart) {
                c2->removeMarkings(true);
            }
            Fraction timeStretch = stf->timeStretch(tick);
            Fraction mlen = (measure->endTick() - tick) * timeStretch;
            Fraction len = mlen > rest ? rest : mlen;
            std::vector<TDuration> dl = toRhythmicDurationList(len, false, (tick - measure->tick()) * timeStretch,
                                                               score->sigmap()->timesig(tick).nominal(), measure, MAX_DOTS, timeStretch);
            if (dl.empty()) {
                LOGD("Could not make durations for: %d/%d", len.numerator(), len.denominator());
                return;
            }
            TDuration d = dl[0];
            c2->setDurationType(d);
            c2->setTicks(d.fraction());
            score->undoAddCR(c2, measure, tick);

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
            rest -= c2->ticks();
            tick += c2->actualTicksAt(tick);
            firstpart = false;
            c = c2;
        }
    } else if (cr->isRest()) {
        // split Rest
        Rest* r       = toRest(cr);
        Fraction rest = r->ticks();

        bool firstpart = true;
        while (!rest.isZero()) {
            Rest* r2      = firstpart ? r : toRest(r->clone());
            measure       = score->tick2measure(tick);
            Fraction timeStretch = stf->timeStretch(tick);
            Fraction mlen = (measure->endTick() - tick) * timeStretch;
            Fraction len  = rest > mlen ? mlen : rest;
            std::vector<TDuration> dl = toRhythmicDurationList(len, true, (tick - measure->tick()) * timeStretch,
                                                               score->sigmap()->timesig(tick).nominal(), measure, MAX_DOTS, timeStretch);
            if (dl.empty()) {
                LOGD("Could not make durations for: %d/%d", len.numerator(), len.denominator());
                return;
            }
            TDuration d = dl[0];
            r2->setDurationType(d);
            r2->setTicks(d.isMeasure() ? measure->stretchedLen(stf) : d.fraction());
            score->undoAddCR(r2, measure, tick);
            rest -= r2->ticks();
            tick += r2->actualTicksAt(tick);
            firstpart = false;
        }
    } else if (cr->isMeasureRepeat()) {
        MeasureRepeat* mr = toMeasureRepeat(cr);
        std::vector<TDuration> list = toDurationList(mr->ticks(), true);
        for (auto dur : list) {
            Rest* r = Factory::createRest(score->dummy()->segment(), dur);
            r->setTrack(cr->track());
            Fraction rest = r->ticks();
            while (!rest.isZero()) {
                Rest* r2      = toRest(r->clone());
                measure       = score->tick2measure(tick);
                Fraction timeStretch = stf->timeStretch(tick);
                Fraction mlen = (measure->endTick() - tick) * timeStretch;
                Fraction len  = rest > mlen ? mlen : rest;
                std::vector<TDuration> dl = toDurationList(len, false);
                if (dl.empty()) {
                    LOGD("Could not make durations for: %d/%d", len.numerator(), len.denominator());
                    return;
                }
                TDuration d = dl[0];
                r2->setTicks(d.fraction());
                r2->setDurationType(d);
                score->undoAddCR(r2, measure, tick);
                rest -= r2->ticks();
                tick += r2->actualTicksAt(tick);
            }
            delete r;
        }
        delete cr;
    }
}

//---------------------------------------------------------
//   pasteSymbols
//
//    pastes a list of symbols into cr and following ChordRest's
//
//    (Note: info about delta ticks is currently ignored)
//---------------------------------------------------------
void Paste::pasteSymbols(Transaction&, XmlReader& e, ChordRest* dst)
{
    //! NOTE Needs refactoring - reading should be separated from insertion
    //! (we read the elements into some structure, then inserted them)
    rw::RWRegister::reader()->pasteSymbols(e, dst);
}

bool Paste::repeatListSelection(Transaction& tx, Score* score)
{
    TRACEFUNC;

    InputState& is = score->inputState();

    std::vector<Note*> notes = score->selection().noteList();
    std::sort(notes.begin(), notes.end(), [](const Note* a, const Note* b) {
        return std::make_tuple(a->track(), a->tick(), a->pitch())
               < std::make_tuple(b->track(), b->tick(), b->pitch());
    });

    std::vector<EngravingItem*> toSelect;
    std::unordered_set<const Chord*> foundChords;

    // Parenthesis logic: group new notes by the left parenthesis (if any) of their old equivalent. Once all
    // new notes have been created we can call cmdAddParentheses on each group...
    // Use a vector of pairs to preserve insertion order
    using NoteList = std::vector<Note*>;
    std::vector<std::pair<const Parenthesis*, NoteList> > parenEntries;

    for (Note* n : notes) {
        if (n->isGrace() || n->incomingPartialTie() || n->outgoingPartialTie()) {
            continue;
        }

        const Chord* sourceChord = n->chord();
        is.setTrack(sourceChord->track());

        const bool addFlag = muse::contains(foundChords, sourceChord);
        if (!addFlag) {
            // If the note doesn't belong to a chord we've seen before...
            foundChords.emplace(sourceChord);
            is.setSegment(sourceChord->segment());
            is.moveToNextInputPos();

            if (is.noteEntryMode()) {
                // In note input mode - use the duration of from the note input panel (InputState)...
                IF_ASSERT_FAILED(is.duration().isValid()) {
                    LOGE() << "Invalid InputState duration";
                    is.setDuration(sourceChord->durationType());
                }
            } else {
                // Otherwise set it based on the duration of the previous chord...
                is.setDuration(sourceChord->durationType());
            }
        }

        NoteVal nval = n->noteVal();
        Note* newNote = NoteInput::addPitch(tx, score, nval, addFlag);
        IF_ASSERT_FAILED(newNote) {
            continue;
        }

        Chord* newChord = newNote->chord();

        newChord->updateArticulations(sourceChord->articulationSymbolIds());

        const TremoloSingleChord* oldTsc = sourceChord->tremoloSingleChord();
        if (!newChord->tremoloSingleChord() && oldTsc) {
            TremoloSingleChord* newTsc = oldTsc->clone();
            newTsc->setParent(newChord);
            newTsc->setTrack(newChord->track());
            score->doUndoAddElement(newTsc);
        }

        toSelect.push_back(newNote);

        const Parenthesis* leftParen = n->parenthesisInfo() ? n->parenthesisInfo()->leftParen() : nullptr;
        if (!leftParen) {
            continue;
        }

        NoteList notesForParen;
        notesForParen.emplace_back(newNote);

        // All tied notes should be parenthesized...
        Tie* tie = newNote->tieBack();
        while (tie) {
            Note* tiedNote = tie->startNote();
            if (!tiedNote) {
                break;
            }
            notesForParen.emplace_back(tiedNote);
            tie = tiedNote->tieBack();
        }

        auto search = std::find_if(parenEntries.begin(), parenEntries.end(),
                                   [leftParen](const std::pair<const Parenthesis*, NoteList>& p) {
            return p.first == leftParen;
        });
        if (search != parenEntries.end()) {
            search->second.insert(search->second.end(), notesForParen.begin(), notesForParen.end());
            continue;
        }

        parenEntries.emplace_back(leftParen, std::move(notesForParen));
    }

    for (auto& [paren, noteList] : parenEntries) {
        EditParentheses::addParenthesesToNotes(tx, noteList);
    }

    score->select(toSelect, SelectType::ADD);
    return !toSelect.empty();
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

    const Staff* staff = target->staff();
    const StaffGroup staffGroup = staff->staffType(segment->tick())->group();
    DirectionV stemDirection = DirectionV::AUTO;
    if (staffGroup == StaffGroup::PERCUSSION) {
        const Drumset* ds = staff->part()->instrument(segment->tick())->drumset();
        DO_ASSERT(ds);

        if (ds) {
            stemDirection = ds->stemDirection(with->noteVal().pitch);
        }
    }

    segment = target->score()->setNoteRest(segment, target->track(),
                                           with->noteVal(), duration, stemDirection, false, {}, false, &target->score()->inputState());
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

static EngravingItem* pasteSystemObject(Transaction& tx, EditData& srcData, EngravingItem* target)
{
    if (!target) {
        return nullptr;
    }

    if (srcData.element && srcData.element->isTimeSig()) {
        // Don't allow copypasting time signatures
        return nullptr;
    }

    Score* targetScore = target->score();
    Staff* targetStaff = target->staff();
    if (!targetScore || !targetStaff) {
        return nullptr;
    }

    // System objects can only be pasted on the *top* staff of an instrument
    Part* targetPart = targetStaff->part();
    targetStaff = targetPart ? targetPart->staves().front() : nullptr;
    if (!targetStaff) {
        return nullptr;
    }

    if (targetStaff == targetScore->staff(0) || targetStaff->isSystemObjectStaff()) {
        return target->drop(tx, srcData);
    }

    tx.push(new AddSystemObjectStaff(targetStaff));

    const std::vector<EngravingItem*> topSystemObjects = collectSystemObjects(targetScore);
    const staff_idx_t staffIdx = targetStaff->idx();
    const Fraction targetTick = target->tick();

    EngravingItem* pastedItem = nullptr;

    for (EngravingItem* obj : topSystemObjects) {
        const bool visible = obj->type() == srcData.dropElement->type() && obj->tick() == targetTick;

        EngravingItem* copy = obj->linkedClone();
        copy->setVisible(visible);
        copy->setStaffIdx(staffIdx);
        targetScore->undoAddElement(copy, false /*addToLinkedStaves*/);

        if (visible) {
            pastedItem = copy;
        }
    }

    if (!pastedItem) {
        pastedItem = target->drop(tx, srcData);
    }

    return pastedItem;
}

//---------------------------------------------------------
//   cmdPaste
//---------------------------------------------------------

bool Paste::paste(Transaction& tx, Score* score, const IMimeData* ms, MuseScoreView* view, Fraction scale)
{
    if (!ms) {
        LOGE() << "No MIME data given";
        return false;
    }

    if (score->selection().isNone()) {
        LOGE() << "No target selection";
        MScore::setError(MsError::NO_DEST);
        return false;
    }

    if (ms->hasFormat(mimeSymbolFormat)) {
        muse::ByteArray data = ms->data(mimeSymbolFormat);
        return pasteSymbol(tx, score, data, view, scale);
    }

    if (ms->hasFormat(mimeStaffListFormat)) {
        muse::ByteArray data = ms->data(mimeStaffListFormat);
        return pasteStaffList(tx, score, data, scale);
    }

    if (ms->hasFormat(mimeSymbolListFormat)) {
        muse::ByteArray data = ms->data(mimeSymbolListFormat);
        return pasteSymbolList(tx, score, data);
    }

    if (ms->hasImage()) {
        muse::ByteArray ba;
        auto buffer = Buffer::opened(IODevice::WriteOnly, &ba);

        muse::GlobalInject<muse::draw::IImageProvider> imageProvider;
        auto px = ms->imageData();
        imageProvider()->saveAsPng(px, &buffer);

        std::unique_ptr<Image> image(new Image(score->dummy()));
        image->setImageType(ImageType::RASTER);
        image->loadFromData("paste", ba);

        std::vector<EngravingItem*> droppedElements;
        std::vector<EngravingItem*> targetElements = score->selection().elements();
        for (EngravingItem* target : targetElements) {
            score->addRefresh(target->pageBoundingRect()); // layout() ?!

            EngravingItem* nel = image->clone();
            EditData ddata(view);
            ddata.dropElement = nel;

            if (target->acceptDrop(ddata)) {
                EngravingItem* dropped = target->drop(tx, ddata);
                if (dropped) {
                    droppedElements.emplace_back(dropped);
                }

                if (score->selection().element()) {
                    score->addRefresh(score->selection().element()->pageBoundingRect());
                }
            }
        }

        score->select(droppedElements);
        return true;
    }

    LOGE() << "Unsupported MIME data (formats: " << ms->formats() << ")";
    return false;
}
}

bool Paste::pasteSymbol(Transaction& tx, Score* score, muse::ByteArray& data, MuseScoreView* view, Fraction scale)
{
    std::vector<EngravingItem*> droppedElements;

    PointF dragOffset;
    Fraction duration(1, 4);

    std::unique_ptr<EngravingItem> el(EngravingItem::readMimeData(score, data, &dragOffset, &duration));
    if (!el) {
        return false;
    }

    duration *= scale;
    if (!TDuration(duration).isValid()) {
        return false;
    }

    std::vector<EngravingItem*> targetElements;
    if (score->selection().isNone()) {
        UNREACHABLE;
        return false;
    }

    // TODO: make this as smart as `NotationInteraction::applyPaletteElement`,
    // without duplicating logic. (Currently, for range selections, we only
    // paste onto the "top-left corner" for non-measure based elements.)
    bool unique;
    targetElements = filterTargetElements(score->selection(), el.get(), unique);
    if (!unique && score->selection().isRange()) {
        // The usage of `firstElementForNavigation` is inspired by `NotationInteraction::applyPaletteElement`.
        Segment* firstSegment = score->selection().startSegment();
        targetElements = { firstSegment->firstElementForNavigation(score->selection().staffStart()) };
    }

    if (targetElements.empty()) {
        LOGE() << "No valid target elements in selection";
        MScore::setError(MsError::NO_DEST);
        return false;
    }

    const bool systemObj = el->systemFlag();

    for (EngravingItem* target : targetElements) {
        score->addRefresh(target->pageBoundingRect()); // layout() ?!
        el->setTrack(target->track());

        EditData ddata(view);
        ddata.pos = target->pagePos();
        ddata.dropElement = el.get();
        ddata.track = target->track();

        if (!target->acceptDrop(ddata)) {
            continue;
        }

        if (!el->isNote() || (target = prepareTarget(target, toNote(el.get()), duration))) {
            ddata.dropElement = el->clone();

            EngravingItem* dropped = systemObj ? pasteSystemObject(tx, ddata, target) : target->drop(tx, ddata);
            if (dropped) {
                droppedElements.emplace_back(dropped);
            }
        }
    }

    score->select(droppedElements);
    return true;
}

bool Paste::pasteStaffList(Transaction& tx, Score* score, muse::ByteArray& data, Fraction scale)
{
    if (MScore::debugMode) {
        LOGD() << "Pasting staff list: " << data.data();
    }

    ChordRest* cr = nullptr;
    if (score->selection().isRange()) {
        cr = score->selection().firstChordRest();
    } else if (score->selection().isSingle()) {
        EngravingItem* e = score->selection().element();
        Measure* measure = e->findMeasure();
        cr = measure ? measure->findChordRest(e->tick(), e->track()) : nullptr;
        if (!cr) {
            LOGE() << "Cannot paste staff list onto " << e->typeName();
            MScore::setError(MsError::DEST_NO_CR);
            return false;
        }
    }

    if (!cr) {
        MScore::setError(MsError::NO_DEST);
        return false;
    }

    if (cr->tuplet() && cr->tick() != cr->topTuplet()->tick()) {
        MScore::setError(MsError::DEST_TUPLET);
        return false;
    }

    XmlReader xmlReader(data);
    return pasteStaff(tx, score, xmlReader, cr->segment(), cr->staffIdx(), scale);
}

bool Paste::pasteSymbolList(Transaction& tx, Score* score, muse::ByteArray& data)
{
    if (MScore::debugMode) {
        LOGD() << "Pasting element list: " << data.data();
    }

    ChordRest* cr = nullptr;
    if (score->selection().isRange()) {
        cr = score->selection().firstChordRest();
    } else if (score->selection().isSingle()) {
        EngravingItem* e = score->selection().element();
        Measure* measure = e->findMeasure();
        cr = measure ? measure->findChordRest(e->tick(), e->track()) : nullptr;
        if (!cr) {
            LOGE() << "Cannot paste element list onto " << e->typeName();
            MScore::setError(MsError::DEST_NO_CR);
            return false;
        }
    }

    if (!cr) {
        MScore::setError(MsError::NO_DEST);
        return false;
    }

    XmlReader xmlReader(data);
    pasteSymbols(tx, xmlReader, cr);
    return true;
}
