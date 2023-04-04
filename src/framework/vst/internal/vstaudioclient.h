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
#ifndef MU_VST_VSTAUDIOCLIENT_H
#define MU_VST_VSTAUDIOCLIENT_H

#include "audio/audiotypes.h"
#include "mpe/events.h"

#include "vstplugin.h"
#include "vsttypes.h"

namespace mu::vst {
class VstAudioClient
{
public:
    VstAudioClient() = default;
    ~VstAudioClient();

    void init(audio::AudioPluginType type, VstPluginPtr plugin, audio::audioch_t&& audioChannelsCount = 2);

    bool handleEvent(const VstEvent& event);
    bool handleParamChange(const PluginParamInfo& param);
    void setVolumeGain(const audio::gain_t newVolumeGain);

    audio::samples_t process(float* output, audio::samples_t samplesPerChannel);
    void flush();

    void setBlockSize(unsigned int samples);
    void setSampleRate(unsigned int sampleRate);

    ParamsMapping paramsMapping(const std::set<Steinberg::Vst::CtrlNumber>& controllers) const;

private:
    struct SamplesInfo {
        unsigned int sampleRate = 0;
        unsigned int samplesPerBlock = 0;

        bool isValid()
        {
            return sampleRate > 0 && samplesPerBlock > 0;
        }
    };

    IAudioProcessorPtr pluginProcessor() const;
    PluginComponentPtr pluginComponent() const;

    void setUpProcessData();
    void updateProcessSetup();
    void extractInputSamples(const audio::samples_t& sampleCount, const float* sourceBuffer);
    bool fillOutputBuffer(unsigned int samples, float* output);

    void ensureActivity();
    void disableActivity();

    void flushBuffers();

    bool m_isActive = false;
    audio::gain_t m_volumeGain = 1.f; // 0.0 - 1.0

    VstPluginPtr m_pluginPtr = nullptr;
    mutable PluginComponentPtr m_pluginComponent = nullptr;

    SamplesInfo m_samplesInfo;

    std::vector<int> m_activeOutputBusses;
    std::vector<int> m_activeInputBusses;

    VstEventList m_eventList;
    VstParameterChanges m_paramChanges;
    VstProcessData m_processData;
    VstProcessContext m_processContext;

    bool m_needUnprepareProcessData = false;

    audio::AudioPluginType m_type = audio::AudioPluginType::Undefined;
    audio::audioch_t m_audioChannelsCount = 0;
};
}

#endif // MU_VST_VSTAUDIOCLIENT_H
