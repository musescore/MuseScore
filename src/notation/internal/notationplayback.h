/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_NOTATION_NOTATIONPLAYBACK_H
#define MU_NOTATION_NOTATIONPLAYBACK_H

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "engraving/playback/playbackmodel.h"

#include "../inotationplayback.h"
#include "igetscore.h"
#include "inotationconfiguration.h"

namespace mu::engraving {
class Score;
}

namespace mu::notation {
class NotationPlayback : public INotationPlayback, public muse::async::Asyncable
{
    INJECT(INotationConfiguration, configuration)

public:
    NotationPlayback(IGetScore* getScore, muse::async::Notification notationChanged, const muse::modularity::ContextPtr& iocCtx);

    void init() override;
    void reload() override;

    const engraving::InstrumentTrackId& metronomeTrackId() const override;
    engraving::InstrumentTrackId chordSymbolsTrackId(const muse::ID& partId) const override;
    bool isChordSymbolsTrack(const engraving::InstrumentTrackId& trackId) const override;

    const muse::mpe::PlaybackData& trackPlaybackData(const engraving::InstrumentTrackId& trackId) const override;

    void triggerEventsForItems(const std::vector<const EngravingItem*>& items, muse::mpe::duration_t duration, bool flushSound) override;
    void triggerMetronome(muse::midi::tick_t tick) override;
    void triggerCountIn(muse::midi::tick_t tick, muse::secs_t& countInDuration) override;
    void triggerControllers(const muse::mpe::ControllerChangeEventList& list, notation::staff_idx_t staffIdx, int tick) override;

    engraving::InstrumentTrackIdSet existingTrackIdSet() const override;
    muse::async::Channel<engraving::InstrumentTrackId> trackAdded() const override;
    muse::async::Channel<engraving::InstrumentTrackId> trackRemoved() const override;

    muse::audio::secs_t totalPlayTime() const override;
    muse::async::Channel<muse::audio::secs_t> totalPlayTimeChanged() const override;

    muse::audio::secs_t playedTickToSec(muse::midi::tick_t tick) const override;
    muse::midi::tick_t secToPlayedTick(muse::audio::secs_t sec) const override;
    muse::midi::tick_t secToTick(muse::audio::secs_t sec) const override;

    muse::RetVal<muse::midi::tick_t> playPositionTickByRawTick(muse::midi::tick_t tick) const override;
    muse::RetVal<muse::midi::tick_t> playPositionTickByElement(const EngravingItem* element) const override;

    void addLoopBoundary(LoopBoundaryType boundaryType, muse::midi::tick_t tick) override;
    void setLoopBoundariesEnabled(bool enabled) override;
    const LoopBoundaries& loopBoundaries() const override;
    muse::async::Notification loopBoundariesChanged() const override;

    const Tempo& multipliedTempo(muse::midi::tick_t tick) const override;
    MeasureBeat beat(muse::midi::tick_t tick) const override;
    muse::midi::tick_t beatToRawTick(int measureIndex, int beatIndex) const override;

    double tempoMultiplier() const override;
    void setTempoMultiplier(double multiplier) override;

    void addSoundFlags(const std::vector<mu::engraving::StaffText*>& staffTextList) override;
    void removeSoundFlags(const engraving::InstrumentTrackIdSet& trackIdSet) override;
    bool hasSoundFlags(const engraving::InstrumentTrackIdSet& trackIdSet) override;

private:
    engraving::Score* score() const;

    void addLoopIn(int tick);
    void addLoopOut(int tick);
    void updateLoopBoundaries();
    void updateTotalPlayTime();

    bool doAddSoundFlag(mu::engraving::StaffText* staffText);

    std::vector<mu::engraving::StaffText*> collectStaffText(const mu::engraving::InstrumentTrackIdSet& trackIdSet,
                                                            bool withSoundFlags) const;

    IGetScore* m_getScore = nullptr;

    muse::async::Notification m_notationChanged;

    LoopBoundaries m_loopBoundaries;
    muse::async::Notification m_loopBoundariesChanged;

    muse::audio::secs_t m_totalPlayTime = 0;
    muse::async::Channel<muse::audio::secs_t> m_totalPlayTimeChanged;

    mutable Tempo m_currentTempo;

    mutable engraving::PlaybackModel m_playbackModel;
};
}

#endif // MU_NOTATION_NOTATIONPLAYBACK_H
