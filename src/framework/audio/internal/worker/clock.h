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
#ifndef MU_AUDIO_CLOCK_H
#define MU_AUDIO_CLOCK_H

#include <memory>
#include <list>
#include <atomic>
#include "async/asyncable.h"
#include "async/channel.h"

namespace mu::audio {
class Clock : public async::Asyncable
{
public:
    Clock();

    enum Status {
        Stoped = 0,
        Paused,
        Running
    };

    using SyncCallback = std::function<void (time_t)>;

    using time_t = unsigned long;

    //! return current position in samples
    time_t time() const;

    //! return current position in seconds
    float timeInSeconds() const;

    //! return current position in milliseconds
    time_t timeInMiliSeconds() const;

    void setSampleRate(unsigned int sampleRate);
    void forward(time_t samples);

    void start();
    void reset();
    void stop();
    void pause();
    void seek(time_t time);
    void seekMiliseconds(time_t value);
    void seekSeconds(float seconds);

    async::Channel<time_t> timeChanged() const;
    void addBeforeCallback(SyncCallback callback);
    void addAfterCallback(SyncCallback callback);

private:
    void runCallbacks(const std::list<SyncCallback>& list, time_t milliseconds);

    std::atomic<Status> m_status = Stoped;
    time_t m_time = 0;
    unsigned int m_sampleRate = 1;

    async::Channel<time_t> m_timeChanged;
    std::list<SyncCallback> m_beforeCallbacks = {};
    std::list<SyncCallback> m_afterCallbacks = {};
};
}

#endif // MU_AUDIO_CLOCK_H
