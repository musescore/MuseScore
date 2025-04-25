/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#pragma once

#include <gmock/gmock.h>

#include "ui/iuiconfiguration.h"

namespace muse::ui {
class UiConfigurationMock : public IUiConfiguration
{
public:
    MOCK_METHOD(ThemeList, themes, (), (const, override));
    MOCK_METHOD(QStringList, possibleAccentColors, (), (const, override));
    MOCK_METHOD(QStringList, possibleFontFamilies, (), (const, override));
    MOCK_METHOD(void, setNonTextFonts, (const QStringList&), (override));

    MOCK_METHOD(bool, isDarkMode, (), (const, override));
    MOCK_METHOD(void, setIsDarkMode, (bool), (override));

    MOCK_METHOD(bool, isHighContrast, (), (const, override));
    MOCK_METHOD(void, setIsHighContrast, (bool), (override));

    MOCK_METHOD(const ThemeInfo&, currentTheme, (), (const, override));
    MOCK_METHOD(async::Notification, currentThemeChanged, (), (const, override));
    MOCK_METHOD(void, setCurrentTheme, (const ThemeCode&), (override));
    MOCK_METHOD(void, setCurrentThemeStyleValue, (ThemeStyleKey, const Val&), (override));
    MOCK_METHOD(void, resetThemes, (), (override));

    MOCK_METHOD(bool, isFollowSystemThemeAvailable, (), (const, override));
    MOCK_METHOD(ValNt<bool>, isFollowSystemTheme, (), (const, override));
    MOCK_METHOD(void, setFollowSystemTheme, (bool), (override));

    MOCK_METHOD(std::string, fontFamily, (), (const, override));
    MOCK_METHOD(void, setFontFamily, (const std::string&), (override));
    MOCK_METHOD(int, fontSize, (FontSizeType), (const, override));
    MOCK_METHOD(void, setBodyFontSize, (int), (override));
    MOCK_METHOD(async::Notification, fontChanged, (), (const, override));

    MOCK_METHOD(std::string, iconsFontFamily, (), (const, override));
    MOCK_METHOD(int, iconsFontSize, (IconSizeType), (const, override));
    MOCK_METHOD(async::Notification, iconsFontChanged, (), (const, override));

    MOCK_METHOD(io::path_t, appIconPath, (), (const, override));

    MOCK_METHOD(std::string, musicalFontFamily, (), (const, override));
    MOCK_METHOD(int, musicalFontSize, (), (const, override));
    MOCK_METHOD(async::Notification, musicalFontChanged, (), (const, override));

    MOCK_METHOD(std::string, defaultFontFamily, (), (const, override));
    MOCK_METHOD(int, defaultFontSize, (), (const, override));

    MOCK_METHOD(void, resetFonts, (), (override));

    MOCK_METHOD(double, guiScaling, (), (const, override));
    MOCK_METHOD(double, physicalDpi, (), (const, override));
    MOCK_METHOD(double, logicalDpi, (), (const, override));

    MOCK_METHOD(void, setPhysicalDotsPerInch, (std::optional<double>), (override));

    MOCK_METHOD(ValNt<QByteArray>, pageState, (const QString&), (const, override));
    MOCK_METHOD(void, setPageState, (const QString&, const QByteArray&), (override));

    MOCK_METHOD(QByteArray, windowGeometry, (), (const, override));
    MOCK_METHOD(void, setWindowGeometry, (const QByteArray&), (override));
    MOCK_METHOD(async::Notification, windowGeometryChanged, (), (const, override));

    MOCK_METHOD(bool, isGlobalMenuAvailable, (), (const, override));

    MOCK_METHOD(void, applyPlatformStyle, (QWindow*), (override));

    MOCK_METHOD(bool, isVisible, (const QString&, bool), (const, override));
    MOCK_METHOD(void, setIsVisible, (const QString&, bool), (override));
    MOCK_METHOD(async::Notification, isVisibleChanged, (const QString&), (const, override));

    MOCK_METHOD(QString, uiItemState, (const QString&), (const, override));
    MOCK_METHOD(void, setUiItemState, (const QString&, const QString&), (override));
    MOCK_METHOD(async::Notification, uiItemStateChanged, (const QString&), (const, override));

    MOCK_METHOD(ToolConfig, toolConfig, (const QString&, const ToolConfig&), (const, override));
    MOCK_METHOD(void, setToolConfig, (const QString&, const ToolConfig&), (override));
    MOCK_METHOD(async::Notification, toolConfigChanged, (const QString&), (const, override));

    MOCK_METHOD(int, flickableMaxVelocity, (), (const, override));

    MOCK_METHOD(int, tooltipDelay, (), (const, override));
};
}
