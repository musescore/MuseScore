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

#include "tempotimeline.h"

#include <algorithm>
#include <cmath>
#include <iterator>

#include "automation/tempovalues.h"
#include "types/constants.h"

#include "global/realfn.h"
#include "global/log.h"

using namespace mu::engraving;

static constexpr int MIN_TEMPO_RESAMPLE_STEPS = 8;

static TempoValues resampleTempoValues(const AutomationCurve& tempoCurve)
{
    TRACEFUNC;

    // No tempo markings at all - hold the default tempo
    if (tempoCurve.empty()) {
        return { { 0, Constants::DEFAULT_TEMPO.val } };
    }

    // Start with every authored point, denormalized back to bps,
    // plus a default-tempo point at tick 0 if the curve doesn't already start there
    TempoValues authored;
    authored.reserve(tempoCurve.size() + 1);
    if (tempoCurve.begin()->first > 0) {
        authored.emplace_back(0, Constants::DEFAULT_TEMPO.val);
    }

    for (const auto& [utick, point] : tempoCurve) {
        authored.emplace_back(utick, denormalizeTempo(point.value.outValue).val);
    }

    // Fill in the ramp shape of gradual tempo changes (accelerando/ritardando) between where
    // they start and where they arrive.
    // anchorUTick/anchorNorm track where the CURRENT ramp started, only advancing on a real
    // ramp arrival, so a marking inside another ramp's span doesn't restart it.
    // currentNorm is the actual last-known tempo, updated by every point - used to check if a
    // new ramp is flat, so it doesn't get bridged to an old, unrelated ramp's anchor
    TempoValues resampled;

    int anchorUTick = tempoCurve.begin()->first;
    real_t anchorNorm = tempoCurve.begin()->second.value.outValue;
    real_t currentNorm = anchorNorm;

    // Resampled ticks only increase across the whole loop below, so this hint can walk
    // `authored` forward once instead of a find() at every resampled tick
    auto authoredHint = authored.begin();

    for (auto it = std::next(tempoCurve.begin()); it != tempoCurve.end(); ++it) {
        if (!std::holds_alternative<AutomationPoint::ExplicitArrival>(it->second.value.inValue)) {
            currentNorm = it->second.value.outValue;
            continue;
        }

        const std::optional<AutomationPoint::Ease> pointEase = ease(it->second);
        const real_t arrivalNorm = muse::mpe::resolveInValue(it->second.value, std::nullopt);
        const bool isFlat = (!pointEase || pointEase->isNone()) && muse::RealIsEqual(arrivalNorm.raw(), currentNorm.raw());
        if (isFlat) {
            anchorUTick = it->first;
            anchorNorm = it->second.value.outValue;
            currentNorm = anchorNorm;
            continue;
        }

        const int segmentTicks = it->first - anchorUTick;
        const int steps = std::max(MIN_TEMPO_RESAMPLE_STEPS, segmentTicks / Constants::DIVISION);
        for (int s = 1; s < steps; ++s) {
            const int utick = anchorUTick + segmentTicks * s / steps;

            while (authoredHint != authored.end() && authoredHint->first < utick) {
                ++authoredHint;
            }
            if (authoredHint != authored.end() && authoredHint->first == utick) {
                continue; // there is already an authored point at this tick
            }
            if (!resampled.empty() && resampled.back().first == utick) {
                continue; // rounding produced the same tick as the previous step in this ramp
            }

            const muse::real_t t = muse::real_t(s) / muse::real_t(steps);
            const real_t normalized = muse::mpe::evaluateAt(it->second.value, anchorNorm, t);
            const BeatsPerSecond bps = denormalizeTempo(normalized);
            resampled.emplace_back(utick, bps.val);
        }

        anchorUTick = it->first;
        anchorNorm = it->second.value.outValue;
        currentNorm = anchorNorm;
    }

    if (resampled.empty()) {
        return authored;
    }

    TempoValues merged;
    merged.reserve(authored.size() + resampled.size());
    std::merge(authored.begin(), authored.end(), resampled.begin(), resampled.end(), std::back_inserter(merged),
               [](const std::pair<int, double>& a, const std::pair<int, double>& b) { return a.first < b.first; });
    return merged;
}

void TempoTimeline::rebuild(const AutomationCurve& tempoCurve, const PausesMap& pauses)
{
    rebuild(resampleTempoValues(tempoCurve), pauses);
}

void TempoTimeline::rebuild(const TempoValues& tempoValues, const PausesMap& pauses)
{
    TRACEFUNC;

    m_points.clear();
    m_pauses = pauses;
    m_utick2utimeHint = 0;
    m_utime2utickHint = 0;
    m_tempoHint = 0;

    m_points.reserve(tempoValues.size() + pauses.size());

    // Merge-walk values (tick->bps) and pauses (tick->held seconds) by tick in one pass
    auto vit = tempoValues.begin();
    auto pit = pauses.begin();

    while (vit != tempoValues.end() || pit != pauses.end()) {
        const bool valueIsNext = pit == pauses.end() || (vit != tempoValues.end() && vit->first <= pit->first);
        const int utick = valueIsNext ? vit->first : pit->first;

        if (valueIsNext) {
            double time = 0.0;
            if (!m_points.empty()) {
                const TempoTimePoint& prev = m_points.back();
                time = prev.time + double(utick - prev.utick) / (Constants::DIVISION * prev.bps);
            }
            // Clamp to the valid tempo range so a zero/negative bps can't make time math divide by zero
            const double bps = std::clamp(vit->second, Constants::MIN_TEMPO.val, Constants::MAX_TEMPO.val);
            m_points.push_back({ utick, time, bps, 0.0 });
            ++vit;
        }

        // A pause can land on the same tick as the value point just emitted above
        if (pit != pauses.end() && pit->first == utick) {
            if (m_points.empty()) {
                // No tempo value at or before this pause - anchor with the default tempo
                m_points.push_back({ utick, 0.0, Constants::DEFAULT_TEMPO.val, 0.0 });
            }
            const TempoTimePoint& prev = m_points.back();
            const double time = prev.utick == utick
                                ? prev.time
                                : prev.time + double(utick - prev.utick) / (Constants::DIVISION * prev.bps);
            m_points.push_back({ utick, time + pit->second, prev.bps, pit->second });
            ++pit;
        }
    }
}

double TempoTimeline::utick2utime(int utick) const
{
    const size_t n = m_points.size();
    if (n == 0) {
        return 0.;
    }

    bool hintValid = false;
    if (m_utick2utimeHint < n) {
        const int hintUtick = m_points[m_utick2utimeHint].utick;
        const bool isLastPoint = (m_utick2utimeHint + 1 == n);
        hintValid = utick >= hintUtick
                    && (isLastPoint || utick < m_points[m_utick2utimeHint + 1].utick);
    }

    if (!hintValid) {
        const auto it = std::upper_bound(m_points.begin(), m_points.end(), utick,
                                         [](int u, const TempoTimePoint& p) { return u < p.utick; });
        m_utick2utimeHint = it == m_points.begin() ? 0 : size_t(std::prev(it) - m_points.begin());
    }

    const TempoTimePoint& from = m_points[m_utick2utimeHint];
    const double nominalTime = from.time + double(utick - from.utick) / (Constants::DIVISION * from.bps);
    return nominalTime / m_tempoMultiplier.val;
}

int TempoTimeline::utime2utick(double time) const
{
    const size_t n = m_points.size();
    if (n == 0) {
        return 0;
    }

    const double nominalSecs = time * m_tempoMultiplier.val;

    bool hintValid = false;
    if (m_utime2utickHint < n) {
        const double hintTime = m_points[m_utime2utickHint].time;
        const bool isLastPoint = (m_utime2utickHint + 1 == n);
        hintValid = nominalSecs >= hintTime
                    && (isLastPoint || nominalSecs < m_points[m_utime2utickHint + 1].time);
    }

    if (!hintValid) {
        const auto it = std::upper_bound(m_points.begin(), m_points.end(), nominalSecs,
                                         [](double s, const TempoTimePoint& p) { return s < p.time; });
        m_utime2utickHint = it == m_points.begin() ? 0 : size_t(std::prev(it) - m_points.begin());
    }

    const size_t idx = m_utime2utickHint;
    const TempoTimePoint& from = m_points[idx];

    if (idx + 1 < n) {
        const TempoTimePoint& next = m_points[idx + 1];
        if (next.pause > 0.0 && nominalSecs >= next.time - next.pause) {
            return next.utick; // still inside next's pause window: hold there
        }
    }

    const double elapsed = nominalSecs - from.time;
    return from.utick + int(std::lround(elapsed * Constants::DIVISION * from.bps));
}

BeatsPerSecond TempoTimeline::tempo(int utick) const
{
    const size_t n = m_points.size();
    if (n == 0) {
        return Constants::DEFAULT_TEMPO;
    }

    bool hintValid = false;
    if (m_tempoHint < n) {
        const int hintUtick = m_points[m_tempoHint].utick;
        const bool isLastPoint = (m_tempoHint + 1 == n);
        hintValid = utick >= hintUtick
                    && (isLastPoint || utick < m_points[m_tempoHint + 1].utick);
    }

    if (!hintValid) {
        const auto it = std::upper_bound(m_points.begin(), m_points.end(), utick,
                                         [](int u, const TempoTimePoint& p) { return u < p.utick; });
        m_tempoHint = it == m_points.begin() ? 0 : size_t(std::prev(it) - m_points.begin());
    }

    return BeatsPerSecond(m_points[m_tempoHint].bps);
}

void TempoTimeline::setTempoMultiplier(const BeatsPerSecond& bps)
{
    IF_ASSERT_FAILED(bps.val > 0.0) {
        return;
    }

    m_tempoMultiplier = bps;
}
