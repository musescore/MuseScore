/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <memory>

#include "types/string.h"
#include "types/bytearray.h"

#include "async/notification.h"
#include "audio/common/audiotypes.h"
#include "engraving/types/types.h"
#include "playback/playbacktypes.h"
#include "notation/inotationsolomutestate.h"

namespace mu::project {
using AudioInputParams = muse::audio::AudioInputParams;
using TrackInputParamsMap = std::unordered_map<engraving::InstrumentTrackId, AudioInputParams>;

//! NOTE This model (structure) is not used in the audio module, there are other models.
struct AudioOutputParams {
    muse::audio::AudioFxChain fxChain;
    muse::audio::volume_db_t volume = 0.f;
    muse::audio::balance_t balance = 0.f;
    muse::audio::AuxSendsParams auxSends;
    bool solo = false;
    bool muted = false;
    bool forceMute = false;

    bool operator ==(const AudioOutputParams& other) const
    {
        return fxChain == other.fxChain
               && muse::is_equal(volume, other.volume)
               && muse::is_equal(balance, other.balance)
               && auxSends == other.auxSends
               && solo == other.solo
               && muted == other.muted
               && forceMute == other.forceMute;
    }

    muse::audio::ControlParams control() const
    {
        muse::audio::ControlParams control;
        control.volume = volume;
        control.balance = balance;
        control.muted = muted;
        return control;
    }

    void setControl(const muse::audio::ControlParams& control)
    {
        if (!control.volume.hasAutomation()) {
            volume = std::get<muse::audio::volume_db_t>(control.volume.value());
        }
        if (!control.balance.hasAutomation()) {
            balance = std::get<muse::audio::balance_t>(control.balance.value());
        }
        muted = control.muted;
    }
};

//! NOTE Describes an audio clip attached to the score.
//! In the first version only a single clip is supported.
struct AudioClipInfo {
    muse::String fileName;      //! name of the file inside the score package, i.e. Audio/<fileName>
    int startTick = 0;          //! score position (in ticks) the clip is aligned to
    double inOffset = 0.0;      //! offset into the audio file, in seconds (trim the head)
    double outOffset = -1.0;    //! offset at which playback stops, in seconds; negative means "until the end"

    bool isValid() const { return !fileName.isEmpty(); }

    bool operator==(const AudioClipInfo& other) const
    {
        return fileName == other.fileName
               && startTick == other.startTick
               && muse::is_equal(inOffset, other.inOffset)
               && muse::is_equal(outOffset, other.outOffset);
    }

    bool operator!=(const AudioClipInfo& other) const { return !this->operator==(other); }
};

class IProjectAudioSettings
{
public:
    using SoloMuteState = notation::INotationSoloMuteState::SoloMuteState;

    virtual ~IProjectAudioSettings() = default;

    virtual bool hasAnyAudioSettings() const = 0;

    virtual const AudioOutputParams& masterAudioOutputParams() const = 0;
    virtual void setMasterAudioOutputParams(const AudioOutputParams& params) = 0;

    virtual bool containsAuxOutputParams(muse::audio::aux_channel_idx_t index) const = 0;
    virtual const AudioOutputParams& auxOutputParams(muse::audio::aux_channel_idx_t index) const = 0;
    virtual void setAuxOutputParams(muse::audio::aux_channel_idx_t index, const AudioOutputParams& params,
                                    bool notifySettingsChanged = true) = 0;

    virtual const TrackInputParamsMap& allTrackInputParams() const = 0;
    virtual const AudioInputParams& trackInputParams(const engraving::InstrumentTrackId& trackId) const = 0;
    virtual void setTrackInputParams(const engraving::InstrumentTrackId& trackId, const AudioInputParams& params,
                                     bool notifySettingsChanged = true) = 0;
    virtual void clearTrackInputParams() = 0;
    virtual muse::async::Channel<engraving::InstrumentTrackId> trackInputParamsChanged() const = 0;

    virtual bool trackHasExistingOutputParams(const engraving::InstrumentTrackId& trackId) const = 0;
    virtual const AudioOutputParams& trackOutputParams(const engraving::InstrumentTrackId& trackId) const = 0;
    virtual void setTrackOutputParams(const engraving::InstrumentTrackId& trackId, const AudioOutputParams& params,
                                      bool notifySettingsChanged = true) = 0;

    virtual const SoloMuteState& auxSoloMuteState(muse::audio::aux_channel_idx_t index) const = 0;
    virtual void setAuxSoloMuteState(muse::audio::aux_channel_idx_t index, const SoloMuteState& state) = 0;
    virtual muse::async::Channel<muse::audio::aux_channel_idx_t, SoloMuteState> auxSoloMuteStateChanged() const = 0;

    virtual void removeTrackParams(const engraving::InstrumentTrackId& trackId) = 0;

    virtual const playback::SoundProfileName& activeSoundProfile() const = 0;
    virtual void setActiveSoundProfile(const playback::SoundProfileName& profileName) = 0;

    //! NOTE Audio clip that is played along with the score (v1: at most one)
    virtual const AudioClipInfo& audioClip() const = 0;
    virtual void setAudioClip(const AudioClipInfo& clip) = 0;
    virtual muse::ByteArray audioClipData() const = 0;
    virtual void setAudioClipData(const muse::ByteArray& data) = 0;
    virtual muse::async::Notification audioClipChanged() const = 0;

    virtual muse::async::Notification settingsChanged() const = 0;
};

using IProjectAudioSettingsPtr = std::shared_ptr<IProjectAudioSettings>;
}
