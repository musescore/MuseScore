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

#include "macosplatformtheme.h"

#include <Cocoa/Cocoa.h>

using namespace mu::framework;
using namespace mu::async;

id<NSObject> darkModeObserverToken;

MacOSPlatformTheme::MacOSPlatformTheme()
{
    NSDistributedNotificationCenter* n = [NSDistributedNotificationCenter defaultCenter];
    darkModeObserverToken = [n addObserverForName:@"AppleInterfaceThemeChangedNotification"
                                           object:nil
                                            queue:nil
                                       usingBlock:^(NSNotification*) { m_darkModeSwitched.send(isDarkMode()); }];
}

MacOSPlatformTheme::~MacOSPlatformTheme()
{
    NSDistributedNotificationCenter* n = [NSDistributedNotificationCenter defaultCenter];
    [n removeObserver:darkModeObserverToken];
}

bool MacOSPlatformTheme::isFollowSystemThemeAvailable() const
{
    return true;
}

bool MacOSPlatformTheme::isDarkMode() const
{
    NSString* systemMode = [[NSUserDefaults standardUserDefaults] stringForKey:@"AppleInterfaceStyle"];
    return ([systemMode isEqualToString:@"Dark"]);
}

Channel<bool> MacOSPlatformTheme::darkModeSwitched() const
{
    return m_darkModeSwitched;
}

void MacOSPlatformTheme::setAppThemeDark(bool dark)
{
    [NSApp setAppearance:[NSAppearance appearanceNamed:dark ? NSAppearanceNameDarkAqua : NSAppearanceNameAqua]];
}
