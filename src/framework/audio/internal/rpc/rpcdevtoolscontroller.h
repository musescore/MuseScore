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
#ifndef MU_AUDIO_RPCDEVTOOLSCONTROLLER_H
#define MU_AUDIO_RPCDEVTOOLSCONTROLLER_H

#include <optional>

#include "rpccontrollerbase.h"
#include "internal/worker/audioengine.h"

namespace mu::audio::rpc {
class RpcDevToolsController : public RpcControllerBase
{
public:

    TargetName target() const override;

protected:

    void doBind() override;

    AudioEngine* audioEngine() const;

    std::optional<unsigned int> m_sineChannelId;
    std::optional<unsigned int> m_noiseChannel;
};
}

#endif // MU_AUDIO_RPCDEVTOOLSCONTROLLER_H
