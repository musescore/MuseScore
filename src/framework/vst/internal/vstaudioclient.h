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
#ifndef MUSE_VST_VSTAUDIOCLIENT_H
#define MUSE_VST_VSTAUDIOCLIENT_H

#include "audioplugins/audiopluginstypes.h"

#include "../ivstplugininstance.h"
#include "../vsttypes.h"

namespace muse::vst {
class VstAudioClient
{
public:
    VstAudioClient() = default;
    ~VstAudioClient();

    void init(audioplugins::AudioPluginType type, IVstPluginInstancePtr instance, muse::audio::audioch_t audioChannelsCount = 2);

    void loadSupportedParams();

    bool handleEvent(const VstEvent& event);
    bool handleParamChange(const ParamChangeEvent& param);
    void setVolumeGain(const muse::audio::gain_t newVolumeGain);

    muse::audio::samples_t process(float* output, muse::audio::samples_t samplesPerChannel, muse::audio::msecs_t playbackPosition = 0);

    void flush();
    void allNotesOff();

    audio::samples_t maxSamplesPerBlock() const;
    void setMaxSamplesPerBlock(audio::samples_t samples);

    void setSampleRate(unsigned int sampleRate);

    ParamsMapping paramsMapping(const std::set<Steinberg::Vst::CtrlNumber>& controllers) const;

private:
    struct SamplesInfo {
        unsigned int sampleRate = 0;
        audio::samples_t maxSamplesPerBlock = 0;

        bool isValid()
        {
            return sampleRate > 0 && maxSamplesPerBlock > 0;
        }
    };

    IAudioProcessorPtr pluginProcessor() const;
    PluginComponentPtr pluginComponent() const;

    void setUpProcessData();
    void updateProcessSetup();
    void extractInputSamples(muse::audio::samples_t sampleCount, const float* sourceBuffer);

    bool fillOutputBufferInstrument(muse::audio::samples_t sampleCount, float* output);
    bool fillOutputBufferFx(muse::audio::samples_t sampleCount, float* output);

    void ensureActivity();
    void disableActivity();

    void flushBuffers();

    void addParamChange(const ParamChangeEvent& param);

    bool m_isActive = false;
    muse::audio::gain_t m_volumeGain = 1.f; // 0.0 - 1.0

    IVstPluginInstancePtr m_pluginPtr = nullptr;
    mutable PluginComponentPtr m_pluginComponent = nullptr;

    SamplesInfo m_samplesInfo;

    std::vector<int> m_activeOutputBusses;
    std::vector<int> m_activeInputBusses;

    VstEventList m_eventList;
    VstParameterChanges m_paramChanges;
    VstProcessData m_processData;
    VstProcessContext m_processContext;

    std::unordered_map<size_t, VstEvent> m_playingNotes;
    std::unordered_set<PluginParamId> m_playingParams;

    std::unordered_map<PluginParamId, PluginParamInfo> m_pluginParamInfoMap;

    bool m_needUnprepareProcessData = false;

    audioplugins::AudioPluginType m_type = audioplugins::AudioPluginType::Undefined;
    audio::audioch_t m_audioChannelsCount = 0;
};
}

#endif // MUSE_VST_VSTAUDIOCLIENT_H
