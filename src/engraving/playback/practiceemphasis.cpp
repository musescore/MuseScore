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

#include "practiceemphasis.h"

#include "practiceemphasisdata.h"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

#include "dom/masterscore.h"
#include "dom/note.h"
#include "dom/score.h"

#include "mpe/mpetypes.h"

#include "renderingcontext.h"
#include "utils/expressionutils.h"

using namespace mu::engraving;
using namespace muse::mpe;

namespace {
muse::real_t scaleNormalized(muse::real_t normalized, float gainDb)
{
    const dynamic_level_t level = dynamicLevelFromNormalized(normalized);
    const dynamic_level_t scaled = PracticeEmphasis::scaleDynamicLevel(level, gainDb);
    return muse::real_t(static_cast<double>(scaled) / static_cast<double>(MAX_DYNAMIC_LEVEL));
}

void scaleCurve(DynamicAutomationMap& curve, float gainDb)
{
    if (gainDb == 0.f || curve.empty()) {
        return;
    }

    for (auto it = curve.begin(); it != curve.end(); ++it) {
        muse::mpe::AutomationPoint& point = it->second;
        point.outValue = scaleNormalized(point.outValue, gainDb);
        if (auto* arrival = std::get_if<muse::mpe::AutomationPoint::ExplicitArrival>(&point.inValue)) {
            arrival->value = scaleNormalized(arrival->value, gainDb);
        }
    }
}

DynamicAutomationMap constantCurve(dynamic_level_t level)
{
    muse::mpe::AutomationPoint point;
    point.outValue = muse::real_t(static_cast<double>(level) / static_cast<double>(MAX_DYNAMIC_LEVEL));

    DynamicAutomationMap curve;
    curve[timestamp_t(0)] = point;
    return curve;
}

bool partOwnsStaff(const DynamicAutomationLayers& layers, int staffIndex)
{
    for (auto it = layers.cbegin(); it != layers.cend(); ++it) {
        if (static_cast<int>(it->first / MAX_VOICES) == staffIndex) {
            return true;
        }
    }
    return false;
}
}

dynamic_level_t PracticeEmphasis::scaleDynamicLevel(dynamic_level_t level, float gainDb)
{
    if (gainDb == 0.f) {
        return level;
    }

    const float steps = gainDb / DB_PER_DYNAMIC_STEP;
    const float scaled = static_cast<float>(level) + steps * static_cast<float>(DYNAMIC_LEVEL_STEP);
    const float clamped = std::clamp(scaled,
                                     static_cast<float>(MIN_DYNAMIC_LEVEL),
                                     static_cast<float>(MAX_DYNAMIC_LEVEL));
    return static_cast<dynamic_level_t>(std::lround(clamped));
}

void PracticeEmphasis::applyToNote(const Note* note, NominalNoteCtx& ctx)
{
    if (!note) {
        return;
    }

    const Score* score = note->score();
    const MasterScore* master = score ? score->masterScore() : nullptr;
    if (!master) {
        return;
    }
    const PracticeEmphasisData& data = master->practiceEmphasis();
    if (!data.playbackEngaged()) {
        return;
    }

    const bool emphasized = data.isEmphasized(note);
    const float gain = emphasized ? data.document().emphasizedDb : data.otherGainDb();
    ctx.dynamicLevel = scaleDynamicLevel(ctx.dynamicLevel, gain);
    if (emphasized) {
        ctx.staffIdx = static_cast<staff_idx_t>(ctx.staffIdx + EMPHASIS_STAFF_OFFSET);
    }
}

void PracticeEmphasis::applyToDynamics(const Score* score, DynamicAutomationLayers& layers)
{
    if (!score || layers.empty()) {
        return;
    }
    const MasterScore* master = score->masterScore();
    if (!master) {
        return;
    }
    const PracticeEmphasisData& data = master->practiceEmphasis();
    if (!data.playbackEngaged()) {
        return;
    }

    std::vector<std::pair<layer_idx_t, DynamicAutomationMap> > companions;
    for (const std::pair<int, int>& voice : data.emphasizedVoices(score)) {
        if (!partOwnsStaff(layers, voice.first)) {
            continue;
        }
        const layer_idx_t sourceKey = makeLayerIdx(static_cast<staff_layer_idx_t>(voice.first),
                                                   static_cast<voice_layer_idx_t>(voice.second));
        auto source = layers.find(sourceKey);
        DynamicAutomationMap curve = source != layers.end() ? source->second : constantCurve(NATURAL_DYNAMIC_LEVEL);
        scaleCurve(curve, data.document().emphasizedDb);
        const layer_idx_t companionKey = makeLayerIdx(static_cast<staff_layer_idx_t>(voice.first + EMPHASIS_STAFF_OFFSET),
                                                      static_cast<voice_layer_idx_t>(voice.second));
        companions.emplace_back(companionKey, std::move(curve));
    }

    for (auto it = layers.begin(); it != layers.end(); ++it) {
        if (static_cast<int>(it->first / MAX_VOICES) >= EMPHASIS_STAFF_OFFSET) {
            continue;
        }
        scaleCurve(it->second, data.otherGainDb());
    }
    for (auto& companion : companions) {
        layers[companion.first] = std::move(companion.second);
    }
}
