/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

 #pragma once

 #include "audiotypes.h"

namespace muse::audio {
struct TimePosition {
    inline samples_t samples() const { return m_samples; }
    inline sample_rate_t sampleRate() const { return m_sampleRate; }
    inline secs_t time() const { return m_time; }

    TimePosition() = default;
    TimePosition(const TimePosition& other) = default;
    TimePosition& operator=(const TimePosition& other) = default;

    inline bool operator==(const TimePosition& other) const
    {
        return m_samples == other.m_samples && m_sampleRate == other.m_sampleRate;
    }

    inline bool operator!=(const TimePosition& other) const { return !this->operator==(other); }

    inline bool isValid() const { return m_sampleRate > 0; }

    inline void forward(samples_t delta)
    {
        IF_ASSERT_FAILED(m_sampleRate > 0) {
            return;
        }
        m_samples += delta;
        m_time += static_cast<double>(delta) / m_sampleRate;
    }

    inline void forward(const TimePosition& delta)
    {
        IF_ASSERT_FAILED(delta.sampleRate() == m_sampleRate) {
            return;
        }
        forward(delta.samples());
    }

    inline TimePosition forwarded(samples_t delta) const
    {
        IF_ASSERT_FAILED(m_sampleRate > 0) {
            return TimePosition();
        }
        return TimePosition(m_samples + delta, m_sampleRate);
    }

    inline TimePosition forwarded(const TimePosition& delta) const
    {
        IF_ASSERT_FAILED(delta.sampleRate() == m_sampleRate) {
            return TimePosition();
        }
        return forwarded(delta.samples());
    }

    static inline TimePosition zero(sample_rate_t sampleRate)
    {
        IF_ASSERT_FAILED(sampleRate > 0) {
            return TimePosition();
        }
        return TimePosition(0, sampleRate);
    }

    static inline TimePosition fromSamples(samples_t samples, sample_rate_t sampleRate)
    {
        IF_ASSERT_FAILED(sampleRate > 0) {
            return TimePosition();
        }

        return TimePosition(samples, sampleRate);
    }

    static inline TimePosition fromTime(secs_t time, sample_rate_t sampleRate)
    {
        IF_ASSERT_FAILED(sampleRate > 0) {
            return TimePosition();
        }

        IF_ASSERT_FAILED(time >= 0.0) {
            return TimePosition();
        }

        return TimePosition(static_cast<samples_t>(std::llround(time.raw() * sampleRate)), sampleRate);
    }

private:
    inline TimePosition(samples_t samples, sample_rate_t sampleRate)
        : m_samples(samples)
        , m_sampleRate(sampleRate)
        , m_time(sampleRate > 0 ? static_cast<double>(samples) / sampleRate : 0.0)
    {}

    samples_t m_samples = 0;
    sample_rate_t m_sampleRate = 0;
    secs_t m_time = 0.0;     // cache
};
}
