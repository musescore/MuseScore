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
#include "sequencer.h"

using namespace mu::audio;

RPCSequencer::RPCSequencer()
{
}

void RPCSequencer::play()
{
    rpc::Message m(
        m_target,
        "play",
        {}
        );
    rpcClient()->send(m);
}

void RPCSequencer::pause()
{
    rpc::Message m(
        m_target,
        "pause",
        {}
        );
    rpcClient()->send(m);
}

void RPCSequencer::stop()
{
    rpc::Message m(
        m_target,
        "stop",
        {}
        );
    rpcClient()->send(m);
}

void RPCSequencer::seek(unsigned long position)
{
    rpc::Message m(
        m_target,
        "seek",
        { position }
        );
    rpcClient()->send(m);
}

void RPCSequencer::rewind()
{
    rpc::Message m(
        m_target,
        "rewind",
        {}
        );
    rpcClient()->send(m);
}

void RPCSequencer::setLoop(unsigned long from, unsigned long to)
{
    rpc::Message m(
        m_target,
        "setLoop",
        { from, to }
        );
    rpcClient()->send(m);
}

void RPCSequencer::unsetLoop()
{
    rpc::Message m(
        m_target,
        "unsetLoop",
        {}
        );
    rpcClient()->send(m);
}
