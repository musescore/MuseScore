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
#ifndef MUSE_VST_VSTFXPROCESSOR_H
#define MUSE_VST_VSTFXPROCESSOR_H

#include <memory>

#include "async/asyncable.h"
#include "audio/ifxprocessor.h"
#include "audio/iaudioconfiguration.h"

#include "internal/vstaudioclient.h"
#include "internal/vstplugin.h"
#include "vsttypes.h"

namespace muse::vst {
class VstFxProcessor : public muse::audio::IFxProcessor, public async::Asyncable
{
    INJECT(muse::audio::IAudioConfiguration, config)
public:
    explicit VstFxProcessor(VstPluginPtr&& pluginPtr, const muse::audio::AudioFxParams& params);

    void init();

    muse::audio::AudioFxType type() const override;
    const muse::audio::AudioFxParams& params() const override;
    async::Channel<muse::audio::AudioFxParams> paramsChanged() const override;
    void setSampleRate(unsigned int sampleRate) override;
    bool active() const override;
    void setActive(bool active) override;
    void process(float* buffer, unsigned int sampleCount) override;

private:
    bool m_inited = false;

    VstPluginPtr m_pluginPtr = nullptr;
    std::unique_ptr<VstAudioClient> m_vstAudioClient = nullptr;

    muse::audio::AudioFxParams m_params;
    async::Channel<muse::audio::AudioFxParams> m_paramsChanges;
};

using VstFxPtr = std::shared_ptr<VstFxProcessor>;
}

#endif // MUSE_VST_VSTFXPROCESSOR_H
