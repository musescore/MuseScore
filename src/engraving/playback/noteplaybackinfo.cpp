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

#include "noteplaybackinfo.h"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <functional>
#include <limits>
#include <optional>
#include <set>
#include <vector>

#include "containers.h"
#include "mpe/mpetypes.h"

#include "engraving/automation/automationdata.h"
#include "engraving/automation/dynamicvalues.h"
#include "engraving/infrastructure/eidregister.h"

#include "playbackmarkings.h"

#include "../dom/breath.h"
#include "../dom/capo.h"
#include "../dom/chord.h"
#include "../dom/dynamic.h"
#include "../dom/fermata.h"
#include "../dom/hairpin.h"
#include "../dom/masterscore.h"
#include "../dom/note.h"
#include "../dom/ottava.h"
#include "../dom/repeatlist.h"
#include "../dom/staff.h"
#include "../dom/segment.h"
#include "../dom/spanner.h"
#include "../dom/tempotimeline.h"
#include "../dom/tie.h"

namespace mu::engraving {
bool NotePlaybackSource::operator==(const NotePlaybackSource& src) const
{
    return item == src.item
           && editedInAutomation == src.editedInAutomation;
}

bool NotePlaybackInfo::isValid() const
{
    return note
           && !passes.empty();
}

namespace noteplaybackinfo_detail {
const EngravingItem* itemFromEID(const MasterScore* score, const EID& eid)
{
    const EngravingObject* obj = score->eidRegister()->itemFromEID(eid);
    return (obj && obj->isEngravingItem())
           ? static_cast<const EngravingItem*>(obj)
           : nullptr;
}

void addSource(std::vector<NotePlaybackSource>& sources, const NotePlaybackSource& source)
{
    if (std::ranges::none_of(sources,
                             [&source](const NotePlaybackSource& src) {
        return src.item == source.item;
    }
                             )) {
        sources.push_back(source);
    }
}

bool isGradualChange(const EngravingItem* item)
{
    return item
           && (item->isHairpin() || item->isGradualTempoChange());
}

//! A change has no effect yet on its first tick, where it starts from the value before it
void addGradualChangeSources(
    const MasterScore* score,
    const Note* note,
    int tick,
    std::vector<NotePlaybackSource>& sources,
    std::predicate<const Spanner*> auto predicate)
{
    if (!score || !note) {
        return;
    }

    const auto& intervals = score->spannerMap().findOverlapping(tick, tick);

    for (const auto& interval : intervals) {
        const Spanner* spanner = interval.value;

        if (spanner && spanner->playSpanner()
            && (spanner->tick().ticks() < tick) && (tick < spanner->tick2().ticks())
            && predicate(spanner)
            ) {
            addSource(sources, { spanner, false });
        }
    }
}

//! The point a curve value at utick is taken from (the last point at or before it)
void addSourceFromCurve(
    const MasterScore* score,
    const AutomationCurve& curve,
    utick_t utick,
    std::vector<NotePlaybackSource>& sources)
{
    const auto it = muse::findLessOrEqual(curve, utick);

    if (it == curve.cend()) {
        return;
    }

    const AutomationPoint& point = it->second;
    NotePlaybackSource source;

    if (point.itemId.has_value()) {
        source.item = itemFromEID(score, point.itemId.value());

        if (!source.item) {
            return; // stale id (e.g. the item was just deleted)
        }

        source.editedInAutomation = !point.generated;
    }

    addSource(sources, source);
}

//! Mirrors ScoreAutomationController::resolveKeys()
bool dynamicAppliesToNote(const EngravingItem* dynamic, const Note* note)
{
    if (dynamic && note) {
        switch (dynamic->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>()) {
        case VoiceAssignment::ALL_VOICE_IN_INSTRUMENT:
            return dynamic->part() == note->part();
        case VoiceAssignment::ALL_VOICE_IN_STAFF:
            return dynamic->staffIdx() == note->staffIdx();
        case VoiceAssignment::CURRENT_VOICE_ONLY:
            return dynamic->staffIdx() == note->staffIdx()
                   && dynamic->voice() == note->voice();
        }
    }

    return false;
}

//! Gradual changes (hairpins, accel./rit.) that are in progress at tick
//! The curve point before a note inside one can belong to an unrelated marking at the change's start
void addGradualTempoChangeSources(
    const MasterScore* score,
    const Note* note,
    int tick,
    std::vector<NotePlaybackSource>& tempoSources)
{
    addGradualChangeSources(score, note, tick, tempoSources,
                            [](const Spanner* spanner) {
        return spanner->isGradualTempoChange();
    }
                            );
}

void addGradualDynamicChangeSources(
    const MasterScore* score,
    const Note* note,
    int tick,
    std::vector<NotePlaybackSource>& dynamicSources)
{
    addGradualChangeSources(score, note, tick, dynamicSources,
                            [&note](const Spanner* spanner) {
        return spanner->isHairpin() && dynamicAppliesToNote(spanner, note);
    }
                            );
}

//! The dynamic a hairpin ends on, if playback uses one (mirrors findEndDynamicType in ScoreAutomationController)
const Dynamic* hairpinEndDynamic(const Hairpin* hairpin)
{
    const auto playedDynamic = [](const EngravingItem* item) -> const Dynamic* {
        return (item && item->isDynamic() && toDynamic(item)->playDynamic())
               ? toDynamic(item)
               : nullptr;
    };

    if (hairpin->spannerSegments().empty()) {
        const Segment* endSegment = hairpin->endSegment();

        if (!endSegment) {
            return nullptr;
        }

        const track_idx_t track = hairpin->track();

        for (const EngravingItem* item : endSegment->findAnnotations(ElementType::DYNAMIC, track, track)) {
            if (const Dynamic* dynamic = playedDynamic(item)) {
                return dynamic;
            }
        }

        return nullptr;
    }

    const LineSegment* segment = hairpin->backSegment();

    if (!segment) {
        return nullptr;
    }

    if (const Dynamic* dynamic = playedDynamic(segment->ldata()->itemSnappedAfter())) {
        return dynamic;
    }

    return playedDynamic(toHairpinSegment(segment)->findElementToSnapAfter(false /*ignoreInvisible*/, true /*requirePlayable*/));
}

//! A curve point at a hairpin's end can carry the hairpin's id even when its level is the dynamic the hairpin
//! ends on (one point per tick; whichever was added first keeps its id). Once the note is past the hairpin,
//! that dynamic is what sets its level, so credit it instead.
void creditHairpinEndDynamics(std::vector<NotePlaybackSource>& sources, int attackTick)
{
    std::vector<NotePlaybackSource> credited;

    for (NotePlaybackSource source : sources) {
        const bool isPastHairpin = (source.item != nullptr)
                                   && (source.item->isHairpin())
                                   && (attackTick >= toHairpin(source.item)->tick2().ticks());

        if (isPastHairpin) {
            if (const Dynamic* dynamic = hairpinEndDynamic(toHairpin(source.item))) {
                source.item = dynamic;
            }
        }

        addSource(credited, source);
    }

    sources = std::move(credited);
}

const AutomationCurve* noteDynamicsCurve(const AutomationData& automation, const Note* note)
{
    const Staff* staff = note->staff();

    if (!staff) {
        return nullptr;
    }

    if (!staff->isPrimaryStaff() && staff->primaryStaff()) {
        staff = staff->primaryStaff();
    }

    // Same lookup as PlaybackContext::dynamicsCurve
    const AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, staff->id(), note->voice());
    const AutomationCurve* curve = &automation.curve(key);

    if (curve->empty()) {
        curve = &automation.curve(key.withoutVoice());
    }

    return curve;
}

//! Ties joining the note to the notes before and after it, in score order (only ties that playback honours)
void addTieMarkings(const Note* note, std::vector<NotePlaybackMarking>& markings)
{
    std::set<const Tie*> seen;
    std::vector<NotePlaybackMarking> backwardTies;

    for (const Note* current = note; current;) {
        const Tie* tie = current->tieBack();

        if (!tie || !tie->playSpanner() || !seen.insert(tie).second) {
            break;
        }

        backwardTies.push_back({ tie, {} });
        current = tie->startNote();
    }

    markings.insert(markings.end(), backwardTies.rbegin(), backwardTies.rend());

    for (const Note* current = note; current;) {
        const Tie* tie = current->tieFor();

        if (!tie || !tie->playSpanner() || !seen.insert(tie).second) {
            break;
        }

        markings.push_back({ tie, {} });
        current = tie->endNote();
    }
}

//! The chords of the notes the note is tied to and from, in score order (only ties that playback honours)
std::vector<const Chord*> tieChainChords(const Note* note)
{
    std::vector<const Chord*> chords;
    std::set<const Note*> seen;

    for (const Note* current = note->firstTiedNote(false /*ignorePlayback*/); current && seen.insert(current).second;) {
        chords.push_back(current->chord());

        const Tie* tie = current->tieFor();

        current = (tie && tie->playSpanner())
                  ? tie->endNote()
                  : nullptr;
    }

    return chords;
}

//! Played fermatas and breath marks on any chord of the tie chain: they stretch or cut the note.
//! A fermata stretches time for the whole score at its tick, whichever staff it's on.
void addPauseMarkings(const Note* note, std::vector<NotePlaybackMarking>& markings)
{
    for (const Chord* chord : tieChainChords(note)) {
        if (const Segment* segment = chord->segment()) {
            for (const EngravingItem* annotation : segment->annotations()) {
                if (annotation->isFermata() && toFermata(annotation)->play()) {
                    markings.push_back({ annotation, {} });
                }
            }
        }

        if (const Breath* breath = chord->hasBreathMark()) {
            markings.push_back({ breath, {} });
        }
    }
}

//! What a gradual change (hairpin, accel./rit.) starts from: the source of the curve's value just before the
//! change takes over at startUTick (a marking at the same tick as the change keeps the point there)
std::optional<NotePlaybackSource> gradualChangeOrigin(
    const MasterScore* score,
    const AutomationCurve& curve,
    utick_t startUTick,
    const EngravingItem* change)
{
    for (auto it = muse::findLessOrEqual(curve, startUTick); it != curve.cend();) {
        const AutomationPoint& point = it->second;

        if (!point.itemId.has_value()) {
            return NotePlaybackSource { nullptr, false };
        }

        const EngravingItem* item = itemFromEID(score, point.itemId.value());

        if (!item) {
            return std::nullopt; // stale id
        }

        if (item != change) {
            return NotePlaybackSource { item, !point.generated };
        }

        if (it == curve.cbegin()) {
            break;
        }

        --it;
    }

    return std::nullopt;
}

//! gradualChangeOrigin for each gradual change among sources; toCurveUTick converts a score tick to the curve's
void addGradualChangeOrigins(
    const MasterScore* score,
    const AutomationCurve& curve,
    const std::vector<NotePlaybackSource>& sources,
    const std::function<utick_t(int)>& toCurveUTick,
    std::vector<NotePlaybackSource>& origins)
{
    for (const NotePlaybackSource& source : sources) {
        if (!isGradualChange(source.item)) {
            continue;
        }

        const int startTick = toSpanner(source.item)->tick().ticks();
        const std::optional<NotePlaybackSource> origin = gradualChangeOrigin(score, curve, toCurveUTick(startTick), source.item);

        if (!origin) {
            continue;
        }

        // e.g. a hairpin right after another one starts from the dynamic the first one ends on
        std::vector<NotePlaybackSource> credited { origin.value() };
        creditHairpinEndDynamics(credited, startTick);

        for (const NotePlaybackSource& src : credited) {
            addSource(origins, src);
        }
    }
}

//! A gradual change has no effect yet on its first tick: a curve point there can still carry the change's id, so
//! credit the source of the value it starts from instead (unless the point was edited in the automation lane)
void creditUnstartedGradualChanges(
    const MasterScore* score,
    const AutomationCurve& curve,
    int tick,
    const std::function<utick_t(int)>& toCurveUTick,
    std::vector<NotePlaybackSource>& sources)
{
    std::vector<NotePlaybackSource> credited;

    for (const NotePlaybackSource& source : sources) {
        const bool isUnstarted = isGradualChange(source.item)
                                 && !source.editedInAutomation
                                 && (toSpanner(source.item)->tick().ticks() >= tick);

        if (!isUnstarted) {
            addSource(credited, source);
            continue;
        }

        const std::optional<NotePlaybackSource> origin = gradualChangeOrigin(score, curve, toCurveUTick(tick), source.item);

        if (origin) {
            addSource(credited, origin.value());
        }
    }

    // e.g. a hairpin right after another one starts from the dynamic the first one ends on
    creditHairpinEndDynamics(credited, tick);
    sources = std::move(credited);
}

//! The ottava whose pitch shift playback uses for the note (mirrors Staff::updateOttava: the latest-starting one wins)
const Ottava* playedOttava(const Note* note)
{
    const Chord* chord = note->chord();
    const int tick = chord->tick().ticks();

    if (chord->staff()->pitchOffset(chord->tick()) == 0) {
        return nullptr;
    }

    const Ottava* result = nullptr;

    for (const auto& interval : note->score()->spannerMap().findOverlapping(tick, tick)) {
        const Spanner* spanner = interval.value;

        const bool appliesToNote = (spanner->isOttava())
                                   && (spanner->playSpanner())
                                   && (spanner->staffIdx() == note->staffIdx())
                                   && (spanner->tick().ticks() <= tick)
                                   && (tick < spanner->tick2().ticks());

        if (appliesToNote && (!result || (spanner->tick() >= result->tick()))) {
            result = toOttava(spanner);
        }
    }

    return result;
}

//! The capo playback transposes the note by, if any (mirrors Note::ottaveCapoFret)
const Capo* playedCapo(const Note* note)
{
    const Chord* chord = note->chord();
    const staff_idx_t staffIdx = note->staffIdx();

    // The capo in effect is the last one on the staff at or before the note
    for (const Segment* segment = chord->segment(); segment; segment = segment->prev1()) {
        for (const EngravingItem* annotation : segment->annotations()) {
            if (!annotation->isCapo() || (annotation->staffIdx() != staffIdx)) {
                continue;
            }

            const CapoParams& params = toCapo(annotation)->params();

            const bool transposesNote = (params.active)
                                        && (params.transposeMode == CapoParams::TransposeMode::PLAYBACK_ONLY)
                                        && (params.fretPosition != 0)
                                        && (!params.ignoredStrings.contains(static_cast<string_idx_t>(note->string())));

            return transposesNote
                   ? toCapo(annotation)
                   : nullptr;
        }
    }

    return nullptr;
}

void addPitchShifts(const Note* note, std::vector<NotePlaybackPitchShift>& shifts)
{
    if (const Ottava* ottava = playedOttava(note)) {
        shifts.push_back({ ottava, ottava->pitchShift() });
    }

    if (const Capo* capo = playedCapo(note)) {
        shifts.push_back({ capo, capo->params().fretPosition });
    }
}
} // namespace noteplaybackinfo_detail

using namespace noteplaybackinfo_detail;

DynamicType nearestOrdinaryDynamic(double normalizedValue)
{
    constexpr double tieThreshold = 1e-9;

    DynamicType result = DynamicType::OTHER;
    double bestDistance = std::numeric_limits<double>::max();

    for (const auto& [type, value] : ORDINARY_DYNAMIC_VALUES) {
        if (type == DynamicType::N) {
            continue;
        }

        const double distance = std::abs(value.raw() - normalizedValue);

        // On a tie prefer the softer one, so 50% (natural) reads as mp/mf consistently
        if ((distance < (bestDistance - tieThreshold))
            || ((std::abs(distance - bestDistance) <= tieThreshold) && (value.raw() < normalizedValue))
            ) {
            bestDistance = distance;
            result = type;
        }
    }

    return result;
}

NotePlaybackInfo collectNotePlaybackInfo(const Note* note, bool expandRepeats)
{
    NotePlaybackInfo info;

    if (!note) {
        return info;
    }

    MasterScore* master = note->masterScore();

    if (!master) {
        return info;
    }

    // Playback (and its automation) always works on the master score
    if (note->score() != master) {
        EngravingItem* linked = note->findLinkedInScore(master);

        if (!linked || !linked->isNote()) {
            return info;
        }

        note = toNote(linked);
    }

    const AutomationDataConstPtr automation = master->automationData();

    if (!automation) {
        return info;
    }

    info.note = note;
    info.attackNote = note->firstTiedNote(false /*ignorePlayback*/);
    info.userVelocity = info.attackNote->userVelocity();

    const int tick = note->chord()->tick().ticks();
    const int attackTick = info.attackNote->chord()->tick().ticks();

    const AutomationCurve* dynCurve = noteDynamicsCurve(*automation, info.attackNote);
    const AutomationCurve& tempoCurve = automation->curve(TEMPO_KEY);

    const TempoTimeline& timeline = master->tempoTimeline(expandRepeats);

    const RepeatList& repeats = master->repeatList(expandRepeats);
    const RepeatList& expandedRepeats = master->expandedRepeatList();

    int passNumber = 0;

    for (const RepeatSegment* seg : repeats) {
        if (tick < seg->tick || tick >= seg->endTick()) {
            continue;
        }

        const int tickOffset = seg->utick - seg->tick;

        NotePlaybackPass pass;

        pass.passNumber = ++passNumber;
        pass.utick = tick + tickOffset;

        // Automation curves are always keyed by expanded uticks; without repeats, playback
        // looks them up at the raw tick (see PlaybackModel/PlaybackContext)
        const int curveUTick = pass.utick;

        // --- Tempo
        pass.tempoValue = timeline.tempo(pass.utick);
        pass.multipliedTempo = pass.tempoValue * timeline.tempoMultiplier();

        const int tempoCurveUTick = expandRepeats ? curveUTick : expandedRepeats.tick2utick(tick);
        pass.isDefaultTempo = muse::findLessOrEqual(tempoCurve, tempoCurveUTick) == tempoCurve.cend();

        addSourceFromCurve(master, tempoCurve, tempoCurveUTick, pass.tempoSources);

        // --- Dynamic (taken at the attack: a tied continuation doesn't re-articulate)
        const int attackUTick = (attackTick >= seg->tick && attackTick < seg->endTick())
                                ? (attackTick + tickOffset)
                                : repeats.tick2utick(attackTick);

        std::optional<real_t> value;

        if (dynCurve) {
            value = evaluateCurveAt(*dynCurve, attackUTick);
        }

        pass.isDefaultDynamic = !value.has_value();

        if (value) {
            pass.dynamicValue = value->raw();
        } else {
            const double dynLevel = static_cast<double>(muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::Natural));
            const double dynMaxLevel = static_cast<double>(muse::mpe::MAX_DYNAMIC_LEVEL);

            pass.dynamicValue = dynLevel / dynMaxLevel;
        }

        if (dynCurve) {
            addSourceFromCurve(master, *dynCurve, attackUTick, pass.dynamicSources);
            creditHairpinEndDynamics(pass.dynamicSources, attackTick);
        }

        addGradualTempoChangeSources(master, note, tick, pass.tempoSources);
        addGradualDynamicChangeSources(master, info.attackNote, attackTick, pass.dynamicSources);

        // --- Where the gradual changes start from
        const auto toPassUTick = [&](int t) -> utick_t {
            return (t >= seg->tick && t < seg->endTick())
                   ? (t + tickOffset)
                   : repeats.tick2utick(t);
        };

        const auto toTempoCurveUTick = [&](int t) -> utick_t {
            return expandRepeats
                   ? toPassUTick(t)
                   : expandedRepeats.tick2utick(t);
        };

        creditUnstartedGradualChanges(master, tempoCurve, tick, toTempoCurveUTick, pass.tempoSources);
        addGradualChangeOrigins(master, tempoCurve, pass.tempoSources, toTempoCurveUTick, pass.tempoOrigins);

        if (dynCurve) {
            creditUnstartedGradualChanges(master, *dynCurve, attackTick, toPassUTick, pass.dynamicSources);
            addGradualChangeOrigins(master, *dynCurve, pass.dynamicSources, toPassUTick, pass.dynamicOrigins);
        }

        info.passes.push_back(std::move(pass));
    }

    if (!info.passes.empty()) {
        addTieMarkings(note, info.markings);
        addPauseMarkings(note, info.markings);

        info.isPlayed = note->play();
        info.soundingPitch = note->ppitch();
        info.soundingTpc = note->playingTpc();

        addPitchShifts(note, info.pitchShifts);
    }

    return info;
}

void addArticulationMarkings(
    NotePlaybackInfo& info,
    const std::map<muse::mpe::ArticulationType,
                   const EngravingItem*>& sources)
{
    std::vector<NotePlaybackMarking> articulationMarkings;

    for (const auto& [type, item] : sources) {
        if (!item) {
            continue;
        }

        const auto it = std::ranges::find_if(articulationMarkings, [item](const NotePlaybackMarking& marking) {
            return marking.item == item;
        });

        if (it != articulationMarkings.end()) {
            it->effects.push_back(type);
        } else {
            articulationMarkings.push_back({ item, { type } });
        }
    }

    // An item found by both (e.g. a breath mark) is listed once, with its effects
    std::erase_if(info.markings, [&articulationMarkings](const NotePlaybackMarking& marking) {
        return std::ranges::any_of(articulationMarkings, [&marking](const NotePlaybackMarking& articulation) {
            return articulation.item == marking.item;
        });
    });

    // Articulations first: they're what most directly shapes the note
    info.markings.insert(info.markings.begin(), articulationMarkings.begin(), articulationMarkings.end());
}

std::vector<const EngravingItem*> listedItems(const NotePlaybackInfo& info)
{
    std::vector<const EngravingItem*> items;

    const auto add = [&items](const EngravingItem* item) {
        if (item && std::ranges::find(items, item) == items.end()) {
            items.push_back(item);
        }
    };

    const auto addSources = [&add](const std::vector<NotePlaybackSource>& sources) {
        for (const NotePlaybackSource& source : sources) {
            add(source.item);
        }
    };

    for (const NotePlaybackPitchShift& shift : info.pitchShifts) {
        add(shift.item);
    }

    add(info.sound.playingTechnique);

    for (const EngravingItem* flag : info.sound.soundFlags) {
        add(flag);
    }

    add(info.sound.syllable);

    for (const NotePlaybackPass& pass : info.passes) {
        if (!pass.isDefaultTempo) {
            addSources(pass.tempoSources);
            addSources(pass.tempoOrigins);
        }

        if (!pass.isDefaultDynamic) {
            addSources(pass.dynamicSources);
            addSources(pass.dynamicOrigins);
        }
    }

    for (const NotePlaybackMarking& marking : info.markings) {
        if (marking.item && isListedMarking(marking.item)) {
            add(marking.item);
        }
    }

    return items;
}
} // namespace mu::engraving
