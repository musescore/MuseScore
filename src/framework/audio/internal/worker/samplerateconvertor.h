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
#ifndef MUSE_AUDIO_SAMPLERATECONVERTOR_H
#define MUSE_AUDIO_SAMPLERATECONVERTOR_H

#include <vector>
#include <deque>

namespace muse::audio {
class SampleRateConvertor
{
public:
    enum Method {
        SINC, FIR
    };
    explicit SampleRateConvertor(const std::vector<float>& data, unsigned int channelsCount, unsigned int sampleRateIn,
                                 unsigned int sampleRateOut);

    //! offline convert full data set
    std::vector<float> convert();

    //! online convert
    unsigned int convert(float* buffer, unsigned int from, unsigned int count);

    void setChannelCount(unsigned int count);
    void setSampleRateIn(unsigned int sampleRate);
    void setSampleRateOut(unsigned int sampleRate);

private:

    //! return value of sinc function (LPF) with consideration of used sampleRates
    double sinc(double value) const;

    //! output sample value
    float y(unsigned int sample, unsigned int channel) const;
    float ySinc(unsigned int sample, unsigned int channel) const;
    float yFIR(unsigned int sample, unsigned int channel) const;

    //! return true if there are samples in input buffer for conversion
    bool availableSamples(unsigned int sample) const;

    //! calculate window function values
    void initWindow();

    const static unsigned int USE_SAMPLES = 16; //!< this value defines the quality and complexity of SRC.
    const std::vector<float>& m_data;

    //! values of window function for USE_SAMPLES
    const static unsigned int FIR_LENGTH = 33;
    unsigned int m_M = 1, m_L = 1;
    std::vector<float> m_fir;
    mutable std::deque<float> m_y;

    unsigned int m_channelsCount;
    unsigned int m_sampleRateIn;
    unsigned int m_sampleRateOut;
    Method m_method = FIR;
};
}

#endif // MUSE_AUDIO_SAMPLERATECONVERTOR_H
