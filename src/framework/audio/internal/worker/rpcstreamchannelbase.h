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

#ifndef MU_AUDIO_RPCSTREAMCHANNELBASE_H
#define MU_AUDIO_RPCSTREAMCHANNELBASE_H

#include <vector>
#include <cstdint>
#include <map>
#include <mutex>

#include "irpcaudiostreamchannel.h"

namespace mu {
namespace audio {
namespace worker {
class RpcStreamChannelBase : public IRpcAudioStreamChannel
{
public:
    RpcStreamChannelBase();
    ~RpcStreamChannelBase() override;

    StreamID newID() override;

    // Rpc
    void send(const StreamID& id, CallID method, const Args& args) override;
    void listen(const StreamID& id, Handler h) override;
    void unlisten(const StreamID& id) override;
    void listenAll(HandlerAll h) override;
    void unlistenAll() override;

    // Audio
    void registerStream(const StreamID& id,uint16_t samples, uint16_t channels,GetBuffer getBuffer,
                        OnRequestFinished onRequestFinished) override;

    void unregisterStream(const StreamID& id) override;

    void requestAudio(const StreamID& id) override;
    void onGetAudio(const GetAudio& func) override;

protected:

    // Rpc
    virtual void doSend(const StreamID& id, CallID method, const Args& args) = 0;
    virtual void doListen(const StreamID& id, Handler h) = 0;
    virtual void doUnlisten(const StreamID& id) = 0;
    virtual void doListenAll(HandlerAll h) = 0;
    virtual void doUnlistenAll() = 0;

    // Audio

    enum class RequestState {
        FREE = 0,
        REQUESTED,
        WRITED,
    };

    struct Stream {
        StreamID id;
        uint16_t samples = 0;
        uint16_t channels = 0;
        uint16_t bufSize = 0;
        RequestState state = RequestState::FREE;
        std::vector<float> buf;
        Context ctx;

        GetBuffer getBuffer;
        OnRequestFinished onRequestFinished;

        uint32_t bufSizeInBytes() const
        {
            return sizeof(float) * samples * channels;
        }
    };

    virtual void onStreamRegistred(std::shared_ptr<Stream>& stream);
    virtual void onStreamUnregistred(const StreamID& id);
    virtual void onRequestAudio(const StreamID& id);

    const std::shared_ptr<Stream>& stream(const StreamID& id) const;
    bool allStreamsInState(const RequestState& state) const;

    GetAudio m_getAudio;
    std::recursive_mutex m_mutex;
    std::map<StreamID, std::shared_ptr<Stream> > m_streams;

private:

    StreamID m_lastID = 0;
};
}
}
}

#endif // MU_AUDIO_RPCSTREAMCHANNELBASE_H
