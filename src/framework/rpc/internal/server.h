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
#ifndef MU_RPC_SERVER_H
#define MU_RPC_SERVER_H

#include <unordered_map>
#include <queue>
#include <mutex>
#include "irpcserver.h"
#include "irpctarget.h"

namespace mu {
namespace rpc {
class Server : public IRPCServer
{
public:
    Server();

    void registerTarget(target_id id, std::shared_ptr<IRPCTarget> target) override;
    void addCall(Message message) override;
    void invoke() override;

private:
    void invoke(Message message);

    std::queue<Message> m_messages;
    mutable std::mutex m_mutex;

    struct TargetHash {
        std::size_t operator()(const target_id& key) const;
    };

    struct TargetEqual {
        bool operator()(const target_id& lhs, const target_id& rhs) const;
    };

    std::unordered_map<
        target_id,
        std::shared_ptr<IRPCTarget>,
        TargetHash,
        TargetEqual
        > m_targets;
};
} // namespace rpc
} // namespace mu

#endif // MU_RPC_SERVER_H
