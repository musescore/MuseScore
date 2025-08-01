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

#ifndef MUSE_AUDIO_REVERBPROCESSOR_H
#define MUSE_AUDIO_REVERBPROCESSOR_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "../../../ifxprocessor.h"

namespace muse::audio::fx {
class ReverbProcessor : public IFxProcessor
{
public:
    ReverbProcessor(const audio::AudioFxParams& params, audioch_t audioChannelsCount = 2);
    ~ReverbProcessor() override;

    AudioFxType type() const override;
    const AudioFxParams& params() const override;
    async::Channel<audio::AudioFxParams> paramsChanged() const override;
    void setSampleRate(unsigned int sampleRate) override;

    bool active() const override;
    void setActive(bool active) override;

    void process(float* buffer, unsigned int sampleCount) override;

private:
    enum Params
    {
        Quality,
        VelvetIn,
        VelvetOut,

        DryLevel,
        LateLevel,
        ERLevel,
        ERtoLate,

        PreDelayMs,
        StereoSpread,
        ModDelayFreq,
        ModDelayAmp,
        ModType, // currently 0 = sine with regular phase offset | 1 = all channel synced sine

        ReverbTimeMs,
        LateRoomScale,
        TimeLow,
        TimeMid,
        TimeHigh,
        FeedbackTop,
        XoverLowMid,
        XoverMidHigh,

        LowCutFreq,
        HighCutFreq,
        PeakFreq,
        PeakGain,
        PeakQ,

        NumParams,
    };

    struct ParameterInfo
    {
        std::string name;
        std::pair<float, float> range;
    };

    void getParameterInfo(int32_t index, ParameterInfo& info);

    float getParameter(int32_t index);

    void setParameter(int32_t index, float newValue);

    bool setFormat(audioch_t audioChannelsCount, double sampleRate, int32_t maximumBlockSize);

    void deleteSignalBuffers();

    void reset();

    // Base Processor data
    struct Processor
    {
        struct Parameter
        {
            std::string name;
            std::pair<float, float> valueRange = { 0.f, 1.f };
            float currentValue = 0.f;
        };
        std::vector<Parameter> _param;

        void allocateParameters(int num);
        void setupParameter(int index, const std::string& name, std::pair<float, float> valueRange, float initialValue);
        bool setFormat(audioch_t audioChannelsCount, double sampleRate, int32_t maximumBlockSize);

        audioch_t _audioChannelsCount = 0;
        double _sampleRate = 0.0;
        double _sampleT = 0.0;
        int _blockSize = 0;
    };
    Processor m_processor;

    // Specific effect data
    template<int num_lines>
    void _processLines(float** signalPtr, int32_t numSamples);
    static constexpr int max_num_delays = 24;

    void calculateTailParams();
    void calculateModParams();

    struct impl;
    impl* d = nullptr; // aligned pointer

    float m_lateGain = 0.1f;
    float m_lateGainCorrection = 1.f;
    float m_erToLateGain = 0.f;

    int m_delays = 16;

    AudioFxParams m_params;
    async::Channel<audio::AudioFxParams> m_paramsChanged;

    float** m_signalBuffers = nullptr;
};
}

#endif // MUSE_AUDIO_REVERBPROCESSOR_H
