/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#ifndef MUSE_AUDIO_SMOOTHLINEARVALUE_H
#define MUSE_AUDIO_SMOOTHLINEARVALUE_H

#include "vectorops.h"

namespace muse::audio::fx {
template<typename ValueT, int _initialSteps = 1024, typename StepT = double>
class SmoothLinearValue
{
public:
    SmoothLinearValue(const ValueT& startValue = ValueT(0))
    {
        static_assert(std::is_floating_point<StepT>::value, "Internal step type needs to be floating point.");
        m_targetValue = m_currentValue = startValue;
        m_currentSteps = (StepT)_initialSteps;
        m_targetSteps = 0;
        m_deltaValue = 0;
    }

    void setSteps(int steps)
    {
        m_currentSteps = (StepT)std::max(1, steps);
    }

    /** Advance one tick by just adding the delta. Can lead to error accumulation.
     * @return true if target was reached
     */
    bool tick()
    {
        if (m_targetSteps <= 0) {
            m_currentValue = m_targetValue; // get rid of error accumulation
            return true;
        }

        m_targetSteps--;
        m_currentValue += m_deltaValue;
        return false;
    }

    /** Advance one tick and use multiplication to avoid error accumulation
     * @return true if target was reached
     */
    bool preciseTick()
    {
        if (m_targetSteps <= 0) {
            return true;
        }

        m_targetSteps--;
        m_currentValue = m_targetValue - m_targetSteps * m_deltaValue; // if n reaches 0, _value will be exactly targetvalue
        return false;
    }

    /** Advance one tick without checking whether target was reached
     * can lead to error accumulation for large n and small delta
     * warning! will overshoot target if called too many times
     */
    void tickUnchecked()
    {
        m_targetSteps--;
        m_currentValue += m_deltaValue;
    }

    /** Advance one tick without checking whether target was reached
     * uses an extra multiplication to avoid error accumulation
     * warning! will overshoot target if called too many times
     */
    void preciseTickUnchecked()
    {
        m_targetSteps--;
        m_currentValue = m_targetValue - m_targetSteps * m_deltaValue;
    }

    /** Advance multiple ticks
     *  @param n num ticks to advance
     */
    void advanceTicks(int n)
    {
        m_targetSteps -= (double)n;
        if (m_targetSteps < 0) {
            m_targetSteps = 0;
        }
        m_currentValue = m_targetValue - m_targetSteps * m_deltaValue;
    }

    /// Set the value instantly (for initialization)
    void initWithValue(const ValueT& val)
    {
        m_targetValue = m_currentValue = val;
        m_targetSteps = 0;
    }

    /// Set value and target instantly
    void initWithValueAndTarget(const ValueT& val, const ValueT& target)
    {
        initWithValue(val);
        setTargetValue(target);
    }

    /** Sets the target value which is reached after "steps" calls to smoothTick */
    void setTargetValue(const ValueT& val)
    {
        if (m_targetValue != val) {
            m_targetValue = val;
            m_deltaValue = ValueT((1.0 / m_currentSteps) * (m_targetValue - m_currentValue));
            m_targetSteps = m_currentSteps;
        }
    }

    /** @returns the value the smoothed value will reach eventually */
    const ValueT& getTargetValue() const
    {
        return m_targetValue;
    }

    /** @return the current (smoothed) value */
    inline const ValueT& getValue() const
    {
        return m_currentValue;
    }

    /** Go to target immediately.
     * @returns value
     */
    inline const ValueT& setToTarget()
    {
        m_currentValue = m_targetValue;
        m_targetSteps = 0.f;
        return m_currentValue;
    }

    /// check if the smoothed value isn't moving and has a specific (target)value
    bool isStaticAtValue(const ValueT& static_value) const
    {
        return isAtTargetValue() && (getTargetValue() == static_value);
    }

    inline bool isAtTargetValue() const
    {
        return m_targetSteps <= (StepT)0;
    }

    int getRemainingSteps() const
    {
        return (int)m_targetSteps;
    }

    int smoothingSteps() const
    {
        return (int)m_currentSteps;
    }

protected:
    ValueT m_currentValue;
    ValueT m_targetValue;
    ValueT m_deltaValue;
    StepT m_targetSteps;
    StepT m_currentSteps;
};

/// helper logic to apply gain ramps to blocks of audio
inline void apply_smooth_gain(SmoothLinearValue<float>& smooth_value, float** s_in, float** s_out,
                              int num_channels, int num_s)
{
    if (smooth_value.isAtTargetValue()) {
        for (int ch = 0; ch < num_channels; ++ch) {
            vo::constantMultiply(s_in[ch], smooth_value.getTargetValue(), s_out[ch], num_s);
        }
    } else {
        for (int i = 0; i < num_s; ++i) {
            for (int ch = 0; ch < num_channels; ++ch) {
                s_out[ch][i] = s_in[ch][i] * smooth_value.getValue();
            }
            smooth_value.tick();
        }
    }
}
} // namespace muse::audio::fx

#endif // MUSE_AUDIO_SMOOTHLINEARVALUE_H
