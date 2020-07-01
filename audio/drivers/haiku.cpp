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

#include "haiku.h"

#include <MediaDefs.h>
#include <MediaRoster.h>

#include "mscore/seq.h"

namespace Ms {
//---------------------------------------------------------
//   haikuBufferPlayer
//---------------------------------------------------------

void HaikuMediaKit::haikuBufferPlayer(void* cookie, void* buffer, size_t size,
    const media_raw_audio_format& format)
{
    HaikuMediaKit* hmk = (HaikuMediaKit*)cookie;

    hmk->seq->process(size / (2 * sizeof(float)), (float*)buffer);
}

//---------------------------------------------------------
//   HaikuMediaKit
//---------------------------------------------------------

HaikuMediaKit::HaikuMediaKit(Seq* s)
    : Driver(s)
{
    _sampleRate = 48000; // will be replaced by device default sample rate
    state = Transport::STOP;
}

//---------------------------------------------------------
//   ~HaikuMediaKit
//---------------------------------------------------------

HaikuMediaKit::~HaikuMediaKit()
{
    stop();
    delete player;
}


//---------------------------------------------------------
//   init
//   	return false on error
//---------------------------------------------------------

bool HaikuMediaKit::init(bool /*hot*/)
{
    status_t error = B_ERROR;
    BMediaRoster* roster = BMediaRoster::Roster(&error);
    if (error != B_OK) {
        return false;
    }

    media_raw_audio_format format;
    format.frame_rate = _sampleRate;
    format.channel_count = 2;
    format.format = media_raw_audio_format::B_AUDIO_FLOAT;
    format.byte_order = B_MEDIA_LITTLE_ENDIAN;
    format.buffer_size = roster->AudioBufferSizeFor(2,
        media_raw_audio_format::B_AUDIO_FLOAT, _sampleRate);

    player = new BSoundPlayer(&format, "MuseScore", haikuBufferPlayer,
        nullptr, this);

    if (player == NULL || player->InitCheck() != B_OK) {
        qDebug("HaikuMediaKit: BSoundPlayer failed to start up!");
        return false;
    }

    _sampleRate = player->Format().frame_rate;

    return true;
}

//---------------------------------------------------------
//   start
//---------------------------------------------------------

bool HaikuMediaKit::start(bool /* hotPlug */)
{
    if (player == nullptr) {
        return false;
    }

    if (player->Start() != B_OK) {
        return false;
    }

    player->SetHasData(true);

    return true;
}

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

bool HaikuMediaKit::stop()
{
    if (player != nullptr) {
        player->SetHasData(false);
        player->Stop();
    }

    return true;
}

//---------------------------------------------------------
//   getState
//---------------------------------------------------------

Transport HaikuMediaKit::getState()
{
    return state;
}

//---------------------------------------------------------
//   startTransport
//---------------------------------------------------------

void HaikuMediaKit::startTransport()
{
    state = Transport::PLAY;
}

//---------------------------------------------------------
//   stopTransport
//---------------------------------------------------------

void HaikuMediaKit::stopTransport()
{
    state = Transport::STOP;
}
} // namespace Ms
