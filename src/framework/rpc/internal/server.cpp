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
#include "server.h"
#include "log.h"
#include <algorithm>
using namespace mu::rpc;

Server::Server()
    : m_messages(), m_targets()
{
}

void Server::registerTarget(target_id id, std::shared_ptr<IRPCTarget> target)
{
    m_targets[id] = target;
}

void Server::addCall(Message message)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_messages.push(message);
}

void Server::invoke()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    while (m_messages.size()) {
        auto message = m_messages.front();
        invoke(message);
        m_messages.pop();
    }
}

void Server::invoke(Message message)
{
    auto targetPair = m_targets.find(message.target);
    if (targetPair == m_targets.end()) {
        LOGW() << "unknown target: " << message.target.first << message.target.second;
        return;
    }
    auto target = targetPair->second;

    target->invokeRpcReflection(message);
}

std::size_t Server::TargetHash::operator()(const target_id& key) const
{
    return std::hash<std::string> {}(key.first + "#" + std::to_string(key.second));
}

bool Server::TargetEqual::operator()(const target_id& lhs, const target_id& rhs) const
{
    return lhs.first == rhs.first && lhs.second == rhs.second;
}
