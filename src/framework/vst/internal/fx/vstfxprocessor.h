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

#include <memory>

#include "async/asyncable.h"

#include "audio/engine/ifxprocessor.h"

#include "../vstaudioclient.h"
#include "vsttypes.h"

namespace muse::vst {
class VstFxProcessor : public muse::audio::IFxProcessor, public async::Asyncable
{
public:
    explicit VstFxProcessor(IVstPluginInstancePtr&& instance, const muse::audio::AudioFxParams& params);

    void init(const audio::OutputSpec& spec);

    muse::audio::AudioFxType type() const override;
    const muse::audio::AudioFxParams& params() const override;
    async::Channel<muse::audio::AudioFxParams> paramsChanged() const override;
    void setOutputSpec(const audio::OutputSpec& spec) override;
    bool active() const override;
    void setActive(bool active) override;
    void process(float* buffer, unsigned int sampleCount) override;

private:
    bool m_inited = false;

    IVstPluginInstancePtr m_pluginPtr = nullptr;
    std::unique_ptr<VstAudioClient> m_vstAudioClient = nullptr;

    muse::audio::AudioFxParams m_params;
    async::Channel<muse::audio::AudioFxParams> m_paramsChanges;

    audio::OutputSpec m_outputSpec;
};

using VstFxPtr = std::shared_ptr<VstFxProcessor>;
}
