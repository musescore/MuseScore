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
using namespace mu::playback;
using namespace mu::audio;

void PlaybackController::init()
{
    dispatcher()->reg(this, "play", this, &PlaybackController::togglePlay);

    onNotationChanged();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    audioPlayer()->statusChanged().onReceive(this, [this](const PlayStatus&) {
        m_isPlayingChanged.notify();
    });
}

bool PlaybackController::isPlayAllowed() const
{
    return m_notation != nullptr;
}

async::Notification PlaybackController::isPlayAllowedChanged() const
{
    return m_isPlayAllowedChanged;
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

async::Channel<uint32_t> PlaybackController::midiTickPlayed() const
{
    return audioPlayer()->midiTickPlayed();
}

void PlaybackController::playElementOnClick(const notation::Element* e)
{
    if (!configuration()->isPlayElementOnClick()) {
        return;
    }

    IF_ASSERT_FAILED(e) {
        return;
    }

    IF_ASSERT_FAILED(m_notation) {
        return;
    }

    if (e->isHarmony() && !configuration()->isPlayHarmonyOnClick()) {
        return;
    }

    midi::MidiData midiData = m_notation->playback()->playElementMidiData(e);

    LOGD() << midiData.dump(true);

    audioPlayer()->playMidi(midiData);
}

void PlaybackController::onNotationChanged()
{
    if (m_notation) {
        m_notation->playback()->playPositionTickChanged().resetOnReceive(this);
    }

    m_notation = globalContext()->currentNotation();
    if (m_notation) {
        m_notation->playback()->playPositionTickChanged().onReceive(this, [this](int tick) {
            seek(tick);
        });
    }

    m_isPlayAllowedChanged.notify();
}

void PlaybackController::togglePlay()
{
    if (!isPlayAllowed()) {
        LOGW() << "playback not allowed";
        return;
    }

    if (isPlaying()) {
        stop();
    } else {
        play();
    }
}

void PlaybackController::play()
{
    IF_ASSERT_FAILED(m_notation) {
        return;
    }

    auto stream = m_notation->playback()->midiStream();
    audioPlayer()->setMidiStream(stream);

    RetVal<int> tick = m_notation->playback()->playPositionTick();
    if (!tick.ret) {
        LOGE() << "unable play, err: " << tick.ret.toString();
        return;
    }

    seek(tick.val);

    bool ok = audioPlayer()->play();
    if (!ok) {
        LOGE() << "failed play";
        return;
    }
}

void PlaybackController::seek(int tick)
{
    IF_ASSERT_FAILED(m_notation) {
        return;
    }

    float sec = m_notation->playback()->tickToSec(tick);
    audioPlayer()->setPlaybackPosition(sec);
}

void PlaybackController::stop()
{
    audioPlayer()->stop();
}
