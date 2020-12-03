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
#ifndef MU_AUDIO_OSXAUDIODRIVER_H
#define MU_AUDIO_OSXAUDIODRIVER_H

#include <memory>
#include <MacTypes.h>
#include "../../iaudiodriver.h"

struct AudioTimeStamp;
struct AudioQueueBuffer;
struct OpaqueAudioQueue;

namespace mu {
namespace audio {
class OSXAudioDriver : public IAudioDriver
{
public:
    OSXAudioDriver();
    ~OSXAudioDriver();

    std::string name() const override;
    bool open(const Spec& spec, Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;

private:
    static void OnFillBuffer(void* context, OpaqueAudioQueue* queue, AudioQueueBuffer* buffer);
    static void logError(const std::string message, OSStatus error);
    struct Data;

    std::shared_ptr<Data> m_data;
};
}
}
#endif // MU_AUDIO_OSXAUDIODRIVER_H
