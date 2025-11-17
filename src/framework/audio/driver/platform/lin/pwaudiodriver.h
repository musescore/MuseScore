/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#ifndef MUSE_AUDIO_PWAUDIODRIVER_H
#define MUSE_AUDIO_PWAUDIODRIVER_H

#include "async/asyncable.h"

#include "iaudiodriver.h"

#include "audiodeviceslistener.h"

struct pw_thread_loop;
struct pw_context;
struct pw_core;

namespace muse::audio {
class PwRegistry;
class PwStream;

class PwAudioDriver : public IAudioDriver, public async::Asyncable
{
public:
    PwAudioDriver();
    ~PwAudioDriver();

    bool connectedToPwServer() const
    {
        return m_core != nullptr;
    }

    void init() override;

    std::string name() const override;

    AudioDeviceID defaultDevice() const override;

    bool open(const Spec& spec, Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;

    const Spec& activeSpec() const override;
    async::Channel<Spec> activeSpecChanged() const override;

    AudioDeviceList availableOutputDevices() const override;
    async::Notification availableOutputDevicesChanged() const override;
    std::vector<samples_t> availableOutputDeviceBufferSizes() const override;
    std::vector<sample_rate_t> availableOutputDeviceSampleRates() const override;

private:

    Spec m_formatSpec;
    async::Channel<Spec> m_activeSpecChanged;

    pw_thread_loop* m_loop = nullptr;
    pw_context* m_context = nullptr;
    pw_core* m_core = nullptr;

    std::unique_ptr<PwRegistry> m_registry;
    std::unique_ptr<PwStream> m_stream;
};
}

#endif // MUSE_AUDIO_LINUXAUDIODRIVER_H
