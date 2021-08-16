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
#include "pianorollconfiguration.h"
//#include "settings.h"
//#include "playbacktypes.h"

using namespace mu::pianoroll;
using namespace mu::framework;

static const std::string moduleName("pianoroll");

//static const Settings::Key PLAYBACK_CURSOR_TYPE_KEY(moduleName, "application/pianoroll/cursorType");
//static const Settings::Key PLAY_NOTES_WHEN_EDITING(moduleName, "score/note/playOnClick");
//static const Settings::Key PLAY_CHORD_WHEN_EDITING(moduleName, "score/chord/playOnAddNote");
//static const Settings::Key PLAY_HARMONY_WHEN_EDITING(moduleName, "score/harmony/play/onedit");

void PianorollConfiguration::init()
{
//    settings()->setDefaultValue(PLAY_NOTES_WHEN_EDITING, Val(true));
//    settings()->setDefaultValue(PLAY_CHORD_WHEN_EDITING, Val(true));
//    settings()->setDefaultValue(PLAY_HARMONY_WHEN_EDITING, Val(true));
//    settings()->setDefaultValue(PLAYBACK_CURSOR_TYPE_KEY, Val(static_cast<int>(PlaybackCursorType::STEPPED)));
}

//bool PlaybackConfiguration::playNotesWhenEditing() const
//{
//    return settings()->value(PLAY_NOTES_WHEN_EDITING).toBool();
//}

//void PlaybackConfiguration::setPlayNotesWhenEditing(bool value)
//{
//    settings()->setSharedValue(PLAY_NOTES_WHEN_EDITING, Val(value));
//}

//bool PlaybackConfiguration::playChordWhenEditing() const
//{
//    return settings()->value(PLAY_CHORD_WHEN_EDITING).toBool();
//}

//void PlaybackConfiguration::setPlayChordWhenEditing(bool value)
//{
//    settings()->setSharedValue(PLAY_CHORD_WHEN_EDITING, Val(value));
//}

//bool PlaybackConfiguration::playHarmonyWhenEditing() const
//{
//    return settings()->value(PLAY_HARMONY_WHEN_EDITING).toBool();
//}

//void PlaybackConfiguration::setPlayHarmonyWhenEditing(bool value)
//{
//    settings()->setSharedValue(PLAY_HARMONY_WHEN_EDITING, Val(value));
//}

//PlaybackCursorType PlaybackConfiguration::cursorType() const
//{
//    return static_cast<PlaybackCursorType>(settings()->value(PLAYBACK_CURSOR_TYPE_KEY).toInt());
//}
