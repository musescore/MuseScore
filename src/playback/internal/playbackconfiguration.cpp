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
#include "playbackconfiguration.h"

#include "settings.h"
#include "types/string.h"

#include "playbacktypes.h"

using namespace mu::playback;
using namespace mu::framework;

static const std::string moduleName("playback");

static const Settings::Key PLAYBACK_CURSOR_TYPE_KEY(moduleName, "application/playback/cursorType");
static const Settings::Key PLAY_NOTES_WHEN_EDITING(moduleName, "score/note/playOnClick");
static const Settings::Key PLAY_CHORD_WHEN_EDITING(moduleName, "score/chord/playOnAddNote");
static const Settings::Key PLAY_HARMONY_WHEN_EDITING(moduleName, "score/harmony/play/onedit");

static const Settings::Key MIXER_LABELS_SECTION_VISIBLE_KEY(moduleName, "playback/mixer/labelsSectionVisible");
static const Settings::Key MIXER_SOUND_SECTION_VISIBLE_KEY(moduleName, "playback/mixer/soundSectionVisible");
static const Settings::Key MIXER_AUDIO_FX_SECTION_VISIBLE_KEY(moduleName, "playback/mixer/audioFxSectionVisible");
static const Settings::Key MIXER_BALANCE_SECTION_VISIBLE_KEY(moduleName, "playback/mixer/balanceSectionVisible");
static const Settings::Key MIXER_VOLUME_SECTION_VISIBLE_KEY(moduleName, "playback/mixer/volumeSectionVisible");
static const Settings::Key MIXER_FADER_SECTION_VISIBLE_KEY(moduleName, "playback/mixer/faderSectionVisible");
static const Settings::Key MIXER_MUTE_AND_SOLO_SECTION_VISIBLE_KEY(moduleName, "playback/mixer/muteAndSoloSectionVisible");
static const Settings::Key MIXER_TITLE_SECTION_VISIBLE_KEY(moduleName, "playback/mixer/titleSectionVisible");

static const Settings::Key DEFAULT_SOUND_PROFILE_FOR_NEW_PROJECTS(moduleName, "playback/profiles/defaultProfileName");
static const SoundProfileName BASIC_PROFILE_NAME(u"MuseScore Basic");
static const SoundProfileName MUSE_PROFILE_NAME(u"Muse Sounds");

static Settings::Key mixerSectionVisibleKey(MixerSectionType sectionType)
{
    switch (sectionType) {
    case MixerSectionType::Labels: return MIXER_LABELS_SECTION_VISIBLE_KEY;
    case MixerSectionType::Sound: return MIXER_SOUND_SECTION_VISIBLE_KEY;
    case MixerSectionType::AudioFX: return MIXER_AUDIO_FX_SECTION_VISIBLE_KEY;
    case MixerSectionType::Balance: return MIXER_BALANCE_SECTION_VISIBLE_KEY;
    case MixerSectionType::Volume: return MIXER_VOLUME_SECTION_VISIBLE_KEY;
    case MixerSectionType::Fader: return MIXER_FADER_SECTION_VISIBLE_KEY;
    case MixerSectionType::MuteAndSolo: return MIXER_MUTE_AND_SOLO_SECTION_VISIBLE_KEY;
    case MixerSectionType::Title: return MIXER_TITLE_SECTION_VISIBLE_KEY;
    case MixerSectionType::Unknown: break;
    }

    return Settings::Key();
}

void PlaybackConfiguration::init()
{
    settings()->setDefaultValue(PLAY_NOTES_WHEN_EDITING, Val(true));
    settings()->setDefaultValue(PLAY_CHORD_WHEN_EDITING, Val(true));
    settings()->setDefaultValue(PLAY_HARMONY_WHEN_EDITING, Val(true));
    settings()->setDefaultValue(PLAYBACK_CURSOR_TYPE_KEY, Val(PlaybackCursorType::STEPPED));

    for (MixerSectionType sectionType : allMixerSectionTypes()) {
        bool sectionEnabledByDefault = sectionType != MixerSectionType::Volume;
        settings()->setDefaultValue(mixerSectionVisibleKey(sectionType), Val(sectionEnabledByDefault));
    }

    settings()->setDefaultValue(DEFAULT_SOUND_PROFILE_FOR_NEW_PROJECTS, Val(fallbackSoundProfileStr().toStdString()));
}

bool PlaybackConfiguration::playNotesWhenEditing() const
{
    return settings()->value(PLAY_NOTES_WHEN_EDITING).toBool();
}

void PlaybackConfiguration::setPlayNotesWhenEditing(bool value)
{
    settings()->setSharedValue(PLAY_NOTES_WHEN_EDITING, Val(value));
}

bool PlaybackConfiguration::playChordWhenEditing() const
{
    return settings()->value(PLAY_CHORD_WHEN_EDITING).toBool();
}

void PlaybackConfiguration::setPlayChordWhenEditing(bool value)
{
    settings()->setSharedValue(PLAY_CHORD_WHEN_EDITING, Val(value));
}

bool PlaybackConfiguration::playHarmonyWhenEditing() const
{
    return settings()->value(PLAY_HARMONY_WHEN_EDITING).toBool();
}

void PlaybackConfiguration::setPlayHarmonyWhenEditing(bool value)
{
    settings()->setSharedValue(PLAY_HARMONY_WHEN_EDITING, Val(value));
}

PlaybackCursorType PlaybackConfiguration::cursorType() const
{
    return settings()->value(PLAYBACK_CURSOR_TYPE_KEY).toEnum<PlaybackCursorType>();
}

bool PlaybackConfiguration::isMixerSectionVisible(MixerSectionType sectionType) const
{
    return settings()->value(mixerSectionVisibleKey(sectionType)).toBool();
}

void PlaybackConfiguration::setMixerSectionVisible(MixerSectionType sectionType, bool visible)
{
    settings()->setSharedValue(mixerSectionVisibleKey(sectionType), Val(visible));
}

const SoundProfileName& PlaybackConfiguration::basicSoundProfileName() const
{
    return BASIC_PROFILE_NAME;
}

const SoundProfileName& PlaybackConfiguration::museSoundProfileName() const
{
    return MUSE_PROFILE_NAME;
}

SoundProfileName PlaybackConfiguration::defaultProfileForNewProjects() const
{
    return String::fromStdString(settings()->value(DEFAULT_SOUND_PROFILE_FOR_NEW_PROJECTS).toString());
}

void PlaybackConfiguration::setDefaultProfileForNewProjects(const SoundProfileName& name)
{
    settings()->setSharedValue(DEFAULT_SOUND_PROFILE_FOR_NEW_PROJECTS, Val(name.toStdString()));
}

const SoundProfileName& PlaybackConfiguration::fallbackSoundProfileStr() const
{
#ifdef BUILD_MUSESAMPLER_MODULE
    if (musesamplerInfo()->isInstalled()) {
        return MUSE_PROFILE_NAME;
    }
#endif

    return BASIC_PROFILE_NAME;
}
