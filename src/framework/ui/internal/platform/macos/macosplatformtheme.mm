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

using namespace mu::ui;
using namespace mu::async;

id<NSObject> darkModeObserverToken;

void MacOSPlatformTheme::startListening()
{
    NSDistributedNotificationCenter* n = [NSDistributedNotificationCenter defaultCenter];
    darkModeObserverToken = [n addObserverForName:@"AppleInterfaceThemeChangedNotification"
                                           object:nil
                                            queue:nil
                                       usingBlock:^(NSNotification*) { m_darkModeSwitched.send(isDarkMode()); }];
}

void MacOSPlatformTheme::stopListening()
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

void MacOSPlatformTheme::applyPlatformStyle(QWidget* widget)
{
    QColor backgroundColor = widget->palette().window().color();
    NSView* nsView = (__bridge NSView*)reinterpret_cast<void*>(widget->winId());
    NSWindow* nsWindow = [nsView window];
    if (nsWindow) {
        [nsWindow setTitlebarAppearsTransparent:YES];
        [nsWindow setBackgroundColor:[NSColor colorWithRed:backgroundColor.red() / 255.0
                                                     green:backgroundColor.green() / 255.0
                                                      blue:backgroundColor.blue() / 255.0
                                                     alpha:backgroundColor.alpha() / 255.0]];
    }
}
