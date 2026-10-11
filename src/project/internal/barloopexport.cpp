/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "barloopexport.h"

#include <algorithm>
#include <vector>

#include "engraving/dom/barline.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/part.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/spanner.h"

#include "notation/inotationelements.h"
#include "notation/inotationplayback.h"
#include "notation/inotationsolomutestate.h"
#include "notation/internal/masternotation.h"
#include "notation/internal/notation.h"

#include "log.h"

using namespace mu::project;
using namespace mu::notation;
using namespace mu::engraving;

static void dropExcerpts(MasterScore* score)
{
    const std::vector<Excerpt*> excerpts = score->excerpts();
    for (Excerpt* excerpt : excerpts) {
        score->deleteExcerpt(excerpt);
    }
}

//! Excerpt scores are not master scores, so clone the master and drop every other part.
static void keepSourceParts(MasterScore* score, const Score* source)
{
    if (!source || source->isMaster()) {
        return;
    }

    std::vector<muse::ID> keep;
    keep.reserve(source->parts().size());
    for (const Part* part : source->parts()) {
        keep.push_back(part->id());
    }

    std::vector<Part*> remove;
    for (Part* part : score->parts()) {
        const bool kept = std::find(keep.cbegin(), keep.cend(), part->id()) != keep.cend();
        if (!kept) {
            remove.push_back(part);
        }
    }

    for (Part* part : remove) {
        score->cmdRemovePart(part);
    }
}

static MasterScore* cloneAsMaster(Score* source)
{
    if (!source || !source->masterScore()) {
        return nullptr;
    }

    MasterScore* copy = source->masterScore()->clone();
    if (!copy) {
        return nullptr;
    }

    dropExcerpts(copy);
    keepSourceParts(copy, source);
    return copy;
}

//! Repeat signs inside the selection would multiply the repetition count, so the
//! exported copy plays the written bars once per repetition.
static void removeRepeatMarkings(Score* score)
{
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        measure->setRepeatStart(false);
        measure->setRepeatEnd(false);
        if (measure->sectionBreak()) {
            measure->undoSetBreak(false, LayoutBreakType::SECTION);
        }
    }

    auto spanners = score->spannerMap().map();
    for (auto it = spanners.cbegin(); it != spanners.cend(); ++it) {
        Spanner* spanner = it->second;
        if (spanner && spanner->isVolta()) {
            score->removeSpanner(spanner);
        }
    }

    const std::vector<EngravingItem*> elements = score->getChildren(false);
    for (EngravingItem* element : elements) {
        if (element->isMarker() || element->isJump()) {
            score->deleteItem(element);
        } else if (element->isBarLine()) {
            toBarLine(element)->setBarLineType(BarLineType::NORMAL);
        }
    }

    Measure* last = score->lastMeasure();
    if (!last) {
        return;
    }

    last->setEndBarLineType(BarLineType::END, false);
    Segment* lastSegment = last->segments().last();
    if (lastSegment && lastSegment->segmentType() == SegmentType::EndBarLine) {
        for (EngravingItem* element : lastSegment->elist()) {
            if (element && element->isBarLine()) {
                toBarLine(element)->setBarLineType(BarLineType::END);
            }
        }
    }
}

static Fraction appendEndTick(const Score* source, const Fraction& endTick)
{
    const Measure* boundary = source->tick2measure(endTick);
    const Measure* last = source->lastMeasure();
    if (!last) {
        return endTick;
    }

    // tick2measure(last->endTick()) returns the last measure, so an exclusive
    // end at the end of the score has to step one tick past it.
    if (!boundary || boundary->tick() < endTick) {
        return last->endTick() + Fraction::fromTicks(1);
    }

    return endTick;
}

static bool trimToRange(Score* score, const Fraction& startTick, const Fraction& endTick)
{
    if (endTick <= startTick || !score->firstMeasure()) {
        return false;
    }

    Measure* end = score->tick2measure(endTick - Fraction::fromTicks(1));
    if (!end) {
        return false;
    }

    if (Measure* next = end->nextMeasure()) {
        Measure* last = score->lastMeasure();
        if (next && last) {
            score->deleteMeasures(next, last);
        }
    }

    Measure* start = score->tick2measure(startTick);
    if (!start) {
        return false;
    }

    if (Measure* previous = start->prevMeasure()) {
        Measure* first = score->firstMeasure();
        if (first) {
            score->deleteMeasures(first, previous);
        }
    }

    return score->firstMeasure() != nullptr;
}

static MasterScore* buildBarLoop(Score* source, const Fraction& startTick, const Fraction& endTick, int repeats)
{
    std::unique_ptr<MasterScore> pristine(cloneAsMaster(source));
    std::unique_ptr<MasterScore> score(cloneAsMaster(source));
    if (!pristine || !score) {
        return nullptr;
    }

    dropExcerpts(pristine.get());
    dropExcerpts(score.get());

    if (!trimToRange(score.get(), startTick, endTick)) {
        LOGE() << "Selected bar range is not in the score";
        return nullptr;
    }

    const Fraction sourceEnd = appendEndTick(pristine.get(), endTick);
    if (!pristine->tick2measure(startTick)) {
        return nullptr;
    }

    repeats = std::clamp(repeats, 1, 99);
    for (int copy = 1; copy < repeats; ++copy) {
        if (!score->last()) {
            return nullptr;
        }
        if (!score->appendMeasuresFromScore(pristine.get(), startTick, sourceEnd)) {
            LOGE() << "Could not append repetition " << copy + 1;
            return nullptr;
        }
    }

    removeRepeatMarkings(score.get());
    score->updateTicksAndTimeSigMap();
    score->setLayoutAll();
    score->doLayout();

    return score.release();
}

static void copySoloMute(const INotationPtr& from, const INotationPtr& to, const IMasterNotationPtr& master)
{
    if (!from || !to || !master || !master->masterScore() || !master->playback()) {
        return;
    }

    const INotationSoloMuteStatePtr fromState = from->soloMuteState();
    const INotationSoloMuteStatePtr toState = to->soloMuteState();
    if (!fromState || !toState) {
        return;
    }

    for (const Part* part : master->masterScore()->parts()) {
        for (const InstrumentTrackId& trackId : part->instrumentTrackIdSet()) {
            toState->setTrackSoloMuteState(trackId, fromState->trackSoloMuteState(trackId));
        }

        if (part->hasChordSymbol()) {
            const InstrumentTrackId chordsId = master->playback()->chordSymbolsTrackId(part->id());
            toState->setTrackSoloMuteState(chordsId, fromState->trackSoloMuteState(chordsId));
        }
    }
}

std::unique_ptr<BarLoopPrepared> mu::project::prepareBarLoopExport(IMasterNotationPtr master,
                                                                  const INotationPtrList& notations,
                                                                  const SelectedBarsExport& selected)
{
    if (!master || !selected.active || selected.endTick <= selected.startTick || notations.empty()) {
        return nullptr;
    }

    auto* masterNotation = dynamic_cast<MasterNotation*>(master.get());
    if (!masterNotation) {
        return nullptr;
    }

    Score* masterScore = master->notation() ? master->notation()->elements()->msScore() : nullptr;
    if (!masterScore) {
        return nullptr;
    }

    const Fraction startTick = Fraction::fromTicks(selected.startTick);
    const Fraction endTick = Fraction::fromTicks(selected.endTick);
    const int repeats = std::clamp(selected.repeats, 1, 99);

    auto prepared = std::make_unique<BarLoopPrepared>();

    MasterScore* playbackScore = buildBarLoop(masterScore, startTick, endTick, repeats);
    if (!playbackScore) {
        return nullptr;
    }
    prepared->playbackScore = playbackScore;
    prepared->scores.emplace_back(playbackScore);

    for (const INotationPtr& notation : notations) {
        if (!notation || !notation->elements()) {
            return nullptr;
        }

        Score* source = notation->elements()->msScore();
        MasterScore* loopScore = nullptr;
        if (source == masterScore) {
            loopScore = playbackScore;
        } else {
            loopScore = buildBarLoop(source, startTick, endTick, repeats);
            if (!loopScore) {
                return nullptr;
            }
            prepared->scores.emplace_back(loopScore);
        }

        INotationPtr wrapped = std::make_shared<Notation>(masterNotation, notation->iocContext(), loopScore);
        copySoloMute(notation, wrapped, master);
        prepared->notations.push_back(wrapped);
    }

    if (prepared->notations.empty()) {
        return nullptr;
    }

    return prepared;
}
