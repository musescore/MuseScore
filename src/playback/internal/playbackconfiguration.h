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
#ifndef MU_PLAYBACK_PLAYBACKCONFIGURATION_H
#define MU_PLAYBACK_PLAYBACKCONFIGURATION_H

#include "config.h"

#ifdef BUILD_MUSESAMPLER_MODULE
#include "modularity/ioc.h"
#include "musesampler/imusesamplerinfo.h"
#endif

#include "../iplaybackconfiguration.h"

namespace mu::playback {
class PlaybackConfiguration : public IPlaybackConfiguration
{
#ifdef BUILD_MUSESAMPLER_MODULE
    INJECT(playback, musesampler::IMuseSamplerInfo, musesamplerInfo)
#endif

public:
    void init();

    bool playNotesWhenEditing() const override;
    void setPlayNotesWhenEditing(bool value) override;

    bool playChordWhenEditing() const override;
    void setPlayChordWhenEditing(bool value) override;

    bool playHarmonyWhenEditing() const override;
    void setPlayHarmonyWhenEditing(bool value) override;

    PlaybackCursorType cursorType() const override;

    bool isMixerSectionVisible(MixerSectionType sectionType) const override;
    void setMixerSectionVisible(MixerSectionType sectionType, bool visible) override;

    const SoundProfileName& basicSoundProfileName() const override;
    const SoundProfileName& museSoundProfileName() const override;
    SoundProfileName defaultProfileForNewProjects() const override;
    void setDefaultProfileForNewProjects(const SoundProfileName& name) override;
private:
    const SoundProfileName& fallbackSoundProfileStr() const;
};
}

#endif // MU_PLAYBACK_PLAYBACKCONFIGURATION_H
