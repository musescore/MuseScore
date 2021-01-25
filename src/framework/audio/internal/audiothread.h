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

#ifndef MU_AUDIO_AUDIOTHREAD_H
#define MU_AUDIO_AUDIOTHREAD_H

#include <memory>
#include <thread>
#include <atomic>
#include <functional>

#include "iaudiobuffer.h"
#include "modularity/ioc.h"
#include "rpc/queuedrpcchannel.h"

namespace mu::audio {
class AudioThread
{
public:
    AudioThread();
    ~AudioThread();

    using OnStart = std::function<void ()>;
    using OnFinished = std::function<void ()>;

    void run(const OnStart& onStart = nullptr);
    void stop(const OnFinished& onFinished = nullptr);

    void setAudioBuffer(std::shared_ptr<IAudioBuffer> buffer);

    //! use if you don't want to use internal thread
    void loopBody();

    rpc::QueuedRpcChannelPtr channel() const;

private:
    void main();

    OnStart m_onStart;
    OnFinished m_onFinished;
    rpc::QueuedRpcChannelPtr m_channel;
    std::shared_ptr<rpc::RpcControllers> m_controller;
    std::shared_ptr<IAudioBuffer> m_buffer = nullptr;
    std::shared_ptr<std::thread> m_thread = nullptr;
    std::atomic<bool> m_running = false;
};
}

#endif // MU_AUDIO_AUDIOTHREAD_H
