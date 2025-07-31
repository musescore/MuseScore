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
#ifndef MUSE_AUDIO_ABSTRACTAUDIOSOURCE_H
#define MUSE_AUDIO_ABSTRACTAUDIOSOURCE_H

#include "../iaudiosource.h"

namespace muse::audio::worker {
class AbstractAudioSource : public IAudioSource
{
public:
    virtual ~AbstractAudioSource() = default;

    virtual void setSampleRate(unsigned int sampleRate) override;

    bool isActive() const override;
    void setIsActive(bool arg) override;

    async::Channel<unsigned int> audioChannelsCountChanged() const override;

protected:
    unsigned int m_sampleRate = 1;
    async::Channel<unsigned int> m_streamsCountChanged;
    bool m_isActive = false;
};
}

#endif // MUSE_AUDIO_ABSTRACTAUDIOSOURCE_H
