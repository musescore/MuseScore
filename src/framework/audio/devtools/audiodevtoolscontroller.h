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
#ifndef MU_AUDIO_AUDIODEVTOOLSCONTROLLER_H
#define MU_AUDIO_AUDIODEVTOOLSCONTROLLER_H

#include <functional>
#include <optional>

#include "internal/rpc/rpctypes.h"
#include "internal/worker/audioengine.h"

namespace mu::audio {
class AudioDevToolsController
{
public:

    static AudioDevToolsController* instance();

    void handleRpcMsg(const rpc::Msg& msg);

private:
    AudioDevToolsController() = default;

    using Call = std::function<void (const rpc::Args& args)>;
    using Calls = std::map<rpc::Method, Call>;

    void bindMethod(Calls& calls, const rpc::Method& method, const Call& call);
    void doCall(const Calls& calls, const rpc::Msg& msg);

    AudioEngine* audioEngine() const;

    std::optional<unsigned int> m_sineChannelId;
    std::optional<unsigned int> m_noiseChannel;
};
}

#endif // MU_AUDIO_AUDIODEVTOOLSCONTROLLER_H
