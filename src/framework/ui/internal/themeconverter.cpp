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

using namespace mu::ui;

static const QString CODEKEY_KEY("codeKey");
static const QString TITLE_KEY("title");

static const QString BACKGROUND_PRIMARY_COLOR_NAME("backgroundPrimaryColor");
static const QString BACKGROUND_SECONDARY_COLOR_NAME("backgroundSecondaryColor");
static const QString POPUP_BACKGROUND_COLOR_NAME("popupBackgroundColor");
static const QString TEXT_FIELD_COLOR_NAME("textFieldColor");
static const QString ACCENT_COLOR_NAME("accentColor");
static const QString STROKE_COLOR_NAME("strokeColor");
static const QString BUTTON_COLOR_NAME("buttonColor");
static const QString FONT_PRIMARY_COLOR_NAME("fontPrimaryColor");
static const QString FONT_SECONDARY_COLOR_NAME("fontSecondaryColor");
static const QString LINK_COLOR_NAME("linkColor");
static const QString ACCENT_OPACITY_NORMAL_NAME("accentOpacityNormal");
static const QString ACCENT_OPACITY_HOVER_NAME("accentOpacityHover");
static const QString ACCENT_OPACITY_HIT_NAME("accentOpacityHit");
static const QString BUTTON_OPACITY_NORMAL_NAME("buttonOpacityNormal");
static const QString BUTTON_OPACITY_HOVER_NAME("buttonOpacityHover");
static const QString BUTTON_OPACITY_HIT_NAME("buttonOpacityHit");
static const QString ITEM_OPACITY_DISABLED_NAME("itemOpacityDisabled");

QString themeStyleKeyToString(ThemeStyleKey key)
{
    switch (key) {
    case UNKNOWN: return QString();
    case BACKGROUND_PRIMARY_COLOR: return BACKGROUND_PRIMARY_COLOR_NAME;
    case BACKGROUND_SECONDARY_COLOR: return BACKGROUND_SECONDARY_COLOR_NAME;
    case POPUP_BACKGROUND_COLOR: return POPUP_BACKGROUND_COLOR_NAME;
    case TEXT_FIELD_COLOR: return TEXT_FIELD_COLOR_NAME;
    case ACCENT_COLOR: return ACCENT_COLOR_NAME;
    case STROKE_COLOR: return STROKE_COLOR_NAME;
    case BUTTON_COLOR: return BUTTON_COLOR_NAME;
    case FONT_PRIMARY_COLOR: return FONT_PRIMARY_COLOR_NAME;
    case FONT_SECONDARY_COLOR: return FONT_SECONDARY_COLOR_NAME;
    case LINK_COLOR: return LINK_COLOR_NAME;
    case ACCENT_OPACITY_NORMAL: return ACCENT_OPACITY_NORMAL_NAME;
    case ACCENT_OPACITY_HOVER: return ACCENT_OPACITY_HOVER_NAME;
    case ACCENT_OPACITY_HIT: return ACCENT_OPACITY_HIT_NAME;
    case BUTTON_OPACITY_NORMAL: return BUTTON_OPACITY_NORMAL_NAME;
    case BUTTON_OPACITY_HOVER: return BUTTON_OPACITY_HOVER_NAME;
    case BUTTON_OPACITY_HIT: return BUTTON_OPACITY_HIT_NAME;
    case ITEM_OPACITY_DISABLED: return ITEM_OPACITY_DISABLED_NAME;
    }

    return QString();
}

ThemeStyleKey themeStyleKeyFromString(const QString& str)
{
    static const QHash<QString, ThemeStyleKey> keyByStr {
        { BACKGROUND_PRIMARY_COLOR_NAME, BACKGROUND_PRIMARY_COLOR },
        { BACKGROUND_SECONDARY_COLOR_NAME, BACKGROUND_SECONDARY_COLOR },
        { POPUP_BACKGROUND_COLOR_NAME, POPUP_BACKGROUND_COLOR },
        { TEXT_FIELD_COLOR_NAME, TEXT_FIELD_COLOR },
        { ACCENT_COLOR_NAME, ACCENT_COLOR },
        { STROKE_COLOR_NAME, STROKE_COLOR },
        { BUTTON_COLOR_NAME, BUTTON_COLOR },
        { FONT_PRIMARY_COLOR_NAME, FONT_PRIMARY_COLOR },
        { FONT_SECONDARY_COLOR_NAME, FONT_SECONDARY_COLOR },
        { LINK_COLOR_NAME, LINK_COLOR },
        { ACCENT_OPACITY_NORMAL_NAME, ACCENT_OPACITY_NORMAL },
        { ACCENT_OPACITY_HOVER_NAME, ACCENT_OPACITY_HOVER },
        { ACCENT_OPACITY_HIT_NAME, ACCENT_OPACITY_HIT },
        { BUTTON_OPACITY_NORMAL_NAME, BUTTON_OPACITY_NORMAL },
        { BUTTON_OPACITY_HOVER_NAME, BUTTON_OPACITY_HOVER },
        { BUTTON_OPACITY_HIT_NAME, BUTTON_OPACITY_HIT },
        { ITEM_OPACITY_DISABLED_NAME, ITEM_OPACITY_DISABLED }
    };

    return keyByStr.value(str, ThemeStyleKey::UNKNOWN);
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
