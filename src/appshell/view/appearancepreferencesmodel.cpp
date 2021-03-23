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
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

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

QVariantList AppearancePreferencesModel::themes() const
{
    QVariantList result;

    for (const ThemeInfo& theme: allThemes()) {
        result << ThemeConverter::toMap(theme);
    }

    return result;
}

QStringList AppearancePreferencesModel::accentColorSamples() const
{
    static const QStringList samples {
        "#F36565",
        "#F39048",
        "#FFC52F",
        "#63D47B",
        "#70AFEA",
        "#A488F2",
        "#F87BDC"
    };

    return samples;
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

int AppearancePreferencesModel::currentThemeIndex() const
{
    ThemeList themes = allThemes();

    for (int i = 0; i < static_cast<int>(themes.size()); ++i) {
        if (themes[i].codeKey == currentTheme().codeKey) {
            return i;
        }
    }

    return INVALID_INDEX;
}

int AppearancePreferencesModel::currentAccentColorIndex() const
{
    QStringList samples = accentColorSamples();
    QString color = currentTheme().values[ACCENT_COLOR].toString().toLower();

    for (int i = 0; i < static_cast<int>(samples.size()); ++i) {
        if (samples[i].toLower() == color) {
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

bool AppearancePreferencesModel::useSameColorInPalettes() const
{
    return paletteConfiguration()->useNotationForegroundColor();
}

void AppearancePreferencesModel::setCurrentThemeIndex(int index)
{
    ThemeList themes = allThemes();

    if (index < 0 || index >= static_cast<int>(themes.size())) {
        return;
    }

    if (index == currentThemeIndex()) {
        return;
    }

    uiConfiguration()->setCurrentTheme(themes[index].codeKey);
    emit themesChanged();
}

void AppearancePreferencesModel::setCurrentAccentColorIndex(int index)
{
    if (index < 0 || index >= accentColorSamples().size()) {
        return;
    }

    if (index == currentAccentColorIndex()) {
        return;
    }

    QColor color = accentColorSamples()[index];
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
    emit currentFontIndexChanged(index);
}

void AppearancePreferencesModel::setBodyTextSize(int size)
{
    if (size == bodyTextSize() || size <= 0) {
        return;
    }

    uiConfiguration()->setBodyFontSize(size);
    emit bodyTextSizeChanged(size);
}

void AppearancePreferencesModel::setBackgroundUseColor(bool value)
{
    if (value == backgroundUseColor()) {
        return;
    }

    notationConfiguration()->setBackgroundUseColor(value);
    emit backgroundUseColorChanged(value);
}

void AppearancePreferencesModel::setBackgroundColor(const QColor& color)
{
    if (color == backgroundColor()) {
        return;
    }

    notationConfiguration()->setBackgroundColor(color);
    emit backgroundColorChanged(color);
}

void AppearancePreferencesModel::setBackgroundWallpaperPath(const QString& path)
{
    if (path == backgroundWallpaperPath()) {
        return;
    }

    notationConfiguration()->setBackgroundWallpaperPath(path);
    emit backgroundWallpaperPathChanged(path);
}

void AppearancePreferencesModel::setForegroundUseColor(bool value)
{
    if (value == foregroundUseColor()) {
        return;
    }

    notationConfiguration()->setForegroundUseColor(value);
    emit foregroundUseColorChanged(value);
}

void AppearancePreferencesModel::setForegroundColor(const QColor& color)
{
    if (color == foregroundColor()) {
        return;
    }

    notationConfiguration()->setForegroundColor(color);
    emit foregroundColorChanged(color);
}

void AppearancePreferencesModel::setForegroundWallpaperPath(const QString& path)
{
    if (path == foregroundWallpaperPath()) {
        return;
    }

    notationConfiguration()->setForegroundWallpaperPath(path);
    emit foregroundWallpaperPathChanged(path);
}

void AppearancePreferencesModel::setUseSameColorInPalettes(bool value)
{
    if (value == useSameColorInPalettes()) {
        return;
    }

    paletteConfiguration()->setUseNotationForegroundColor(value);
    emit useSameColorInPalettesChanged(value);
}
