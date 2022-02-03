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
#ifndef MU_AUDIO_SANITYSYNTHESIZER_H
#define MU_AUDIO_SANITYSYNTHESIZER_H

#include "isynthesizer.h"

namespace mu::audio::synth {
class SanitySynthesizer : public ISynthesizer
{
public:
    SanitySynthesizer(ISynthesizerPtr synth);

    bool isValid() const override;

    std::string name() const override;
    AudioSourceType type() const override;

    Ret init() override;

    bool isActive() const override;
    void setIsActive(bool arg) override;

    Ret setupSound(const std::vector<midi::Event>& events) override;
    bool handleEvent(const midi::Event& e) override;
    void flushSound() override;

    // IAudioSource
    void setSampleRate(unsigned int sampleRate) override;
    unsigned int audioChannelsCount() const override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;
    samples_t process(float* buffer, samples_t sampleCount) override;

private:

    ISynthesizerPtr m_synth;
};
}

#endif // MU_AUDIO_SANITYSYNTHESIZER_H
