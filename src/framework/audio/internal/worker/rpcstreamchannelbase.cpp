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

#include "rpcstreamchannelbase.h"

#include "log.h"

using namespace mu::audio::worker;

RpcStreamChannelBase::RpcStreamChannelBase()
{
}

RpcStreamChannelBase::~RpcStreamChannelBase()
{
}

StreamID RpcStreamChannelBase::newID()
{
    ++m_lastID;
    return m_lastID;
}

// Rpc
void RpcStreamChannelBase::send(const StreamID& id, CallID method, const Args& args)
{
    doSend(id, method, args);
}

void RpcStreamChannelBase::listen(const StreamID& id, Handler h)
{
    doListen(id, h);
}

void RpcStreamChannelBase::unlisten(const StreamID& id)
{
    doUnlisten(id);
}

void RpcStreamChannelBase::listenAll(HandlerAll h)
{
    doListenAll(h);
}

void RpcStreamChannelBase::unlistenAll()
{
    doUnlistenAll();
}

// Audio
const std::shared_ptr<RpcStreamChannelBase::Stream>& RpcStreamChannelBase::stream(const StreamID& id) const
{
    static std::shared_ptr<Stream> null;

    auto it = m_streams.find(id);
    if (it != m_streams.end()) {
        return it->second;
    }
    return null;
}

void RpcStreamChannelBase::registerStream(const StreamID& id,
                                          uint16_t samples, uint16_t channels,
                                          GetBuffer getBuffer, OnRequestFinished onRequestFinished)
{
    IF_ASSERT_FAILED(!stream(id)) {
        return;
    }

    std::shared_ptr<Stream> s = std::make_shared<Stream>();
    s->id = id;
    s->samples = samples;
    s->channels = channels;
    s->bufSize = samples * channels;
    s->buf.resize(samples * channels);

    s->getBuffer = getBuffer;
    s->onRequestFinished = onRequestFinished;

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    m_streams.insert({ id, s });

    onStreamRegistred(s);

    // LOGI() << "registerStream: " << id;
}

void RpcStreamChannelBase::onStreamRegistred(std::shared_ptr<Stream>& stream)
{
    UNUSED(stream);
}

void RpcStreamChannelBase::unregisterStream(const StreamID& id)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    const std::shared_ptr<Stream>& s = stream(id);
    if (s) {
        s->getBuffer = nullptr;
        s->onRequestFinished = nullptr;
    }

    m_streams.erase(id);

    onStreamUnregistred(id);
    //LOGI() << "unregisterStream: " << id;
}

void RpcStreamChannelBase::onStreamUnregistred(const StreamID& id)
{
    UNUSED(id);
}

bool RpcStreamChannelBase::allStreamsInState(const RequestState& state) const
{
    for (const auto& p : m_streams) {
        if (p.second->state != state) {
            return false;
        }
    }
    return true;
}

void RpcStreamChannelBase::requestAudio(const StreamID& id)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    const std::shared_ptr<Stream>& s = stream(id);
    IF_ASSERT_FAILED(s) {
        return;
    }

    if (s->state == RequestState::FREE) {
        s->state = RequestState::REQUESTED;
    }

    onRequestAudio(id);
}

void RpcStreamChannelBase::onRequestAudio(const StreamID& id)
{
    UNUSED(id);
}

void RpcStreamChannelBase::onGetAudio(const GetAudio& func)
{
    m_getAudio = func;
}
