/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore BVBA and others
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

#include "transporteventsdispatcher.h"

#include "audio/common/rpc/rpcpacker.h"
#include "audio/common/audiosanitizer.h"

using namespace muse::audio::engine;
using namespace muse::audio::rpc;

TransportEventsDispatcher::TransportEventsDispatcher(const modularity::ContextPtr& iocCtx)
    : Contextable(iocCtx),
    m_eventsReceived(async::makeOpt()
                     .name("audio::engine::TransportEventsDispatcher::m_eventsReceived")
                     .disableWaitPendingsOnSend())
{
}

void TransportEventsDispatcher::init()
{
    ONLY_AUDIO_ENGINE_THREAD;

    m_eventsReceived.onReceive(this, [this](const TransportEvents& events) {
        for (const TransportEvent& event : events) {
            rpcChannel()->send(rpc::make_request(Method::TransportEventReceived, RpcPacker::pack(event)));
        }
    });
}

void TransportEventsDispatcher::dispatch(const TransportEvents& events)
{
    ONLY_AUDIO_PROC_THREAD;

    //! Transfer events to the audio engine thread
    m_eventsReceived.send(events);
}
