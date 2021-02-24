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

void PlaybackConfiguration::init()
{
    settings()->setDefaultValue(PLAYBACK_CURSOR_TYPE_KEY, Val(static_cast<int>(PlaybackCursorType::STEPPED)));
}

bool PlaybackConfiguration::isPlayElementOnClick() const
{
    //! TODO Add settings
    return true;
}

bool PlaybackConfiguration::isPlayHarmonyOnClick() const
{
    //! TODO Add settings
    return isPlayElementOnClick();
}

PlaybackCursorType PlaybackConfiguration::cursorType() const
{
    return static_cast<PlaybackCursorType>(settings()->value(PLAYBACK_CURSOR_TYPE_KEY).toInt());
}
