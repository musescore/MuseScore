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
#include "playbackcontroller.h"

#include "log.h"

using namespace mu;
using namespace mu::scene::playback;
using namespace mu::audio;

void PlaybackController::init()
{
    dispatcher()->reg(this, "play", this, &PlaybackController::togglePlay);

    updatePlayAllowance();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        updatePlayAllowance();
    });

    audioPlayer()->statusChanged().onReceive(this, [this](const PlayStatus&) {
        m_isPlayingChanged.notify();
    });
}

bool PlaybackController::isPlayAllowed() const
{
    return m_isPlayAllowed.val;
}

async::Notification PlaybackController::isPlayAllowedChanged() const
{
    return m_isPlayAllowed.notification;
}

bool PlaybackController::isPlaying() const
{
    return audioPlayer()->status() == PlayStatus::PLAYING;
}

async::Notification PlaybackController::isPlayingChanged() const
{
    return m_isPlayingChanged;
}

float PlaybackController::playbackPosition() const
{
    return audioPlayer()->playbackPosition();
}

void PlaybackController::updatePlayAllowance()
{
    auto notation = globalContext()->currentNotation();
    if (notation) {
        m_isPlayAllowed.set(true);
    } else {
        m_isPlayAllowed.set(false);
    }
}

void PlaybackController::togglePlay()
{
    if (!isPlayAllowed()) {
        LOGW() << "playback not allowed";
        return;
    }

    if (isPlaying()) {
        pause();
    } else {
        play();
    }
}

void PlaybackController::play()
{
    auto notation = globalContext()->currentNotation();
    IF_ASSERT_FAILED(notation) {
        return;
    }

    auto stream = notation->playback()->midiStream();
    audioPlayer()->setMidiStream(stream);
    bool ok = audioPlayer()->play();
    if (!ok) {
        LOGE() << "failed play";
        return;
    }
}

void PlaybackController::pause()
{
    audioPlayer()->pause();
}
