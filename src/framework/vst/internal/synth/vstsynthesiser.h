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

#include "async/asyncable.h"
#include "audio/isynthesizer.h"
#include "audio/iaudioconfiguration.h"
#include "audio/audiotypes.h"
#include "modularity/ioc.h"

#include "internal/vstaudioclient.h"
#include "ivstpluginsregister.h"
#include "ivstmodulesrepository.h"
#include "vsttypes.h"

namespace mu::vst {
class VstSynthesiser : public audio::synth::ISynthesizer, public async::Asyncable
{
    INJECT(vst, IVstPluginsRegister, pluginsRegister)
    INJECT(vst, IVstModulesRepository, modulesRepo)
    INJECT(vst, audio::IAudioConfiguration, config)

public:
    explicit VstSynthesiser(VstPluginPtr&& pluginPtr, const audio::AudioInputParams& params);

    Ret init() override;

    bool isValid() const override;
    bool isActive() const override;
    void setIsActive(bool arg) override;

    audio::AudioSourceType type() const override;
    std::string name() const override;

    const audio::AudioInputParams& params() const override;
    async::Channel<audio::AudioInputParams> paramsChanged() const override;

    bool handleEvent(const midi::Event& e) override;
    void flushSound() override;

    Ret setupSound(const std::vector<midi::Event>& events) override;

    // IAudioSource
    void setSampleRate(unsigned int sampleRate) override;
    unsigned int audioChannelsCount() const override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;
    audio::samples_t process(float* buffer, audio::samples_t samplelPerChannel) override;

private:
    VstPluginPtr m_pluginPtr = nullptr;

    std::unique_ptr<VstAudioClient> m_vstAudioClient = nullptr;

    bool m_isActive = false;

    audio::AudioInputParams m_params;

    async::Channel<audio::AudioInputParams> m_paramsChanges;
    async::Channel<unsigned int> m_streamsCountChanged;
};

using VstSynthPtr = std::shared_ptr<VstSynthesiser>;
}

#endif // MU_VST_VSTSYNTHESISER_H
