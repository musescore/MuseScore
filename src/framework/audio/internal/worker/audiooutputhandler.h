/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#pragma once

#include "global/modularity/ioc.h"
#include "global/async/asyncable.h"

#include "ifxresolver.h"
#include "iaudiooutput.h"
#include "igettracksequence.h"
#include "iaudioengine.h"

namespace muse::audio {
class Mixer;

namespace soundtrack {
class SoundTrackWriter;
using SoundTrackWriterPtr = std::shared_ptr<SoundTrackWriter>;
}

class AudioOutputHandler : public IAudioOutput, public Injectable, public async::Asyncable
{
    Inject<fx::IFxResolver> fxResolver = { this };
    Inject<IAudioEngine> audioEngine = { this };

public:
    explicit AudioOutputHandler(IGetTrackSequence* getSequence, const muse::modularity::ContextPtr& iocCtx);

    async::Promise<AudioOutputParams> outputParams(const TrackSequenceId sequenceId, const TrackId trackId) const override;
    void setOutputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioOutputParams& params) override;
    async::Channel<TrackSequenceId, TrackId, AudioOutputParams> outputParamsChanged() const override;

    async::Promise<AudioOutputParams> masterOutputParams() const override;
    void setMasterOutputParams(const AudioOutputParams& params) override;
    void clearMasterOutputParams() override;
    async::Channel<AudioOutputParams> masterOutputParamsChanged() const override;

    async::Promise<AudioResourceMetaList> availableOutputResources() const override;

    async::Promise<AudioSignalChanges> signalChanges(const TrackSequenceId sequenceId, const TrackId trackId) const override;
    async::Promise<AudioSignalChanges> masterSignalChanges() const override;

    async::Promise<bool> saveSoundTrack(const TrackSequenceId sequenceId, const io::path_t& destination,
                                        const SoundTrackFormat& format) override;
    void abortSavingAllSoundTracks() override;

    Progress saveSoundTrackProgress(const TrackSequenceId sequenceId) override;

    void clearAllFx() override;

private:
    std::shared_ptr<Mixer> mixer() const;
    ITrackSequencePtr sequence(const TrackSequenceId id) const;
    void ensureSeqSubscriptions(const ITrackSequencePtr s) const;
    void ensureMixerSubscriptions() const;

    IGetTrackSequence* m_getSequence = nullptr;

    mutable async::Channel<AudioOutputParams> m_masterOutputParamsChanged;
    mutable async::Channel<TrackSequenceId, TrackId, AudioOutputParams> m_outputParamsChanged;

    std::unordered_map<TrackSequenceId, Progress> m_saveSoundTracksProgressMap;
    std::unordered_map<TrackSequenceId, soundtrack::SoundTrackWriterPtr> m_saveSoundTracksWritersMap;
};
}
