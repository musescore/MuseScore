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

#include "global/async/asyncable.h"

#include "isequenceio.h"
#include "audio/worker/internal/igettracks.h"
#include "audiotypes.h"

namespace muse::audio {
class SequenceIO : public ISequenceIO, public async::Asyncable
{
public:
    explicit SequenceIO(worker::IGetTracks* getTracks);

    bool hasTrack(const TrackId id) const override;

    RetVal<AudioInputParams> inputParams(const TrackId id) const override;
    RetVal<AudioOutputParams> outputParams(const TrackId id) const override;

    void setInputParams(const TrackId id, const AudioInputParams& params) override;
    void setOutputParams(const TrackId id, const AudioOutputParams& params) override;

    async::Channel<TrackId, AudioInputParams> inputParamsChanged() const override;
    async::Channel<TrackId, AudioOutputParams> outputParamsChanged() const override;

    AudioSignalChanges audioSignalChanges(const TrackId id) const override;

    InputProcessingProgress inputProcessingProgress(const TrackId id) const override;

private:
    worker::IGetTracks* m_getTracks = nullptr;

    async::Channel<TrackId, AudioInputParams> m_inputParamsChanged;
    async::Channel<TrackId, AudioOutputParams> m_outputParamsChanged;
};
}
