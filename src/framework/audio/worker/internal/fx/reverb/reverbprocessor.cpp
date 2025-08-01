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

#include "reverbprocessor.h"

#include <cassert>
#include <cmath>
#include <complex>
#include <vector>

#include "allpassdispersion.h"
#include "allpassmodulateddelay.h"
#include "iirbiquadfilter.h"
#include "ivndecorrelation.h"
#include "reverbfilters.h"
#include "reverbmatrices.h"
#include "sampledelay.h"
#include "simdtypes.h"

namespace muse::audio::fx {
float fromDecibel(float dB)
{
    return std::pow(10.f, dB / 20.f);
}

class SamplesFloat
{
public:
    ~SamplesFloat()
    {
        for (int ch = 0; ch < num_channels; ch++) {
            dealloc(ch);
        }
    }

    void setSize(int32_t numChannels, int32_t samples)
    {
        for (int ch = 0; ch < num_channels; ch++) {
            dealloc(ch);
        }

        num_channels = numChannels;
        num_samples = samples;
        data.resize(num_channels);
        for (int ch = 0; ch < num_channels; ch++) {
            alloc(ch, num_samples);
        }
    }

    float** getPtrs()
    {
        return data.data();
    }

    float* getPtr(int32_t channel)
    {
        assert(channel < num_channels);
        assert(data[channel]);
        return data[channel];
    }

    const float* getPtr(int32_t channel) const
    {
        assert(channel < num_channels);
        assert(data[channel]);
        return data[channel];
    }

    void assignSamples(int32_t channel, const float* input)
    {
        assert(channel < num_channels);
        assert(data[channel]);
        vo::copy(input, data[channel], num_samples);
    }

    void assignSamples(const SamplesFloat& rhs)
    {
        assert(num_channels == rhs.num_channels);
        assert(num_samples == rhs.num_samples);
        for (int ch = 0; ch < num_channels; ch++) {
            assert(data[ch]);
            vo::copy(rhs.getPtr(ch), getPtr(ch), num_samples);
        }
    }

    void zeroOut()
    {
        for (int ch = 0; ch < num_channels; ch++) {
            vo::setToZero(data[ch], num_samples);
        }
    }

private:
    int32_t num_channels{ 0 };
    int32_t num_samples{ 0 };
    std::vector<float*> data;

    void alloc(int32_t channel, int32_t samples)
    {
        assert(channel < num_channels);
        if (data[channel]) {
            dealloc(channel);
        }
        data[channel] = (float*)simd::aligned_malloc(samples * sizeof(float), 64);
    }

    void dealloc(int32_t channel)
    {
        assert(channel < num_channels);
        if (data[channel]) {
            simd::aligned_free(data[channel]);
            data[channel] = nullptr;
        }
    }
};

class MultiPhaseSinLfo
{
public:
    void setup(int num_taps, float freq, float amplitude, double sampleRate)
    {
        _phase = 0.f;
        _phaseInc = float(6.28318530717958647692 * freq / sampleRate);

        _rotations.resize(num_taps);
        _amplitude = amplitude;
        float phase_delta = float(6.28318530717958647692 / num_taps);
        for (int i = 0; i < num_taps; ++i) {
            _rotations[i] = { std::cos(i * phase_delta), std::sin(i * phase_delta) };
        }
    }

    float getNextMainValue()
    {
        _phase += _phaseInc;
        if (_phase > 6.28318530717958647692f) {
            _phase -= 6.28318530717958647692f;
        }
        _main_val_sin = std::sin(_phase) * _amplitude;
        _main_val_cos = std::cos(_phase) * _amplitude;
        return _main_val_sin;
    }

    float getTapValue(int n) const
    {
        return _main_val_cos * _rotations[n].imag() + _main_val_sin * _rotations[n].real();
    }

private:
    float _phase = 0.f;
    float _phaseInc = 0.1f;
    float _main_val_sin = 0.f, _main_val_cos = 1.f;
    float _amplitude = 1.f;
    std::vector<std::complex<float> > _rotations;
};

static inline const int* _delayTimesForN(int n)
{
    // these are ~logarithmically spaced primes
    static const int delayTimes8[] = { 839, 947, 1069, 1213, 1373, 1549, 1753, 1979 };
    static const int delayTimes12[] = { 839, 911, 991, 1069, 1163, 1259, 1373, 1487, 1613, 1753, 1901, 2063 };
    static const int delayTimes16[] = { 839,  887,  947,  1009, 1069, 1151, 1213, 1289,
                                        1373, 1459, 1549, 1657, 1753, 1861, 1979, 2099 };
    static const int delayTimes24[] = { 839,  877,  911,  947,  991,  1031, 1069, 1117, 1163, 1213, 1259, 1319,
                                        1373, 1427, 1487, 1549, 1613, 1693, 1753, 1823, 1901, 1979, 2063, 2143 };

    switch (n) {
    case 24: return delayTimes24;
    case 16: return delayTimes16;
    case 12: return delayTimes12;
    default: return delayTimes8;
    }
}

static inline float _sqrt_sign(float x)
{
    return std::copysign(std::sqrt(std::abs(x)), x);
}

struct ReverbProcessor::impl
{
    // members requiring alignment first
    IirBiquadFilter::Coeffs<simd::float_x4> damping_cf1_x4[max_num_delays / 4];
    IirBiquadFilter::Coeffs<simd::float_x4> damping_cf2_x4[max_num_delays / 4];
    IirBiquadFilter::DF2State<simd::float_x4> damping_state1_x4[max_num_delays / 4];
    IirBiquadFilter::DF2State<simd::float_x4> damping_state2_x4[max_num_delays / 4];
    reverbfilters::OnePoleFilter<simd::float_x4> ag_filter_x4[max_num_delays / 4];

    AllPassModulatedDelay modDelay[max_num_delays];
    AllPassDispersion disp_ap;

    ImprovedVelvetNoiseDecorrelation ivnd_in[max_num_delays];
    ImprovedVelvetNoiseDecorrelation ivnd_out[max_num_delays];
    ImprovedVelvetNoiseDecorrelation ivnd_er[2];

    SmoothLinearValue<float> dry_gain_smooth;
    SmoothLinearValue<float> late_gain_smooth;
    SmoothLinearValue<float> er_gain_smooth;

    SamplesFloat ivnd_in_buffer;
    SamplesFloat delay_out_buffer;
    SamplesFloat er_buffer;
    SamplesFloat work_buffer;
    SamplesFloat late_buffer;
    MultiPhaseSinLfo sinLfo;

    IirBiquadFilter::DF1Processor<double> loCutFilter, hiCutFilter, peakFilter;

    SparseFirFilter er_fir[2];
    SampleDelay<float, 2> pre_delay;

    int modStep = 32;
    int modCounter = 0;
};

ReverbProcessor::ReverbProcessor(const AudioFxParams& params, audioch_t audioChannelsCount)
    : m_params(params)
{
    d = simd::aligned_new<impl>(64);

    m_processor.allocateParameters(NumParams);
    m_processor.setupParameter(Quality, "Quality", { 1.f, 4.f }, 4);

    m_processor.setupParameter(DryLevel, "DryLevel", { -60.f, 20.f }, -60);
    m_processor.setupParameter(LateLevel, "LateLevel", { -60.f, 20.f }, -20);
    m_processor.setupParameter(ERLevel, "ERDirect", { -60.f, 20.f }, -20);
    m_processor.setupParameter(ERtoLate, "ERtoLate", { -60.f, 20.f }, -20);

    m_processor.setupParameter(PreDelayMs, "PreDelay", { 0.f, 500.f }, 10);
    m_processor.setupParameter(StereoSpread, "Stereo", { 0.f, 150.f }, 110);

    m_processor.setupParameter(ReverbTimeMs, "ReverbTimeMs", { 100.f, 10000.f }, 2200);
    m_processor.setupParameter(LateRoomScale, "LateRoomScale", { 0.5f, 4.f }, 0.8f);
    m_processor.setupParameter(FeedbackTop, "FeedbackTop", { 500.f, 20000.f }, 8000);

    m_processor.setupParameter(TimeLow, "TimeLow", { 50.f, 200.f }, 100);
    m_processor.setupParameter(TimeMid, "TimeMid", { 50.f, 200.f }, 100);
    m_processor.setupParameter(TimeHigh, "TimeHigh", { 50.f, 200.f }, 60);
    m_processor.setupParameter(XoverLowMid, "LowMidFreq", { 50.f, 500.f }, 400);
    m_processor.setupParameter(XoverMidHigh, "MidHighFreq", { 1000.f, 10000.f }, 4000);

    m_processor.setupParameter(ModDelayFreq, "ModFreq", { 0.f, 10.f }, 1.f);
    m_processor.setupParameter(ModDelayAmp, "ModAmp", { 0.f, 1.f }, 0.2f);
    m_processor.setupParameter(ModType, "ModType", { 0.f, 1.f }, 0);

    m_processor.setupParameter(VelvetIn, "VelvetIn", { 0.f, 1.f }, 0);
    m_processor.setupParameter(VelvetOut, "VelvetOut", { 0.f, 1.f }, 1);

    m_processor.setupParameter(LowCutFreq, "LowCut", { 10.f, 25000.f }, 10.f);
    m_processor.setupParameter(HighCutFreq, "HighCut", { 10.f, 25000.f }, 25000.f);

    m_processor.setupParameter(PeakFreq, "PeakFreq", { 10.f, 25000.f }, 375.f);
    m_processor.setupParameter(PeakGain, "PeakGain", { -24.f, +24.f }, 0.f);
    m_processor.setupParameter(PeakQ, "PeakQ", { 0.05f, 16.f }, 1);

    // Refresh parameters that need initialisation
    setParameter(Params::DryLevel, getParameter(Params::DryLevel));
    setParameter(Params::LateLevel, getParameter(Params::LateLevel));
    setParameter(Params::ERLevel, getParameter(Params::ERLevel));
    setParameter(Params::ERtoLate, getParameter(Params::ERtoLate));
    setParameter(Params::ModDelayAmp, getParameter(Params::ModDelayAmp));
    setParameter(Params::LateRoomScale, getParameter(Params::LateRoomScale));
    setParameter(Params::ReverbTimeMs, getParameter(Params::ReverbTimeMs));
    setParameter(Params::LowCutFreq, getParameter(Params::LowCutFreq));
    setParameter(Params::HighCutFreq, getParameter(Params::HighCutFreq));
    setParameter(Params::PeakFreq, getParameter(Params::PeakFreq));
    setParameter(Params::Quality, getParameter(Params::Quality));
    setParameter(Params::PreDelayMs, getParameter(Params::PreDelayMs));
    setParameter(Params::FeedbackTop, getParameter(Params::FeedbackTop));

    setFormat(audioChannelsCount, 44100.0 /*sampleRate*/, 512 /*maximumBlockSize*/);
}

ReverbProcessor::~ReverbProcessor()
{
    simd::aligned_delete(d);
    deleteSignalBuffers();
}

AudioFxType ReverbProcessor::type() const
{
    return AudioFxType::MuseFx;
}

const AudioFxParams& ReverbProcessor::params() const
{
    return m_params;
}

async::Channel<audio::AudioFxParams> ReverbProcessor::paramsChanged() const
{
    return m_paramsChanged;
}

void ReverbProcessor::setSampleRate(unsigned int sampleRate)
{
    if (m_processor._sampleRate == sampleRate) {
        return;
    }

    setFormat(m_processor._audioChannelsCount, sampleRate, m_processor._blockSize);
}

bool ReverbProcessor::active() const
{
    return m_params.active;
}

void ReverbProcessor::setActive(bool active)
{
    m_params.active = active;
}

void ReverbProcessor::process(float* buffer, unsigned int sampleCount)
{
    if (m_processor._blockSize != static_cast<int>(sampleCount)) {
        setFormat(m_processor._audioChannelsCount, m_processor._sampleRate, sampleCount);
    }

    for (samples_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
        size_t offset = sampleIndex * m_processor._audioChannelsCount;

        for (audioch_t audioChannelIndex = 0; audioChannelIndex < m_processor._audioChannelsCount; ++audioChannelIndex) {
            m_signalBuffers[audioChannelIndex][sampleIndex] = buffer[offset + audioChannelIndex];
        }
    }

    switch (m_delays) {
    case 24: _processLines<24>(m_signalBuffers, sampleCount);
        break;
    case 16: _processLines<16>(m_signalBuffers, sampleCount);
        break;
    case 12: _processLines<12>(m_signalBuffers, sampleCount);
        break;
    default: _processLines<8>(m_signalBuffers, sampleCount);
        break;
    }

    for (samples_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
        size_t offset = sampleIndex * m_processor._audioChannelsCount;

        for (audioch_t audioChannelIndex = 0; audioChannelIndex < m_processor._audioChannelsCount; ++audioChannelIndex) {
            buffer[offset + audioChannelIndex] = m_signalBuffers[audioChannelIndex][sampleIndex];
        }
    }
}

void ReverbProcessor::getParameterInfo(int32_t index, ParameterInfo& info)
{
    assert(index < NumParams);
    info.name = m_processor._param[index].name;
    info.range = m_processor._param[index].valueRange;
}

float ReverbProcessor::getParameter(int32_t index)
{
    assert(index < NumParams);
    return m_processor._param[index].currentValue;
}

void ReverbProcessor::calculateTailParams()
{
    const int* delayTimes = _delayTimesForN(m_delays);
    float scale = float(getParameter(LateRoomScale) * m_processor._sampleRate / 44100.);

    const float log_db60 = std::log(fromDecibel(-60.f));

    float timeSmp = float(getParameter(ReverbTimeMs) * 0.001 * m_processor._sampleRate);

    auto decay_per_sample = log_db60 / (timeSmp);
    auto decay_per_sample_l = decay_per_sample / (getParameter(TimeLow) * 0.01f);
    auto decay_per_sample_m = decay_per_sample / (getParameter(TimeMid) * 0.01f);
    auto decay_per_sample_h = decay_per_sample / (getParameter(TimeHigh) * 0.01f);

    // correct gain to keep the level of the first late output sample constant
    m_lateGainCorrection = 1.f / std::exp(delayTimes[0] * scale * decay_per_sample);
    d->late_gain_smooth.setTargetValue(m_lateGain * m_lateGainCorrection);

    for (int i = 0; i < m_delays; ++i) {
        float delay_time = delayTimes[i] * scale;

        float gain_l = std::exp(delay_time * decay_per_sample_l);
        float gain_m = std::exp(delay_time * decay_per_sample_m);
        float gain_h = std::exp(delay_time * decay_per_sample_h);
        IirBiquadFilter::Coeffs<float> cf1, cf2;
        IirBiquadFilter::create3BandToneControl4P<float>(getParameter(XoverLowMid), getParameter(XoverMidHigh),
                                                         gain_l, gain_m, gain_h, (float)m_processor._sampleRate, cf1, cf2);

        float ag_gain = std::exp(delay_time * decay_per_sample_h * 0.5f); // at feedbacktop use half the 'high' delay time
        auto ag_cf = reverbfilters::onePoleCoeffsLowpass1Point<float>(getParameter(FeedbackTop), ag_gain,
                                                                      m_processor._sampleRate);

        // copy coefficients to simd-friendly structures
        int x = i >> 2, y = i & 3;
        d->damping_cf1_x4[x].a1[y] = cf1.a1;
        d->damping_cf1_x4[x].a2[y] = cf1.a2;
        d->damping_cf1_x4[x].b0[y] = cf1.b0;
        d->damping_cf1_x4[x].b1[y] = cf1.b1;
        d->damping_cf1_x4[x].b2[y] = cf1.b2;

        d->damping_cf2_x4[x].a1[y] = cf2.a1;
        d->damping_cf2_x4[x].a2[y] = cf2.a2;
        d->damping_cf2_x4[x].b0[y] = cf2.b0;
        d->damping_cf2_x4[x].b1[y] = cf2.b1;
        d->damping_cf2_x4[x].b2[y] = cf2.b2;

        d->ag_filter_x4[x].cf.b0[y] = ag_cf.b0;
        d->ag_filter_x4[x].cf.b1[y] = ag_cf.b1;
        d->ag_filter_x4[x].cf.a1[y] = ag_cf.a1;
    }
}

void ReverbProcessor::calculateModParams()
{
    const int* delayTimes = _delayTimesForN(m_delays);

    float depthMs = getParameter(ModDelayAmp) * getParameter(LateRoomScale);
    float freqHz = getParameter(ModDelayFreq);

    float modDepthSmp = float(depthMs * 0.001f * m_processor._sampleRate);
    d->sinLfo.setup(m_delays, freqHz * d->modStep, modDepthSmp, m_processor._sampleRate);
    float scale = float(getParameter(LateRoomScale) * m_processor._sampleRate / 44100.);
    for (int i = 0; i < m_delays; ++i) {
        d->modDelay[i].setBaseDelay(delayTimes[i] * scale, modDepthSmp * 2.f);
    }
}

void ReverbProcessor::setParameter(int32_t index, float newValue)
{
    // Store
    assert(index >= 0 && index <= static_cast<int32_t>(m_processor._param.size()));
    auto clampedValue
        =std::min(m_processor._param[index].valueRange.second, std::max(m_processor._param[index].valueRange.first, newValue));
    m_processor._param[index].currentValue = clampedValue;

    // Update
    switch (index) {
    case Params::ModDelayFreq:
    case Params::ModDelayAmp: calculateModParams();
        break;
    case Params::LateRoomScale:
        calculateTailParams();
        calculateModParams();
        break;
    case Params::ReverbTimeMs:
    case Params::TimeLow:
    case Params::TimeMid:
    case Params::TimeHigh:
    case Params::XoverLowMid:
    case Params::XoverMidHigh:
    case Params::FeedbackTop: calculateTailParams();
        break;

    case Params::LowCutFreq:
        d->loCutFilter.setTargetCoefficients(
            IirBiquadFilter::createHighPass1PMatched((double)getParameter(LowCutFreq), m_processor._sampleRate));
        break;

    case Params::HighCutFreq:
        d->hiCutFilter.setTargetCoefficients(
            IirBiquadFilter::createLowPass1PMatched((double)getParameter(HighCutFreq), m_processor._sampleRate));
        break;

    case Params::PeakFreq:
    case Params::PeakGain:
    case Params::PeakQ:
        d->peakFilter.setTargetCoefficients(IirBiquadFilter::createBandShelfMatched(
                                                getParameter(PeakFreq), getParameter(PeakQ), fromDecibel(getParameter(PeakGain)),
                                                m_processor._sampleRate));
        break;

    case Params::DryLevel:
        d->dry_gain_smooth.setTargetValue(
            (newValue > m_processor._param[Params::DryLevel].valueRange.first) ? fromDecibel(newValue) : 0.f);
        break;
    case Params::LateLevel:
        m_lateGain = (newValue > m_processor._param[Params::LateLevel].valueRange.first) ? fromDecibel(newValue) : 0.f;
        d->late_gain_smooth.setTargetValue(m_lateGain * m_lateGainCorrection);
        break;
    case Params::ERLevel:
        d->er_gain_smooth.setTargetValue(
            (newValue > m_processor._param[Params::ERLevel].valueRange.first) ? fromDecibel(newValue) : 0.f);
        break;
    case Params::ERtoLate:
        m_erToLateGain = (newValue > m_processor._param[Params::ERtoLate].valueRange.first) ? fromDecibel(newValue) : 0.f;
        break;
    case Params::Quality:
        switch (int(newValue)) {
        case 4: m_delays = 24;
            break;
        case 3: m_delays = 16;
            break;
        case 2: m_delays = 12;
            break;
        default: m_delays = 8;
            break;
        }
        calculateTailParams();
        calculateModParams();
        reset();
        break;
    case Params::PreDelayMs: {
        int smp = int(newValue * 0.001 * m_processor._sampleRate + 0.5);
        d->pre_delay.setDelaySamples(smp);
        break;
    }
    }
}

bool ReverbProcessor::setFormat(audioch_t audioChannelsCount, double sampleRate, int32_t maximumBlockSize)
{
    IF_ASSERT_FAILED(audioChannelsCount > 0 && audioChannelsCount <= 2) {
        return false;
    }

    if (m_signalBuffers) {
        if (audioChannelsCount > m_processor._audioChannelsCount
            || maximumBlockSize > m_processor._blockSize) {
            deleteSignalBuffers();
        }
    }

    // Store base information
    m_processor.setFormat(audioChannelsCount, sampleRate, maximumBlockSize);

    if (!m_signalBuffers) {
        m_signalBuffers = new float*[m_processor._audioChannelsCount];
        for (audioch_t i = 0; i < m_processor._audioChannelsCount; ++i) {
            m_signalBuffers[i] = new float[maximumBlockSize];
        }
    }

    d->work_buffer.setSize(2, maximumBlockSize);
    d->er_buffer.setSize(2, maximumBlockSize);
    d->late_buffer.setSize(2, maximumBlockSize);

    int max_predelay_smp = int(sampleRate * 0.001 * m_processor._param[Params::PreDelayMs].valueRange.second + 0.5);
    d->pre_delay.allocateForMaxDelaySamples(max_predelay_smp);
    for (int i = 0; i < max_num_delays; ++i) {
        d->ivnd_in[i].configure(i, sampleRate, maximumBlockSize);
        d->ivnd_out[i].configure(32 + i, sampleRate, maximumBlockSize);
    }
    d->ivnd_er[0].configure(31, sampleRate, maximumBlockSize);
    d->ivnd_er[1].configure(63, sampleRate, maximumBlockSize);
    d->ivnd_in_buffer.setSize(max_num_delays, maximumBlockSize);
    d->delay_out_buffer.setSize(max_num_delays, maximumBlockSize);

    for (int i = 0; i < 2; ++i) {
        d->er_fir[i].setFormat(maximumBlockSize, int(0.15 * sampleRate));
        d->er_fir[i].clearImpulses();
    }
    d->er_fir[0].appendImpulse(int(0.0201 * sampleRate), 1.f);
    d->er_fir[1].appendImpulse(int(0.0343 * sampleRate), 1.f);
    d->er_fir[0].appendImpulse(int(0.0382 * sampleRate), 0.4f);
    d->er_fir[1].appendImpulse(int(0.0548 * sampleRate), 0.4f);
    d->er_fir[0].appendImpulse(int(0.0633 * sampleRate), 0.16f);
    d->er_fir[1].appendImpulse(int(0.0775 * sampleRate), 0.16f);
    d->er_fir[0].appendImpulse(int(0.0961 * sampleRate), 0.064f);
    d->er_fir[1].appendImpulse(int(0.1100 * sampleRate), 0.064f);

    auto smoothSteps = int(0.02f * m_processor._sampleRate + 0.5f);
    d->loCutFilter.setSmoothingSteps(smoothSteps);
    d->hiCutFilter.setSmoothingSteps(smoothSteps);
    d->peakFilter.setSmoothingSteps(smoothSteps);
    d->dry_gain_smooth.setSteps(smoothSteps);
    d->late_gain_smooth.setSteps(smoothSteps);
    d->er_gain_smooth.setSteps(smoothSteps);

    calculateModParams();

    // Refresh dependent params
    setParameter(Params::PreDelayMs, getParameter(Params::PreDelayMs));
    setParameter(Params::ModDelayFreq, getParameter(Params::ModDelayFreq));
    setParameter(Params::ReverbTimeMs, getParameter(Params::ReverbTimeMs));
    setParameter(Params::VelvetIn, getParameter(Params::VelvetIn));
    setParameter(Params::LowCutFreq, getParameter(Params::LowCutFreq));
    setParameter(Params::HighCutFreq, getParameter(Params::HighCutFreq));
    setParameter(Params::Quality, getParameter(Params::Quality));

    return true;
}

void ReverbProcessor::deleteSignalBuffers()
{
    for (audioch_t i = 0; i < m_processor._audioChannelsCount; ++i) {
        delete[] m_signalBuffers[i];
    }

    delete[] m_signalBuffers;
    m_signalBuffers = nullptr;
}

void ReverbProcessor::reset()
{
    d->disp_ap.reset();
    d->pre_delay.reset();
    for (int ch = 0; ch < 2; ++ch) {
        d->er_fir[ch].reset();
        d->ivnd_er[ch].reset();
    }
    for (int i = 0; i < m_delays; ++i) {
        d->ivnd_in[i].reset();
        d->ivnd_out[i].reset();
        d->modDelay[i].reset();
    }
    for (int j = 0; j < (m_delays >> 2); ++j) {
        IirBiquadFilter::resetState(d->damping_state1_x4[j]);
        IirBiquadFilter::resetState(d->damping_state2_x4[j]);
        d->ag_filter_x4[j].reset();
    }
    d->loCutFilter.reset();
    d->hiCutFilter.reset();
    d->peakFilter.reset();

    d->dry_gain_smooth.setToTarget();
    d->late_gain_smooth.setToTarget();
    d->er_gain_smooth.setToTarget();
}

template<int num_lines>
void ReverbProcessor::_processLines(float** signalPtr, int32_t numSamples)
{
    static_assert(num_lines == 8 || num_lines == 12 || num_lines == 16 || num_lines == 24);

    d->work_buffer.setSize(2, numSamples);
    auto** work_ptr = d->work_buffer.getPtrs();
    d->er_buffer.setSize(2, numSamples);
    auto** er_ptr = d->er_buffer.getPtrs();
    d->late_buffer.setSize(2, numSamples);
    auto** late_ptr = d->late_buffer.getPtrs();

    // handle mono by using the same buffer twice
    float* signal_in[] = { signalPtr[0], m_processor._audioChannelsCount == 2 ? signalPtr[1] : signalPtr[0] };
    float* signal_out[] = { signalPtr[0], m_processor._audioChannelsCount == 2 ? signalPtr[1] : signalPtr[0] };

    // pre-delay, dispersion and velvet-input
    d->work_buffer.assignSamples(0, signal_in[0]);
    d->work_buffer.assignSamples(1, signal_in[1]);
    d->pre_delay.processBlock(work_ptr, 2, numSamples);

    const bool velvet_input = !RealIsNull(getParameter(VelvetIn));

    bool er_muted = d->er_gain_smooth.isStaticAtValue(0.f);
    bool late_muted = d->late_gain_smooth.isStaticAtValue(0.f);

    // early reflections
    if (!er_muted || (m_erToLateGain > 0.f && !late_muted)) {
        for (int ch = 0; ch < 2; ++ch) {
            d->er_fir[ch].processBlock(work_ptr[ch], er_ptr[ch], numSamples);
            // add to late feedback input (work buffer)
            vo::constantMultiplyAndAdd(er_ptr[ch], m_erToLateGain, work_ptr[ch], numSamples);
        }

        if (velvet_input) {
            for (int ch = 0; ch < 2; ++ch) {
                d->ivnd_er[ch].processBlock(er_ptr[ch], er_ptr[ch], numSamples);
            }
        }
    }

    if (!late_muted) {
        // construct input buffers for all delay lines.
        const float* mirrored_input[num_lines]; // a helper pointer array for non-disp input
        const float** delay_in_ptr;
        if (velvet_input) {
            for (int i = 0; i < num_lines; ++i) {
                d->ivnd_in[i].processBlock(work_ptr[i & 1], d->ivnd_in_buffer.getPtr(i), numSamples);
            }
            delay_in_ptr = (const float**)d->ivnd_in_buffer.getPtrs();
        } else {
            for (int i = 0; i < num_lines; ++i) {
                mirrored_input[i] = work_ptr[i & 1];
            }
            delay_in_ptr = mirrored_input;
        }
        auto delay_out_ptr = d->delay_out_buffer.getPtrs();

        // feedback loop
        for (int cnt = 0; cnt < numSamples; ++cnt) {
            // update delay modulation offsets
            if (d->modCounter++ >= d->modStep) {
                d->modCounter = 0;

                int modType = int(getParameter(Params::ModType));
                switch (modType) {
                case 0: // phase distributed sine waves
                    d->modDelay[0].setModOffset(d->sinLfo.getNextMainValue());
                    for (int i = 1; i < num_lines; ++i) {
                        d->modDelay[i].setModOffset(d->sinLfo.getTapValue(i));
                    }
                    break;
                case 1: {
                    auto offset = d->sinLfo.getNextMainValue();
                    for (int i = 0; i < num_lines; ++i) {
                        d->modDelay[i].setModOffset(offset);
                    }
                    break;
                }
                }
            }

            // delay line outputs / decay filters
            float mat_in[num_lines];
            for (int i = 0; i < num_lines; i += 4) {
                int j = i >> 2;
                simd::float_x4 s = { d->modDelay[i].readSample(), d->modDelay[i + 1].readSample(),
                                     d->modDelay[i + 2].readSample(), d->modDelay[i + 3].readSample() };

                s = d->ag_filter_x4[j].processSample(s);
                s = IirBiquadFilter::processSampleDF2(s, d->damping_cf1_x4[j], d->damping_state1_x4[j]);
                s = IirBiquadFilter::processSampleDF2(s, d->damping_cf2_x4[j], d->damping_state2_x4[j]);

                mat_in[i] = delay_out_ptr[i][cnt] = s[0];
                mat_in[i + 1] = delay_out_ptr[i + 1][cnt] = s[1];
                mat_in[i + 2] = delay_out_ptr[i + 2][cnt] = s[2];
                mat_in[i + 3] = delay_out_ptr[i + 3][cnt] = s[3];
            }
            // Applying the Matrix
            float mat_res[num_lines];
            reverb_matrices::Hadamard<num_lines>(mat_in, mat_res);

            // feeding matrix results and input buffers to the delay lines
            for (int i = 0; i < num_lines; ++i) {
                d->modDelay[i].writeSampleAndAdvance(mat_res[i] + delay_in_ptr[i][cnt]);
            }
        } // end of feedback loop

        // output velvet decorrelation
        if (!RealIsNull(getParameter(VelvetOut))) {
            for (int i = 0; i < num_lines; ++i) {
                d->ivnd_out[i].processBlock(delay_out_ptr[i], delay_out_ptr[i], numSamples);
            }
        }

        // stereo late reverb sum
        vo::subtract(delay_out_ptr[0], delay_out_ptr[3], work_ptr[0], numSamples); // a[0] - a[3]
        vo::subtract(delay_out_ptr[2], delay_out_ptr[1], work_ptr[1], numSamples); // a[2] - a[1]
        for (int i = 4; i < num_lines; i += 4) {
            vo::add(work_ptr[0], delay_out_ptr[i], work_ptr[0], numSamples);    // + a[j + 0]
            vo::subtract(work_ptr[0], delay_out_ptr[i + 3], work_ptr[0], numSamples); // - a[j + 3]
            vo::add(work_ptr[1], delay_out_ptr[i + 2], work_ptr[1], numSamples); // + a[j + 2]
            vo::subtract(work_ptr[1], delay_out_ptr[i + 1], work_ptr[1], numSamples); // - a[j + 1]
        }
        vo::add(work_ptr[0], work_ptr[1], late_ptr[0], numSamples);  // a[0] - a[1] + a[2] - a[3] ...
        vo::subtract(work_ptr[0], work_ptr[1], late_ptr[1], numSamples); // a[0] + a[1] - a[2] - a[3] ...

        apply_smooth_gain(d->late_gain_smooth, late_ptr, late_ptr, 2, numSamples);
    } else {
        d->late_buffer.zeroOut();
    }

    // mix late and er
    if (!er_muted) {
        apply_smooth_gain(d->er_gain_smooth, er_ptr, er_ptr, 2, numSamples);
        for (int ch = 0; ch < 2; ++ch) {
            vo::add(late_ptr[ch], er_ptr[ch], late_ptr[ch], numSamples);
        }
    }

    // stereo spread
    float stereoSpreadFact = getParameter(StereoSpread) * 0.01f;
    auto stereo_1 = std::sqrt(0.5f * (1 + stereoSpreadFact));
    auto stereo_2 = _sqrt_sign(0.5f * (1 - stereoSpreadFact));

    d->work_buffer.assignSamples(d->late_buffer);
    vo::constantMultiply(work_ptr[0], stereo_1, late_ptr[0], numSamples);
    vo::constantMultiplyAndAdd(work_ptr[1], stereo_2, late_ptr[0], numSamples);
    vo::constantMultiply(work_ptr[1], stereo_1, late_ptr[1], numSamples);
    vo::constantMultiplyAndAdd(work_ptr[0], stereo_2, late_ptr[1], numSamples);

    // filtering
    if (!RealIsNull(getParameter(PeakGain))) {
        d->peakFilter.process(late_ptr, 2, numSamples);
    }

    if (getParameter(LowCutFreq) > m_processor._param[Params::LowCutFreq].valueRange.first) {
        d->loCutFilter.process(late_ptr, 2, numSamples);
    }
    if (getParameter(HighCutFreq) < m_processor._param[Params::HighCutFreq].valueRange.second) {
        d->hiCutFilter.process(late_ptr, 2, numSamples);
    }

    // extra dispersion on the entire reverb signal
    d->disp_ap.process(work_ptr, 2, numSamples);

    // output mix
    apply_smooth_gain(d->dry_gain_smooth, signal_in, work_ptr, 2, numSamples);
    vo::add(work_ptr[0], late_ptr[0], signal_out[0], numSamples);
    vo::add(work_ptr[1], late_ptr[1], signal_out[1], numSamples);
}

void ReverbProcessor::Processor::allocateParameters(int num)
{
    _param.resize(num);
}

void ReverbProcessor::Processor::setupParameter(int index, const std::string& name,
                                                std::pair<float, float> valueRange, float initialValue)
{
    if (index >= static_cast<int>(_param.size())) {
        _param.resize(index + 1);
    }

    _param[index].name = name;
    _param[index].valueRange = valueRange;
    _param[index].currentValue = initialValue;
}

bool ReverbProcessor::Processor::setFormat(audioch_t audioChannelsCount, double sampleRate, int32_t maximumBlockSize)
{
    assert(sampleRate > 0.0);
    _sampleRate = sampleRate;
    _sampleT = 1.0 / _sampleRate;
    _audioChannelsCount = audioChannelsCount;
    _blockSize = maximumBlockSize;
    return true;
}
}
