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
#include "playbackconfiguration.h"
#include "settings.h"
#include "playbacktypes.h"

using namespace mu::playback;
using namespace mu::framework;

static const std::string moduleName("playback");

static const Settings::Key PLAYBACK_CURSOR_TYPE_KEY(moduleName, "application/playback/cursorType");
static const Settings::Key PLAY_NOTES_WHEN_EDITING(moduleName, "score/note/playOnClick");
static const Settings::Key PLAY_CHORD_WHEN_EDITING(moduleName, "score/chord/playOnAddNote");
static const Settings::Key PLAY_HARMONY_WHEN_EDITING(moduleName, "score/harmony/play/onedit");

void PlaybackConfiguration::init()
{
    settings()->setDefaultValue(PLAY_NOTES_WHEN_EDITING, Val(true));
    settings()->setDefaultValue(PLAY_CHORD_WHEN_EDITING, Val(true));
    settings()->setDefaultValue(PLAY_HARMONY_WHEN_EDITING, Val(true));
    settings()->setDefaultValue(PLAYBACK_CURSOR_TYPE_KEY, Val(static_cast<int>(PlaybackCursorType::STEPPED)));
}

bool PlaybackConfiguration::playNotesWhenEditing() const
{
    return settings()->value(PLAY_NOTES_WHEN_EDITING).toBool();
}

void PlaybackConfiguration::setPlayNotesWhenEditing(bool value)
{
    settings()->setValue(PLAY_NOTES_WHEN_EDITING, Val(value));
}

bool PlaybackConfiguration::playChordWhenEditing() const
{
    return settings()->value(PLAY_CHORD_WHEN_EDITING).toBool();
}

void PlaybackConfiguration::setPlayChordWhenEditing(bool value)
{
    settings()->setValue(PLAY_CHORD_WHEN_EDITING, Val(value));
}

bool PlaybackConfiguration::playHarmonyWhenEditing() const
{
    return settings()->value(PLAY_HARMONY_WHEN_EDITING).toBool();
}

void PlaybackConfiguration::setPlayHarmonyWhenEditing(bool value)
{
    settings()->setValue(PLAY_HARMONY_WHEN_EDITING, Val(value));
}

PlaybackCursorType PlaybackConfiguration::cursorType() const
{
    return static_cast<PlaybackCursorType>(settings()->value(PLAYBACK_CURSOR_TYPE_KEY).toInt());
}
