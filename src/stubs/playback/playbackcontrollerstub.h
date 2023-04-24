/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_PLAYBACK_PLAYBACKCONTROLLERSTUB_H
#define MU_PLAYBACK_PLAYBACKCONTROLLERSTUB_H

#include "playback/iplaybackcontroller.h"

namespace mu::playback {
class PlaybackControllerStub : public IPlaybackController
{
public:
    bool isPlayAllowed() const override;
    async::Notification isPlayAllowedChanged() const override;

    bool isPlaying() const override;
    async::Notification isPlayingChanged() const override;

    void seek(const midi::tick_t tick) override;
    void seek(const audio::msecs_t msecs) override;
    void reset() override;

    async::Notification playbackPositionChanged() const override;
    async::Channel<uint32_t> midiTickPlayed() const override;
    float playbackPositionInSeconds() const override;

    audio::TrackSequenceId currentTrackSequenceId() const override;
    async::Notification currentTrackSequenceIdChanged() const override;

    const InstrumentTrackIdMap& instrumentTrackIdMap() const override;
    const audio::TrackIdList& auxTrackIdList() const override;

    async::Channel<audio::TrackId> trackAdded() const override;
    async::Channel<audio::TrackId> trackRemoved() const override;

    void playElements(const std::vector<const notation::EngravingItem*>& elements) override;
    void playMetronome(int tick) override;
    void seekElement(const notation::EngravingItem* element) override;

    bool actionChecked(const actions::ActionCode& actionCode) const override;
    async::Channel<actions::ActionCode> actionCheckedChanged() const override;

    QTime totalPlayTime() const override;
    async::Notification totalPlayTimeChanged() const override;

    notation::Tempo currentTempo() const override;
    async::Notification currentTempoChanged() const override;

    notation::MeasureBeat currentBeat() const override;
    audio::msecs_t beatToMilliseconds(int measureIndex, int beatIndex) const override;

    double tempoMultiplier() const override;
    void setTempoMultiplier(double multiplier) override;

    framework::Progress loadingProgress() const override;

    void applyProfile(const SoundProfileName& profileName) override;

    void setNotation(notation::INotationPtr notation) override;
    void setIsExportingAudio(bool exporting) override;
};
}

#endif // MU_PLAYBACK_PLAYBACKCONTROLLERSTUB_H
