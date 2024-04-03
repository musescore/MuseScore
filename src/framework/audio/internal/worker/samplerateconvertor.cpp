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
#include "samplerateconvertor.h"

#include <cmath>

#include "log.h"

using namespace muse::audio;

SampleRateConvertor::SampleRateConvertor(const std::vector<float>& data,
                                         unsigned int channelsCount,
                                         unsigned int sampleRateIn,
                                         unsigned int sampleRateOut)
    : m_data(data), m_fir(), m_channelsCount(channelsCount), m_sampleRateIn(sampleRateIn), m_sampleRateOut(sampleRateOut)
{
    m_fir.resize(FIR_LENGTH, 0);
    m_y.resize(FIR_LENGTH, 0);
    initWindow();
}

std::vector<float> SampleRateConvertor::convert()
{
    std::vector<float> out;
    auto resultSamples = m_data.size() * m_sampleRateOut / (m_channelsCount * m_sampleRateIn);

    out.resize(resultSamples * m_channelsCount);
    for (unsigned sample = 0; sample < resultSamples; ++sample) {
        for (unsigned int channel = 0; channel < m_channelsCount; ++channel) {
            out[sample * m_channelsCount + channel] = y(sample, channel);
        }
    }

    return out;
}

unsigned int SampleRateConvertor::convert(float* buffer, unsigned int from, unsigned int count)
{
    unsigned int sample = from;
    unsigned int converted = 0;

    for (converted = 0; converted < count; ++converted, ++sample) {
        for (unsigned int channel = 0; channel < m_channelsCount; ++channel) {
            if (!availableSamples(sample)) {
                return converted;
            }
            buffer[converted * m_channelsCount + channel] = y(sample, channel);
        }
    }
    return converted;
}

float SampleRateConvertor::y(unsigned int sample, unsigned int channel) const
{
    switch (m_method) {
    case SINC:
        return ySinc(sample, channel);
    case FIR:
        return yFIR(sample, channel);
    }
    return 0.f;
}

void SampleRateConvertor::setChannelCount(unsigned int count)
{
    m_channelsCount = count;
}

void SampleRateConvertor::setSampleRateIn(unsigned int sampleRate)
{
    if (m_sampleRateIn != sampleRate) {
        m_sampleRateIn = sampleRate;
        initWindow();
    }
}

void SampleRateConvertor::setSampleRateOut(unsigned int sampleRate)
{
    if (m_sampleRateOut != sampleRate) {
        m_sampleRateOut = sampleRate;
        initWindow();
    }
}

float SampleRateConvertor::ySinc(unsigned int sample, unsigned int channel) const
{
    double currentOutputSampleTime = sample / static_cast<double>(m_sampleRateOut);
    auto zeroInputSample = std::floor(currentOutputSampleTime * m_sampleRateIn);

    float out = 0.f;
    for (unsigned int i = 0; i < USE_SAMPLES; ++i) {
        int currentSample = zeroInputSample + i - USE_SAMPLES / 2;
        double currentInputSampleTime = currentSample / static_cast<double>(m_sampleRateIn);

        if (currentSample < 0 || currentSample * m_channelsCount + channel >= m_data.size()) {
            continue;
        }
        out += m_data.at(currentSample * m_channelsCount + channel) * sinc(currentOutputSampleTime - currentInputSampleTime);
    }
    return out;
}

float SampleRateConvertor::yFIR(unsigned int sample, unsigned int channel) const
{
    auto x = [this, &channel](int m) -> float {
        int pos = m * m_M / m_L * m_channelsCount + channel;
        if (pos >= 0 && pos < static_cast<int>(m_data.size())) {
            return m_data.at(pos);
        }
        return 0.f;
    };

    float y = 0.f;
    for (unsigned int i = 0; i < FIR_LENGTH; ++i) {
        int t = sample - static_cast<int>(i);
        y += x(t) * m_fir[i];
    }
    return y;
}

bool SampleRateConvertor::availableSamples(unsigned int sample) const
{
    float currentOutputSampleTime = sample / static_cast<float>(m_sampleRateOut);
    auto firstInputSample = std::floor(currentOutputSampleTime * m_sampleRateIn) - USE_SAMPLES / 2;

    // first sample for conversion points out of the input buffer
    if (firstInputSample * m_channelsCount >= m_data.size()) {
        return false;
    }
    return true;
}

inline double zeroBessel(double x)
{
    //return std::cyl_bessel_i(0, x);

    double s = 1, y = 1;
    int m = 1;

    while (y > std::numeric_limits<float>::min()) {
        m += 2;
        y *= x * x / (4 * m * m);
        s += s * y;
    }

    return s;
}

void SampleRateConvertor::initWindow()
{
    int max = std::max(m_sampleRateIn, m_sampleRateOut), min = std::min(m_sampleRateIn, m_sampleRateOut);
    int maxCommonDivider = 1;
    while (max % min != 0) {
        maxCommonDivider = max % min;
        max = min;
        min = maxCommonDivider;
    }
    m_M = m_sampleRateIn / maxCommonDivider;
    m_L = m_sampleRateOut / maxCommonDivider;
    double fStop = std::min(m_sampleRateIn, m_sampleRateOut) / 2,
           fIntermediateSampleRate = m_sampleRateIn * m_M,
           attenuation = 96 /*dB*/;
    int M = FIR_LENGTH;
    int Np = (M - 1) / 2;

    double alpha = 0.1102 * (attenuation - 8.7);
    double A[FIR_LENGTH];

    A[0] = 2 * fStop / fIntermediateSampleRate;
    for (int j = 1; j <= Np; j++) {
        A[j]  = std::sin(2 * j * M_PI * fStop / fIntermediateSampleRate) / j * M_PI;
    }

    for (int j = 0; j <= Np; j++) {
        m_fir[Np + j]  = A[j];
        m_fir[Np + j] *= zeroBessel(alpha * std::sqrt(1 - (j * j / (Np * Np))));
        m_fir[Np + j] /= zeroBessel(alpha);
    }

    for (int j = 0; j < Np; j++) {
        m_fir[j] = m_fir[M - 1 - j];
    }
}

double SampleRateConvertor::sinc(double value) const
{
    if (value == 0) {
        return 1.f;
    }
    double sincArg = M_PI * value * m_sampleRateIn;
    double gain = std::min(1.0, m_sampleRateOut / static_cast<double>(m_sampleRateIn));
    return gain * std::sin(sincArg) / sincArg;
}
