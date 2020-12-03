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
#ifndef MU_AUDIO_RPCSOURCEBASE_H
#define MU_AUDIO_RPCSOURCEBASE_H

#include <string>
#include <memory>
#include <functional>
#include <mutex>

#include "iaudiosource.h"

#include "modularity/ioc.h"
#include "irpcaudiostreamchannel.h"
#include "iaudioengine.h"

#include "audiotypes.h"

namespace mu {
namespace audio {
namespace worker {
class RpcSourceBase : public IAudioSource
{
    INJECT(audio_engine, IRpcAudioStreamChannel, channel)
    INJECT(audio_engine, IAudioEngine, audioEngine)

public:
    ~RpcSourceBase() override;

    void setSampleRate(float samplerate) override;

    SoLoud::AudioSource* source() override;

    void setLoopRegion(const LoopRegion& loop);

protected:

    RpcSourceBase(CallType type, const std::string& name);

    void call(CallMethod method, const Args& args);
    void listen(const std::function<void(CallMethod method, const Args& args)>& func);

    void truncate();

    virtual void onGetAudio(const Context& ctx);

private:

    struct SL;
    struct SLInstance;
    struct Buffer;

    std::mutex m_instanceMutex;
    std::string m_name;
    CallType m_type;

    StreamID m_streamID;
    SL* m_sl = nullptr;
    SLInstance* m_instance = nullptr;
};
}
}
}

#endif // MU_AUDIO_RPCSOURCEBASE_H
