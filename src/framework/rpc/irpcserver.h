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
#ifndef MU_RPC_IRPCSERVER_H
#define MU_RPC_IRPCSERVER_H

#include "modularity/imoduleexport.h"
#include "irpctarget.h"
#include "rpctypes.h"

namespace mu {
namespace rpc {
class IRPCServer : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IServer)

public:
    virtual ~IRPCServer() = default;

    virtual void registerTarget(target_id, std::shared_ptr<IRPCTarget> target) = 0;
    virtual void addCall(Message message) = 0;
    virtual void invoke() = 0;
};
} // namespace rpc
} // namespace mu

#endif // MU_RPC_IRPCSERVER_H
