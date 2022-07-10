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

#include "ornamentsrenderer.h"

#include "realfn.h"

#include "libmscore/chord.h"
#include "playback/metaparsers/notearticulationsparser.h"

using namespace mu::engraving;
using namespace mu::mpe;

static const std::unordered_map<ArticulationType, DisclosureRule> DISCLOSURE_RULES = {
    {
        ArticulationType::Trill,
        {
            /*prefixDurationTicks*/ 0,
            /*prefixPitchOffsets*/ {},
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { 0, 1 },
            /*suffixDurationTicks*/ DEMISEMIQUAVER_TICKS* 2,
            /*suffixPitchOffsets*/ { 0, 1, 0 },
            /*minSupportedNoteDurationTicks*/ DEMISEMIQUAVER_TICKS
        }
    },
    {
        ArticulationType::TrillBaroque,
        {
            /*prefixDurationTicks*/ 0,
            /*prefixPitchOffsets*/ {},
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { 1, 0 },
            /*suffixDurationTicks*/ DEMISEMIQUAVER_TICKS* 2,
            /*suffixPitchOffsets*/ { -1, 0 },
            /*minSupportedNoteDurationTicks*/ DEMISEMIQUAVER_TICKS
        }
    },
    {
        ArticulationType::LinePrall,
        {
            /*prefixDurationTicks*/ DEMISEMIQUAVER_TICKS* 3,
            /*prefixPitchOffsets*/ { 2 * 1, 2 * 1, 2 * 1 },
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { 1, 0 },
            /*suffixDurationTicks*/ DEMISEMIQUAVER_TICKS* 2,
            /*suffixPitchOffsets*/ { 1, 0 },
            /*minSupportedNoteDurationTicks*/ DEMISEMIQUAVER_TICKS
        }
    },
    {
        ArticulationType::UpPrall,
        {
            /*prefixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*prefixPitchOffsets*/ { -1, 0 },
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { 1, 0 },
            /*suffixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*suffixPitchOffsets*/ { 1, 0 },
            /*minSupportedNoteDurationTicks*/ DEMISEMIQUAVER_TICKS
        }
    },
    {
        ArticulationType::UpMordent,
        {
            /*prefixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*prefixPitchOffsets*/ { -1, 0 },
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { 1, 0 },
            /*suffixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*suffixPitchOffsets*/ { -1, 0 },
            /*minSupportedNoteDurationTicks*/ SEMIQUAVER_TICKS
        }
    },
    {
        ArticulationType::UpperMordent,
        {
            /*prefixDurationTicks*/ DEMISEMIQUAVER_TICKS* 2,
            /*prefixPitchOffsets*/ { 0, 1 },
            /*isAlterationsRepeatAllowed*/ false,
            /*alterationStepPitchOffsets*/ { 0 },
            /*suffixDurationTicks*/ 0,
            /*suffixPitchOffsets*/ {},
            /*minSupportedNoteDurationTicks*/ DEMISEMIQUAVER_TICKS
        }
    },
    {
        ArticulationType::LowerMordent,
        {
            /*prefixDurationTicks*/ DEMISEMIQUAVER_TICKS* 2,
            /*prefixPitchOffsets*/ { 0, -1 },
            /*isAlterationsRepeatAllowed*/ false,
            /*alterationStepPitchOffsets*/ { 0 },
            /*suffixDurationTicks*/ 0,
            /*suffixPitchOffsets*/ {},
            /*minSupportedNoteDurationTicks*/ DEMISEMIQUAVER_TICKS
        }
    },
    {
        ArticulationType::MordentWithUpperPrefix,
        {
            /*prefixDurationTicks*/ SEMIQUAVER_TICKS* 4,
            /*prefixPitchOffsets*/ { 1, 1, 1, 0 },
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { 1, 0 },
            /*suffixDurationTicks*/ 0,
            /*suffixPitchOffsets*/ {},
            /*minSupportedNoteDurationTicks*/ SEMIQUAVER_TICKS
        }
    },
    {
        ArticulationType::DownMordent,
        {
            /*prefixDurationTicks*/ SEMIQUAVER_TICKS* 4,
            /*prefixPitchOffsets*/ { 1, 1, 1, 0 },
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { 1, 0 },
            /*suffixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*suffixPitchOffsets*/ { -1, 0 },
            /*minSupportedNoteDurationTicks*/ SEMIQUAVER_TICKS
        }
    },
    {
        ArticulationType::PrallUp,
        {
            /*prefixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*prefixPitchOffsets*/ { 1, 0 },
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { 1, 0 },
            /*suffixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*suffixPitchOffsets*/ { -1, 0 },
            /*minSupportedNoteDurationTicks*/ SEMIQUAVER_TICKS
        }
    },
    {
        ArticulationType::PrallDown,
        {
            /*prefixDurationTicks*/ SEMIQUAVER_TICKS* 2,
            /*prefixPitchOffsets*/ { 1, 0 },
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { 1, 0 },
            /*suffixDurationTicks*/ SEMIQUAVER_TICKS* 4,
            /*suffixPitchOffsets*/ { -1, 0, 0, 0 },
            /*minSupportedNoteDurationTicks*/ SEMIQUAVER_TICKS
        }
    },
    {
        ArticulationType::Turn,
        {
            /*prefixDurationTicks*/ 0,
            /*prefixPitchOffsets*/ {},
            /*isAlterationsRepeatAllowed*/ false,
            /*alterationStepPitchOffsets*/ { 1, 0, -1, 0 },
            /*suffixDurationTicks*/ 0,
            /*suffixPitchOffsets*/ {},
            /*minSupportedNoteDurationTicks*/ 0
        }
    },
    {
        ArticulationType::InvertedTurn,
        {
            /*prefixDurationTicks*/ 0,
            /*prefixPitchOffsets*/ {},
            /*isAlterationsRepeatAllowed*/ false,
            /*alterationStepPitchOffsets*/ { -1, 0, 1, 0 },
            /*suffixDurationTicks*/ 0,
            /*suffixPitchOffsets*/ {},
            /*minSupportedNoteDurationTicks*/ 0
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
        ArticulationType::InvertedTurn
    };

    return types;
}

void OrnamentsRenderer::doRender(const EngravingItem* item, const ArticulationType preferredType,
                                 const RenderingContext& context,
                                 mpe::PlaybackEventList& result)
{
    const Chord* chord = toChord(item);

    IF_ASSERT_FAILED(chord) {
        return;
    }

    for (const Note* note : chord->notes()) {
        if (!isNotePlayable(note)) {
            continue;
        }

        NominalNoteCtx noteCtx(note, context);
        NoteArticulationsParser::buildNoteArticulationMap(note, noteCtx.chordCtx, noteCtx.chordCtx.commonArticulations);

        convert(preferredType, std::move(noteCtx), result, note);
    }
}

void OrnamentsRenderer::ornamentStep2Pitch(const Note* note, const DisclosureRule* rule, ConvertedPitch* result)
{
    // Convert prefix
    if (!rule->prefixStepOffsets.empty()) {
        for (size_t idx = 0; idx < rule->prefixStepOffsets.size(); ++idx) {
            int delta_step = articulationExcursion(note, note, rule->prefixStepOffsets.at(idx));
            result->prefixPitchOffsets.push_back(delta_step * PITCH_LEVEL_STEP);
        }
    }

    // Convert body
    if (!rule->alterationStepOffsets.empty()) {
        for (size_t idx = 0; idx < rule->alterationStepOffsets.size(); ++idx) {
            int delta_step = articulationExcursion(note, note, rule->alterationStepOffsets.at(idx));
            result->alterationStepPitchOffsets.push_back(delta_step * PITCH_LEVEL_STEP);
        }
    }

    // Convert suffix
    if (!rule->suffixStepOffsets.empty()) {
        for (size_t idx = 0; idx < rule->suffixStepOffsets.size(); ++idx) {
            int delta_step = articulationExcursion(note, note, rule->suffixStepOffsets.at(idx));
            result->suffixPitchOffsets.push_back(delta_step * PITCH_LEVEL_STEP);
        }
    }
}

void OrnamentsRenderer::convert(const ArticulationType type, NominalNoteCtx&& noteCtx, mpe::PlaybackEventList& result, const Note* note)
{
    auto search = DISCLOSURE_RULES.find(type);
    if (search == DISCLOSURE_RULES.end()) {
        return;
    }

    ConvertedPitch convertedPitch{};
    ornamentStep2Pitch(note, &search->second, &convertedPitch);

    if (noteCtx.chordCtx.nominalDurationTicks <= search->second.minSupportedNoteDurationTicks) {
        result.push_back(buildNoteEvent(std::move(noteCtx)));
        return;
    }

    // convert prefix
    if (!convertedPitch.prefixPitchOffsets.empty()) {
        createEvents(type, noteCtx, 1, search->second.prefixDurationTicks,
                     noteCtx.chordCtx.nominalDurationTicks, convertedPitch.prefixPitchOffsets, result);
    }

    // convert body
    if (!convertedPitch.alterationStepPitchOffsets.empty()) {
        int alterationsCount = 1;

        if (search->second.isAlterationsRepeatAllowed) {
            alterationsCount = alterationsNumberByTempo(noteCtx.chordCtx.beatsPerSecond.val, noteCtx.chordCtx.nominalDurationTicks);
        }

        createEvents(type, noteCtx, alterationsCount,
                     noteCtx.chordCtx.nominalDurationTicks - search->second.prefixDurationTicks - search->second.suffixDurationTicks,
                     noteCtx.chordCtx.nominalDurationTicks,
                     convertedPitch.alterationStepPitchOffsets, result);
    }

    // convert suffix
    if (!convertedPitch.suffixPitchOffsets.empty()) {
        createEvents(type, noteCtx, 1,
                     search->second.suffixDurationTicks,
                     noteCtx.chordCtx.nominalDurationTicks,
                     convertedPitch.suffixPitchOffsets, result);
    }
}

int OrnamentsRenderer::alterationsNumberByTempo(const double beatsPerSeconds, const int principalNoteDurationTicks)
{
    float ratio = static_cast<float>(principalNoteDurationTicks) / static_cast<float>(CROTCHET_TICKS);

    if (RealIsEqualOrMore(beatsPerSeconds, PRESTISSIMO_BPS_BOUND)) {
        return 0 * ratio;
    }

    if (RealIsEqualOrMore(beatsPerSeconds, PRESTO_BPS_BOUND)) {
        return 1 * ratio;
    }

    if (RealIsEqualOrMore(beatsPerSeconds, MODERATO_BPS_BOUND)) {
        return 2 * ratio;
    }

    return 3 * ratio;
}

void OrnamentsRenderer::createEvents(const ArticulationType type, NominalNoteCtx& noteCtx, const int alterationsCount,
                                     const int availableDurationTicks, const int overallDurationTicks,
                                     const std::vector<mpe::pitch_level_t>& pitchOffsets, mpe::PlaybackEventList& result)
{
    float availableDurationRatio = availableDurationTicks / static_cast<float>(overallDurationTicks);

    size_t totalNotesCount = alterationsCount * pitchOffsets.size();
    float durationStep = (noteCtx.duration * availableDurationRatio) / totalNotesCount;

    for (int alterationStep = 0; alterationStep < alterationsCount; ++alterationStep) {
        for (size_t alterationSubNoteIdx = 0; alterationSubNoteIdx < pitchOffsets.size(); ++alterationSubNoteIdx) {
            NominalNoteCtx subNoteCtx(noteCtx);
            subNoteCtx.duration = durationStep;
            subNoteCtx.pitchLevel += pitchOffsets.at(alterationSubNoteIdx);

            updateArticulationBoundaries(type, subNoteCtx.timestamp,
                                         subNoteCtx.duration, subNoteCtx.chordCtx.commonArticulations);

            result.emplace_back(buildNoteEvent(std::move(subNoteCtx)));

            noteCtx.timestamp += durationStep;
        }
    }
}
