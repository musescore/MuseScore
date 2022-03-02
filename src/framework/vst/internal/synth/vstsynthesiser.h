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

#ifndef MU_VST_VSTSYNTHESISER_H
#define MU_VST_VSTSYNTHESISER_H

#include <memory>

#include "audio/abstractsynthesizer.h"
#include "audio/iaudioconfiguration.h"
#include "audio/audiotypes.h"
#include "modularity/ioc.h"
#include "mpe/events.h"

#include "internal/vstaudioclient.h"
#include "ivstpluginsregister.h"
#include "ivstmodulesrepository.h"
#include "vsttypes.h"

namespace mu::vst {
class VstSynthesiser : public audio::synth::AbstractSynthesizer
{
    INJECT(vst, IVstPluginsRegister, pluginsRegister)
    INJECT(vst, IVstModulesRepository, modulesRepo)
    INJECT(vst, audio::IAudioConfiguration, config)

public:
    explicit VstSynthesiser(VstPluginPtr&& pluginPtr, const audio::AudioInputParams& params);

    bool isValid() const override;

    audio::AudioSourceType type() const override;
    std::string name() const override;

    void revokePlayingNotes() override;
    void flushSound() override;

    bool hasAnythingToPlayback(const audio::msecs_t from, const audio::msecs_t to) const;

    void setupSound(const mpe::PlaybackSetupData& setupData) override;

    // IAudioSource
    void setSampleRate(unsigned int sampleRate) override;
    unsigned int audioChannelsCount() const override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;
    audio::samples_t process(float* buffer, audio::samples_t samplesPerChannel) override;

private:
    void handleMainStreamEvents(const audio::msecs_t nextMsecs);
    void handleOffStreamEvents(const audio::msecs_t nextMsecs);

    void handleAlreadyPlayingEvents(const audio::msecs_t from, const audio::msecs_t to);

    Ret init();

    VstPluginPtr m_pluginPtr = nullptr;

    std::unique_ptr<VstAudioClient> m_vstAudioClient = nullptr;

    async::Channel<unsigned int> m_streamsCountChanged;

    std::list<mpe::PlaybackEvent> m_playingEvents;
};

using VstSynthPtr = std::shared_ptr<VstSynthesiser>;
}

#endif // MU_VST_VSTSYNTHESISER_H
