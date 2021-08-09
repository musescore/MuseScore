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
#ifndef VSTAUDIOCLIENT_H
#define VSTAUDIOCLIENT_H

#include "modularity/ioc.h"
#include "audio/iaudioconfiguration.h"

#include "vstplugin.h"
#include "vsttypes.h"

namespace mu::vst {
class VstAudioClient
{
    INJECT(vst, audio::IAudioConfiguration, config)

public:
    VstAudioClient() = default;
    ~VstAudioClient();

    void init(VstPluginPtr plugin);

    bool handleEvent(const midi::Event& e);

    void process(float* output, unsigned int samples);

    void setBlockSize(unsigned int samples);
    void setSampleRate(unsigned int sampleRate);

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
    void fillOutputBuffer(unsigned int samples, float* output);

    bool convertMidiToVst(const mu::midi::Event& in, VstEvent& out) const;

    VstPluginPtr m_pluginPtr = nullptr;
    mutable PluginComponentPtr m_pluginComponent = nullptr;

    SamplesInfo m_samplesInfo;

    VstEventList m_eventList;
    VstProcessData m_processData;
    VstProcessContext m_processContext;
};
}

#endif // VSTAUDIOCLIENT_H
