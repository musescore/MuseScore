//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_AUDIO_COREAUDIODRIVER_H
#define MU_AUDIO_COREAUDIODRIVER_H

#include "iaudiodriver.h"
#include <thread>
#include <atomic>

namespace mu::audio {
class CoreAudioDriver : public IAudioDriver
{
public:
    CoreAudioDriver();
    ~CoreAudioDriver();

    std::string name() const override;
    bool open(const Spec& spec, Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;
    std::string outputDevice() const override;
    bool selectOutputDevice(const std::string& name) override;
    std::vector<std::string> availableOutputDevices() const override;
    async::Notification availableOutputDevicesChanged() const override;
    void resume() override;
    void suspend() override;

private:
    void clean();

    std::atomic<bool> m_active { false };
    std::thread m_thread;
};
}

#endif // MU_AUDIO_COREAUDIODRIVER_H
