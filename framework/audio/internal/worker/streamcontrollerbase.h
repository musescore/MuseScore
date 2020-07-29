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

#ifndef MU_AUDIO_STREAMCONTROLLERBASE_H
#define MU_AUDIO_STREAMCONTROLLERBASE_H

#include <vector>
#include <memory>
#include <string>
#include <map>

#include "workertypes.h"
#include "audiotypes.h"
#include "../../iaudiosource.h"
#include "../loopsource.h"

namespace SoLoud {
class AudioSourceInstance;
}

namespace mu {
namespace audio {
namespace worker {
class StreamControllerBase
{
public:
    StreamControllerBase();
    virtual ~StreamControllerBase();

    virtual void createStream(const StreamID& id, const std::string& name);
    void destroyStream(const StreamID& id);
    bool hasStream(const StreamID& id) const;

    void setSampleRate(const StreamID& id, float samplerate);
    void setLoopRegion(const StreamID& id, const LoopRegion& loop);

    // Instance
    void streamInstance_create(const StreamID& id);
    void streamInstance_destroy(const StreamID& id);
    void streamInstance_init(const StreamID& id, float samplerate, uint16_t chans, double streamTime, double streamPos);
    void streamInstance_seek_frame(const StreamID& id, float sec);

    void getAudio(const StreamID& id, float* buf, uint32_t samples, uint32_t bufSize, Context* ctx);

protected:

    struct Stream {
        StreamID id;
        std::string name;
        std::shared_ptr<IAudioSource> source;
        std::shared_ptr<LoopSource> loopStream;
        SoLoud::AudioSourceInstance* instance = nullptr;
        bool inited = false;
        std::vector<float> buf;
    };

    std::shared_ptr<Stream> stream(const StreamID& id) const;

    virtual std::shared_ptr<IAudioSource> makeSource(const StreamID& id, const std::string& name) const = 0;
    virtual void fillAudioContext(const std::shared_ptr<Stream>& s, Context* ctx);

private:

    std::map<StreamID, std::shared_ptr<Stream> > m_streams;
};
}
}
}

#endif // MU_AUDIO_STREAMCONTROLLERBASE_H
