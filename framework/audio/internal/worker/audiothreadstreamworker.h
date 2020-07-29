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

#ifndef MU_AUDIO_AUDIOTHREADSTREAMWORKER_H
#define MU_AUDIO_AUDIOTHREADSTREAMWORKER_H

#include <memory>
#include <thread>
#include <atomic>

#include "queuedrpcstreamchannel.h"

namespace mu {
namespace audio {
namespace worker {
class RpcStreamController;
class AudioThreadStreamWorker
{
public:
    AudioThreadStreamWorker(const std::shared_ptr<QueuedRpcStreamChannel>& chan);
    ~AudioThreadStreamWorker();

    void run();
    void stop();

private:

    static void AudioStreamProcess(AudioThreadStreamWorker* self);
    void doAudioStreamProcess();

    std::shared_ptr<QueuedRpcStreamChannel> m_channel;
    std::shared_ptr<RpcStreamController> m_controller;
    std::shared_ptr<std::thread> m_thread;
    std::atomic<bool> m_running{ false };
};
}
}
}

#endif // MU_AUDIO_AUDIOTHREADSTREAMWORKER_H
