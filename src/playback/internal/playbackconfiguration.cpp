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

using namespace mu::playback;

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

bool PlaybackConfiguration::isMidiInputEnabled() const
{
    return notationConfiguration()->isMidiInputEnabled();
}

void PlaybackConfiguration::setIsMidiInputEnabled(bool enabled)
{
    notationConfiguration()->setIsMidiInputEnabled(enabled);
}

bool PlaybackConfiguration::isAutomaticallyPanEnabled() const
{
    return notationConfiguration()->isAutomaticallyPanEnabled();
}

void PlaybackConfiguration::setIsAutomaticallyPanEnabled(bool enabled)
{
    notationConfiguration()->setIsAutomaticallyPanEnabled(enabled);
}

bool PlaybackConfiguration::isPlayRepeatsEnabled() const
{
    return notationConfiguration()->isPlayRepeatsEnabled();
}

void PlaybackConfiguration::setIsPlayRepeatsEnabled(bool enabled)
{
    notationConfiguration()->setIsPlayRepeatsEnabled(enabled);
}

bool PlaybackConfiguration::isMetronomeEnabled() const
{
    return notationConfiguration()->isMetronomeEnabled();
}

void PlaybackConfiguration::setIsMetronomeEnabled(bool enabled)
{
    notationConfiguration()->setIsMetronomeEnabled(enabled);
}

bool PlaybackConfiguration::isCountInEnabled() const
{
    return notationConfiguration()->isCountInEnabled();
}

void PlaybackConfiguration::setIsCountInEnabled(bool enabled)
{
    notationConfiguration()->setIsCountInEnabled(enabled);
}
