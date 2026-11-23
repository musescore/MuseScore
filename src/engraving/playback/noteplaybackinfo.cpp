/*0000000000000deeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeefffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
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
#include <limits>
#include <optional>
#include <vector>

#include "containers.h"
#include "mpe/mpetypes.h"

#include "engraving/automation/automationdata.h"
#include "engraving/automation/dynamicvalues.h"
#include "engraving/infrastructure/eidregister.h"

#include "../dom/chord.h"
#include "../dom/masterscore.h"
#include "../dom/note.h"
#include "../dom/repeatlist.h"
#include "../dom/spanner.h"
#include "../dom/staff.h"
#include "../dom/tempotimeline.h"

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
    return obj && obj->isEngravingItem() ? static_cast<const EngravingItem*>(obj) : nullptr;
}

void addSource(std::vector<NotePlaybackSource>& sources, const NotePlaybackSource& source)
{
    if (std::ranges::any_of(sources,
                            [&source](const NotePlaybackSource& src) {
        return src.item == source.item;
    }
                            )) {
        return;
    }

    sources.push_back(source);
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
bool dynamicAppliesToNote(const EngravingItem* item, const Note* note)
{
    switch (item->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>()) {
    case VoiceAssignment::ALL_VOICE_IN_INSTRUMENT:
        return item->part() == note->part();
    case VoiceAssignment::ALL_VOICE_IN_STAFF:
        return item->staffIdx() == note->staffIdx();
    case VoiceAssignment::CURRENT_VOICE_ONLY:
        return item->staffIdx() == note->staffIdx()
               && item->voice() == note->voice();
    }

    return false;
}

//! Gradual changes (hairpins, accel./rit.) that are in progress at tick
//! The curve point before a note inside one can belong to an unrelated marking at the change's start
void addGradualChangeSources(
    const MasterScore* score,
    const Note* note,
    int tick,
    std::vector<NotePlaybackSource>& tempoSources,
    std::vector<NotePlaybackSource>& dynamicSources)
{
    const auto& intervals = score->spannerMap().findOverlapping(tick, tick);

    for (const auto& interval : intervals) {
        const Spanner* spanner = interval.value;

        if (!spanner->playSpanner() || tick < spanner->tick().ticks() || tick >= spanner->tick2().ticks()) {
            continue;
        }

        if (spanner->isHairpin()) {
            if (dynamicAppliesToNote(spanner, note)) {
                addSource(dynamicSources, { spanner, false });
            }
        } else if (spanner->isGradualTempoChange()) {
            addSource(tempoSources, { spanner, false });
        }
    }
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
} // namespace noteplaybackinfo_detail

using namespace noteplaybackinfo_detail;

DynamicType nearestOrdinaryDynamic(double normalizedValue)
{
    DynamicType result = DynamicType::OTHER;
    double bestDistance = std::numeric_limits<double>::max();

    for (const auto& [type, value] : ORDINARY_DYNAMIC_VALUES) {
        if (type == DynamicType::N) {
            continue;
        }

        const double distance = std::abs(value.raw() - normalizedValue);

        // On a tie prefer the softer one, so 50% (natural) reads as mp/mf consistently
        if ((distance < bestDistance - 1e-9)
            || ((std::abs(distance - bestDistance) <= 1e-9)
                && (value.raw() < normalizedValue)
                )
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
        }

        std::vector<NotePlaybackSource> gradualTempoSources;
        std::vector<NotePlaybackSource> gradualDynamicSources;

        addGradualChangeSources(master, note, tick, gradualTempoSources, gradualDynamicSources /* unused */);

        for (const NotePlaybackSource& src : gradualTempoSources) {
            addSource(pass.tempoSources, src);
        }

        gradualTempoSources.clear();
        gradualDynamicSources.clear();

        addGradualChangeSources(master, info.attackNote, attackTick, gradualTempoSources, gradualDynamicSources);

        for (const NotePlaybackSource& src : gradualDynamicSources) {
            addSource(pass.dynamicSources, src);
        }

        info.passes.push_back(std::move(pass));
    }

    return info;
}
} // namespace mu::engraving
