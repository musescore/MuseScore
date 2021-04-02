//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "stubplatformtheme.h"

using namespace mu::ui;
using namespace mu::async;

void StubPlatformTheme::startListening()
{
}

void StubPlatformTheme::stopListening()
{
}

bool StubPlatformTheme::isFollowSystemThemeAvailable() const
{
    return false;
}

ThemeCode StubPlatformTheme::themeCode() const
{
    return LIGHT_THEME_CODE;
}

Channel<ThemeCode> StubPlatformTheme::themeCodeChanged() const
{
    return m_channel;
}

void StubPlatformTheme::applyPlatformStyleOnAppForTheme(ThemeCode)
{
}

void StubPlatformTheme::applyPlatformStyleOnWindowForTheme(QWidget*, ThemeCode)
{
}
