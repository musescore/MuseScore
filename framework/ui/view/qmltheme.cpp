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
    m_font.setFamily(configuration()->fontFamily());
    m_font.setPointSize(configuration()->fontSize());

    configuration()->themeTypeChanged().onReceive(this, [this](const IUiConfiguration::ThemeType) {
        update();
    });

    configuration()->fontFamilyChanged().onReceive(this, [this](const QString& fontFamily) {
        m_font.setFamily(fontFamily);

        update();
    });

    configuration()->fontSizeChanged().onReceive(this, [this](const int fontSize) {
        m_font.setPointSize(fontSize);

        update();
    });
}

void QmlTheme::update()
{
    emit themeChanged();
}

QColor QmlTheme::backgroundColor() const
{
    return currentThemeProperites().value(BACKGROUND_COLOR).toString();
}

QColor QmlTheme::popupBackgroundColor() const
{
    return currentThemeProperites().value(POPUP_BACKGROUND_COLOR).toString();
}

QColor QmlTheme::textFieldColor() const
{
    return currentThemeProperites().value(TEXT_FIELD_COLOR).toString();
}

QColor QmlTheme::accentColor() const
{
    return currentThemeProperites().value(ACCENT_COLOR).toString();
}

QColor QmlTheme::strokeColor() const
{
    return currentThemeProperites().value(STROKE_COLOR).toString();
}

QColor QmlTheme::buttonColor() const
{
    return currentThemeProperites().value(BUTTON_COLOR).toString();
}

QColor QmlTheme::fontColor() const
{
    return currentThemeProperites().value(FONT_COLOR).toString();
}

QFont QmlTheme::font() const
{
    return m_font;
}

qreal QmlTheme::accentOpacityNormal() const
{
    return currentThemeProperites().value(ACCENT_OPACITY_NORMAL).toReal();
}

qreal QmlTheme::accentOpacityHover() const
{
    return currentThemeProperites().value(ACCENT_OPACITY_HOVER).toReal();
}

qreal QmlTheme::accentOpacityHit() const
{
    return currentThemeProperites().value(ACCENT_OPACITY_HIT).toReal();
}

qreal QmlTheme::buttonOpacityNormal() const
{
    return currentThemeProperites().value(BUTTON_OPACITY_NORMAL).toReal();
}

qreal QmlTheme::buttonOpacityHover() const
{
    return currentThemeProperites().value(BUTTON_OPACITY_HOVER).toReal();
}

qreal QmlTheme::buttonOpacityHit() const
{
    return currentThemeProperites().value(BUTTON_OPACITY_HIT).toReal();
}

QHash<int, QVariant> QmlTheme::currentThemeProperites() const
{
    if (configuration()->themeType() == IUiConfiguration::ThemeType::DARK_THEME) {
        return DARK_THEME;
    }

    return LIGHT_THEME;
}

