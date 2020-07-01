//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2020 Jacob Secunda
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

#ifndef __HAIKU_H__
#define __HAIKU_H__

#include "config.h"
#include "driver.h"

#include <SoundPlayer.h>

namespace Ms {
class Synth;
class Seq;
enum class Transport : char;

//---------------------------------------------------------
//   Haiku Media Kit Driver
//---------------------------------------------------------

class HaikuMediaKit : public Driver
{
    int _sampleRate;
    Transport state;

    BSoundPlayer* player;

    static void haikuBufferPlayer(void* cookie, void* buffer, size_t size,
        const media_raw_audio_format& format);

public:
    HaikuMediaKit(Seq*);
    virtual ~HaikuMediaKit();
    virtual bool init(bool hot = false);
    virtual bool start(bool hotPlug = false);
    virtual bool stop();
    virtual Transport getState() override;
    virtual void stopTransport();
    virtual void startTransport();
    virtual int sampleRate() const { return _sampleRate; }
    virtual int bufferSize() { return player != nullptr ? player->Format().buffer_size : 0; }
};
} // namespace Ms
#endif
