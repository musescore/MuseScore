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

#ifndef MU_AUDIO_AUDIOIOHANDLER_H
#define MU_AUDIO_AUDIOIOHANDLER_H

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "iaudiooutput.h"
#include "imixer.h"
#include "igettracksequence.h"

namespace mu::audio {
class AudioOutputHandler : public IAudioOutput, public async::Asyncable
{
    INJECT(audio, IMixer, mixer)

public:
    explicit AudioOutputHandler(IGetTrackSequence* getSequence);

    async::Promise<AudioOutputParams> outputParams(const TrackSequenceId sequenceId, const TrackId trackId) const override;
    void setOutputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioOutputParams& params) override;
    async::Channel<TrackSequenceId, TrackId, AudioOutputParams> outputParamsChanged() const override;

    async::Promise<AudioOutputParams> masterOutputParams() const override;
    void setMasterOutputParams(const AudioOutputParams& params) override;
    async::Channel<AudioOutputParams> masterOutputParamsChanged() const override;

    async::Channel<audioch_t, float> masterSignalAmplitudeChanged() const override;
    async::Channel<audioch_t, volume_dbfs_t> masterVolumePressureChanged() const override;

private:
    ITrackSequencePtr sequence(const TrackSequenceId id) const;
    void ensureSubscriptions(const ITrackSequencePtr s) const;

    IGetTrackSequence* m_getSequence = nullptr;

    mutable async::Channel<TrackSequenceId, TrackId, AudioOutputParams> m_outputParamsChanged;
};
}

#endif // MU_AUDIO_AUDIOIOHANDLER_H
