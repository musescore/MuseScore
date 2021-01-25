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
#include "rpccontrollers.h"

#include "log.h"

using namespace mu::audio;
using namespace mu::audio::rpc;

void RpcControllers::reg(const IRpcControllerPtr& controller)
{
    IF_ASSERT_FAILED(controller) {
        return;
    }
    m_controllers.push_back(controller);
    if (m_channel) {
        controller->init(m_channel);
    }
}

void RpcControllers::init(const IRpcChannelPtr& channel)
{
    m_channel = channel;
    if (m_channel) {
        for (auto& cont : m_controllers) {
            cont->init(channel);
        }

        m_listenID = m_channel->listen([this](const Msg& msg) {
            for (auto& cont : m_controllers) {
                if (cont->target() == msg.target.name) {
                    cont->handle(msg);
                }
            }
        });
    }
}

void RpcControllers::deinit()
{
    if (m_channel) {
        m_channel->unlisten(m_listenID);
        m_listenID = -1;
    }
}
