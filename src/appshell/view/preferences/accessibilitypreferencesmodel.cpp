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

#include "accessibilitypreferencesmodel.h"

#include "ui/internal/themeconverter.h"

using namespace mu::appshell;
using namespace mu::ui;

AccessibilityPreferencesModel::AccessibilityPreferencesModel(QObject* parent)
    : QObject(parent)
{
}

QVariantList AccessibilityPreferencesModel::highContrastThemes() const
{
    QVariantList result;

    for (const ThemeInfo& theme : uiConfiguration()->themes()) {
        if (theme.codeKey == HIGH_CONTRAST_BLACK_THEME_CODE || theme.codeKey == HIGH_CONTRAST_WHITE_THEME_CODE) {
            result << ThemeConverter::toMap(theme);
        }
    }

    return result;
}

QString AccessibilityPreferencesModel::currentThemeCode() const
{
    return QString::fromStdString(uiConfiguration()->currentTheme().codeKey);
}

void AccessibilityPreferencesModel::load()
{
    uiConfiguration()->currentThemeChanged().onNotify(this, [this]() {
        emit themesChanged();
    });
}

void AccessibilityPreferencesModel::setNewColor(const QColor& newColor, const QString& propertyName)
{
    //!NOTE: Considered using a "switch()" statement here, but it would require a type conversion
    //! from std::string to some form of a primitive literal. This has a workaround by implementing
    //! a hash function, but i went for the "if/else if" ladder instead since we don't have that many cases anyway.

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

void AccessibilityPreferencesModel::setCurrentThemeCode(const QString& themeCode)
{
    if (themeCode == currentThemeCode()) {
        return;
    }

    ThemeList hcThemes;

    for (const ThemeInfo& theme : uiConfiguration()->themes()) {
        if (theme.codeKey == HIGH_CONTRAST_BLACK_THEME_CODE || theme.codeKey == HIGH_CONTRAST_WHITE_THEME_CODE) {
            hcThemes.push_back(theme);
        }
    }

    for (const ThemeInfo& theme : hcThemes) {
        if (themeCode == QString::fromStdString(theme.codeKey)) {
            uiConfiguration()->setCurrentTheme(theme.codeKey);
        }
    }
    emit themesChanged();
}
