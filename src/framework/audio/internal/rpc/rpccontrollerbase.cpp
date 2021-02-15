//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "rpccontrollerbase.h"

#include "log.h"

using namespace mu::audio::rpc;

void RpcControllerBase::init(const IRpcChannelPtr& channel)
{
    m_channel = channel;
}

void RpcControllerBase::handle(const Msg& msg)
{
    if (m_calls.empty()) {
        doBind();
    }

    doCall(msg);
}

bool RpcControllerBase::isSerialized() const
{
    IF_ASSERT_FAILED(m_channel) {
        return false;
    }
    return m_channel->isSerialized();
}

void RpcControllerBase::bindMethod(const rpc::Method& method, const Call& call)
{
    m_calls.insert({ method, call });
}

void RpcControllerBase::doCall(const rpc::Msg& msg)
{
    auto it = m_calls.find(msg.method);
    if (it == m_calls.end()) {
        LOGE() << "not found method: " << msg.method;
        return;
    }

    it->second(msg.args);
}

void RpcControllerBase::sendToMain(const Msg& msg)
{
    IF_ASSERT_FAILED(m_channel) {
        return;
    }
    m_channel->send(msg);
}
