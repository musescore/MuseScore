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
#include <cmath>
#include <map>
#include <vector>

#include "engraving/dom/barline.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurerepeat.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/tremolotwochord.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/tupletmap.h"
#include "engraving/dom/playtechannotation.h"
#include "engraving/dom/repeatlist.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/soundflag.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/tiemap.h"

#include "engraving/types/constants.h"

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
    if (!source->isMaster()) {
        // Keep the part's own page format, spacing, and header text.
        copy->setStyle(source->style());
        copy->startCmd(muse::TranslatableString::untranslatable("Export selected bars"));
        keepSourceParts(copy, source);
        copy->endCmd();
    }
    return copy;
}

//! The passage has already been written out in playback order. Drop the signs so a
//! later copy of that passage is not treated as another volta, jump, or repeat.
static void removeRepeatMarkings(Score* score)
{
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        // Direct flag writes are not part of the command, so a later layout can still see the
        // old repeat. The undo change is what playback's repeat list actually honors.
        measure->undoChangeProperty(Pid::REPEAT_START, false);
        measure->undoChangeProperty(Pid::REPEAT_END, false);
        measure->undoChangeProperty(Pid::REPEAT_JUMP, false);
        measure->undoChangeProperty(Pid::REPEAT_COUNT, 1);
        if (measure->sectionBreak()) {
            measure->undoSetBreak(false, LayoutBreakType::SECTION);
        }

        const std::vector<EngravingItem*> measureElements = measure->el();
        for (EngravingItem* element : measureElements) {
            if (element->ownershipParent() && (element->isMarker() || element->isJump())) {
                score->deleteItem(element);
            }
        }

        for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            const std::vector<EngravingItem*> annotations = segment->annotations();
            for (EngravingItem* element : annotations) {
                if (element->ownershipParent() && (element->isMarker() || element->isJump())) {
                    score->deleteItem(element);
                }
            }
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

static void retargetHarmonies(EngravingItem* element, Score* score)
{
    if (!element) {
        return;
    }
    if (element->isHarmony()) {
        element->setScore(score);
    }
    for (EngravingItem* child : element->getChildren(false)) {
        retargetHarmonies(child, score);
    }
}

//! cloneMeasure keeps the written notes but drops the measure flags playback needs.
//! A measure repeat (%) is silent without its staff count: the sign has no notes of
//! its own, and the count is what points at the bar it should play.
static void copyMeasurePlaybackState(Measure* dest, const Measure* source)
{
    if (!dest || !source) {
        return;
    }

    dest->setRepeatStart(source->repeatStart());
    dest->setRepeatEnd(source->repeatEnd());
    dest->setRepeatJump(source->repeatJump());

    const staff_idx_t staves = std::min(dest->mstaves().size(), source->mstaves().size());
    for (staff_idx_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
        dest->setMeasureRepeatCount(source->measureRepeatCount(staffIdx), staffIdx);
    }
}

//! A copied one-bar repeat (%) keeps the sign and drops the staff count that says which
//! bar it plays. Playback then has nothing to sound. Put that count back when the sign
//! itself still says it repeats a single bar.
static void repairOneMeasureRepeatCounts(Score* score)
{
    const staff_idx_t staves = static_cast<staff_idx_t>(score->nstaves());
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (staff_idx_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
            if (measure->measureRepeatCount(staffIdx) > 0) {
                continue;
            }

            const track_idx_t from = staff2track(staffIdx);
            const track_idx_t to = from + VOICES;
            for (Segment* segment = measure->first(SegmentType::ChordRest); segment; segment = segment->next(SegmentType::ChordRest)) {
                for (track_idx_t track = from; track < to; ++track) {
                    EngravingItem* element = segment->element(track);
                    if (element && element->isMeasureRepeat() && toMeasureRepeat(element)->numMeasures() == 1) {
                        measure->setMeasureRepeatCount(1, staffIdx);
                    }
                }
            }
        }
    }
}

static void detachUnplayableLinks(Chord* chord)
{
    if (!chord) {
        return;
    }

    for (Chord* grace : chord->graceNotes()) {
        detachUnplayableLinks(grace);
    }

    for (Note* note : chord->notes()) {
        // The copy keeps a tie that still points at the source bar. Layout follows it
        // and can drop the note. The note itself should sound either way.
        if (Tie* tie = note->tieFor(); tie && !tie->endNote()) {
            note->setTieFor(nullptr);
            delete tie;
        }
    }

    if (TremoloTwoChord* tremolo = chord->tremoloTwoChord(); tremolo && (!tremolo->chord1() || !tremolo->chord2())) {
        chord->setTremoloTwoChord(nullptr, false);
        delete tremolo;
    }
}

static bool staffHasWrittenNotes(const Measure* measure, staff_idx_t staffIdx)
{
    const track_idx_t from = staff2track(staffIdx);
    const track_idx_t to = from + VOICES;
    for (const Segment* segment = measure->first(SegmentType::ChordRest); segment; segment = segment->next(SegmentType::ChordRest)) {
        for (track_idx_t track = from; track < to; ++track) {
            const EngravingItem* element = segment->element(track);
            if (element && element->isChord()) {
                return true;
            }
        }
    }
    return false;
}

//! Replace one staff of `dest` with the written notes of the same staff in `source`.
static bool copyStaffNotes(Measure* dest, const Measure* source, staff_idx_t staffIdx)
{
    if (!staffHasWrittenNotes(source, staffIdx)) {
        return false;
    }

    Score* score = dest->score();
    const track_idx_t from = staff2track(staffIdx);
    const track_idx_t to = from + VOICES;

    std::vector<EngravingItem*> remove;
    for (Segment* segment = dest->first(SegmentType::ChordRest); segment; segment = segment->next(SegmentType::ChordRest)) {
        for (track_idx_t track = from; track < to; ++track) {
            if (EngravingItem* element = segment->element(track)) {
                remove.push_back(element);
            }
        }
    }
    for (EngravingItem* element : remove) {
        Segment* segment = toSegment(element->ownershipParent());
        segment->remove(element);
        delete element;
    }

    TupletMap tupletMap;
    for (const Segment* sourceSegment = source->first(SegmentType::ChordRest); sourceSegment;
         sourceSegment = sourceSegment->next(SegmentType::ChordRest)) {
        for (track_idx_t track = from; track < to; ++track) {
            const EngravingItem* element = sourceSegment->element(track);
            if (!element || !element->isChordRest() || element->isMeasureRepeat()) {
                continue;
            }

            EngravingItem* clone = element->clone();
            ChordRest* chordRest = toChordRest(clone);
            chordRest->setScore(score);
            chordRest->setTrack(track);

            if (const Tuplet* sourceTuplet = toChordRest(element)->tuplet()) {
                Tuplet* sourceKey = const_cast<Tuplet*>(sourceTuplet);
                Tuplet* tuplet = tupletMap.findNew(sourceKey);
                if (!tuplet) {
                    tuplet = Factory::copyTuplet(*sourceTuplet);
                    tuplet->clear();
                    tuplet->setTrack(track);
                    tuplet->setScore(score);
                    tuplet->setOwnershipParent(dest);
                    tupletMap.add(sourceKey, tuplet);
                }
                chordRest->setTuplet(tuplet);
                tuplet->add(chordRest);
            }

            if (chordRest->isChord()) {
                detachUnplayableLinks(toChord(chordRest));
            }

            Segment* segment = dest->getSegmentR(SegmentType::ChordRest, sourceSegment->rtick());
            segment->add(chordRest);
        }
    }

    return true;
}

//! A measure repeat is silent unless playback can see the bar it stands for. Write that
//! bar's notes in, so a later copy of the passage does not depend on the sign surviving.
static void realizeMeasureRepeats(Score* score)
{
    repairOneMeasureRepeatCounts(score);

    struct Referral {
        Measure* dest = nullptr;
        Measure* source = nullptr;
        staff_idx_t staffIdx = 0;
    };

    std::vector<Referral> referrals;
    const staff_idx_t staves = static_cast<staff_idx_t>(score->nstaves());
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (staff_idx_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
            if (measure->measureRepeatCount(staffIdx) <= 0) {
                continue;
            }

            MeasureRepeat* repeat = measure->measureRepeatElement(staffIdx);
            if (!repeat) {
                continue;
            }

            Measure* source = const_cast<Measure*>(repeat->referringMeasure(measure));
            if (!source || source == measure) {
                continue;
            }

            referrals.push_back({ measure, source, staffIdx });
        }
    }

    for (const Referral& referral : referrals) {
        // Earlier repeats in this list are already notes, so a chain of one-bar
        // repeats copies the lick rather than another sign.
        if (!copyStaffNotes(referral.dest, referral.source, referral.staffIdx)) {
            continue;
        }
        referral.dest->setMeasureRepeatCount(0, referral.staffIdx);
    }
}

//! Copy [startTick, endTick) onto the end of dest.
//! Score::appendMeasuresFromScore also joins pickup measures and keeps the copied
//! lines attached to the source score. Layout of that copy then crashes.
static bool appendRange(Score* dest, Score* source, const Fraction& startTick, const Fraction& endTick)
{
    if (!dest || !source || !dest->last()) {
        return false;
    }

    Measure* first = source->tick2measure(startTick);
    if (!first) {
        return false;
    }

    const Fraction tickOfAppend = dest->last()->endTick();
    TieMap tieMap;
    Fraction curTick = tickOfAppend;
    for (MeasureBase* measure = first; measure && measure->tick() < endTick; measure = measure->next()) {
        if (!measure->isMeasure()) {
            continue;
        }
        Measure* copy = toMeasure(measure)->cloneMeasure(dest, curTick, &tieMap);
        curTick += copy->ticks();
        copy->setScore(dest);
        copyMeasurePlaybackState(copy, toMeasure(measure));
        dest->measures()->append(copy);
    }
    if (curTick == tickOfAppend) {
        return false;
    }

    for (Measure* measure = dest->tick2measure(tickOfAppend); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            for (EngravingItem* annotation : segment->annotations()) {
                retargetHarmonies(annotation, dest);
            }
            for (EngravingItem* element : segment->elist()) {
                retargetHarmonies(element, dest);
            }
        }
    }

    const std::multimap<int, Spanner*>& spanners = source->spanner();
    const auto lower = spanners.lower_bound(startTick.ticks());
    const auto upper = spanners.upper_bound(endTick.ticks());
    for (auto it = lower; it != upper; ++it) {
        Spanner* spanner = it->second;
        if (!spanner || spanner->tick2() > endTick) {
            continue;
        }

        Spanner* copy = toSpanner(spanner->clone());
        // The clone still points at the source score's notes and systems.
        copy->setStartElement(nullptr);
        copy->setEndElement(nullptr);
        for (SpannerSegment* segment : copy->spannerSegments()) {
            segment->setSystem(nullptr);
        }
        copy->eraseSpannerSegments();
        copy->setScore(dest);
        copy->moveToDummy();
        copy->setTick(spanner->tick() - startTick + tickOfAppend);
        copy->setTick2(spanner->tick2() - startTick + tickOfAppend);
        copy->computeStartElement();
        copy->computeEndElement();
        if (!copy->startElement() || !copy->endElement()) {
            delete copy;
            continue;
        }
        dest->addElement(copy);
    }

    return true;
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

struct TickSpan {
    Fraction start;
    Fraction end;
};

struct CarriedTechnique {
    track_idx_t track = 0;
    PlayingTechniqueType type = PlayingTechniqueType::Undefined;
    TextStyleType textStyle = TextStyleType::STAFF;
    String text;
    bool play = true;
    Fraction tick;
};

struct CarriedSound {
    track_idx_t track = 0;
    TextStyleType textStyle = TextStyleType::STAFF;
    String text;
    StringList presets;
    String playingTechnique;
    bool play = true;
    bool applyToAllStaves = false;
    Fraction tick;
};

struct CarriedPlayback {
    bool stampTempo = false;
    BeatsPerSecond tempo = Constants::DEFAULT_TEMPO;
    std::map<track_idx_t, CarriedTechnique> techniques;
    std::map<track_idx_t, CarriedSound> sounds;
};

static bool soundFlagAffectsPlayback(const SoundFlag* flag)
{
    return flag && flag->play() && (!flag->soundPresets().empty() || !flag->playingTechnique().empty());
}

static bool hasOpeningAnnotation(const Measure* measure, ElementType type, track_idx_t track)
{
    const Segment* chord = measure->first(SegmentType::ChordRest);
    const Fraction tick = chord ? chord->tick() : measure->tick();
    for (const Segment* segment = measure->first(); segment && segment->tick() <= tick; segment = segment->next()) {
        for (const EngravingItem* annotation : segment->annotations()) {
            if (annotation->type() != type) {
                continue;
            }
            if (track != muse::nidx && annotation->track() != track) {
                continue;
            }
            if (annotation->isTempoText() && !toTempoText(annotation)->playTempoText()) {
                continue;
            }
            return true;
        }
    }
    return false;
}

//! Tempo and playing technique stay in effect until the next marking. Trimming the
//! bars that hold those markings would otherwise fall back to 120 BPM and arco.
static CarriedPlayback capturePlaybackState(const Score* score, const Fraction& startTick)
{
    CarriedPlayback carried;
    carried.tempo = score->tempo(startTick);

    for (const Measure* measure = score->firstMeasure(); measure && measure->tick() < startTick; measure = measure->nextMeasure()) {
        for (const Segment* segment = measure->first(); segment; segment = segment->next()) {
            if (segment->tick() >= startTick) {
                break;
            }
            for (const EngravingItem* annotation : segment->annotations()) {
                if (annotation->isTempoText() && toTempoText(annotation)->playTempoText()) {
                    carried.stampTempo = true;
                    continue;
                }

                if (annotation->isPlayTechAnnotation()) {
                    const PlayTechAnnotation* technique = toPlayTechAnnotation(annotation);
                    if (!technique->playPlayTechAnnotation()) {
                        continue;
                    }
                    const PlayingTechniqueType type = technique->techniqueType();
                    if (type == PlayingTechniqueType::Undefined) {
                        continue;
                    }

                    CarriedTechnique& carriedTechnique = carried.techniques[technique->track()];
                    carriedTechnique.track = technique->track();
                    carriedTechnique.type = type;
                    carriedTechnique.textStyle = technique->textStyleType();
                    carriedTechnique.text = technique->xmlText();
                    carriedTechnique.play = true;
                    carriedTechnique.tick = segment->tick();

                    if (type == PlayingTechniqueType::Natural || type == PlayingTechniqueType::Open) {
                        carried.sounds.erase(technique->track());
                    }
                    continue;
                }

                if (!annotation->isStaffText()) {
                    continue;
                }
                const StaffText* staffText = toStaffText(annotation);
                const SoundFlag* flag = staffText->soundFlag();
                if (!soundFlagAffectsPlayback(flag)) {
                    continue;
                }

                CarriedSound& carriedSound = carried.sounds[staffText->track()];
                carriedSound.track = staffText->track();
                carriedSound.textStyle = staffText->textStyleType();
                carriedSound.text = staffText->xmlText();
                carriedSound.presets = flag->soundPresets();
                carriedSound.playingTechnique = flag->playingTechnique();
                carriedSound.play = flag->play();
                carriedSound.applyToAllStaves = flag->applyToAllStaves();
                carriedSound.tick = segment->tick();
            }
        }
    }

    for (auto it = carried.techniques.begin(); it != carried.techniques.end();) {
        const CarriedTechnique& technique = it->second;
        if (technique.type == PlayingTechniqueType::Natural || technique.type == PlayingTechniqueType::Open) {
            it = carried.techniques.erase(it);
            continue;
        }
        auto sound = carried.sounds.find(technique.track);
        if (sound != carried.sounds.end() && sound->second.tick < technique.tick) {
            sound->second.playingTechnique.clear();
        }
        ++it;
    }

    return carried;
}

static Segment* openingSegment(Measure* measure)
{
    if (Segment* segment = measure->first(SegmentType::ChordRest)) {
        return segment;
    }
    return measure->first();
}

static bool hasOpeningSoundFlag(const Measure* measure, track_idx_t track)
{
    const Segment* chord = measure->first(SegmentType::ChordRest);
    const Fraction tick = chord ? chord->tick() : measure->tick();
    for (const Segment* segment = measure->first(); segment && segment->tick() <= tick; segment = segment->next()) {
        for (const EngravingItem* annotation : segment->annotations()) {
            if (!annotation->isStaffText() || annotation->track() != track) {
                continue;
            }
            if (soundFlagAffectsPlayback(toStaffText(annotation)->soundFlag())) {
                return true;
            }
        }
    }
    return false;
}

static void stampPlaybackState(Score* score, const CarriedPlayback& carried, Measure* measure)
{
    Segment* segment = measure ? openingSegment(measure) : nullptr;
    if (!measure || !segment || score->ntracks() == 0) {
        return;
    }

    if (carried.stampTempo && !hasOpeningAnnotation(measure, ElementType::TEMPO_TEXT, muse::nidx)) {
        TempoText* tempo = Factory::createTempoText(segment);
        tempo->setTrack(0);
        tempo->setFollowText(false);
        tempo->setTempo(carried.tempo);
        const int bpm = std::lround(carried.tempo.toBPM().val);
        tempo->setXmlText(String(u"%1").arg(bpm));
        tempo->setScore(score);
        score->undoAddElement(tempo, false);
    }

    for (const auto& pair : carried.techniques) {
        const CarriedTechnique& carriedTechnique = pair.second;
        if (carriedTechnique.track >= score->ntracks()) {
            continue;
        }
        if (hasOpeningAnnotation(measure, ElementType::PLAYTECH_ANNOTATION, carriedTechnique.track)) {
            continue;
        }

        PlayTechAnnotation* annotation = Factory::createPlayTechAnnotation(segment, carriedTechnique.type,
                                                                           carriedTechnique.textStyle);
        annotation->setTrack(carriedTechnique.track);
        annotation->setXmlText(carriedTechnique.text);
        annotation->setPlayPlayTechAnnotation(carriedTechnique.play);
        annotation->setScore(score);
        score->undoAddElement(annotation, false);
    }

    for (const auto& pair : carried.sounds) {
        const CarriedSound& carriedSound = pair.second;
        if (carriedSound.track >= score->ntracks()) {
            continue;
        }
        if (carriedSound.presets.empty() && carriedSound.playingTechnique.empty()) {
            continue;
        }
        if (hasOpeningSoundFlag(measure, carriedSound.track)) {
            continue;
        }

        StaffText* staffText = Factory::createStaffText(segment, carriedSound.textStyle);
        staffText->setTrack(carriedSound.track);
        staffText->setXmlText(carriedSound.text);
        staffText->setScore(score);

        SoundFlag* flag = Factory::createSoundFlag(staffText);
        flag->setSoundPresets(carriedSound.presets);
        flag->setPlayingTechnique(carriedSound.playingTechnique);
        flag->setPlay(carriedSound.play);
        flag->setApplyToAllStaves(carriedSound.applyToAllStaves);
        staffText->add(flag);
        score->undoAddElement(staffText, false);
    }
}

static void appendContiguous(const Measure* measure, const Measure*& runStart, const Measure*& previous, std::vector<TickSpan>& spans)
{
    if (!measure) {
        return;
    }
    if (measure->isMMRest()) {
        const Measure* first = measure->mmRestFirst();
        const Measure* last = measure->mmRestLast();
        if (!first || !last || first == measure) {
            return;
        }
        for (const Measure* underlying = first; underlying; underlying = underlying->nextMeasure()) {
            appendContiguous(underlying, runStart, previous, spans);
            if (underlying == last) {
                break;
            }
        }
        return;
    }

    if (!runStart) {
        runStart = measure;
        previous = measure;
        return;
    }
    if (previous && previous->endTick() == measure->tick()) {
        previous = measure;
        return;
    }

    spans.push_back({ runStart->tick(), previous->endTick() });
    runStart = measure;
    previous = measure;
}

//! One play-through of the trimmed passage, with repeats, voltas, and jumps written out in order.
static std::vector<TickSpan> expandedPlaybackSpans(MasterScore* score)
{
    std::vector<TickSpan> spans;
    score->invalidateRepeatList();
    const RepeatList& repeats = score->repeatList(/*expandRepeats*/ true, /*updateTies*/ false);

    for (const RepeatSegment* segment : repeats) {
        const Measure* runStart = nullptr;
        const Measure* previous = nullptr;
        for (const Measure* measure : segment->measureList()) {
            appendContiguous(measure, runStart, previous, spans);
        }
        if (runStart && previous) {
            spans.push_back({ runStart->tick(), previous->endTick() });
        }
    }

    return spans;
}

static bool spansAreWrittenOrder(const std::vector<TickSpan>& spans, const Score* score)
{
    if (spans.size() != 1 || !score->firstMeasure() || !score->lastMeasure()) {
        return false;
    }
    return spans.front().start == score->firstMeasure()->tick() && spans.front().end == score->lastMeasure()->endTick();
}

static bool unrollRepeatedPassage(MasterScore* score, MasterScore* source, const std::vector<TickSpan>& spans)
{
    if (spans.empty()) {
        return false;
    }
    if (spansAreWrittenOrder(spans, score)) {
        return true;
    }

    Measure* firstKeep = score->tick2measure(spans.front().start);
    Measure* lastKeep = score->tick2measure(spans.front().end - Fraction::fromTicks(1));
    if (!firstKeep || !lastKeep) {
        return false;
    }

    Measure* prefixEnd = firstKeep->prevMeasure();
    Measure* suffixStart = lastKeep->nextMeasure();
    Measure* suffixEnd = score->lastMeasure();
    if (prefixEnd) {
        score->deleteMeasures(score->firstMeasure(), prefixEnd);
    }
    if (suffixStart && suffixEnd) {
        score->deleteMeasures(suffixStart, suffixEnd);
    }

    for (size_t index = 1; index < spans.size(); ++index) {
        if (!appendRange(score, source, spans[index].start, spans[index].end)) {
            LOGE() << "Could not write out repeated passage";
            return false;
        }
    }

    return score->firstMeasure() != nullptr;
}

static MasterScore* buildBarLoop(Score* source, const Fraction& startTick, const Fraction& endTick, int repeats)
{
    std::unique_ptr<MasterScore> passage(cloneAsMaster(source));
    std::unique_ptr<MasterScore> score(cloneAsMaster(source));
    if (!passage || !score) {
        return nullptr;
    }

    dropExcerpts(passage.get());
    dropExcerpts(score.get());

    const CarriedPlayback carried = capturePlaybackState(score.get(), startTick);

    // These edits go through the undo stack. Outside a transaction each command is
    // applied and destroyed immediately, which frees measures the next step still uses
    // and leaves playback with a non-finite duration. Audio export then aborts.
    const muse::TranslatableString commandName = muse::TranslatableString::untranslatable("Export selected bars");
    passage->startCmd(commandName);
    if (!trimToRange(passage.get(), startTick, endTick)) {
        LOGE() << "Selected bar range is not in the score";
        passage->endCmd(true);
        return nullptr;
    }
    passage->updateTicksAndTimeSigMap();
    passage->endCmd();

    score->startCmd(commandName);
    if (!trimToRange(score.get(), startTick, endTick)) {
        score->endCmd(true);
        return nullptr;
    }
    score->updateTicksAndTimeSigMap();

    // Spans are in the trimmed passage's tick space. Later loops copy from `passage`,
    // which is not unrolled, so this list has to be captured before `score` grows.
    const std::vector<TickSpan> spans = expandedPlaybackSpans(score.get());
    if (!unrollRepeatedPassage(score.get(), passage.get(), spans)) {
        score->endCmd(true);
        return nullptr;
    }

    // The sounded bars are written out in order. Clear the signs so playback does not
    // honor them a second time, and so a jump cannot end the score before the next loop.
    removeRepeatMarkings(score.get());
    stampPlaybackState(score.get(), carried, score->firstMeasure());
    score->updateTicksAndTimeSigMap();
    score->endCmd();

    repeats = std::clamp(repeats, 1, 99);
    if (repeats > 1) {
        score->startCmd(commandName);
        for (int copy = 1; copy < repeats; ++copy) {
            if (!score->lastMeasure()) {
                score->endCmd(true);
                return nullptr;
            }
            const Fraction loopTick = score->lastMeasure()->endTick();
            for (const TickSpan& span : spans) {
                if (!appendRange(score.get(), passage.get(), span.start, span.end)) {
                    LOGE() << "Could not append repetition " << copy + 1;
                    score->endCmd(true);
                    return nullptr;
                }
            }
            stampPlaybackState(score.get(), carried, score->tick2measure(loopTick));
        }
        // Copies taken from the trimmed passage still have its repeat signs.
        removeRepeatMarkings(score.get());
        score->updateTicksAndTimeSigMap();
        score->endCmd();
    }

    // Signs are still in place here, including on bars appended for later loops.
    // Turn them into the notes they stand for before layout, or the next loop plays
    // the written bar and then silence where the repeat sign was.
    score->startCmd(commandName);
    realizeMeasureRepeats(score.get());
    score->updateTicksAndTimeSigMap();
    score->endCmd();

    // endCmd updates automation incrementally from the deleted range. Rebuild it from
    // the bars that remain so the exported duration matches those bars.
    score->initAutomation();
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
