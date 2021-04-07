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

#include "themeconverter.h"

#include "log.h"

using namespace mu::ui;

static const QString CODEKEY_KEY("codeKey");
static const QString TITLE_KEY("title");

static const std::vector<std::pair<ThemeStyleKey, QString > > s_keys = {
    { UNKNOWN, QString() },
    { BACKGROUND_PRIMARY_COLOR, "backgroundPrimaryColor" },
    { BACKGROUND_SECONDARY_COLOR, "backgroundSecondaryColor" },
    { POPUP_BACKGROUND_COLOR, "popupBackgroundColor" },
    { TEXT_FIELD_COLOR, "textFieldColor" },
    { ACCENT_COLOR, "accentColor" },
    { STROKE_COLOR, "strokeColor" },
    { BUTTON_COLOR, "buttonColor" },
    { FONT_PRIMARY_COLOR, "fontPrimaryColor" },
    { FONT_SECONDARY_COLOR, "fontSecondaryColor" },
    { LINK_COLOR, "linkColor" },
    { FOCUS_COLOR, "focusColor" },
    { ACCENT_OPACITY_NORMAL, "accentOpacityNormal" },
    { ACCENT_OPACITY_HOVER, "accentOpacityHover" },
    { ACCENT_OPACITY_HIT, "accentOpacityHit" },
    { BUTTON_OPACITY_NORMAL, "buttonOpacityNormal" },
    { BUTTON_OPACITY_HOVER, "buttonOpacityHover" },
    { BUTTON_OPACITY_HIT, "buttonOpacityHit" },
    { ITEM_OPACITY_DISABLED, "itemOpacityDisabled" },
};

static const QString& themeStyleKeyToString(ThemeStyleKey key)
{
    for (const auto& p : s_keys) {
        if (p.first == key) {
            return p.second;
        }
    }

    IF_ASSERT_FAILED_X(false, QString("not found string key for enum key: %1").arg(static_cast<int>(key))) {
    }

    static const QString null;
    return null;
}

static ThemeStyleKey themeStyleKeyFromString(const QString& str)
{
    for (const auto& p : s_keys) {
        if (p.second == str) {
            return p.first;
        }
    }

    IF_ASSERT_FAILED_X(false, QString("not found enum key for string key: %1").arg(str)) {
    }

    return ThemeStyleKey::UNKNOWN;
}

QVariantMap ThemeConverter::toMap(const ThemeInfo& theme)
{
    QVariantMap obj;

    obj[CODEKEY_KEY] = QString::fromStdString(theme.codeKey);
    obj[TITLE_KEY] = QString::fromStdString(theme.title);

    for (ThemeStyleKey key : theme.values.keys()) {
        obj[themeStyleKeyToString(key)] = theme.values[key];
    }

    return obj;
}

ThemeInfo ThemeConverter::fromMap(const QVariantMap& map)
{
    ThemeInfo theme;
    theme.codeKey = map[CODEKEY_KEY].toString().toStdString();
    theme.title = map[TITLE_KEY].toString().toStdString();

    for (const QString& key : map.keys()) {
        theme.values[themeStyleKeyFromString(key)] = map[key];
    }

    return theme;
}
