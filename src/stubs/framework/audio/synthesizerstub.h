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
#ifndef MU_AUDIO_SYNTHESIZERSTUB_H
#define MU_AUDIO_SYNTHESIZERSTUB_H

#include "audio/abstractsynthesizer.h"

namespace mu::audio::synth {
class SynthesizerStub : public AbstractSynthesizer
{
public:
    SynthesizerStub(const audio::AudioSourceParams& params);

    void setSampleRate(unsigned int sampleRate) override;

    unsigned int audioChannelsCount() const override;

    async::Channel<unsigned int> audioChannelsCountChanged() const override;

    samples_t process(float* buffer, samples_t samplesPerChannel) override;

    std::string name() const override;
    AudioSourceType type() const override;

    msecs_t playbackPosition() const override;
    void setPlaybackPosition(const msecs_t newPosition) override;

    void flushSound() override;

    bool isValid() const override;
    bool isActive() const override;
    void setIsActive(bool arg) override;

private:
    void setupSound(const mpe::PlaybackSetupData& setupData) override;
    void setupEvents(const mpe::PlaybackData& playbackData) override;
};
}

#endif // MU_AUDIO_SYNTHESIZERSTUB_H
