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

#include "notationautomationdisplaymapping.h"

#include <algorithm>

#include "global/realfn.h"
#include "translation.h"

#include "engraving/automation/dynamicvalues.h"
#include "engraving/automation/tempovalues.h"

using namespace mu::notation;
using namespace mu::engraving;

namespace mu::notation {
struct DynamicsDisplayRange {
    muse::real_t minSounding = 0.0;
    muse::real_t maxSounding = 0.0;
    double fadeOutHeight = 0.0;
};
}

// Only the levels a sound source plays differently get their own height on the lane
static const DynamicsDisplayRange MUSE_SAMPLER_DYNAMICS_RANGE {
    mu::engraving::ORDINARY_DYNAMIC_VALUES.at(mu::engraving::DynamicType::PPPPP),
    mu::engraving::ORDINARY_DYNAMIC_VALUES.at(mu::engraving::DynamicType::FFF),
    0.01
};
static const DynamicsDisplayRange DEFAULT_DYNAMICS_RANGE {
    mu::engraving::ORDINARY_DYNAMIC_VALUES.at(mu::engraving::DynamicType::PPP),
    mu::engraving::ORDINARY_DYNAMIC_VALUES.at(mu::engraving::DynamicType::FFF)
};

static double dynamicsValueToDisplay(muse::real_t value, const DynamicsDisplayRange& range)
{
    if (value <= range.minSounding) {
        if (muse::RealIsNull(range.fadeOutHeight)) {
            return 0.0;
        }
        return std::max(muse::real_t(0.0), value / range.minSounding * range.fadeOutHeight);
    }

    const double t = (value - range.minSounding) / (range.maxSounding - range.minSounding);
    return std::min(1.0, range.fadeOutHeight + t * (1.0 - range.fadeOutHeight));
}

static muse::real_t dynamicsValueFromDisplay(double displayValue, const DynamicsDisplayRange& range)
{
    if (displayValue <= range.fadeOutHeight) {
        if (muse::RealIsNull(range.fadeOutHeight)) {
            return range.minSounding;
        }
        return muse::real_t(std::max(0.0, displayValue) / range.fadeOutHeight * range.minSounding);
    }

    const double t = (displayValue - range.fadeOutHeight) / (1.0 - range.fadeOutHeight);
    return muse::real_t(range.minSounding + t * (range.maxSounding - range.minSounding));
}

// Must match muse::audio::VOLUME_DB_MIN/MAX
static constexpr double VOLUME_RANGE_MIN_DB = -60.0;
static constexpr double VOLUME_RANGE_MAX_DB = 12.0;

// Mirrors VolumeSlider.qml's fader curve, so dragging a point feels like moving the mixer fader -
// a plain linear map would put 0dB at 83% up the lane instead of 75%
static constexpr double VOLUME_LOCAL_CENTER_DB = -24.0;
static constexpr double VOLUME_LOGICAL_CENTER_DB = -12.0;
static constexpr double VOLUME_HIGH_ACCURACY_STEP = 1.5;
static constexpr double VOLUME_LOW_ACCURACY_STEP = 0.75;

// logical (actual) dB -> local (linear-in-display) dB
static double volumeLogicalDbToLocalDb(double logicalDb)
{
    if (logicalDb > VOLUME_LOGICAL_CENTER_DB) {
        const double diff = VOLUME_RANGE_MAX_DB - logicalDb;
        return VOLUME_RANGE_MAX_DB - diff * VOLUME_HIGH_ACCURACY_STEP;
    }

    const double diff = VOLUME_LOGICAL_CENTER_DB - logicalDb;
    return VOLUME_LOCAL_CENTER_DB - diff * VOLUME_LOW_ACCURACY_STEP;
}

// local (linear-in-display) dB -> logical (actual) dB
static double volumeLocalDbToLogicalDb(double localDb)
{
    if (localDb > VOLUME_LOCAL_CENTER_DB) {
        const double diff = VOLUME_RANGE_MAX_DB - localDb;
        return VOLUME_RANGE_MAX_DB - diff / VOLUME_HIGH_ACCURACY_STEP;
    }

    const double diff = VOLUME_LOCAL_CENTER_DB - localDb;
    return VOLUME_LOGICAL_CENTER_DB - diff / VOLUME_LOW_ACCURACY_STEP;
}

static double volumeValueToDisplay(muse::real_t value)
{
    const double logicalDb = VOLUME_RANGE_MIN_DB + static_cast<double>(value) * (VOLUME_RANGE_MAX_DB - VOLUME_RANGE_MIN_DB);
    const double localDb = volumeLogicalDbToLocalDb(logicalDb);
    const double display = (localDb - VOLUME_RANGE_MIN_DB) / (VOLUME_RANGE_MAX_DB - VOLUME_RANGE_MIN_DB);
    return std::clamp(display, 0.0, 1.0);
}

// Mirrors the volume fader curve above, anchored on the default tempo instead of 0dB - a plain
// linear map would put the default tempo (120bpm) at ~12% up the lane instead of the center
// This is a UI-only display cap - the score model still allows tempos up to Constants::MAX_TEMPO
static constexpr double TEMPO_RANGE_MAX_BPM = 300.0;
static const double TEMPO_LOGICAL_CENTER_BPM = mu::engraving::Constants::DEFAULT_TEMPO.toBPM().val;
static const double TEMPO_LOCAL_CENTER_BPM = TEMPO_RANGE_MAX_BPM / 2.0;
static const double TEMPO_HIGH_ACCURACY_STEP = (TEMPO_RANGE_MAX_BPM - TEMPO_LOCAL_CENTER_BPM)
                                               / (TEMPO_RANGE_MAX_BPM - TEMPO_LOGICAL_CENTER_BPM);
static const double TEMPO_LOW_ACCURACY_STEP = TEMPO_LOCAL_CENTER_BPM / TEMPO_LOGICAL_CENTER_BPM;

// logical (actual) bpm -> local (linear-in-display) bpm
static double tempoLogicalBpmToLocalBpm(double logicalBpm)
{
    if (logicalBpm > TEMPO_LOGICAL_CENTER_BPM) {
        const double diff = TEMPO_RANGE_MAX_BPM - logicalBpm;
        return TEMPO_RANGE_MAX_BPM - diff * TEMPO_HIGH_ACCURACY_STEP;
    }

    const double diff = TEMPO_LOGICAL_CENTER_BPM - logicalBpm;
    return TEMPO_LOCAL_CENTER_BPM - diff * TEMPO_LOW_ACCURACY_STEP;
}

// local (linear-in-display) bpm -> logical (actual) bpm
static double tempoLocalBpmToLogicalBpm(double localBpm)
{
    if (localBpm > TEMPO_LOCAL_CENTER_BPM) {
        const double diff = TEMPO_RANGE_MAX_BPM - localBpm;
        return TEMPO_RANGE_MAX_BPM - diff / TEMPO_HIGH_ACCURACY_STEP;
    }

    const double diff = TEMPO_LOCAL_CENTER_BPM - localBpm;
    return TEMPO_LOGICAL_CENTER_BPM - diff / TEMPO_LOW_ACCURACY_STEP;
}

qreal NotationAutomationDisplayMapping::defaultDisplayValue(AutomationType type)
{
    switch (type) {
    case AutomationType::Dynamics:
    case AutomationType::Tempo:
        return 0.0;
    case AutomationType::Volume: {
        const muse::real_t zeroDb((0.0 - VOLUME_RANGE_MIN_DB) / (VOLUME_RANGE_MAX_DB - VOLUME_RANGE_MIN_DB));
        return volumeValueToDisplay(zeroDb);
    }
    case AutomationType::Pan:
        return 0.5;
    case AutomationType::Unknown:
        break;
    }

    return 0.0;
}

static const DynamicsDisplayRange& dynamicsDisplayRange(bool isMuseSampler)
{
    return isMuseSampler ? MUSE_SAMPLER_DYNAMICS_RANGE : DEFAULT_DYNAMICS_RANGE;
}

// Values are stored normalized [0, 1]; Dynamics rescales that into its own sub-range for display,
// Volume and Tempo additionally apply a fader curve, other types map 1:1 onto the display range
double NotationAutomationDisplayMapping::toDisplay(AutomationType type, bool isMuseSampler, muse::real_t value)
{
    if (type == AutomationType::Dynamics) {
        return dynamicsValueToDisplay(value, dynamicsDisplayRange(isMuseSampler));
    }

    if (type == AutomationType::Volume) {
        return volumeValueToDisplay(value);
    }

    if (type == AutomationType::Tempo) {
        const double logicalBpm = mu::engraving::denormalizeTempo(value).toBPM().val;
        const double localBpm = tempoLogicalBpmToLocalBpm(logicalBpm);
        const double display = localBpm / TEMPO_RANGE_MAX_BPM;
        return std::clamp(display, 0.0, 1.0);
    }

    return std::clamp(static_cast<double>(value), 0.0, 1.0);
}

// existingValue preserves values beyond the display range when the drag lands back at the same clamped edge
muse::real_t NotationAutomationDisplayMapping::fromDisplay(AutomationType type, bool isMuseSampler, double displayValue,
                                                           std::optional<muse::real_t> existingValue)
{
    if (type == AutomationType::Dynamics) {
        const DynamicsDisplayRange& dynamicsRange = dynamicsDisplayRange(isMuseSampler);
        if (existingValue) {
            const double existingDisplay = dynamicsValueToDisplay(*existingValue, dynamicsRange);
            const bool atTop = muse::RealIsEqualOrMore(displayValue, 1.0) && muse::RealIsEqualOrMore(existingDisplay, 1.0);
            const bool atBottom = muse::RealIsEqualOrLess(displayValue, 0.0) && muse::RealIsEqualOrLess(existingDisplay, 0.0);
            if (atTop || atBottom) {
                return *existingValue;
            }
        }

        return dynamicsValueFromDisplay(displayValue, dynamicsRange);
    }

    if (type == AutomationType::Volume) {
        const double localDb = VOLUME_RANGE_MIN_DB + displayValue * (VOLUME_RANGE_MAX_DB - VOLUME_RANGE_MIN_DB);
        const double logicalDb = volumeLocalDbToLogicalDb(localDb);
        const double normalized = (logicalDb - VOLUME_RANGE_MIN_DB) / (VOLUME_RANGE_MAX_DB - VOLUME_RANGE_MIN_DB);
        return muse::real_t(normalized);
    }

    if (type == AutomationType::Tempo) {
        if (existingValue && muse::RealIsEqualOrMore(displayValue, 1.0)
            && muse::RealIsEqualOrMore(toDisplay(type, isMuseSampler, *existingValue), 1.0)) {
            return *existingValue;
        }

        const double localBpm = displayValue * TEMPO_RANGE_MAX_BPM;
        const double logicalBpm = tempoLocalBpmToLogicalBpm(localBpm);
        const muse::real_t normalized = mu::engraving::normalizeTempo(mu::engraving::BeatsPerSecond::fromBPM(logicalBpm));
        return std::clamp(normalized, mu::engraving::MIN_NORMALIZED_TEMPO, mu::engraving::MAX_NORMALIZED_TEMPO);
    }

    return muse::real_t(displayValue);
}

QString NotationAutomationDisplayMapping::valueText(AutomationType type, muse::real_t value)
{
    const double v = static_cast<double>(value);

    switch (type) {
    case AutomationType::Dynamics:
        return QString("%1%").arg(v * 100.0, 0, 'g', 4);
    case AutomationType::Volume: {
        const double db = VOLUME_RANGE_MIN_DB + v * (VOLUME_RANGE_MAX_DB - VOLUME_RANGE_MIN_DB);
        return muse::qtrc("notation/automation", "%1 dB").arg(db, 0, 'f', 1);
    }
    case AutomationType::Tempo: {
        const double bpm = mu::engraving::denormalizeTempo(value).toBPM().val;
        return muse::qtrc("notation/automation", "%1 BPM").arg(bpm, 0, 'g', 4);
    }
    case AutomationType::Pan: {
        const int pan = qRound((v - 0.5) * 200.0);
        if (pan == 0) {
            return muse::qtrc("notation/automation", "Center");
        }
        return (pan < 0 ? muse::qtrc("notation/automation", "Left %1") : muse::qtrc("notation/automation", "Right %1")).arg(qAbs(pan));
    }
    case AutomationType::Unknown:
        break;
    }

    return QString::number(v, 'g', 4);
}
