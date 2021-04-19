/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_AUDIO_RPCCONTROLLERBASE_H
#define MU_AUDIO_RPCCONTROLLERBASE_H

#include <functional>
#include <map>

#include "irpccontroller.h"
#include "internal/worker/audioengine.h"

namespace mu::audio::rpc {
class RpcControllerBase : public IRpcController
{
public:

    void init(const IRpcChannelPtr& channel) override;

    void handle(const Msg& msg) override;

protected:

    virtual void doBind() = 0;

    RpcControllerBase() = default;

    using Call = std::function<void (const rpc::Args& args)>;
    using Calls = std::map<rpc::Method, Call>;

    bool isSerialized() const;
    void bindMethod(const rpc::Method& method, const Call& call);
    void doCall(const rpc::Msg& msg);
    void sendToMain(const Msg& msg);

    IRpcChannelPtr m_channel;
    Calls m_calls;
};
}

#endif // MU_AUDIO_RPCCONTROLLERBASE_H
