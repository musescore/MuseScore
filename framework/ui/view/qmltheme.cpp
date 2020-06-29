//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#include "qmltheme.h"

#include <QVariant>
#include "preferencekeys.h"
#include <QSettings>

using namespace mu::framework;

static const QHash<int, QVariant> DARK_THEME {
    { QmlTheme::BACKGROUND_COLOR, "#262626" },
    { QmlTheme::POPUP_BACKGROUND_COLOR, "#2C2C2C" },
    { QmlTheme::TEXT_FIELD_COLOR, "#1A1A1A" },
    { QmlTheme::ACCENT_COLOR, "#0062C2" },
    { QmlTheme::STROKE_COLOR, "#747474" },
    { QmlTheme::BUTTON_COLOR, "#595959" },
    { QmlTheme::FONT_COLOR, "#EBEBEB" },

    { QmlTheme::ACCENT_OPACITY_NORMAL, 0.8 },
    { QmlTheme::ACCENT_OPACITY_HOVER, 1.0 },
    { QmlTheme::ACCENT_OPACITY_HIT, 0.5 },

    { QmlTheme::BUTTON_OPACITY_NORMAL, 0.8 },
    { QmlTheme::BUTTON_OPACITY_HOVER, 1.0 },
    { QmlTheme::BUTTON_OPACITY_HIT, 0.5 },
};

static const QHash<int, QVariant> LIGHT_THEME {
    { QmlTheme::BACKGROUND_COLOR, "#E3E3E3" },
    { QmlTheme::POPUP_BACKGROUND_COLOR, "#E3E3E3" },
    { QmlTheme::TEXT_FIELD_COLOR, "#FFFFFF" },
    { QmlTheme::ACCENT_COLOR, "#0062C2" },
    { QmlTheme::STROKE_COLOR, "#D1D1D1" },
    { QmlTheme::BUTTON_COLOR, "#BDBDBD" },
    { QmlTheme::FONT_COLOR, "#000000" },

    { QmlTheme::ACCENT_OPACITY_NORMAL, 0.3 },
    { QmlTheme::ACCENT_OPACITY_HOVER, 0.15 },
    { QmlTheme::ACCENT_OPACITY_HIT, 0.5 },

    { QmlTheme::BUTTON_OPACITY_NORMAL, 0.7 },
    { QmlTheme::BUTTON_OPACITY_HOVER, 0.5 },
    { QmlTheme::BUTTON_OPACITY_HIT, 1.0 },
};

QmlTheme::QmlTheme(QObject* parent) :
    QObject(parent)
{
    QSettings settings;
    m_font.setFamily(settings.value(PREF_UI_THEME_FONTFAMILY, "FreeSans").toString());
    m_font.setPointSize(settings.value(PREF_UI_THEME_FONTSIZE, 12).toInt());
}

void QmlTheme::update()
{
    emit themeChanged();
}

QColor QmlTheme::backgroundColor() const
{
    return DARK_THEME.value(BACKGROUND_COLOR).toString();
}

QColor QmlTheme::popupBackgroundColor() const
{
    return DARK_THEME.value(POPUP_BACKGROUND_COLOR).toString();
}

QColor QmlTheme::textFieldColor() const
{
    return DARK_THEME.value(TEXT_FIELD_COLOR).toString();
}

QColor QmlTheme::accentColor() const
{
    return DARK_THEME.value(ACCENT_COLOR).toString();
}

QColor QmlTheme::strokeColor() const
{
    return DARK_THEME.value(STROKE_COLOR).toString();
}

QColor QmlTheme::buttonColor() const
{
    return DARK_THEME.value(BUTTON_COLOR).toString();
}

QColor QmlTheme::fontColor() const
{
    return DARK_THEME.value(FONT_COLOR).toString();
}

QFont QmlTheme::font() const
{
    return m_font;
}

qreal QmlTheme::accentOpacityNormal() const
{
    return DARK_THEME.value(ACCENT_OPACITY_NORMAL).toReal();
}

qreal QmlTheme::accentOpacityHover() const
{
    return DARK_THEME.value(ACCENT_OPACITY_HOVER).toReal();
}

qreal QmlTheme::accentOpacityHit() const
{
    return DARK_THEME.value(ACCENT_OPACITY_HIT).toReal();
}

qreal QmlTheme::buttonOpacityNormal() const
{
    return DARK_THEME.value(BUTTON_OPACITY_NORMAL).toReal();
}

qreal QmlTheme::buttonOpacityHover() const
{
    return DARK_THEME.value(BUTTON_OPACITY_HOVER).toReal();
}

qreal QmlTheme::buttonOpacityHit() const
{
    return DARK_THEME.value(BUTTON_OPACITY_HIT).toReal();
}

