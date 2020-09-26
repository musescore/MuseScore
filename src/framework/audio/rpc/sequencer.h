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
#ifndef MU_AUDIO_RPCSEQUENCER_H
#define MU_AUDIO_RPCSEQUENCER_H

#include "rpc/irpcclient.h"
#include "modularity/ioc.h"

namespace mu::audio {
class RPCSequencer
{
    INJECT(rpc, rpc::IRPCClient, rpcClient)

public:
    RPCSequencer();

    void play();
    void pause();
    void stop();
    void seek(unsigned long position);
    void rewind();
    void setLoop(unsigned long from, unsigned long to);
    void unsetLoop();

private:
    rpc::target_id m_target = { "sequencer", 0 };
};
}

#endif // MU_AUDIO_RPCSEQUENCER_H
