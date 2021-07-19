/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "appearancepreferencesmodel.h"

#include "ui/internal/themeconverter.h"

#include "log.h"
#include "translation.h"

using namespace mu::appshell;
using namespace mu::notation;
using namespace mu::ui;

static constexpr int INVALID_INDEX = -1;

AppearancePreferencesModel::AppearancePreferencesModel(QObject* parent)
    : QObject(parent)
{
}

void AppearancePreferencesModel::init()
{
    uiConfiguration()->currentThemeChanged().onNotify(this, [this]() {
        emit themesChanged();
    });

    uiConfiguration()->fontChanged().onNotify(this, [this]() {
        emit currentFontIndexChanged();
        emit bodyTextSizeChanged();
    });

    notationConfiguration()->backgroundChanged().onNotify(this, [this]() {
        emit backgroundColorChanged();
        emit backgroundUseColorChanged();
        emit backgroundWallpaperPathChanged();
    });

    notationConfiguration()->foregroundChanged().onNotify(this, [this]() {
        emit foregroundColorChanged();
        emit foregroundUseColorChanged();
        emit foregroundWallpaperPathChanged();
    });
}

QVariantList AppearancePreferencesModel::generalThemes() const
{
    QVariantList result;

    for (const ThemeInfo& theme: allThemes()) {
        if (theme.codeKey == LIGHT_THEME_CODE || theme.codeKey == DARK_THEME_CODE) {
            result << ThemeConverter::toMap(theme);
        }
    }

    return result;
}

QVariantList AppearancePreferencesModel::highContrastThemes() const
{
    QVariantList result;

    for (const ThemeInfo& theme : allThemes()) {
        if (theme.codeKey == HIGH_CONTRAST_BLACK_THEME_CODE || theme.codeKey == HIGH_CONTRAST_WHITE_THEME_CODE) {
            result << ThemeConverter::toMap(theme);
        }
    }

    return result;
}

QStringList AppearancePreferencesModel::accentColors() const
{
    return uiConfiguration()->possibleAccentColors();
}

void AppearancePreferencesModel::resetThemeToDefault()
{
    uiConfiguration()->resetCurrentThemeToDefault(currentTheme().codeKey);
    notationConfiguration()->resetCurrentBackgroundColorToDefault();
    emit backgroundColorChanged();
    emit themesChanged();
}

bool AppearancePreferencesModel::enableHighContrastChecked()
{
    if (isCurrentThemeHighContrast()) {
        return true;
    } else {
        return false;
    }
}

void AppearancePreferencesModel::loadLastUsedGeneralTheme()
{
    uiConfiguration()->loadLastUsedGeneralTheme();
}

void AppearancePreferencesModel::loadLastUsedHighContrastTheme()
{
    uiConfiguration()->loadLastUsedHighContrastTheme();
}

void AppearancePreferencesModel::setNewColor(const QColor& newColor, const QString& propertyName)
{
    //! NOTE: Considered using a "switch()" statement here, but it would require a type conversion
    //! from std::string to some form of a primitive literal. This has a workaround by implementing
    //! a hash function, but I went for the "if/else if" ladder instead since we don't have that many cases anyway.

    if (propertyName.toStdString() == "Accent Color:") {
        uiConfiguration()->setCurrentThemeStyleValue(ThemeStyleKey::ACCENT_COLOR, Val(newColor));
    } else if (propertyName.toStdString() == "Text and Icons:") {
        uiConfiguration()->setCurrentThemeStyleValue(ThemeStyleKey::FONT_PRIMARY_COLOR, Val(newColor));
    } else if (propertyName.toStdString() == "Disabled Text:") {
        return;
    } else if (propertyName.toStdString() == "Border Color:") {
        uiConfiguration()->setCurrentThemeStyleValue(ThemeStyleKey::STROKE_COLOR, Val(newColor));
    }
    emit themesChanged();
}

QStringList AppearancePreferencesModel::allFonts() const
{
    return uiConfiguration()->possibleFontFamilies();
}

QString AppearancePreferencesModel::wallpaperPathFilter() const
{
    return qtrc("appshell", "Images") + " (*.jpg *.jpeg *.png);;"
           + qtrc("appshell", "All") + " (*)";
}

QString AppearancePreferencesModel::wallpapersDir() const
{
    return notationConfiguration()->wallpapersDefaultDirPath().toQString();
}

QString AppearancePreferencesModel::currentThemeCode() const
{
    return QString::fromStdString(currentTheme().codeKey);
}

int AppearancePreferencesModel::currentAccentColorIndex() const
{
    QStringList allColors = accentColors();
    QString color = currentTheme().values[ACCENT_COLOR].toString().toLower();

    for (int i = 0; i < static_cast<int>(allColors.size()); ++i) {
        if (allColors[i].toLower() == color) {
            return i;
        }
    }

    return INVALID_INDEX;
}

ThemeInfo AppearancePreferencesModel::currentTheme() const
{
    return uiConfiguration()->currentTheme();
}

ThemeList AppearancePreferencesModel::allThemes() const
{
    return uiConfiguration()->themes();
}

bool AppearancePreferencesModel::isCurrentThemeHighContrast() const
{
    return uiConfiguration()->currentTheme().codeKey == HIGH_CONTRAST_BLACK_THEME_CODE
           || uiConfiguration()->currentTheme().codeKey == HIGH_CONTRAST_WHITE_THEME_CODE;
}

bool AppearancePreferencesModel::isCurrentThemeGeneral() const
{
    return uiConfiguration()->currentTheme().codeKey == LIGHT_THEME_CODE || uiConfiguration()->currentTheme().codeKey == DARK_THEME_CODE;
}

int AppearancePreferencesModel::currentFontIndex() const
{
    QString currentFont = QString::fromStdString(uiConfiguration()->fontFamily());
    return allFonts().indexOf(currentFont);
}

int AppearancePreferencesModel::bodyTextSize() const
{
    return uiConfiguration()->fontSize(FontSizeType::BODY);
}

bool AppearancePreferencesModel::backgroundUseColor() const
{
    return notationConfiguration()->backgroundUseColor();
}

QColor AppearancePreferencesModel::backgroundColor() const
{
    return notationConfiguration()->backgroundColor();
}

QString AppearancePreferencesModel::backgroundWallpaperPath() const
{
    return notationConfiguration()->backgroundWallpaperPath().toQString();
}

bool AppearancePreferencesModel::foregroundUseColor() const
{
    return notationConfiguration()->foregroundUseColor();
}

QColor AppearancePreferencesModel::foregroundColor() const
{
    return notationConfiguration()->foregroundColor();
}

QString AppearancePreferencesModel::foregroundWallpaperPath() const
{
    return notationConfiguration()->foregroundWallpaperPath().toQString();
}

void AppearancePreferencesModel::setCurrentThemeCode(const QString& themeCode)
{
    if (themeCode == currentThemeCode()) {
        return;
    }

    for (const ThemeInfo& theme : allThemes()) {
        if (themeCode == QString::fromStdString(theme.codeKey)) {
            uiConfiguration()->setCurrentTheme(theme.codeKey);
        }
    }
    emit themesChanged();
}

void AppearancePreferencesModel::setCurrentAccentColorIndex(int index)
{
    if (index < 0 || index >= accentColors().size()) {
        return;
    }

    if (index == currentAccentColorIndex()) {
        return;
    }

    QColor color = accentColors()[index];
    uiConfiguration()->setCurrentThemeStyleValue(ThemeStyleKey::ACCENT_COLOR, Val(color));
    emit themesChanged();
}

void AppearancePreferencesModel::setCurrentFontIndex(int index)
{
    QStringList fonts = allFonts();

    if (index < 0 || index >= fonts.size()) {
        return;
    }

    uiConfiguration()->setFontFamily(fonts[index].toStdString());
    emit currentFontIndexChanged();
}

void AppearancePreferencesModel::setBodyTextSize(int size)
{
    if (size == bodyTextSize() || size <= 0) {
        return;
    }

    uiConfiguration()->setBodyFontSize(size);
    emit bodyTextSizeChanged();
}

void AppearancePreferencesModel::setBackgroundUseColor(bool value)
{
    if (value == backgroundUseColor()) {
        return;
    }

    notationConfiguration()->setBackgroundUseColor(value);
    emit backgroundUseColorChanged();
}

void AppearancePreferencesModel::setBackgroundColor(const QColor& color)
{
    if (color == backgroundColor()) {
        return;
    }

    notationConfiguration()->setBackgroundColor(color);
    emit backgroundColorChanged();
}

void AppearancePreferencesModel::setBackgroundWallpaperPath(const QString& path)
{
    if (path == backgroundWallpaperPath()) {
        return;
    }

    notationConfiguration()->setBackgroundWallpaperPath(path);
    emit backgroundWallpaperPathChanged();
}

void AppearancePreferencesModel::setForegroundUseColor(bool value)
{
    if (value == foregroundUseColor()) {
        return;
    }

    notationConfiguration()->setForegroundUseColor(value);
    emit foregroundUseColorChanged();
}

void AppearancePreferencesModel::setForegroundColor(const QColor& color)
{
    if (color == foregroundColor()) {
        return;
    }

    notationConfiguration()->setForegroundColor(color);
    emit foregroundColorChanged();
}

void AppearancePreferencesModel::setForegroundWallpaperPath(const QString& path)
{
    if (path == foregroundWallpaperPath()) {
        return;
    }

    notationConfiguration()->setForegroundWallpaperPath(path);
    emit foregroundWallpaperPathChanged();
}
