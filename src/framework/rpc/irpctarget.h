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
#ifndef MU_RPC_IRPCTARGET_H
#define MU_RPC_IRPCTARGET_H

#include <map>
#include <string>
#include <functional>
#include "rpctypes.h"

namespace mu {
namespace rpc {
class IRPCTarget
{
public:
    virtual ~IRPCTarget() = default;
    Variable invokeRpcReflection(Message message)
    {
        auto reflectionPair = m_rpcReflection.find(message.method);
        if (reflectionPair != m_rpcReflection.end()) {
            return reflectionPair->second(message.args);
        }
        return {};
    }

protected:
    virtual void buildRpcReflection() = 0;
    std::map<std::string, std::function<Variable(ArgumentList)> > m_rpcReflection;
};
}
}
#endif // MU_RPC_IRPCTARGET_H
