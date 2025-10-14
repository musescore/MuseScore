/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "../iaudioengineconfiguration.h"

#include "audio/common/audiotypes.h"

namespace muse::audio::engine {
class AudioEngineConfiguration : public IAudioEngineConfiguration
{
public:
    AudioEngineConfiguration() = default;

    void setConfig(const AudioEngineConfig& conf) override;

    bool autoProcessOnlineSoundsInBackground() const override;
    async::Channel<bool> autoProcessOnlineSoundsInBackgroundChanged() const override;

    AudioInputParams defaultAudioInputParams() const override;

    size_t desiredAudioThreadNumber() const override;
    size_t minTrackCountForMultithreading() const override;

private:

    AudioEngineConfig m_conf;

    async::Channel<bool> m_autoProcessOnlineSoundsInBackgroundChanged;
};
}
