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
#ifndef MU_AUDIO_IPLAYER_H
#define MU_AUDIO_IPLAYER_H

#include "async/channel.h"

namespace mu::audio {
class IPlayer
{
public:
    enum Status {
        Stoped = 0,
        Running,
        Error
    };

    virtual ~IPlayer() = default;

    virtual Status status() const = 0;
    virtual async::Channel<Status> statusChanged() const = 0;

    virtual bool isRunning() const = 0;

    virtual void play() = 0;
    virtual void seek(unsigned long miliseconds) = 0;
    virtual void stop() = 0;

    virtual unsigned long miliseconds() const = 0;
    virtual void forwardTime(unsigned long miliseconds) = 0;
};
}
#endif // MU_AUDIO_IPLAYER_H
