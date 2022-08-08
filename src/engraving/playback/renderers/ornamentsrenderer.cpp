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

#include "libmscore/utils.h"
#include "playback/metaparsers/notearticulationsparser.h"

using namespace mu::engraving;
using namespace mu::mpe;

static const std::unordered_map<ArticulationType, DisclosurePattern> DISCLOSURE_RULES = {
    {
        ArticulationType::Trill,
        {
            /*prefixDurationTicks*/ 0,
            /*prefixPitchOffsets*/ {},
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { 0, PITCH_LEVEL_STEP },
            /*suffixDurationTicks*/ DEMISEMIQUAVER_TICKS* 2,
            /*suffixPitchOffsets*/ { 0, PITCH_LEVEL_STEP, 0 },
            /*minSupportedNoteDurationTicks*/ DEMISEMIQUAVER_TICKS
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
            /*minSupportedNoteDurationTicks*/ DEMISEMIQUAVER_TICKS
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
            /*minSupportedNoteDurationTicks*/ DEMISEMIQUAVER_TICKS
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
            /*minSupportedNoteDurationTicks*/ DEMISEMIQUAVER_TICKS
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
            /*minSupportedNoteDurationTicks*/ SEMIQUAVER_TICKS
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
            /*minSupportedNoteDurationTicks*/ DEMISEMIQUAVER_TICKS
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
            /*minSupportedNoteDurationTicks*/ DEMISEMIQUAVER_TICKS
        }
    },
    {
        ArticulationType::UpperMordentBaroque,
        {
            /*prefixDurationTicks*/ DEMISEMIQUAVER_TICKS * 3,
            /*prefixPitchOffsets*/ { PITCH_LEVEL_STEP, 0, PITCH_LEVEL_STEP },
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
            /*prefixPitchOffsets*/ { PITCH_LEVEL_STEP, PITCH_LEVEL_STEP, PITCH_LEVEL_STEP, 0 },
            /*isAlterationsRepeatAllowed*/ true,
            /*alterationStepPitchOffsets*/ { PITCH_LEVEL_STEP, 0 },
            /*suffixDurationTicks*/ 0,
            /*suffixPitchOffsets*/ {},
            /*minSupportedNoteDurationTicks*/ SEMIQUAVER_TICKS
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
            /*minSupportedNoteDurationTicks*/ SEMIQUAVER_TICKS
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
            /*minSupportedNoteDurationTicks*/ SEMIQUAVER_TICKS
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
            /*minSupportedNoteDurationTicks*/ SEMIQUAVER_TICKS
        }
    },
    {
        ArticulationType::Turn,
        {
            /*prefixDurationTicks*/ 0,
            /*prefixPitchOffsets*/ {},
            /*isAlterationsRepeatAllowed*/ false,
            /*alterationStepPitchOffsets*/ { PITCH_LEVEL_STEP, 0, -PITCH_LEVEL_STEP, 0 },
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
            /*alterationStepPitchOffsets*/ { -PITCH_LEVEL_STEP, 0, PITCH_LEVEL_STEP, 0 },
            /*suffixDurationTicks*/ 0,
            /*suffixPitchOffsets*/ {},
            /*minSupportedNoteDurationTicks*/ 0
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
            /*minSupportedNoteDurationTicks*/ SEMIQUAVER_TICKS
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
            /*minSupportedNoteDurationTicks*/ SEMIQUAVER_TICKS
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
                                 const RenderingContext& context,
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

    const DisclosurePattern& nominalPattern = search->second;

    for (const Note* note : chord->notes()) {
        if (!isNotePlayable(note)) {
            continue;
        }

        NominalNoteCtx noteCtx(note, context);
        NoteArticulationsParser::buildNoteArticulationMap(note, noteCtx.chordCtx, noteCtx.chordCtx.commonArticulations);

        convert(preferredType, nominalPattern.buildActualPattern(note), std::move(noteCtx), result);
    }
}

void OrnamentsRenderer::convert(const ArticulationType type, const DisclosurePattern& patern, NominalNoteCtx&& noteCtx,
                                mpe::PlaybackEventList& result)
{
    if (noteCtx.chordCtx.nominalDurationTicks <= patern.minSupportedNoteDurationTicks) {
        result.push_back(buildNoteEvent(std::move(noteCtx)));
        return;
    }

    // convert prefix
    if (!patern.prefixPitchOffsets.empty()) {
        createEvents(type, noteCtx, 1, patern.prefixDurationTicks,
                     noteCtx.chordCtx.nominalDurationTicks, patern.prefixPitchOffsets, result);
    }

    // convert body
    if (!patern.alterationStepPitchOffsets.empty()) {
        int alterationsCount = 1;

        if (patern.isAlterationsRepeatAllowed) {
            alterationsCount = alterationsNumberByTempo(noteCtx.chordCtx.beatsPerSecond.val, noteCtx.chordCtx.nominalDurationTicks);
        }

        createEvents(type, noteCtx, alterationsCount,
                     noteCtx.chordCtx.nominalDurationTicks - patern.prefixDurationTicks - patern.suffixDurationTicks,
                     noteCtx.chordCtx.nominalDurationTicks,
                     patern.alterationStepPitchOffsets, result);
    }

    // convert suffix
    if (!patern.suffixPitchOffsets.empty()) {
        createEvents(type, noteCtx, 1,
                     patern.suffixDurationTicks,
                     noteCtx.chordCtx.nominalDurationTicks,
                     patern.suffixPitchOffsets, result);
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

DisclosurePattern DisclosurePattern::buildActualPattern(const Note* note) const
{
    DisclosurePattern result = *this;

    result.updatePitchOffsets(note, result.prefixPitchOffsets);
    result.updatePitchOffsets(note, result.alterationStepPitchOffsets);
    result.updatePitchOffsets(note, result.suffixPitchOffsets);

    return result;
}

void DisclosurePattern::updatePitchOffsets(const Note* note, std::vector<mpe::pitch_level_t>& pitchOffsets)
{
    for (auto& pitchOffset : pitchOffsets) {
        pitchOffset *= std::abs(chromaticPitchSteps(note, note, pitchOffset / mpe::PITCH_LEVEL_STEP));
    }
}
