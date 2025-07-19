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

#ifndef MU_PROJECT_IPROJECTAUDIOSETTINGS_H
#define MU_PROJECT_IPROJECTAUDIOSETTINGS_H

#include <memory>

#include "audio/audiotypes.h"
#include "engraving/types/types.h"
#include "playback/playbacktypes.h"
#include "notation/inotationsolomutestate.h"
#include "types/retval.h"

namespace mu::project {
class IProjectAudioSettings
{
public:
    using SoloMuteState = notation::INotationSoloMuteState::SoloMuteState;

    virtual ~IProjectAudioSettings() = default;

    virtual const muse::audio::AudioOutputParams& masterAudioOutputParams() const = 0;
    virtual void setMasterAudioOutputParams(const muse::audio::AudioOutputParams& params) = 0;

    virtual bool containsAuxOutputParams(muse::audio::aux_channel_idx_t index) const = 0;
    virtual const muse::audio::AudioOutputParams& auxOutputParams(muse::audio::aux_channel_idx_t index) const = 0;
    virtual void setAuxOutputParams(muse::audio::aux_channel_idx_t index, const muse::audio::AudioOutputParams& params) = 0;

    virtual const muse::audio::AudioInputParams& trackInputParams(const engraving::InstrumentTrackId& trackId) const = 0;
    virtual void setTrackInputParams(const engraving::InstrumentTrackId& trackId, const muse::audio::AudioInputParams& params) = 0;
    virtual void clearTrackInputParams() = 0;
    virtual muse::async::Channel<engraving::InstrumentTrackId> trackInputParamsChanged() const = 0;

    virtual bool trackHasExistingOutputParams(const engraving::InstrumentTrackId& trackId) const = 0;
    virtual const muse::audio::AudioOutputParams& trackOutputParams(const engraving::InstrumentTrackId& trackId) const = 0;
    virtual void setTrackOutputParams(const engraving::InstrumentTrackId& trackId, const muse::audio::AudioOutputParams& params) = 0;

    virtual const SoloMuteState& auxSoloMuteState(muse::audio::aux_channel_idx_t index) const = 0;
    virtual void setAuxSoloMuteState(muse::audio::aux_channel_idx_t index, const SoloMuteState& state) = 0;
    virtual muse::async::Channel<muse::audio::aux_channel_idx_t, SoloMuteState> auxSoloMuteStateChanged() const = 0;

    virtual void removeTrackParams(const engraving::InstrumentTrackId& trackId) = 0;

    virtual const playback::SoundProfileName& activeSoundProfile() const = 0;
    virtual void setActiveSoundProfile(const playback::SoundProfileName& profileName) = 0;

    virtual muse::async::Notification settingsChanged() const = 0;
};

using IProjectAudioSettingsPtr = std::shared_ptr<IProjectAudioSettings>;
}

#endif // MU_PROJECT_IPROJECTAUDIOSETTINGS_H
