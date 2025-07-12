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

#include "windowsplatformtheme.h"

#include <winrt/windows.foundation.h>
#include <winrt/windows.ui.h>

using namespace muse::ui;
using namespace muse::async;

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::UI::ViewManagement;

static bool isColorLight(const Windows::UI::Color& c)
{
    return ((5 * c.G) + (2 * c.R) + c.B) > (8 * 128);
}

WindowsPlatformTheme::WindowsPlatformTheme()
{
    m_isSystemThemeDark.val = isSystemThemeCurrentlyDark();
}

WindowsPlatformTheme::~WindowsPlatformTheme()
{
    stopListening();
}

void WindowsPlatformTheme::startListening()
{
    if (!m_uiColorValuesChangedToken) {
        m_uiColorValuesChangedToken = m_uiSettings.ColorValuesChanged(
            [this](const UISettings&, const IInspectable&) {
            const bool isDarkTheme = isSystemThemeCurrentlyDark();
            if (isDarkTheme != m_isSystemThemeDark.val) {
                m_isSystemThemeDark.set(isDarkTheme);
            }
        });
    }
}

void WindowsPlatformTheme::stopListening()
{
    if (m_uiColorValuesChangedToken) {
        m_uiSettings.ColorValuesChanged(m_uiColorValuesChangedToken);
        m_uiColorValuesChangedToken = {};
    }
}

bool WindowsPlatformTheme::isFollowSystemThemeAvailable() const
{
    return true;
}

bool WindowsPlatformTheme::isSystemThemeDark() const
{
    return m_isSystemThemeDark.val;
}

bool WindowsPlatformTheme::isGlobalMenuAvailable() const
{
    return false;
}

Notification WindowsPlatformTheme::platformThemeChanged() const
{
    return m_isSystemThemeDark.notification;
}

void WindowsPlatformTheme::applyPlatformStyleOnAppForTheme(const ThemeCode&)
{
}

void WindowsPlatformTheme::applyPlatformStyleOnWindowForTheme(QWindow*, const ThemeCode&)
{
}

bool WindowsPlatformTheme::isSystemThemeCurrentlyDark() const
{
    // https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/ui/apply-windows-themes#know-when-dark-mode-is-enabled
    const Windows::UI::Color foregroundColor = m_uiSettings.GetColorValue(UIColorType::Foreground);
    return isColorLight(foregroundColor);
}
