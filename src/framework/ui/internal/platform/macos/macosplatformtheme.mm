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
id<NSObject> contrastObserverToken = nil;

void MacOSPlatformTheme::startListening()
{
    if (!darkModeObserverToken) {
        darkModeObserverToken = [[NSDistributedNotificationCenter defaultCenter]
                                 addObserverForName:@"AppleInterfaceThemeChangedNotification"
                                 object:nil
                                 queue:nil
                                 usingBlock:^(NSNotification*) {
                                     m_platformThemeChanged.notify();
                                 }];
    }

    if (!contrastObserverToken) {
        contrastObserverToken = [[[NSWorkspace sharedWorkspace] notificationCenter]
                                 addObserverForName:NSWorkspaceAccessibilityDisplayOptionsDidChangeNotification
                                 object:nil
                                 queue:nil
                                 usingBlock:^(NSNotification*) {
                                     m_platformThemeChanged.notify();
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

ThemeCode MacOSPlatformTheme::platformThemeCode() const
{
    if (isSystemHighContrast()) {
        if (isSystemDarkMode()) {
            return HIGH_CONTRAST_BLACK_THEME_CODE;
        }

        return HIGH_CONTRAST_WHITE_THEME_CODE;
    }

    if (isSystemDarkMode()) {
        return DARK_THEME_CODE;
    }

    return LIGHT_THEME_CODE;
}

Notification MacOSPlatformTheme::platformThemeChanged() const
{
    return m_platformThemeChanged;
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

void MacOSPlatformTheme::applyPlatformStyleOnAppForTheme(const ThemeCode& themeCode)
{
    // The system will turn these appearance names into their high contrast
    // counterparts automatically if system high contrast is enabled
    if (isLightTheme(themeCode)) {
        [NSApp setAppearance:[NSAppearance appearanceNamed:NSAppearanceNameAqua]];
    } else {
        [NSApp setAppearance:[NSAppearance appearanceNamed:NSAppearanceNameDarkAqua]];
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
