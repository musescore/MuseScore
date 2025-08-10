/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "ornamentsrenderer.h"

#include "realfn.h"

#include "dom/ornament.h"
#include "dom/utils.h"

#include "playback/metaparsers/notearticulationsparser.h"
#include "playback/utils/expressionutils.h"

#include "noterenderer.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

namespace mu::engraving {
struct IntervalsInfo {
    bool intervalBelowIsAuto = false;
    bool intervalAboveIsAuto = false;

    Interval intervalBelow;
    Interval intervalAbove;
};

struct DisclosurePattern {
    int prefixDurationTicks = 0;
    std::vector<mpe::pitch_level_t> prefixPitchOffsets;

    bool isAlterationsRepeatAllowed = false;
    std::vector<mpe::pitch_level_t> alterationStepPitchOffsets;

    int suffixDurationTicks = 0;
    std::vector<mpe::pitch_level_t> suffixPitchOffsets;

    struct DurationBoundaries {
        float lowTempoDurationTicks = 0.f;
        float mediumTempoDurationTicks = 0.f;
        float highTempoDurationTicks = 0.f;
    };

    DurationBoundaries boundaries;

    float subNoteDurationTicks(const double bps) const;
    DisclosurePattern buildActualPattern(const Note* note, const IntervalsInfo& intervalsInfo, const double bps) const;

private:
    void updatePitchOffsets(const Note* note, const IntervalsInfo& intervalsInfo, std::vector<mpe::pitch_level_t>& pitchOffsets);
};

static IntervalsInfo makeIntervalsInfo(const OrnamentInterval& below, const OrnamentInterval& above)
{
    IntervalsInfo result;
    result.intervalBelowIsAuto = below.type == IntervalType::AUTO;
    result.intervalAboveIsAuto = above.type == IntervalType::AUTO;
    result.intervalBelow = Interval::fromOrnamentInterval(below);
    result.intervalAbove = Interval::fromOrnamentInterval(above);

    return result;
}
}

static const std::unordered_map<ArticulationType, DisclosurePattern> DISCLOSURE_RULES = {
    {
        ArticulationType::Trill,
        {
            /*prefixDurationTicks*/ 0,
            /*prefixPitchOffsets*/ {},
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { 0, PITCH_LEVEL_STEP },
            /*suffixDurationTicks*/ 0,
            /*suffixPitchOffsets*/ {},
            /*minSupportedNoteDurationTicks*/ { QUAVER_TICKS / 10.f, DEMISEMIQUAVER_TICKS,
                                                SEMIQUAVER_TICKS }
        }
    },
    {
        ArticulationType::TrillBaroque,
        {
            /*prefixDurationTicks*/ 0,
            /*prefixPitchOffsets*/ {},
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { PITCH_LEVEL_STEP, 0 },
            /*suffixDurationTicks*/ DEMISEMIQUAVER_TICKS* 2,
            /*suffixPitchOffsets*/ { -PITCH_LEVEL_STEP, 0 },
            /*minSupportedNoteDurationTicks*/ { QUAVER_TICKS / 10.f, DEMISEMIQUAVER_TICKS,
                                                SEMIQUAVER_TICKS }
        }
    },
    {
        ArticulationType::LinePrall,
        {
            /*prefixDurationTicks*/ DEMISEMIQUAVER_TICKS* 3,
            /*prefixPitchOffsets*/ { 2 * PITCH_LEVEL_STEP, 2 * PITCH_LEVEL_STEP, 2 * PITCH_LEVEL_STEP },
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { PITCH_LEVEL_STEP, 0 },
            /*suffixDurationTicks*/ DEMISEMIQUAVER_TICKS* 2,
            /*suffixPitchOffsets*/ { PITCH_LEVEL_STEP, 0 },
            /*minSupportedNoteDurationTicks*/ { DEMISEMIQUAVER_TICKS, DEMISEMIQUAVER_TICKS, DEMISEMIQUAVER_TICKS }
        }
    },
    {
        ArticulationType::UpPrall,
        {
            /*prefixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*prefixPitchOffsets*/ { -PITCH_LEVEL_STEP, 0 },
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { PITCH_LEVEL_STEP, 0 },
            /*suffixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*suffixPitchOffsets*/ { PITCH_LEVEL_STEP, 0 },
            /*minSupportedNoteDurationTicks*/ { DEMISEMIQUAVER_TICKS, DEMISEMIQUAVER_TICKS, DEMISEMIQUAVER_TICKS }
        }
    },
    {
        ArticulationType::UpMordent,
        {
            /*prefixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*prefixPitchOffsets*/ { -PITCH_LEVEL_STEP, 0 },
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { PITCH_LEVEL_STEP, 0 },
            /*suffixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*suffixPitchOffsets*/ { -PITCH_LEVEL_STEP, 0 },
            /*minSupportedNoteDurationTicks*/ { SEMIQUAVER_TICKS, SEMIQUAVER_TICKS, SEMIQUAVER_TICKS }
        }
    },
    {
        ArticulationType::UpperMordent,
        {
            /*prefixDurationTicks*/ DEMISEMIQUAVER_TICKS* 2,
            /*prefixPitchOffsets*/ { 0, PITCH_LEVEL_STEP },
            /*isAlterationsRepeatAllowed*/ false,
            /*alterationStepPitchOffsets*/ { 0 },
            /*suffixDurationTicks*/ 0,
            /*suffixPitchOffsets*/ {},
            /*minSupportedNoteDurationTicks*/ { DEMISEMIQUAVER_TICKS / 2.f, DEMISEMIQUAVER_TICKS, SEMIQUAVER_TICKS }
        }
    },
    {
        ArticulationType::LowerMordent,
        {
            /*prefixDurationTicks*/ DEMISEMIQUAVER_TICKS* 2,
            /*prefixPitchOffsets*/ { 0, -PITCH_LEVEL_STEP },
            /*isAlterationsRepeatAllowed*/ false,
            /*alterationStepPitchOffsets*/ { 0 },
            /*suffixDurationTicks*/ 0,
            /*suffixPitchOffsets*/ {},
            /*minSupportedNoteDurationTicks*/ { DEMISEMIQUAVER_TICKS / 2.f, DEMISEMIQUAVER_TICKS, SEMIQUAVER_TICKS }
        }
    },
    {
        ArticulationType::UpperMordentBaroque,
        {
            /*prefixDurationTicks*/ DEMISEMIQUAVER_TICKS* 3,
            /*prefixPitchOffsets*/ { PITCH_LEVEL_STEP, 0, PITCH_LEVEL_STEP },
            /*isAlterationsRepeatAllowed*/ false,
            /*alterationStepPitchOffsets*/ { 0 },
            /*suffixDurationTicks*/ 0,
            /*suffixPitchOffsets*/ {},
            /*minSupportedNoteDurationTicks*/ { DEMISEMIQUAVER_TICKS / 2.f, DEMISEMIQUAVER_TICKS, SEMIQUAVER_TICKS }
        }
    },
    {
        ArticulationType::MordentWithUpperPrefix,
        {
            /*prefixDurationTicks*/ SEMIQUAVER_TICKS* 4,
            /*prefixPitchOffsets*/ { PITCH_LEVEL_STEP, PITCH_LEVEL_STEP, PITCH_LEVEL_STEP, 0 },
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { PITCH_LEVEL_STEP, 0 },
            /*suffixDurationTicks*/ 0,
            /*suffixPitchOffsets*/ {},
            /*minSupportedNoteDurationTicks*/ { SEMIQUAVER_TICKS, SEMIQUAVER_TICKS, SEMIQUAVER_TICKS }
        }
    },
    {
        ArticulationType::DownMordent,
        {
            /*prefixDurationTicks*/ SEMIQUAVER_TICKS* 4,
            /*prefixPitchOffsets*/ { PITCH_LEVEL_STEP, PITCH_LEVEL_STEP, PITCH_LEVEL_STEP, 0 },
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { PITCH_LEVEL_STEP, 0 },
            /*suffixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*suffixPitchOffsets*/ { -PITCH_LEVEL_STEP, 0 },
            /*minSupportedNoteDurationTicks*/ { SEMIQUAVER_TICKS, SEMIQUAVER_TICKS, SEMIQUAVER_TICKS }
        }
    },
    {
        ArticulationType::PrallUp,
        {
            /*prefixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*prefixPitchOffsets*/ { PITCH_LEVEL_STEP, 0 },
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { PITCH_LEVEL_STEP, 0 },
            /*suffixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*suffixPitchOffsets*/ { -PITCH_LEVEL_STEP, 0 },
            /*minSupportedNoteDurationTicks*/ { SEMIQUAVER_TICKS, SEMIQUAVER_TICKS, SEMIQUAVER_TICKS }
        }
    },
    {
        ArticulationType::PrallDown,
        {
            /*prefixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*prefixPitchOffsets*/ { PITCH_LEVEL_STEP, 0 },
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { PITCH_LEVEL_STEP, 0 },
            /*suffixDurationTicks*/ SEMIQUAVER_TICKS* 4,
            /*suffixPitchOffsets*/ { -PITCH_LEVEL_STEP, 0, 0, 0 },
            /*minSupportedNoteDurationTicks*/ { SEMIQUAVER_TICKS, SEMIQUAVER_TICKS, SEMIQUAVER_TICKS }
        }
    },
    {
        ArticulationType::Turn,
        {
            /*prefixDurationTicks*/ DEMISEMIQUAVER_TICKS* 3,
            /*prefixPitchOffsets*/ { PITCH_LEVEL_STEP, 0, -PITCH_LEVEL_STEP },
            /*isAlterationsRepeatAllowed*/ false,
            /*alterationStepPitchOffsets*/ { 0 },
            /*suffixDurationTicks*/ 0,
            /*suffixPitchOffsets*/ {},
            /*minSupportedNoteDurationTicks*/ { DEMISEMIQUAVER_TICKS / 2.f, DEMISEMIQUAVER_TICKS, SEMIQUAVER_TICKS }
        }
    },
    {
        ArticulationType::InvertedTurn,
        {
            /*prefixDurationTicks*/ DEMISEMIQUAVER_TICKS* 3,
            /*prefixPitchOffsets*/ { -PITCH_LEVEL_STEP, 0, PITCH_LEVEL_STEP },
            /*isAlterationsRepeatAllowed*/ false,
            /*alterationStepPitchOffsets*/ { 0 },
            /*suffixDurationTicks*/ 0,
            /*suffixPitchOffsets*/ {},
            /*minSupportedNoteDurationTicks*/ { DEMISEMIQUAVER_TICKS / 2.f, DEMISEMIQUAVER_TICKS, SEMIQUAVER_TICKS }
        }
    },
    {
        ArticulationType::Tremblement,
        {
            /*prefixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*prefixPitchOffsets*/ { PITCH_LEVEL_STEP, 0 },
            /*isAlterationsRepeatAllowed*/ false,
            /*alterationStepPitchOffsets*/ { PITCH_LEVEL_STEP, 0 },
            /*suffixDurationTicks*/ 0,
            /*suffixPitchOffsets*/ {},
            /*minSupportedNoteDurationTicks*/ { SEMIQUAVER_TICKS, SEMIQUAVER_TICKS, SEMIQUAVER_TICKS }
        }
    },
    {
        ArticulationType::PrallMordent,
        {
            /*prefixDurationTicks*/ 0,
            /*prefixPitchOffsets*/ {},
            /*isAlterationsRepeatAllowed*/ false,
            /*alterationStepPitchOffsets*/ { PITCH_LEVEL_STEP, 0, -PITCH_LEVEL_STEP, 0 },
            /*suffixDurationTicks*/ 0,
            /*suffixPitchOffsets*/ {},
            /*minSupportedNoteDurationTicks*/ { SEMIQUAVER_TICKS, SEMIQUAVER_TICKS, SEMIQUAVER_TICKS }
        }
    },
};

const ArticulationTypeSet& OrnamentsRenderer::supportedTypes()
{
    static const ArticulationTypeSet types = {
        ArticulationType::Trill, ArticulationType::TrillBaroque,
        ArticulationType::LinePrall, ArticulationType::UpPrall,
        ArticulationType::UpMordent, ArticulationType::UpperMordent,
        ArticulationType::LowerMordent, ArticulationType::MordentWithUpperPrefix,
        ArticulationType::DownMordent, ArticulationType::PrallUp,
        ArticulationType::PrallDown, ArticulationType::Turn,
        ArticulationType::InvertedTurn, ArticulationType::Tremblement,
        ArticulationType::PrallMordent, ArticulationType::UpperMordentBaroque,
        ArticulationType::LowerMordentBaroque
    };

    return types;
}

void OrnamentsRenderer::doRender(const EngravingItem* item, const ArticulationType preferredType,
                                 const RenderingContext& ctx,
                                 mpe::PlaybackEventList& result)
{
    const Chord* chord = toChord(item);
    IF_ASSERT_FAILED(chord) {
        return;
    }

    auto search = DISCLOSURE_RULES.find(preferredType);
    if (search == DISCLOSURE_RULES.end()) {
        return;
    }

    IntervalsInfo intervalsInfo;
    bool isArticulation = false;

    if (Ornament* ornament = chord->findOrnament(true)) {
        intervalsInfo = makeIntervalsInfo(ornament->intervalBelow(), ornament->intervalAbove());
        isArticulation = muse::contains(chord->articulations(), static_cast<Articulation*>(ornament));
    } else {
        intervalsInfo = makeIntervalsInfo(DEFAULT_ORNAMENT_INTERVAL, DEFAULT_ORNAMENT_INTERVAL);
    }

    const DisclosurePattern& nominalPattern = search->second;

    for (const Note* note : chord->notes()) {
        if (!NoteRenderer::shouldRender(note, ctx, ctx.commonArticulations)) {
            continue;
        }

        RenderingContext ornamentCtx(ctx);
        if (isArticulation && note->tieFor()) {
            applyTiedNotesDuration(note, preferredType, ornamentCtx);
        }

        NominalNoteCtx noteCtx(note, ornamentCtx);
        NoteArticulationsParser::buildNoteArticulationMap(note, noteCtx.chordCtx, noteCtx.articulations);

        convert(preferredType, nominalPattern.buildActualPattern(note, intervalsInfo, ornamentCtx.beatsPerSecond.val),
                std::move(noteCtx), result);
    }
}

void OrnamentsRenderer::applyTiedNotesDuration(const Note* note, const ArticulationType ornamentType, RenderingContext& ctx)
{
    const Note* lastTiedNote = note->lastTiedNote(false);
    if (!lastTiedNote || lastTiedNote == note) {
        return;
    }

    ctx.nominalPositionEndTick = lastTiedNote->chord()->endTick().ticks();
    ctx.nominalDuration = timestampFromTicks(ctx.score, ctx.nominalPositionEndTick + ctx.positionTickOffset) - ctx.nominalTimestamp;
    ctx.nominalDurationTicks = ctx.nominalPositionEndTick - ctx.nominalPositionStartTick;

    mpe::ArticulationMeta& meta = ctx.commonArticulations.at(ornamentType).meta;
    meta.overallDuration = ctx.nominalDuration;
}

void OrnamentsRenderer::convert(const ArticulationType type, const DisclosurePattern& pattern, NominalNoteCtx&& noteCtx,
                                mpe::PlaybackEventList& result)
{
    if (noteCtx.chordCtx.nominalDurationTicks <= pattern.boundaries.lowTempoDurationTicks) {
        result.emplace_back(buildNoteEvent(std::move(noteCtx)));
        return;
    }

    // convert prefix
    if (!pattern.prefixPitchOffsets.empty()) {
        createEvents(type, noteCtx, 1, pattern.prefixDurationTicks,
                     noteCtx.chordCtx.nominalDurationTicks, pattern.prefixPitchOffsets, result);
    }

    // convert body
    if (!pattern.alterationStepPitchOffsets.empty()) {
        int alterationsCount = 1;

        if (pattern.isAlterationsRepeatAllowed) {
            alterationsCount = alterationsNumberByTempo(noteCtx.chordCtx.beatsPerSecond.val,
                                                        noteCtx.chordCtx.nominalDurationTicks,
                                                        pattern);
        }

        if (alterationsCount == 0) {
            result.emplace_back(buildNoteEvent(std::move(noteCtx)));
        } else {
            createEvents(type, noteCtx, alterationsCount,
                         noteCtx.chordCtx.nominalDurationTicks - pattern.prefixDurationTicks - pattern.suffixDurationTicks,
                         noteCtx.chordCtx.nominalDurationTicks,
                         pattern.alterationStepPitchOffsets, result);
        }
    }

    // convert suffix
    if (!pattern.suffixPitchOffsets.empty()) {
        createEvents(type, noteCtx, 1,
                     pattern.suffixDurationTicks,
                     noteCtx.chordCtx.nominalDurationTicks,
                     pattern.suffixPitchOffsets, result);
    }
}

int OrnamentsRenderer::alterationsNumberByTempo(const double beatsPerSeconds, const int principalNoteDurationTicks,
                                                const DisclosurePattern& pattern)
{
    float subNotesCount = principalNoteDurationTicks / pattern.subNoteDurationTicks(beatsPerSeconds);

    return static_cast<int>(std::max(subNotesCount / 2, 0.f));
}

void OrnamentsRenderer::createEvents(const ArticulationType type, NominalNoteCtx& noteCtx, const int alterationsCount,
                                     const int availableDurationTicks, const int overallDurationTicks,
                                     const std::vector<mpe::pitch_level_t>& pitchOffsets, mpe::PlaybackEventList& result)
{
    float availableDurationRatio = availableDurationTicks / static_cast<float>(overallDurationTicks);

    size_t totalNotesCount = alterationsCount * pitchOffsets.size();
    float durationStep = (noteCtx.duration * availableDurationRatio) / totalNotesCount;

    track_idx_t trackIdx = staff2track(noteCtx.staffIdx, noteCtx.voiceIdx);

    for (int alterationStep = 0; alterationStep < alterationsCount; ++alterationStep) {
        for (size_t alterationSubNoteIdx = 0; alterationSubNoteIdx < pitchOffsets.size(); ++alterationSubNoteIdx) {
            NominalNoteCtx subNoteCtx(noteCtx);
            subNoteCtx.duration = durationStep;
            subNoteCtx.pitchLevel += pitchOffsets.at(alterationSubNoteIdx);

            int utick = timestampToTick(subNoteCtx.chordCtx.score, subNoteCtx.timestamp);
            subNoteCtx.dynamicLevel = noteCtx.chordCtx.playbackCtx->appliableDynamicLevel(trackIdx, utick);

            updateArticulationBoundaries(type, subNoteCtx.timestamp,
                                         subNoteCtx.duration, subNoteCtx.articulations);

            result.emplace_back(buildNoteEvent(std::move(subNoteCtx)));

            noteCtx.timestamp += durationStep;
        }
    }
}

float DisclosurePattern::subNoteDurationTicks(const double bps) const
{
    if (muse::RealIsEqualOrMore(bps, PRESTISSIMO_BPS_BOUND)) {
        return boundaries.highTempoDurationTicks;
    }

    if (muse::RealIsEqualOrMore(bps, MODERATO_BPS_BOUND)) {
        return boundaries.mediumTempoDurationTicks;
    }

    return boundaries.lowTempoDurationTicks;
}

DisclosurePattern DisclosurePattern::buildActualPattern(const Note* note, const IntervalsInfo& intervalsInfo, const double bps) const
{
    DisclosurePattern result = *this;

    result.updatePitchOffsets(note, intervalsInfo, result.prefixPitchOffsets);
    result.updatePitchOffsets(note, intervalsInfo, result.alterationStepPitchOffsets);
    result.updatePitchOffsets(note, intervalsInfo, result.suffixPitchOffsets);

    float subNoteTicks = subNoteDurationTicks(bps);

    result.prefixDurationTicks = muse::RealRound(prefixPitchOffsets.size() * subNoteTicks, 0);
    result.suffixDurationTicks = muse::RealRound(suffixPitchOffsets.size() * subNoteTicks, 0);

    return result;
}

void DisclosurePattern::updatePitchOffsets(const Note* note, const IntervalsInfo& intervalsInfo,
                                           std::vector<mpe::pitch_level_t>& pitchOffsets)
{
    for (pitch_level_t& pitchOffset : pitchOffsets) {
        if (pitchOffset == 0) {
            continue;
        }

        semitone_t semitones = 0;

        if (pitchOffset < 0) {
            if (intervalsInfo.intervalBelowIsAuto) {
                semitones = std::abs(chromaticPitchSteps(note, note, -intervalsInfo.intervalBelow.diatonic));
            } else {
                semitones = intervalsInfo.intervalBelow.chromatic;
            }
        } else if (pitchOffset > 0) {
            if (intervalsInfo.intervalAboveIsAuto) {
                semitones = std::abs(chromaticPitchSteps(note, note, intervalsInfo.intervalAbove.diatonic));
            } else {
                semitones = intervalsInfo.intervalAbove.chromatic;
            }
        }

        pitchOffset *= semitones;
    }
}
