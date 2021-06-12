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

int AccessibilityPreferencesModel::currentThemeIndex() const
{
    ThemeList hcThemes;                                        //creation of hcThemes ensures that we only search in
                                                               //a list of High Contrast Themes and not the entire list of Themes
    for (const ThemeInfo& theme : uiConfiguration()->themes()) {
        if (theme.codeKey == HIGH_CONTRAST_BLACK_THEME_CODE || theme.codeKey == HIGH_CONTRAST_WHITE_THEME_CODE) {
            hcThemes.push_back(theme);
        }
    }

    for (int i = 0; i < static_cast<int>(hcThemes.size()); ++i) {
        if (hcThemes[i].codeKey == (uiConfiguration()->currentTheme()).codeKey) {
            return i;
        }
    }

    return -1;
}

void AccessibilityPreferencesModel::setCurrentThemeIndex(int index)
{
    ThemeList hcThemes;

    for (const ThemeInfo& theme : uiConfiguration()->themes()) {
        if (theme.codeKey == HIGH_CONTRAST_BLACK_THEME_CODE || theme.codeKey == HIGH_CONTRAST_WHITE_THEME_CODE) {
            hcThemes.push_back(theme);
        }
    }

    if (index < 0 || index >= static_cast<int>(hcThemes.size())) {
        return;
    }

    if (index == currentThemeIndex()) {
        return;
    }

    uiConfiguration()->setCurrentTheme(hcThemes[index].codeKey);
    emit themesChanged();
}
