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
#ifndef MU_AUDIO_IRPCAUDIOSTREAMCHANNEL_H
#define MU_AUDIO_IRPCAUDIOSTREAMCHANNEL_H

#include <string>
#include <functional>

#include "modularity/imoduleexport.h"

#include "workertypes.h"

namespace mu {
namespace audio {
namespace worker {
class IRpcAudioStreamChannel : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IRpcAudioStreamChannel)

public:

    virtual ~IRpcAudioStreamChannel() = default;

    virtual StreamID newID() = 0;

    // Rpc
    virtual void send(const StreamID& id, CallID method, const Args& args) = 0;

    using Handler = std::function<void (CallID method, const Args& args)>;
    using HandlerAll = std::function<void (const StreamID& id, CallID method, const Args& args)>;

    using GetBuffer = std::function<float* (uint32_t samples, Context& ctx)>;
    using OnRequestFinished = std::function<void ()>;
    using GetAudio = std::function<void (const StreamID& id, float* buf, uint32_t samples, uint32_t bufSize,
                                         Context* ctx)>;

    virtual void listen(const StreamID& id, Handler h) = 0;
    virtual void unlisten(const StreamID& id) = 0;

    virtual void listenAll(HandlerAll h) = 0;
    virtual void unlistenAll() = 0;

    // Audio
    virtual void registerStream(const StreamID& id,uint16_t samples, uint16_t channels, GetBuffer getBuffer,
                                OnRequestFinished onRequestFinished) = 0;

    virtual void unregisterStream(const StreamID& id) = 0;

    virtual void requestAudio(const StreamID& id) = 0;

    virtual void onGetAudio(const GetAudio& func) = 0;
};
}
}
}

#endif // MU_AUDIO_IRPCAUDIOSTREAMCHANNEL_H
