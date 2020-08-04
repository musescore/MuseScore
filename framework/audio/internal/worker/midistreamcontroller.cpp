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
#include "midistreamcontroller.h"

#include <soloud.h>
#include "log.h"
#include "ptrutils.h"

#include "internal/midisource.h"

using namespace mu::midi;
using namespace mu::audio;
using namespace mu::audio::worker;

MidiStreamController::MidiStreamController()
{
}

std::shared_ptr<IAudioSource> MidiStreamController::makeSource(const StreamID& id, const std::string& name) const
{
    UNUSED(id);
    std::shared_ptr<MidiSource> midi = std::make_shared<MidiSource>(name);
    return std::move(midi);
}

void MidiStreamController::fillAudioContext(const std::shared_ptr<Stream>& s, Context* ctx)
{
    MidiSource* midiSource = mu::ptr::checked_cast<MidiSource>(s->source.get());
    IF_ASSERT_FAILED(midiSource) {
        return;
    }

    midiSource->fillPlayContext(ctx);
}

MidiSource* MidiStreamController::midiStream(const StreamID& id) const
{
    std::shared_ptr<Stream> s = stream(id);
    IF_ASSERT_FAILED(s) {
        return nullptr;
    }
    return mu::ptr::checked_cast<MidiSource>(s->source.get());
}

void MidiStreamController::loadMIDI(const StreamID& id, const std::shared_ptr<MidiStream>& data)
{
    std::shared_ptr<Stream> s = stream(id);
    IF_ASSERT_FAILED(s) {
        return;
    }

    MidiSource* midi = midiStream(id);
    IF_ASSERT_FAILED(midi) {
        return;
    }

    midi->loadMIDI(data);
}

void MidiStreamController::setPlaybackSpeed(const StreamID& id, float speed)
{
    MidiSource* midi = midiStream(id);
    IF_ASSERT_FAILED(midi) {
        return;
    }
    midi->setPlaybackSpeed(speed);
}

void MidiStreamController::setIsTrackMuted(const StreamID& id, uint16_t ti, bool mute)
{
    MidiSource* midi = midiStream(id);
    IF_ASSERT_FAILED(midi) {
        return;
    }
    midi->setIsTrackMuted(ti, mute);
}

void MidiStreamController::setTrackVolume(const StreamID& id, uint16_t ti, float volume)
{
    MidiSource* midi = midiStream(id);
    IF_ASSERT_FAILED(midi) {
        return;
    }
    midi->setTrackVolume(ti, volume);
}

void MidiStreamController::setTrackBalance(const StreamID& id, uint16_t ti, float balance)
{
    MidiSource* midi = midiStream(id);
    IF_ASSERT_FAILED(midi) {
        return;
    }
    midi->setTrackBalance(ti, balance);
}
