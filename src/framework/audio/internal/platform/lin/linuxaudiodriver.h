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

#ifndef MUSE_AUDIO_LINUXAUDIODRIVER_H
#define MUSE_AUDIO_LINUXAUDIODRIVER_H

#include "async/asyncable.h"

#include "framework/midi/imidiinport.h"
#include "framework/midi/midimodule.h"
#include "iaudiodriver.h"

#include "audiodeviceslistener.h"
#include "playback/iplaybackcontroller.h"

namespace muse::audio {
class LinuxAudioDriver : public IAudioDriver, public async::Asyncable
{
    Inject<mu::playback::IPlaybackController> playbackController;
    Inject<muse::midi::IMidiInPort> midiInPort;
public:
    LinuxAudioDriver();
    ~LinuxAudioDriver();

    void init() override;

    std::string name() const override;
    bool open(const Spec& spec, Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;

    const Spec& activeSpec() const override;

    AudioDeviceID outputDevice() const override;
    bool selectOutputDevice(const AudioDeviceID& deviceId) override;
    bool resetToDefaultOutputDevice() override;
    async::Notification outputDeviceChanged() const override;

    AudioDeviceList availableOutputDevices() const override;
    async::Notification availableOutputDevicesChanged() const override;

    unsigned int outputDeviceBufferSize() const override;
    bool setOutputDeviceBufferSize(unsigned int bufferSize) override;
    async::Notification outputDeviceBufferSizeChanged() const override;

    std::vector<unsigned int> availableOutputDeviceBufferSizes() const override;

    int audioDelayCompensate() const override;
    void setAudioDelayCompensate(const int frames) override;

    bool pushMidiEvent(muse::midi::Event& e) override;
    std::vector<muse::midi::MidiDevice> availableMidiDevices(muse::midi::MidiPortDirection direction) const override;

    unsigned int outputDeviceSampleRate() const override;
    bool setOutputDeviceSampleRate(unsigned int sampleRate) override;
    async::Notification outputDeviceSampleRateChanged() const override;

    std::vector<unsigned int> availableOutputDeviceSampleRates() const override;

    void resume() override;
    void suspend() override;

private:
    bool makeDevice(const AudioDeviceID& deviceId);
    bool reopen(const AudioDeviceID& deviceId, Spec newSpec);
    async::Notification m_outputDeviceChanged;

    mutable std::mutex m_devicesMutex;
    AudioDevicesListener m_devicesListener;
    async::Notification m_availableOutputDevicesChanged;

    std::string m_deviceId;

    async::Notification m_bufferSizeChanged;
    async::Notification m_sampleRateChanged;
    int m_audioDelayCompensate;

    struct IAudioDriver::Spec m_spec;
    std::unique_ptr<AudioDriverState> m_current_audioDriverState;
};
}

#endif // MUSE_AUDIO_LINUXAUDIODRIVER_H
