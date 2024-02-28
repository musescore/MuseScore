/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "macosplatformtheme.h"
#include "log.h"

#include <Cocoa/Cocoa.h>
#include <QApplication>
#include <QPalette>
#include <QWindow>

using namespace mu::ui;
using namespace mu::async;

id<NSObject> darkModeObserverToken = nil;
id<NSObject> accentColorObserverToken = nil;

void MacOSPlatformTheme::startListening()
{
    // set up dark mode observer
    if (!darkModeObserverToken) {
        darkModeObserverToken = [[NSDistributedNotificationCenter defaultCenter]
                                 addObserverForName:@"AppleInterfaceThemeChangedNotification"
                                 object:nil
                                 queue:nil
                                 usingBlock:^(NSNotification*) {
                                     m_platformThemeChanged.notify();
                                 }];
    }

    // set up accent color observer
    if (!accentColorObserverToken) {
        accentColorObserverToken = [[NSDistributedNotificationCenter defaultCenter]
                                    addObserverForName:@"AppleColorPreferencesChangedNotification"
                                    object:nil
                                    queue:nil
                                    usingBlock:^(NSNotification*) {
                                        m_platformThemeChanged.notify();
                                    }];
    }
}

void MacOSPlatformTheme::stopListening()
{
    // clean up dark mode observer
    if (darkModeObserverToken) {
        [[NSDistributedNotificationCenter defaultCenter] removeObserver:darkModeObserverToken];
        darkModeObserverToken = nil;
    }

    // clean up accent color observer
    if (accentColorObserverToken) {
        [[NSDistributedNotificationCenter defaultCenter] removeObserver:accentColorObserverToken];
        accentColorObserverToken = nil;
    }
}

bool MacOSPlatformTheme::isFollowSystemThemeAvailable() const
{
    // Supported from macOS 10.14, which is our minimum supported version
    return true;
}

bool MacOSPlatformTheme::isSystemThemeDark() const
{
    NSString* systemMode = [[NSUserDefaults standardUserDefaults] stringForKey:@"AppleInterfaceStyle"];
    return [systemMode isEqualToString:@"Dark"];
}

bool MacOSPlatformTheme::isGlobalMenuAvailable() const
{
    return true;
}

int MacOSPlatformTheme::getAccentColorIndex()
{
    // get macOS int representation of system accent color
    NSNumber* accentNum = [[NSUserDefaults standardUserDefaults] objectForKey:@"AppleAccentColor"];
    LOGD() << [[NSUserDefaults standardUserDefaults] objectForKey:@"AppleAccentColor"];
    /*
     * Notes:
     *
     * > The object returned from the call to objectForKey is an NSNumber
     * > use [nameOfObject intValue] to get the integer value contained within
     * > HOWEVER, you need to check if the returned NSNumber is null, as THAT value is what
     *   corresponds to the "Multicolor" option
     * > Rest of the mappings are as follows:
     *
     *   int | color option | MuseScore accent color index
     *   -------------------------------------------------
     *   -1  | graphite     | N/A
     *    0  | red          | 0
     *    1  | orange       | 1
     *    2  | yellow       | 2
     *    3  | green        | 3
     *    4  | blue         | 4
     *    5  | purple       | 5
     *    6  | pink         | 6
    */

    int MSIndex;

    if (accentNum == 0) {
        LOGD() << "Color is multicolor, defaulting to blue";
        MSIndex = 4;
    } else {
        LOGD() << "User color is defined, converting index...";
        switch ([accentNum intValue]) {
        case -1:
            LOGD() << "Color is graphite -> switching to blue";
            MSIndex = 4;
            break;
        case 0:
            LOGD() << "Color is red";
            MSIndex = 0;
            break;
        case 1:
            LOGD() << "Color is orange";
            MSIndex = 1;
            break;
        case 2:
            LOGD() << "Color is yellow";
            MSIndex = 2;
            break;
        case 3:
            LOGD() << "Color is green";
            MSIndex = 3;
            break;
        case 4:
            LOGD() << "Color is blue";
            MSIndex = 4;
            break;
        case 5:
            LOGD() << "Color is purple";
            MSIndex = 5;
            break;
        case 6:
            LOGD() << "Color is pink";
            MSIndex = 6;
            break;
        default:
            LOGD() << "Unexpected value, defaulting to blue";
            MSIndex = 4;
            break;
        }
    }

    return MSIndex;
}

Notification MacOSPlatformTheme::platformThemeChanged() const
{
    return m_platformThemeChanged;
}

void MacOSPlatformTheme::applyPlatformStyleOnAppForTheme(const ThemeCode& themeCode)
{
    // The system will turn these appearance names into their high contrast
    // counterparts automatically if system high contrast is enabled
    if (isDarkTheme(themeCode)) {
        [NSApp setAppearance:[NSAppearance appearanceNamed:NSAppearanceNameDarkAqua]];
    } else {
        [NSApp setAppearance:[NSAppearance appearanceNamed:NSAppearanceNameAqua]];
    }
}

void MacOSPlatformTheme::applyPlatformStyleOnWindowForTheme(QWindow* window, const ThemeCode&)
{
    if (!window) {
        return;
    }

    QColor backgroundColor = QApplication::palette().window().color();
    NSView* nsView = (__bridge NSView*)reinterpret_cast<void*>(window->winId());
    NSWindow* nsWindow = [nsView window];
    if (nsWindow) {
        [nsWindow setTitlebarAppearsTransparent:YES];
        [nsWindow setBackgroundColor:[NSColor colorWithRed:backgroundColor.red() / 255.0
                                      green:backgroundColor.green() / 255.0
                                      blue:backgroundColor.blue() / 255.0
                                      alpha:backgroundColor.alpha() / 255.0]];
    }
}
