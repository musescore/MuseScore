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
#ifndef MU_AUDIO_AUDIODRIVERSTUB_H
#define MU_AUDIO_AUDIODRIVERSTUB_H

#include "audio/iaudiodriver.h"

namespace muse::audio {
class AudioDriverStub : public IAudioDriver
{
public:
    void init() override;

    std::string name() const override;
    bool open(const Spec& spec, Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;

    const Spec& activeSpec() const override;

    AudioDeviceID outputDevice() const override;
    bool selectOutputDevice(const AudioDeviceID& id) override;
    bool resetToDefaultOutputDevice() override;
    async::Notification outputDeviceChanged() const override;

    AudioDeviceList availableOutputDevices() const override;
    async::Notification availableOutputDevicesChanged() const override;
    bool pushMidiEvent(muse::midi::Event& e) override;
    std::vector<muse::midi::MidiDevice> availableMidiDevices(muse::midi::MidiPortDirection direction) const override;

    int audioDelayCompensate() const override;
    void setAudioDelayCompensate(const int frames) override;

    unsigned int sampleRate() const override;
    bool setSampleRate(unsigned int sampleRate) override;
    async::Notification sampleRateChanged() const override;

    unsigned int outputDeviceBufferSize() const override;
    bool setOutputDeviceBufferSize(unsigned int bufferSize) override;
    async::Notification outputDeviceBufferSizeChanged() const override;

    std::vector<unsigned int> availableOutputDeviceBufferSizes() const override;

    unsigned int outputDeviceSampleRate() const override;
    bool setOutputDeviceSampleRate(unsigned int bufferSize) override;
    async::Notification outputDeviceSampleRateChanged() const override;

    std::vector<unsigned int> availableOutputDeviceSampleRates() const override;

    void resume() override;
    void suspend() override;
};
}

#endif // MU_AUDIO_AUDIODRIVERSTUB_H
