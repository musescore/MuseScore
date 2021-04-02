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
#include "log.h"

#include <Cocoa/Cocoa.h>
#include <QWidget>

using namespace mu::ui;
using namespace mu::async;

id<NSObject> darkModeObserverToken = nil;
id<NSObject> contrastObserverToken = nil;

void MacOSPlatformTheme::startListening()
{
    if (!darkModeObserverToken) {
        darkModeObserverToken = [[NSDistributedNotificationCenter defaultCenter]
                                 addObserverForName:@"AppleInterfaceThemeChangedNotification"
                                 object:nil
                                 queue:nil
                                 usingBlock:^(NSNotification*) {
            m_channel.send(themeCode());
        }];
    }

    if (!contrastObserverToken) {
        contrastObserverToken = [[[NSWorkspace sharedWorkspace] notificationCenter]
                                 addObserverForName:NSWorkspaceAccessibilityDisplayOptionsDidChangeNotification
                                 object:nil
                                 queue:nil
                                 usingBlock:^(NSNotification*) {
            m_channel.send(themeCode());
        }];
    }
}

void MacOSPlatformTheme::stopListening()
{
    if (darkModeObserverToken) {
        [[NSDistributedNotificationCenter defaultCenter] removeObserver:darkModeObserverToken];
        darkModeObserverToken = nil;
    }

    if (contrastObserverToken) {
        [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:contrastObserverToken];
        contrastObserverToken = nil;
    }
}

bool MacOSPlatformTheme::isFollowSystemThemeAvailable() const
{
    // Supported from macOS 10.14, which is our minimum supported version
    return true;
}

ThemeCode MacOSPlatformTheme::themeCode() const
{
    if (isSystemDarkMode()) {
        if (isSystemHighContrast()) {
            return HIGH_CONTRAST_THEME_CODE;
        }
        return DARK_THEME_CODE;
    }
    //! NOTE When system is in light mode, don't automatically use
    //! high contrast theme, because it is too dark.
    //! Light high contrast theme would be nice.
    return LIGHT_THEME_CODE;
}

Channel<ThemeCode> MacOSPlatformTheme::themeCodeChanged() const
{
    return m_channel;
}

bool MacOSPlatformTheme::isSystemDarkMode() const
{
    NSString* systemMode = [[NSUserDefaults standardUserDefaults] stringForKey:@"AppleInterfaceStyle"];
    return [systemMode isEqualToString:@"Dark"];
}

bool MacOSPlatformTheme::isSystemHighContrast() const
{
    return [[NSWorkspace sharedWorkspace] accessibilityDisplayShouldIncreaseContrast];
}

void MacOSPlatformTheme::applyPlatformStyleOnAppForTheme(ThemeCode themeCode)
{
    // The system will turn these appearance names into their high contrast
    // counterparts automatically if system high contrast is enabled
    if (themeCode == LIGHT_THEME_CODE) {
        [NSApp setAppearance:[NSAppearance appearanceNamed:NSAppearanceNameAqua]];
    } else {
        [NSApp setAppearance:[NSAppearance appearanceNamed:NSAppearanceNameDarkAqua]];
    }
}

void MacOSPlatformTheme::applyPlatformStyleOnWindowForTheme(QWidget* widget, ThemeCode)
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
