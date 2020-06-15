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
    { QmlTheme::BACKGROUND_COLOR, "#2D2D30" },
    { QmlTheme::POPUP_BACKGROUND_COLOR, "#2D2D30" },
    { QmlTheme::HIGHLIGHT_COLOR, "0062C2" },
    { QmlTheme::STROKE_COLOR, "#25000000" },
    { QmlTheme::BUTTON_COLOR_NORMAL, "#484848" },
    { QmlTheme::BUTTON_COLOR_HOVER, "#C0C0C0" },
    { QmlTheme::BUTTON_COLOR_HIT, "#ADADAD" },
    { QmlTheme::FONT_COLOR, "#EBEBEB" }
};

static const QHash<int, QVariant> LIGHT_THEME {
    { QmlTheme::BACKGROUND_COLOR, "#E3E3E3" },
    { QmlTheme::POPUP_BACKGROUND_COLOR, "#E3E3E3" },
    { QmlTheme::HIGHLIGHT_COLOR, "0062C2" },
    { QmlTheme::STROKE_COLOR, "#25000000" },
    { QmlTheme::BUTTON_COLOR_NORMAL, "#CECECE" },
    { QmlTheme::BUTTON_COLOR_HOVER, "#C0C0C0" },
    { QmlTheme::BUTTON_COLOR_HIT, "#ADADAD" },
    { QmlTheme::FONT_COLOR, "#000000" }
};

QmlTheme::QmlTheme(QObject* parent) :
    QObject(parent)
{
    QSettings settings;
    m_font.setFamily(settings.value(PREF_UI_THEME_FONTFAMILY).toString());
    m_font.setPointSize(settings.value(PREF_UI_THEME_FONTSIZE).toInt());
}

void QmlTheme::update()
{
    emit themeChanged();
}

QColor QmlTheme::backgroundColor() const
{
    return LIGHT_THEME.value(BACKGROUND_COLOR).toString();
}

QColor QmlTheme::popupBackgroundColor() const
{
    return LIGHT_THEME.value(POPUP_BACKGROUND_COLOR).toString();
}

QColor QmlTheme::highlightColor() const
{
    return LIGHT_THEME.value(HIGHLIGHT_COLOR).toString();
}

QColor QmlTheme::strokeColor() const
{
    return LIGHT_THEME.value(STROKE_COLOR).toString();
}

QColor QmlTheme::buttonColorNormal() const
{
    return LIGHT_THEME.value(BUTTON_COLOR_NORMAL).toString();
}

QColor QmlTheme::buttonColorHover() const
{
    return LIGHT_THEME.value(BUTTON_COLOR_HOVER).toString();
}

QColor QmlTheme::buttonColorHit() const
{
    return LIGHT_THEME.value(BUTTON_COLOR_HIT).toString();
}

QColor QmlTheme::fontColor() const
{
    return LIGHT_THEME.value(FONT_COLOR).toString();
}

QFont QmlTheme::font() const
{
    return m_font;
}

