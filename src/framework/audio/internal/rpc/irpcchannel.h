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
#ifndef MU_AUDIO_IRPCCHANNEL_H
#define MU_AUDIO_IRPCCHANNEL_H

#include <memory>
#include <functional>

#include "modularity/imoduleexport.h"
#include "rpctypes.h"

namespace mu::audio::rpc {
class IRpcChannel : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(mu::audio::IRpcChannel)

public:
    virtual ~IRpcChannel() = default;

    using ListenID = int;
    using Handler = std::function<void (const Msg& msg)>;

    virtual bool isSerialized() const = 0;

    virtual void send(const Msg& msg) = 0;

    virtual ListenID listen(Handler h) = 0;
    virtual void unlisten(ListenID id) = 0;
};

using IRpcChannelPtr = std::shared_ptr<IRpcChannel>;
}

#endif // MU_AUDIO_IRPCCHANNEL_H
